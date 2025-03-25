#ifndef BPF_TRACE_H
#define BPF_TRACE_H

struct fs_stat {
    char fs[64];
    char mount_point[64];
    unsigned long long stat;
    uint8_t load_status;
} __attribute__((packed));

#endif