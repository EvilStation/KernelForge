#include "gtk_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int 
is_kernel_file (const char *filename) {
    return (
        strstr(filename, "vmlinuz") ||
        strstr(filename, "bzImage") ||
        strstr(filename, "kernel") ||
        strstr(filename, ".efi") ||
        strstr(filename, ".EFI")
    );
}

// Рекурсивная функция для поиска файлов ядер
void find_kernel_files(const char *dir_path, char ***kernels_arr, int *kernels_count) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char path[PATH_MAX];
    struct stat statbuf;

    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем текущую и родительскую директории
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Создаем полный путь к элементу
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        // Получаем информацию о файле
        if (stat(path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                // Если это директория, вызываем функцию рекурсивно
                find_kernel_files(path, kernels_arr, kernels_count);
            } else if (S_ISREG(statbuf.st_mode) && is_kernel_file(entry->d_name)) {
                // Если это файл ядра, добавляем его в массив
                *kernels_arr = realloc(*kernels_arr, (*kernels_count + 1) * sizeof(char*));
                (*kernels_arr)[*kernels_count] = strdup(path);
                (*kernels_count)++;
            }
        }
    }

    closedir(dir);
}

// Получение массива строк с путями к найденным файлам ядер
char** 
get_kernels_files_arr (int *count) {
    char **kernels_arr = NULL;
    int kernels_count = 0;
    find_kernel_files (EFI_DIR, &kernels_arr, &kernels_count);
    *count = kernels_count;
    return kernels_arr;
}
