#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include "bpf_trace.h"

#define CHECK_INTERVAL 10 // 5 минут в секундах

static volatile bool running = true;

void handle_signal(int sig) {
    running = false;
}

static int handle_ringbuf_event(void *ctx, void *data, size_t size) {
    int *event = data;
    if (!event || size < sizeof(int)) {
        return 0;
    }
    
    printf("Received event: idx=%d\n", *event);
    
    pid_t pid = fork();
    if (pid == 0) {
        setsid();
        execl("./banner", "./banner", NULL);
        _exit(0);
    } else if (pid < 0) {
        perror("Failed to fork");
    }
    
    return 0;
}

void check_fs_map(int map_fd) {
    for (int i = 0; i < 30; i++) {
        struct fs_stat value;
        if (bpf_map_lookup_elem(map_fd, &i, &value)) {
            fprintf(stderr, "Failed to lookup FS key %d\n", i);
            continue;
        }

        if (value.fs[0] == '\0' || value.mount_point[0] == '\0') {
            continue;
        }

        if (value.stat == 0 && value.load_status != 0) {
            printf("Unused FS detected: %s at %s (resetting status)\n",
                   value.fs, value.mount_point);
            value.load_status = 0;
            bpf_map_update_elem(map_fd, &i, &value, BPF_EXIST);
        }
    }
}

void check_modules_map(int map_fd) {
    for (int i = 0; i < 50; i++) {
        struct mod_stat value;
        if (bpf_map_lookup_elem(map_fd, &i, &value)) {
            fprintf(stderr, "Failed to lookup module key %d\n", i);
            continue;
        }

        if (value.mod[0] == '\0') {
            continue;
        }

        if (value.stat == 0 && value.load_status != 1) {
            printf("Inactive module detected: %s (setting load_status=1)\n",
                   value.mod);
            value.load_status = 1;
            bpf_map_update_elem(map_fd, &i, &value, BPF_EXIST);
        }
    }
}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Открываем мапу файловых систем
    int fs_map_fd = bpf_obj_get(PIN_FS_MAP_PATH);
    if (fs_map_fd < 0) {
        perror("Failed to open FS BPF map");
        return 1;
    }

    // Открываем мапу статуса модулей
    int modules_map_fd = bpf_obj_get(PIN_MODULES_LOAD_STATUS_MAP_PATH);
    if (modules_map_fd < 0) {
        perror("Failed to open modules BPF map");
        close(fs_map_fd);
        return 1;
    }

    // Настраиваем ring buffer
    int rb_fd = bpf_obj_get(PIN_RINGBUF_PATH);
    if (rb_fd < 0) {
        perror("Failed to get ringbuf map fd");
        close(fs_map_fd);
        close(modules_map_fd);
        return 1;
    }

    struct ring_buffer *rb = ring_buffer__new(rb_fd, handle_ringbuf_event, NULL, NULL);
    if (!rb) {
        fprintf(stderr, "Failed to create ring buffer\n");
        close(fs_map_fd);
        close(modules_map_fd);
        close(rb_fd);
        return 1;
    }

    printf("Daemon started. Monitoring FS and modules...\n");
    time_t last_check = time(NULL);

    while (running) {
        int err = ring_buffer__poll(rb, 1000);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "Ring buffer error: %d\n", err);
            break;
        }

        if (difftime(time(NULL), last_check) >= CHECK_INTERVAL) {
            check_fs_map(fs_map_fd);
            check_modules_map(modules_map_fd);
            last_check = time(NULL);
        }
    }

    ring_buffer__free(rb);
    close(fs_map_fd);
    close(modules_map_fd);
    close(rb_fd);
    printf("Daemon stopped\n");
    return 0;
}