#include "compile_kernel.h"

static void enable(const char *options[], int options_count) {
    for (int i = 0; i < options_count; i++) 
    {
        printf("Enabling %s...", options[i]);

        char command[256];
        snprintf(command, sizeof(command), "/usr/src/linux/scripts/config --enable %s", options[i]);

        if (!run_command(command, "/usr/src/linux")) {
            fprintf(stderr, "Error: Enabling %s failed.\n", options[i]);
            return;
        }
    }
}

void 
enable_kvm_amd() {
    const char *options[] = {
        "VIRTUALIZATION",
        "64BIT",
        "HIGH_RES_TIMERS",
        "KVM",
        "KVM_AMD"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Enabling KVM AMD support...\n");
    enable(options, options_count);
    printf("KVM AMD support enabled successfully.\n");
}

void 
enable_kvm_intel() {
    const char *options[] = {
        "VIRTUALIZATION",
        "64BIT",
        "HIGH_RES_TIMERS",
        "KVM",
        "KVM_INTEL"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Enabling KVM INTEL support...\n");
    enable(options, options_count);
    printf("KVM INTEL support enabled successfully.\n");
}


void 
enable_selinux() {
    const char *options[] = {
        "NET",
        "INET",
        "AUDIT",
        "SECURITY",
        "SECURITY_NETWORK",
        "SECURITY_SELINUX"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Enabling SELINUX support...\n");
    enable(options, options_count);
    printf("SELINUX support enabled successfully.\n");
}

void
enable_aa() {
    const char *options[] = {
        "NET",
        "SECURITY",
        "SECURITY_APPARMOR"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Enabling APPARMOR support...\n");
    enable(options, options_count);
    printf("APPARMOR support enabled successfully.\n");
}

void 
enable_debug() {
    const char *options[] = {
        "DEBUG_KERNEL"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Enabling DEBUG support...\n");
    enable(options, options_count);
    printf("DEBUG support enabled successfully.\n");
}

void 
enable_profiling() {
    const char *options[] = {
        "PROFILING"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Enabling PROFILING support...\n");
    enable(options, options_count);
    printf("PROFILING support enabled successfully.\n");
}

void
enable_kernel_module_perf() {
    const char *options[] = {
        "BPF_SYSCALL",
        "BPF_JIT",
        "BPF_JIT_ALWAYS_ON",
        "BPF_UNPRIV_DEFAULT_OFF",
        "BPF_PRELOAD",
        "DEBUG_INFO_DWARF4",
        "DEBUG_INFO_BTF",
        "DEBUG_INFO_BTF_MODULES",
        "MODULE_ALLOW_BTF_MISMATCH",
        "FUNCTION_TRACER"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Enabling kernel_module_perf support...\n");
    enable(options, options_count);
    printf("kernel_module_perf support enabled successfully.\n");
}

void 
enable_config_symbol(const char *symbol) {
    char command[256];
    printf("Enabling %s...\n", symbol);
    snprintf(command, sizeof(command), "/usr/src/linux/scripts/config --enable %s", symbol);
    run_command(command, "/usr/src/linux");
}