#include "gtk_app.h"
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <stdio.h>


void bpf_trace_run () {
    if (is_bpf_running()) {
        printf("Bpf tracing is running.\n");
    } else {
        printf("Loading bpf tracing programm...\n");
        g_spawn_command_line_sync("./bpf_loader", NULL, NULL, NULL, NULL);
    }
}

int bpf_get_map(struct fs_stat *bpf_map, int size) {
    if (!bpf_map) {
        fprintf(stderr, "Error: bpf_map is NULL\n");
        return -1;
    }
    int map_fd = bpf_obj_get(PIN_MAP_PATH);
    if (map_fd < 0) {
        perror("Error getting Bpf map");
        return -1;
    }

    for (int i = 0; i < size; i++) {
        struct fs_stat value;
        if (bpf_map_lookup_elem(map_fd, &i, &value) == 0) {
            bpf_map[i] = value;
        } else {
            perror("Failed to lookup element in BPF map");
            close(map_fd);
            return -1;
        }
    }

    close(map_fd);
    return 0;
}

// Очистка памяти
void free_map(GHashTable *map) {
    g_hash_table_destroy(map);
}

int is_bpf_running() {
    int prog_fd = bpf_obj_get(PIN_LINK_PATH);
    if (prog_fd < 0) {
        perror("BPF not working yet");
        return 0; // Не запущена
    } else {
        printf("BPF is active (fd = %d)\n", prog_fd);
        close(prog_fd);
        return 1; // Программа активна
    }
}