#include "compile_kernel.h"

static void disable(const char *options[], int options_count) {
    for (int i = 0; i < options_count; i++) 
    {
        printf("Disabling %s...\n", options[i]);

        char command[256];
        snprintf(command, sizeof(command), "/usr/src/linux/scripts/config --disable %s", options[i]);

        if (!run_command(command, "/usr/src/linux")) {
            fprintf(stderr, "Error: Disabling %s failed.\n", options[i]);
            return;
        }
    }
}

void 
disable_kvm_amd() {
    const char *options[] = {
        "KVM_AMD",
        "KVM",
        "VIRTUALIZATION",
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Disabling KVM AMD support...\n");
    disable(options, options_count);
    printf("KVM AMD support disabled successfully.\n");
}

void 
disable_kvm_intel() {
    const char *options[] = {
        "KVM_INTEL",
        "KVM",
        "VIRTUALIZATION",
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Disabling KVM intel support...\n");
    disable(options, options_count);
    printf("KVM intel support disabled successfully.\n");
}

void 
disable_selinux() {
    const char *options[] = {
        "SECURITY_SELINUX"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Disabling SELINUX support...\n");
    disable(options, options_count);
    printf("SELINUX support disabled successfully.\n");
}

void
disable_aa() {
        const char *options[] = {
        "SECURITY_APPARMOR"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Disabling APPARMOR support...\n");
    disable(options, options_count);
    printf("APPARMOR support disabled successfully.\n");
}

void 
disable_debug() {
    const char *options[] = {
        "DEBUG_KERNEL"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Disabling DEBUG support...\n");
    disable(options, options_count);
    printf("DEBUG support disabled successfully.\n");
}

void 
disable_profiling() {
    const char *options[] = {
        "PROFILING"
    };
    int options_count = sizeof(options) / sizeof(options[0]);

    printf("Disabling PROFILING support...\n");
    disable(options, options_count);
    printf("PROFILING support disabled successfully.\n");
}