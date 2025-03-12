#include "gtk_app.h"
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


// void
// bpf_trace_run () {
//     if (is_bpf_running()) {
//         printf("Программа уже работает.\n");
//     } else {
//         printf("Программа не запущена. Загружаем...\n");
//         system("./bpf_loader");  // Загружаем программу
//     }
// }

// GHashTable* bpf_get_map() {
//     int map_fd = bpf_obj_get(PIN_MAP_PATH);
//     if (map_fd < 0) {
//         perror("Ошибка открытия eBPF-мапы");
//         return NULL;
//     }

//     GHashTable *hash_table = g_hash_table_new(g_str_hash, g_int_equal);

//     char key[256], next_key[256];  // Строки для ключей
//     uint64_t value;

//     // Инициализация ключа для начала итерации
//     if (bpf_map_get_next_key(map_fd, NULL, key) != 0) {
//         // fprintf(stderr, "Не удалось получить первый ключ (fd: %d): ", map_fd);
//         perror("");
//         close(map_fd);
//         return hash_table;  // Возвращаем пустую таблицу, если мапа пуста
//     }

//     do {
//         // Получение значения для текущего ключа
//         if (bpf_map_lookup_elem(map_fd, key, &value) == 0) {
//             // Копируем данные для хранения в хэш-таблице
//             char *key_copy = g_strdup(key);  // Копируем строковый ключ
//             uint64_t *value_copy = g_new(uint64_t, 1);
//             *value_copy = value;

//             // Вставляем в хэш-таблицу
//             g_hash_table_insert(hash_table, key_copy, value_copy);
//         } else {
//             fprintf(stderr, "Не удалось получить значение для ключа %s\n", key);
//         }

//         // Переход к следующему ключу
//     } while (bpf_map_get_next_key(map_fd, key, next_key) == 0 && (strcpy(key, next_key), 1));

//     close(map_fd);
//     return hash_table;
// }


// // Функция для вывода содержимого хэш-таблицы
// void print_map(GHashTable *map) {
//     GHashTableIter iter;
//     gpointer key, value;

//     g_hash_table_iter_init(&iter, map);
//     while (g_hash_table_iter_next(&iter, &key, &value)) {
//         printf("Ключ: %s, Значение: %lu\n", (char *)key, *(uint64_t *)value);
//     }
// }

// // Очистка памяти
// void free_map(GHashTable *map) {
//     g_hash_table_destroy(map);
// }

// int is_bpf_running() {
//     int prog_fd = bpf_obj_get(PIN_LINK_PATH);
//     if (prog_fd < 0) {
//         perror("eBPF программа не запущена или не закреплена");
//         return 0; // Не запущена
//     } else {
//         printf("eBPF программа активна (fd = %d)\n", prog_fd);
//         close(prog_fd);
//         return 1; // Программа активна
//     }
// }