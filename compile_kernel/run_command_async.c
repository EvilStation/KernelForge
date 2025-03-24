#include "compile_kernel.h"

gboolean run_command_async(const char *command, const gchar *working_directory) {
    GError *error = NULL;
    gchar **argv = NULL;
    gboolean success;
    GPid child_pid;
    gint stdout_fd, stderr_fd;
    GIOChannel *stdout_channel, *stderr_channel;
    gchar buffer[1024];
    gsize bytes_read;
    gint exit_status;

    if (!g_shell_parse_argv(command, NULL, &argv, &error)) {
        fprintf(stderr, "Error parsing command: %s\n", error->message);
        g_error_free(error);
        return FALSE;
    }

    success = g_spawn_async_with_pipes(
        working_directory,  // Рабочий каталог
        argv,               // Аргументы команды
        NULL,               // Переменные окружения (NULL для текущих)
        G_SPAWN_SEARCH_PATH, // Флаги
        NULL,               // Функция настройки дочернего процесса
        NULL,               // Данные для функции настройки
        &child_pid,         // PID дочернего процесса
        NULL,               // STDIN (не используем)
        &stdout_fd,         // STDOUT
        &stderr_fd,         // STDERR
        &error              // Ошибки
    );

    if (!success) {
        fprintf(stderr, "Error: %s\n", error->message);
        g_error_free(error);
        g_strfreev(argv);
        return FALSE;
    }

    stdout_channel = g_io_channel_unix_new(stdout_fd);
    stderr_channel = g_io_channel_unix_new(stderr_fd);

    // Чтение вывода в реальном времени
    while (TRUE) {
        GIOStatus status;

        // Чтение stdout
        status = g_io_channel_read_chars(stdout_channel, buffer, sizeof(buffer) - 1, &bytes_read, NULL);
        if (status == G_IO_STATUS_NORMAL) {
            buffer[bytes_read] = '\0';
            printf("%s", buffer);
        } else if (status == G_IO_STATUS_EOF) {
            break;
        }

        // Чтение stderr
        status = g_io_channel_read_chars(stderr_channel, buffer, sizeof(buffer) - 1, &bytes_read, NULL);
        if (status == G_IO_STATUS_NORMAL) {
            buffer[bytes_read] = '\0';
            fprintf(stderr, "%s", buffer);
        } else if (status == G_IO_STATUS_EOF) {
            break;
        }
    }

    g_io_channel_unref(stdout_channel);
    g_io_channel_unref(stderr_channel);

    // Ожидание завершения процесса
    g_spawn_close_pid(child_pid);

    if (!g_spawn_check_wait_status(exit_status, &error)) {
        fprintf(stderr, "Command failed with exit status: %d\n", exit_status);
        g_error_free(error);
        g_strfreev(argv);
        return FALSE;
    }

    g_strfreev(argv);
    return TRUE;
}