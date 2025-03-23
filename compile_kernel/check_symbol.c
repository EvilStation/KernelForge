#include "compile_kernel_data.h"
#include "compile_kernel.h"

gboolean
check_symbol(const gchar *symbol) {
    FILE *file = fopen(CONFIG_PATH, "r");
    gboolean found = FALSE;
    gboolean enabled = FALSE;
    gchar line[256];

    while (fgets(line, sizeof(line), file)) {
        // Удаляем символ новой строки
        g_strchomp(line);

        // Проверяем, начинается ли строка с искомого символа
        if (g_str_has_prefix(line, symbol)) {
            found = TRUE;

            // Проверяем, включен ли символ
            gchar *value = strchr(line, '=');
            if (value && (*(value + 1) == 'y' || *(value + 1) == 'm')) {
                enabled = TRUE;
            }
            break;
        }
    }

    fclose(file);
    return enabled;
}