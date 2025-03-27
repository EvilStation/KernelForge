#ifndef GTK_APP_H
#define GTK_APP_H

#include <adwaita.h>
#include "compile_kernel_data.h"
#include "bpf_trace.h"

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
int bpf_get_fs_stat_map(struct fs_stat *bpf_map, int size);
int bpf_get_modules_stat_map(struct mod_stat *bpf_map, int size);
void print_map(GHashTable *map);
void free_map(GHashTable *map);
int is_bpf_running();
char** get_kernels_files_arr (int *count);
int start_compile_kernel(GenKernelData2 *gen_kernel_data2) ;

#endif