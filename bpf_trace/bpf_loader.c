#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "bpf_trace.skel.h"
#include "bpf_trace.h"

#define PIN_LINK_PATH "/sys/fs/bpf/kf/do_sys_openat2"
#define PIN_FS_MAP_PATH "/sys/fs/bpf/kf/fs_mount_map"
#define PIN_RINGBUF_PATH "/sys/fs/bpf/kf/mount_events"

#define MAX_PATH_LEN 128

// Заполняет BPF-мапу из /proc/mounts
void 
load_mounts_to_bpf(int map_fd) {
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

int 
main(int argc, char **argv) {
    struct bpf_trace_bpf *obj;
    struct bpf_link *link;
    int fs_map_fd;

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
    load_mounts_to_bpf(fs_map_fd);

    // Пиннинг мапы с точками монтирования
    if (bpf_obj_pin(fs_map_fd, PIN_FS_MAP_PATH) < 0) {
        perror("Failed to pin BPF fs_map");
        bpf_trace_bpf__destroy(obj);
        return 1;
    }

    // Освобождаем ресурсы, но eBPF-программа продолжает работать
    bpf_trace_bpf__destroy(obj);

    printf("Size of struct fs_stat: %zu\n", sizeof(struct fs_stat));
    return 0;
}
