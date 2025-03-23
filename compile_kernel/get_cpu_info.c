#include <cpuid.h>
#include "compile_kernel.h"

void get_cpu_model() {
    char brand[0x40];
    unsigned int eax, ebx, ecx, edx;
    
    __cpuid(0x80000002, eax, ebx, ecx, edx);
    memcpy(brand, &eax, sizeof(eax));
    memcpy(brand + 4, &ebx, sizeof(ebx));
    memcpy(brand + 8, &ecx, sizeof(ecx));
    memcpy(brand + 12, &edx, sizeof(edx));

    __cpuid(0x80000003, eax, ebx, ecx, edx);
    memcpy(brand + 16, &eax, sizeof(eax));
    memcpy(brand + 20, &ebx, sizeof(ebx));
    memcpy(brand + 24, &ecx, sizeof(ecx));
    memcpy(brand + 28, &edx, sizeof(edx));

    __cpuid(0x80000004, eax, ebx, ecx, edx);
    memcpy(brand + 32, &eax, sizeof(eax));
    memcpy(brand + 36, &ebx, sizeof(ebx));
    memcpy(brand + 40, &ecx, sizeof(ecx));
    memcpy(brand + 44, &edx, sizeof(edx));

    brand[48] = '\0';
    printf("CPU Model: %s\n", brand);
}
