#ifndef COMPILE_KERNEL_H
#define COMPILE_KERNEL_H

#include <stdio.h>
#include <glib.h>

gboolean check_symbol(const gchar *symbol);
gboolean check_module_db(const gchar *module);
gboolean run_command(const char *command, const gchar *working_directory);
gboolean run_command_async(const char *command, const gchar *working_directory);
void enable_config_symbol(const char *symbol);
void enable_kvm_amd();
void enable_kvm_intel();
void enable_selinux();
void enable_aa();
void enable_debug();
void enable_profiling();
void enable_kernel_module_perf();
void disable_kvm_amd();
void disable_kvm_intel();
void disable_selinux();
void disable_aa();
void disable_debug();
void disable_profiling();
void get_cpu_model();
void get_pci_info();
void init_modules_symbs();

#endif