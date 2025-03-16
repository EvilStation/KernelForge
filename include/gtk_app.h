#ifndef GTK_APP_H
#define GTK_APP_H

#include <adwaita.h>

#define PIN_LINK_PATH "/sys/fs/bpf/kf/do_sys_openat2"
#define PIN_MAP_PATH  "/sys/fs/bpf/kf/fs_mount_map"
#define EFI_DIR "/efi"

typedef struct
{
  GtkBuilder *builder;
  GObject *window;
} BuildDialogData;

#ifdef KERNEL_FORMAT_ARR
const char *kernel_out_format_arr[] = {"EFI_STUB", "UKI"};
#else
extern const char *kernel_out_format_arr[];
#endif

typedef struct {
    gboolean kvm_inc_status;
    gboolean selinux_inc_status;
    gboolean aa_inc_status;
    gboolean kernel_module_perf_inc_status;
    gint kernel_format;
} GenKernelData;

typedef struct {
    GenKernelData gen_kernel_data;
    BuildDialogData *build_dialog_data;
} GenKernelData2;

void bpf_trace_run ();
GHashTable* bpf_get_map();
void print_map(GHashTable *map);
void free_map(GHashTable *map);
int is_bpf_running();
char** get_kernels_files_arr (int *count);
int start_compile_kernel(GenKernelData2 gen_kernel_data2) ;

#endif