#ifndef GTK_APP_H
#define GTK_APP_H

#include <adwaita.h>
#include "compile_kernel_data.h"
#include "bpf_trace.h"

#define PIN_LINK_PATH "/sys/fs/bpf/kf/do_sys_openat2"
#define PIN_MAP_PATH  "/sys/fs/bpf/kf/fs_mount_map"
#define EFI_DIR "/efi"

typedef struct
{
  GtkBuilder *builder;
  GObject *window;
} BuildDialogData;

typedef struct {
    GenKernelData gen_kernel_data;
    BuildDialogData *build_dialog_data;
    GPid *pid;
} GenKernelData2;

void bpf_trace_run();
int bpf_get_map(struct fs_stat *bpf_map, int size);
void print_map(GHashTable *map);
void free_map(GHashTable *map);
int is_bpf_running();
char** get_kernels_files_arr (int *count);
int start_compile_kernel(GenKernelData2 *gen_kernel_data2) ;

#endif