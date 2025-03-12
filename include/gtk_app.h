#pragma once

#include <gtk/gtk.h>

#define PIN_LINK_PATH "/sys/fs/bpf/kf/do_sys_openat2"
#define PIN_MAP_PATH  "/sys/fs/bpf/kf/fs_mount_map"
#define EFI_DIR "/boot"

void bpf_trace_run ();
GHashTable* bpf_get_map();
void print_map(GHashTable *map);
void free_map(GHashTable *map);
int is_bpf_running();
char** get_kernels_files_arr (int *count);