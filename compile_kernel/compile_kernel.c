#include "compile_kernel_data.h"
#include "compile_kernel.h"

const char *kernel_out_format_arr[2] = {"EFI_STUB", "UKI"};

int
main(int argc, char *argv[], char *envp[]) {

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    GenKernelData gen_kernel_data;
    gen_kernel_data.virt_status = atoi(argv[1]);
    gen_kernel_data.sec_status = atoi(argv[2]);
    gen_kernel_data.perf_status = atoi(argv[3]);
    gen_kernel_data.kernel_module_perf_status = atoi(argv[4]);
    gen_kernel_data.kernel_format = atoi(argv[5]);

    printf("virt_status: %d\n", gen_kernel_data.virt_status);
    printf("sec_status: %d\n", gen_kernel_data.sec_status);
    printf("perf_status: %d\n", gen_kernel_data.perf_status);
    printf("kernel_format: %s\n", kernel_out_format_arr[gen_kernel_data.kernel_format]);
    printf("kernel_module_perf_status: %d\n", gen_kernel_data.kernel_module_perf_status);

    get_cpu_model();
    get_pci_info();

    fprintf(stdout, "Running make defconfig...\n");
    if (!run_command("make defconfig", "/usr/src/linux")) {
        fprintf(stderr, "Error: make defconfig failed.\n");
        return 1;
    }

    init_modules_symbs();

    if ((check_symbol("CONFIG_KVM") < gen_kernel_data.virt_status) && check_symbol("CONFIG_CPU_SUP_AMD"))
        enable_kvm_amd();
    else if ((check_symbol("CONFIG_KVM") > gen_kernel_data.virt_status) && check_symbol("CONFIG_CPU_SUP_AMD"))
        disable_kvm_amd();
    
    if ((check_symbol("CONFIG_KVM") < gen_kernel_data.virt_status) && check_symbol("CONFIG_CPU_SUP_INTEL"))
        enable_kvm_intel();
    else if ((check_symbol("CONFIG_KVM") > gen_kernel_data.virt_status) && check_symbol("CONFIG_CPU_SUP_INTEL"))
        disable_kvm_intel();

    if ((check_symbol("CONFIG_DEFAULT_SECURITY_SELINUX") && check_symbol("CONFIG_DEFAULT_SECURITY_APPARMOR")) < gen_kernel_data.sec_status) {
        enable_selinux();
        enable_aa();
    } else if ((check_symbol("CONFIG_DEFAULT_SECURITY_SELINUX") && check_symbol("CONFIG_DEFAULT_SECURITY_APPARMOR")) > gen_kernel_data.sec_status) {
        disable_selinux();
        disable_aa();
    }

    if ((check_symbol("CONFIG_DEBUG_KERNEL") && check_symbol("CONFIG_PROFILING")) < gen_kernel_data.perf_status) {
        enable_debug();
        enable_profiling();
    } else if ((check_symbol("CONFIG_DEBUG_KERNEL") && check_symbol("CONFIG_PROFILING")) > gen_kernel_data.perf_status) {
        disable_debug();
        disable_profiling();
    }

    if (gen_kernel_data.kernel_module_perf_status > 0) {
        enable_kernel_module_perf();
    }

    printf("Running make...\n");
    if (!run_command("make -O", "/usr/src/linux")) {
        fprintf(stderr, "Error: make failed.\n");
        return 1;
    } 
    printf("Running make modules_install...\n");
    if (!run_command("make -O modules_install", "/usr/src/linux")) {
        fprintf(stderr, "Error: make modules_install failed.\n");
        return 1;
    }
    printf("Running make install...\n");
    if (!run_command("make -O install", "/usr/src/linux")) {
        fprintf(stderr, "Error: make install failed.\n");
        return 1;
    } 

    printf("Kernel compilation completed successfully.\n");
    return 0;
}