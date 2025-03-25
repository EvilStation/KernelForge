#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <time.h>
#include <signal.h>
#include "bpf_trace.h" // Ваш заголовочный файл с struct fs_stat

#define CHECK_INTERVAL 300 // 5 минут в секундах
#define MAP_PATH "/sys/fs/bpf/kf/fs_mount_map"

static volatile bool running = true;

void handle_signal(int sig) {
    running = false;
}

int main() {
    // Установка обработчика сигналов для graceful shutdown
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Открываем BPF-мапу
    int map_fd = bpf_obj_get(MAP_PATH);
    if (map_fd < 0) {
        perror("Failed to open BPF map");
        return 1;
    }

    printf("Started FS monitor daemon. Checking every 5 minutes...\n");

    while (running) {
        time_t now = time(NULL);
        printf("[%.*s] Checking FS usage...\n", 24, ctime(&now));

        // Проверяем все записи в мапе
        for (int i = 0; i < 30; i++) {
            struct fs_stat value;
            if (bpf_map_lookup_elem(map_fd, &i, &value)) {
                fprintf(stderr, "Failed to lookup key %d\n", i);
                continue;
            }

            // Пропускаем пустые записи
            if (value.fs[0] == '\0' || value.mount_point[0] == '\0') {
                continue;
            }

            // Если статистика == 0, сбрасываем статус
            if (value.stat == 0 && value.load_status != 0) {
                printf("Unused FS detected: %s at %s (resetting status)\n",
                       value.fs, value.mount_point);

                value.load_status = 0;
                if (bpf_map_update_elem(map_fd, &i, &value, BPF_EXIST)) {
                    fprintf(stderr, "Failed to update map at key %d\n", i);
                }
            } else if (value.stat > 0 && value.load_status == 0) {
                // Опционально: сброс статистики для переиспользуемых ФС
                value.stat = 0;
                bpf_map_update_elem(map_fd, &i, &value, BPF_EXIST);
            }
        }

        // Ожидаем указанный интервал (с проверкой running)
        for (int i = 0; i < CHECK_INTERVAL && running; i++) {
            sleep(1);
        }
    }

    close(map_fd);
    printf("Daemon stopped gracefully\n");
    return 0;
}