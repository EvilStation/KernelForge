#include <glib.h>
#include <stdio.h>


static gboolean run_command(const char *command) {
    GError *error = NULL;
    gchar *stdout_data = NULL;
    gchar *stderr_data = NULL;
    gint exit_status;

    gboolean success = g_spawn_command_line_sync(
        command,
        &stdout_data,
        &stderr_data,
        &exit_status,
        &error
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

    g_free(stdout_data);
    g_free(stderr_data);

    return success && (exit_status == 0);
}

int
main(int argc, char *argv[], char *envp[]) {

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    GString *command = g_string_new("");
    for (int i = 1; i < argc; i++) {
        g_string_append(command, argv[i]);
        if (i < argc - 1) {
            g_string_append(command, " ");
        }
    }
    printf("COMMAND: %s\n", command->str);
    
    printf("Running modprobed-db store...\n");
    if (!run_command("modprobed-db store")) {
        fprintf(stderr, "Error: modprobed-db store failed.\n");
        return 1;
    }

    printf("Running make localmodconfig...\n");
    if (!run_command("make LSMOD=/root/.config/modprobed.db localmodconfig -C /usr/src/linux")) {
        fprintf(stderr, "Error: make localmodconfig failed.\n");
        return 1;
    }

    printf("Running make...\n");
    if (!run_command("make -C /usr/src/linux")) {
        fprintf(stderr, "Error: make failed.\n");
        return 1;
    }

    printf("Kernel compilation completed successfully.\n");
    return 0;
}