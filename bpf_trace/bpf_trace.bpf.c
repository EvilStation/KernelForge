#define __TARGET_ARCH_x86

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_tracing.h>
#include "bpf_trace.h"

struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 30);         // Максимальное количество элементов в массиве
    __type(key, int);                // Ключ для массива (обычно индекс)
    __type(value, struct fs_stat);  // Значение — структура fs_stat
} fs_mount_map SEC(".maps");

static __always_inline int starts_with(const char cs[64], const char ct[64])
{
    // Если ct (mount_point) пустой - сразу возвращаем 0
    if (ct[0] == '\0') {
        return 0;
    }

    int i = 0;
    // Сравниваем символы до конца `ct` или пока не достигнем конца cs
    while (ct[i] && i < 64) {
        if (cs[i] != ct[i]) return 0;
        i++;
    }

    // Если в `cs` после `ct` стоит '/' или конец строки — это корректное совпадение
    // Также учитываем случай, когда ct может быть пустой строкой ("/")
    return (i == 1 || (i < 64 && (cs[i] == '/' || cs[i] == '\0')));
}


SEC("kprobe/do_sys_openat2")
int 
monitor_openat2(struct pt_regs *ctx) {
    const char *filename = (const char *)PT_REGS_PARM2(ctx);  // Получаем путь
    char filename_buff[64];
    long ret;

    ret = bpf_probe_read_user_str(filename_buff, sizeof(filename_buff), filename);
    if (ret <= 0 || filename_buff[0] != '/') {
        return 0;
    }
        
    struct fs_stat *value;
    int idx;
    for (int i = 0; i < 30; i++) {
        idx = i;
        value = bpf_map_lookup_elem(&fs_mount_map, &idx);
        if (!value || value->fs[0] == '\0' || value->mount_point[0] == '\0') 
            continue;
        if (starts_with(filename_buff, value->mount_point)) {
            value->stat++;
            bpf_map_update_elem(&fs_mount_map, &idx, value, BPF_ANY);
        }
    }

    return 0;
}

char LICENSE[] SEC("license") = "GPL";
