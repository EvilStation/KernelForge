#include "compile_kernel.h"

gboolean 
run_command(const char *command, const gchar *working_directory) {
    GError *error = NULL;
    gchar *stdout_data = NULL;
    gchar *stderr_data = NULL;
    gint exit_status;
    gchar **argv = NULL;

    if (!g_shell_parse_argv(command, NULL, &argv, &error)) {
        fprintf(stderr, "Error parsing command: %s\n", error->message);
        g_error_free(error);
        return FALSE;
    }

    gboolean success = g_spawn_sync(
        working_directory,  // Рабочий каталог
        argv,               // Аргументы команды
        NULL,               // Переменные окружения (NULL для текущих)
        G_SPAWN_SEARCH_PATH, // Флаги
        NULL,               // Функция настройки дочернего процесса
        NULL,               // Данные для функции настройки
        &stdout_data,       // Стандартный вывод
        &stderr_data,       // Стандартный вывод ошибок
        &exit_status,       // Статус завершения
        &error              // Ошибки
    );

    printf("%s\n", stdout_data);
    printf("%s\n", stderr_data);

    if (!success) {
        fprintf(stderr, "Error: %s\n", error->message);
        g_error_free(error);
    }

    if (exit_status != 0) {
        fprintf(stderr, "Command failed with exit status: %d\n", exit_status);
    }

    g_strfreev(argv);
    g_free(stdout_data);
    g_free(stderr_data);

    return success && (exit_status == 0);
}