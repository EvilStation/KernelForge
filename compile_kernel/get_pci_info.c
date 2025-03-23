#include <pci/pci.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "compile_kernel.h"
#include "compile_kernel_data.h"

// Функция для получения имени модуля драйвера
const char *get_driver_name(const char *pci_device_path) {
    static char driver_name[256];
    char driver_link[256];
    ssize_t len;

    // Формируем путь к символьной ссылке на драйвер
    snprintf(driver_link, sizeof(driver_link), "%s/driver", pci_device_path);

    // Читаем символьную ссылку
    len = readlink(driver_link, driver_name, sizeof(driver_name) - 1);
    if (len == -1) {
        return "No driver";
    }

    driver_name[len] = '\0';

    // Извлекаем имя модуля из пути
    char *module_name = strrchr(driver_name, '/');
    if (module_name) {
        return module_name + 1;
    }

    return driver_name;
}

void get_pci_info() {
    struct pci_access *pacc;
    struct pci_dev *dev;
    char vendor_name[1024], device_name[1024], class_name[1024];
    char pci_device_path[256];
    FILE *file;

    file = fopen(LSMOD_PATH, "w");
    if (!file) {
        perror("Failed to open output file");
        return;
    }

    pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);

    for (dev = pacc->devices; dev; dev = dev->next) {
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES);

        // Получаем название вендора и устройства
        pci_lookup_name(pacc, vendor_name, sizeof(vendor_name), PCI_LOOKUP_VENDOR, dev->vendor_id);
        pci_lookup_name(pacc, device_name, sizeof(device_name), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);

        // Получаем описание класса устройства
        pci_lookup_name(pacc, class_name, sizeof(class_name), PCI_LOOKUP_CLASS, dev->device_class);

        // Формируем путь к устройству в /sys/bus/pci/devices/
        snprintf(pci_device_path, sizeof(pci_device_path), "/sys/bus/pci/devices/%04x:%02x:%02x.%d",
                 dev->domain, dev->bus, dev->dev, dev->func);

        // Получаем имя драйвера (модуля)
        const char *driver_name = get_driver_name(pci_device_path);

        if (driver_name && driver_name != "No driver") {
            // Записываем имя драйвера в файл
            fprintf(file, "%s\n", driver_name);
        }

        // Выводим информацию
        printf("Device: %04x:%02x:%02x.%d Vendor: %s Device: %s Type: %s Driver: %s\n",
               dev->domain, dev->bus, dev->dev, dev->func,
               vendor_name, device_name, class_name, driver_name);
    }

    fclose(file);
    pci_cleanup(pacc);
}