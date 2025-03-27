#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "bpf_trace.skel.h"
#include "bpf_trace.h"

#define PIN_LINK_PATH "/sys/fs/bpf/kf/do_sys_openat2"
#define PIN_FS_MAP_PATH "/sys/fs/bpf/kf/fs_mount_map"
#define PIN_MODULES_LOAD_STATUS_MAP_PATH "/sys/fs/bpf/kf/modules_load_status_map"
#define PIN_RINGBUF_PATH "/sys/fs/bpf/kf/mount_events_ringbuf"

#define MAX_PATH_LEN 128

#define MAX_MODULES 50
#define MAX_SYMBOLS 150
#define SYMBOL_LEN 64
#define MODULE_NAME_LEN 64

// Заполняет BPF-мапу из /proc/mounts
void 
load_mounts_to_map(int map_fd) {
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) {
        perror("fopen");
        return;
    }

    char line[256];
    int i = 0;
    while (fgets(line, sizeof(line), fp)) {
        char fs[64], mountp[MAX_PATH_LEN], type[64];
        if (sscanf(line, "%s %s %s", type, mountp, fs) != 3) continue;

        printf("Adding mount: %s (FS: %s)\n", mountp, fs);

        struct fs_stat fs_s;
        strncpy(fs_s.fs, fs, sizeof(fs_s.fs));
        strncpy(fs_s.mount_point, mountp, sizeof(fs_s.mount_point));
        fs_s.stat = 0;
        fs_s.load_status = 1;
        bpf_map_update_elem(map_fd, &i, &fs_s, BPF_ANY);
        ++i;
    }
    fclose(fp);
}

void load_modules_stat_to_map(int map_fd) {
        FILE *fp = fopen("/proc/modules", "r");
    if (!fp) {
        perror("fopen");
        return;
    }

    char line[256];
    int key = 0;
    while (fgets(line, sizeof(line), fp)) {
        struct mod_stat mod_info = {};
        unsigned long size;
        int refcount;
        char depends[128];
        char state;

        // Формат строки: name size refcount depends state
        if (sscanf(line, "%63s %lu %d %127s %c", 
                  mod_info.mod, &size, &refcount, depends, &state) < 3) {
            continue;
        }

        printf("Adding module: %s (refcount: %d)\n", mod_info.mod, refcount);

        mod_info.stat = refcount;
        bpf_map_update_elem(map_fd, &key, &mod_info, BPF_ANY);
        
        key++;
        if (key >= 50) break; // Не превышаем размер мапы
    }
    fclose(fp);

FILE *kallsyms = fopen("/proc/kallsyms", "r");
if (!kallsyms) {
    fprintf(stderr, "Failed to open /proc/kallsyms: %s\n", strerror(errno));
    return;
}

while (fgets(line, sizeof(line), kallsyms)) {
    char module[64] = {0};
    char symbol[64] = {0};
    unsigned long addr;
    char type;

    if (sscanf(line, "%lx %c %63s %63s", &addr, &type, symbol, module) != 4)
        continue;
    
    // Удаляем скобки [modname] -> modname
    size_t mod_len = strlen(module);
    if (mod_len >= 2 && module[0] == '[' && module[mod_len-1] == ']') {
        memmove(module, module+1, mod_len-2);
        module[mod_len-2] = '\0';
    }

    for (int i = 0; i < 50; i++) {
        struct mod_stat mod_info = {};
        if (bpf_map_lookup_elem(map_fd, &i, &mod_info) != 0)
            continue;
        
        if (strlen(mod_info.mod) == 0)
            continue;

        if (type != 'T')
            continue;
            
        if (strcmp(module, mod_info.mod) == 0) {
            // Проверяем границы массива
            if (mod_info.symbol_count >= MAX_SYMBOLS) {
                fprintf(stderr, "Symbol limit reached for module %s\n", module);
                break;
            }
            
            // Копируем символ
            strncpy(mod_info.symbols[mod_info.symbol_count], symbol, SYMBOL_LEN-1);
            mod_info.symbols[mod_info.symbol_count][SYMBOL_LEN-1] = '\0';
            mod_info.symbol_count++;
            
            // Обновляем данные в мапе
            if (bpf_map_update_elem(map_fd, &i, &mod_info, BPF_ANY) != 0) {
                fprintf(stderr, "Failed to update map for module %s: %s\n", 
                       module, strerror(errno));
            }
            break;
        }
    }
}
fclose(kallsyms);

}

int 
main(int argc, char **argv) {
    struct bpf_trace_bpf *obj;
    struct bpf_link *link;
    int fs_map_fd, modules_load_status_map_fd, mount_events_ringbuf_fd;

    // Открытие и загрузка eBPF-программы
    obj = bpf_trace_bpf__open_and_load();
    if (!obj) {
        perror("Failed to load BPF object");
        return 1;
    }

    // Привязываем программу к системному вызову
    link = bpf_program__attach(obj->progs.monitor_openat2);
    if (!link) {
        perror("Failed to attach BPF program");
        bpf_trace_bpf__destroy(obj);
        return 1;
    }

    // Пиннинг программы
    if (bpf_link__pin(link, PIN_LINK_PATH) < 0) {
        perror("Failed to pin BPF link");
        bpf_trace_bpf__destroy(obj);
        return 1;
    }

    // Получение дескриптора карты с файловыми системами
    fs_map_fd = bpf_map__fd(obj->maps.fs_mount_map);
    if (fs_map_fd < 0) {
        perror("Failed to get BPF fs_map");
        bpf_trace_bpf__destroy(obj);
        return 1;
    }

    // Заполняем мапу файловых систем
    load_mounts_to_map(fs_map_fd);

    // Пиннинг мапы с точками монтирования
    if (bpf_obj_pin(fs_map_fd, PIN_FS_MAP_PATH) < 0) {
        perror("Failed to pin BPF fs_map");
        bpf_trace_bpf__destroy(obj);
        return 1;
    }

    modules_load_status_map_fd = bpf_map__fd(obj->maps.modules_load_status_map);
    if (modules_load_status_map_fd < 0) {
        perror("Failed to get BPF modules_load_status_map");
        bpf_trace_bpf__destroy(obj);
        return 1;
    }

    load_modules_stat_to_map(modules_load_status_map_fd);

    if (bpf_obj_pin(modules_load_status_map_fd, PIN_MODULES_LOAD_STATUS_MAP_PATH) < 0) {
        perror("Failed to pin BPF modules_load_status_map");
        bpf_trace_bpf__destroy(obj);
        return 1;
    }

    mount_events_ringbuf_fd = bpf_map__fd(obj->maps.mount_events);
    if (mount_events_ringbuf_fd < 0) {
        perror("Failed to get BPF mount_events_ringbuf");
        bpf_trace_bpf__destroy(obj);
        return 1;
    }

    if (bpf_obj_pin(mount_events_ringbuf_fd, PIN_RINGBUF_PATH) < 0) {
        perror("Failed to pin BPF mount_events_ringbuf_fd");
        bpf_trace_bpf__destroy(obj);
        return 1;
    }

    // Освобождаем ресурсы, но eBPF-программа продолжает работать
    bpf_trace_bpf__destroy(obj);

    return 0;
}
