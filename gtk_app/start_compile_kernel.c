#include "gtk_app.h"

typedef struct {
    GIOChannel *stdout_channel;
    GIOChannel *stderr_channel;
    BuildDialogData *build_dialog_data;
} OnCompileEndData;


gchar* 
remove_ansi_sequences(const gchar *input) 
{
    GString *output = g_string_new("");
    const gchar *p = input;
    while (*p) {
        if (*p == '\x1b') {
            // Пропускаем ANSI-последовательность
            while (*p && *p != 'm') {
                p++;
            }
            if (*p == 'm') {
                p++;
            }
        } else {
            g_string_append_c(output, *p);
            p++;
        }
    }
    return g_string_free(output, FALSE);
}

static gboolean 
read_output(GIOChannel *channel, GIOCondition condition, gpointer user_data) 
{   
    if (condition & G_IO_HUP) {
        // Канал закрыт, завершаем чтение
        return FALSE;
    }
    GtkTextView *compilation_view = GTK_TEXT_VIEW(user_data);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(compilation_view);
    GtkTextMark *mark;
    GtkTextIter iter;
    gchar *line = NULL;
    gsize length = 0;

    // Читаем строку из канала
    if (g_io_channel_read_line(channel, &line, &length, NULL, NULL) == G_IO_STATUS_NORMAL) {
        // Добавляем строку в GtkTextView
        gtk_text_buffer_get_end_iter(buffer, &iter);
        printf("%s", line);
        gchar *cleaned_line = remove_ansi_sequences(line);
        gtk_text_buffer_insert(buffer, &iter, cleaned_line, -1);
        mark = gtk_text_buffer_get_insert(buffer);
        gtk_text_view_scroll_to_mark(compilation_view, mark, 0.0, TRUE, 0.0, 1.0);
        g_free(cleaned_line);

        // // Прокручиваем текст вниз
        // gtk_text_view_scroll_to_iter(compilation_view, &iter, 0.0, FALSE, 0.0, 1.0);
    }

    g_free(line);
    return TRUE; // Продолжаем чтение
}

static void 
on_compile_end(GPid pid, gint status, gpointer user_data) 
{   
    OnCompileEndData *on_compile_end_data = (OnCompileEndData *)user_data; 
    BuildDialogData *build_dialog_data = on_compile_end_data->build_dialog_data;
    GIOChannel *stdout_channel = on_compile_end_data->stdout_channel;
    GIOChannel *stderr_channel = on_compile_end_data->stderr_channel;

    if (WIFEXITED(status)) {
        printf("Child process %d exited with status: %d\n", pid, WEXITSTATUS(status));
    } else {
        printf("Child process %d exited abnormally\n", pid);
    }

    if (stdout_channel) {
        g_io_channel_shutdown(stdout_channel, TRUE, NULL);
        g_io_channel_unref(stdout_channel);
        stdout_channel = NULL;
    }

    if (stderr_channel) {
        g_io_channel_shutdown(stderr_channel, TRUE, NULL);
        g_io_channel_unref(stderr_channel);
        stderr_channel = NULL;
    }

     // Очищаем GtkTextView
    GObject *compilation_view = gtk_builder_get_object(build_dialog_data->builder, "compilation_view");
    if (compilation_view) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(compilation_view));
        if (buffer) {
            gtk_text_buffer_set_text(buffer, "", -1); // Очищаем содержимое
        }
    }

    // Закрываем диалог после завершения сборки
    GObject *build_dialog = gtk_builder_get_object(build_dialog_data->builder, "build_dialog");
    adw_dialog_force_close(ADW_DIALOG(build_dialog));

    // Освобождаем PID
    g_spawn_close_pid(pid);
}

int
start_compile_kernel(GenKernelData2 *gen_kernel_data2) 
{   
    BuildDialogData *build_dialog_data = gen_kernel_data2->build_dialog_data;
    GenKernelData gen_kernel_data = gen_kernel_data2->gen_kernel_data;

    GtkBuilder *builder = build_dialog_data->builder;
    GtkWidget *window = GTK_WIDGET(build_dialog_data->window);
    
    GObject *build_dialog = gtk_builder_get_object(builder, "build_dialog");
    adw_dialog_present(ADW_DIALOG(build_dialog), window);

    GObject *compilation_view = gtk_builder_get_object(builder, "compilation_view");

    gchar *command = g_strdup_printf(
        "./kernel_compiler %i %i %i %i %i",
        gen_kernel_data.virt_status,
        gen_kernel_data.sec_status,
        gen_kernel_data.perf_status,
        gen_kernel_data.kernel_module_perf_status,
        gen_kernel_data.kernel_format
    );

    GPid *pid = gen_kernel_data2->pid;
    gint stdout_fd, stderr_fd;
    GError *error = NULL;
    gchar **argv = NULL;
    g_shell_parse_argv(command, NULL, &argv, NULL);

    gboolean success = g_spawn_async_with_pipes(
        NULL,           // Рабочий каталог (NULL для текущего)
        argv,           // Аргументы команды
        NULL,           // Переменные окружения (NULL для текущих)
        G_SPAWN_DO_NOT_REAP_CHILD, // Флаги
        NULL,           // Функция для настройки дочернего процесса
        NULL,           // Данные для функции настройки
        pid,           // PID дочернего процесса
        NULL,           // stdin (не используем)
        &stdout_fd,     // stdout
        &stderr_fd,     // stderr
        &error          // Ошибки
    );

    if (!success) {
        fprintf(stderr, "Error: %s\n", error->message);
        g_error_free(error);
        g_free(command);
        g_strfreev(argv);
        return -1;
    }

    // Создаем GIOChannel для чтения stdout и stderr
    GIOChannel *stdout_channel = g_io_channel_unix_new(stdout_fd);
    GIOChannel *stderr_channel = g_io_channel_unix_new(stderr_fd);

    // Устанавливаем callback для чтения stdout
    g_io_add_watch(stdout_channel, G_IO_IN | G_IO_HUP, read_output, compilation_view);

    // Устанавливаем callback для чтения stderr
    g_io_add_watch(stderr_channel, G_IO_IN | G_IO_HUP, read_output, compilation_view);

    OnCompileEndData *on_compile_end_data = g_new(OnCompileEndData, 1);
    on_compile_end_data->stdout_channel = stdout_channel;
    on_compile_end_data->stderr_channel = stderr_channel;
    on_compile_end_data->build_dialog_data = build_dialog_data;

    // Устанавливаем callback для отслеживания завершения процесса
    g_child_watch_add(*pid, on_compile_end, on_compile_end_data);

    // Освобождаем память
    g_free(command);
    g_strfreev(argv);

    return 0;
}