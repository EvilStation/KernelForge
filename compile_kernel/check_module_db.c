#include "compile_kernel_data.h"
#include "compile_kernel.h"

gboolean
check_module_db(const gchar *module) {
    FILE *file = fopen(LSMOD_PATH, "r");
    gboolean found = FALSE;
    gchar line[256];

    while (fgets(line, sizeof(line), file)) {
        // Удаляем символ новой строки
        g_strchomp(line);

        // Проверяем, начинается ли строка с искомого символа
        if (g_str_has_prefix(line, module)) {
            found = TRUE;
            break;
        }
    }

    fclose(file);
    return found;
}