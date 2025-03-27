#ifndef BPF_TRACE_H
#define BPF_TRACE_H

#define PIN_LINK_PATH "/sys/fs/bpf/kf/do_sys_openat2"
#define PIN_FS_MAP_PATH "/sys/fs/bpf/kf/fs_mount_map"
#define PIN_MODULES_LOAD_STATUS_MAP_PATH "/sys/fs/bpf/kf/modules_load_status_map"
#define PIN_RINGBUF_PATH "/sys/fs/bpf/kf/mount_events_ringbuf"

struct fs_stat {
    char fs[64];
    char mount_point[64];
    unsigned long long stat;
    uint8_t load_status;
} __attribute__((packed));

struct mod_stat {
    char mod[64];
    unsigned long long stat;
    uint8_t load_status;
    char symbols[150][64];
    int symbol_count;
} __attribute__((packed));

#endif