#ifndef COMPILE_KERNEL_DATA_H
#define COMPILE_KERNEL_DATA_H

#include <glib.h>

extern const char *kernel_out_format_arr[2];

typedef struct {
    gboolean virt_status;
    gboolean sec_status;
    gboolean perf_status;
    gboolean kernel_module_perf_status;
    gint kernel_format;
} GenKernelData;

#define CONFIG_PATH "/usr/src/linux/.config"
#define LSMOD_PATH "/root/.config/lsmod.kf"

#endif 