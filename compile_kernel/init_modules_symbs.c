#include "compile_kernel.h"

void init_modules_symbs() {
    const char *command = "make LSMOD=/root/.config/lsmod.kf localmodconfig -C /usr/src/linux";
    FILE *output;
    char line[1024];

    // Временный файл для сохранения вывода
    const char *temp_file = "/tmp/make_localmodconfig_output";

    // Выполняем команду и сохраняем вывод в файл
    char full_command[512];
    snprintf(full_command, sizeof(full_command), "%s 2> %s", command, temp_file);

    if (system(full_command) != 0) {
        fprintf(stderr, "Failed to run make localmodconfig\n");
        return;
    }

    // Открываем временный файл для чтения
    output = fopen(temp_file, "r");
    if (!output) {
        perror("Failed to open temporary file");
        return;
    }

    // Анализируем вывод построчно
    while (fgets(line, sizeof(line), output)) {
        // Убираем символы новой строки и возврата каретки
        line[strcspn(line, "\r\n")] = '\0';

        // Отладочный вывод
        printf("Read line: %s\n", line);

        // Ищем строку "did not have configs ..."
        const char *prefix = "did not have configs ";
        char *match = strstr(line, prefix);

        if (match) {
            // Извлекаем имя символа
            char *symbol_start = match + strlen(prefix);

            // Отладочный вывод
            printf("Found match: %s\n", symbol_start);

            // Включаем символ в конфигурацию
            enable_config_symbol(symbol_start);
        }
    }

    fclose(output);

    // Удаляем временный файл
    remove(temp_file);
}