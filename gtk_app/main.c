#include "gtk_app.h"
#include <math.h>
#include <time.h>

GenKernelData2 *gen_kernel_data2 = NULL;

// Структура для хранения временных данных
typedef struct {
    time_t timestamp;
    guint64 stat_value;
} TimeStat;

// Структура для данных одного графика
typedef struct {
    gchar *fs_name;          // Название файловой системы
    gchar *mount_point;      // Точка монтирования
    gint load_status;        // Статус загрузки
    GtkWidget *drawing_area; // Область рисования
    //GtkWidget *mount_label;  // Метка с точкой монтирования
    GtkWidget *connect_btn;  // Кнопка подключения
    GArray *time_stats;      // Массив значений (время, stat)
    guint64 max_stat;        // Максимальное значение stat
    GdkRGBA line_color;      // Цвет линии
} SingleGraphData;

// Структура для управления всеми графиками
typedef struct {
    GtkWidget *container;    // Контейнер для графиков
    GList *graphs;           // Список SingleGraphData
    time_t start_time;       // Время начала сбора данных
} MultiGraphManager;

// Цвета для графиков
static const char *graph_colors[] = {
    "#3584e4", "#e66100", "#5d3a9b", "#e69f00", "#56b4e9",
    "#009e73", "#f0e442", "#0072b2", "#d55e00", "#cc79a7"
};

// Функция освобождения данных одного графика
static void free_single_graph(gpointer data) {
    SingleGraphData *graph = (SingleGraphData *)data;
    if (graph) {
        g_free(graph->fs_name);
        g_free(graph->mount_point);
        g_array_free(graph->time_stats, TRUE);
        g_free(graph);
    }
}

// Функция освобождения менеджера графиков
static void free_graph_manager(gpointer data) {
    MultiGraphManager *manager = (MultiGraphManager *)data;
    if (manager) {
        g_list_free_full(manager->graphs, free_single_graph);
        g_free(manager);
    }
}

// Функция рисования одного графика
static void draw_single_graph(GtkDrawingArea *area,
                            cairo_t *cr,
                            int width,
                            int height,
                            gpointer user_data) {
    SingleGraphData *graph = (SingleGraphData *)user_data;
    
    // Очищаем фон
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    
    if (!graph || !graph->time_stats || graph->time_stats->len < 2) return;
    
    // Настройки внешнего вида
    const double padding = 40.0;
    const double available_width = width - 2 * padding;
    const double available_height = height - 2 * padding;
    
    // Получаем временной диапазон
    TimeStat *first = &g_array_index(graph->time_stats, TimeStat, 0);
    TimeStat *last = &g_array_index(graph->time_stats, TimeStat, graph->time_stats->len - 1);
    time_t time_range = last->timestamp - first->timestamp;
    if (time_range == 0) time_range = 1;
    
    // Рисуем оси
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 1.0);
    
    // Ось X
    cairo_move_to(cr, padding, height - padding);
    cairo_line_to(cr, width - padding, height - padding);
    
    // Ось Y
    cairo_move_to(cr, padding, height - padding);
    cairo_line_to(cr, padding, padding);
    cairo_stroke(cr);
    
    // Подпись названия графика
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, 
                         CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);
    
    cairo_text_extents_t extents;
    cairo_text_extents(cr, graph->fs_name, &extents);
    cairo_move_to(cr, 
                 (width - extents.width) / 2, 
                 padding - 10);
    cairo_show_text(cr, graph->fs_name);
    
    // Рисуем сетку и подписи значений
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, 
                         CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 10);
    
    for (int i = 0; i <= 5; i++) {
        double y = height - padding - (i * available_height / 5);
        
        // Сетка
        cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
        cairo_move_to(cr, padding, y);
        cairo_line_to(cr, width - padding, y);
        cairo_stroke(cr);
        
        // Подписи значений
        char label[32];
        guint64 value = (guint64)(graph->max_stat * i / 5);
        snprintf(label, sizeof(label), "%" G_GUINT64_FORMAT, value);
        
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_text_extents(cr, label, &extents);
        cairo_move_to(cr, 
                     padding - extents.width - 10, 
                     y + extents.height / 2);
        cairo_show_text(cr, label);
    }
    
    // Рисуем линию графика
    gdk_cairo_set_source_rgba(cr, &graph->line_color);
    cairo_set_line_width(cr, 2.0);
    cairo_new_path(cr);
    
    // Первая точка
    TimeStat *ts = &g_array_index(graph->time_stats, TimeStat, 0);
    double x = padding + ((double)(ts->timestamp - first->timestamp) / time_range) * available_width;
    double y = height - padding - ((double)ts->stat_value / graph->max_stat) * available_height;
    cairo_move_to(cr, x, y);
    
    // Остальные точки
    for (guint i = 1; i < graph->time_stats->len; i++) {
        ts = &g_array_index(graph->time_stats, TimeStat, i);
        x = padding + ((double)(ts->timestamp - first->timestamp) / time_range) * available_width;
        y = height - padding - ((double)ts->stat_value / graph->max_stat) * available_height;
        
        // Проверка на корректность координат
        if (!isnan(x) && !isnan(y) && isfinite(x) && isfinite(y)) {
            cairo_line_to(cr, x, y);
        }
    }
    
    cairo_stroke(cr);
    
    // Рисуем точки
    for (guint i = 0; i < graph->time_stats->len; i++) {
        ts = &g_array_index(graph->time_stats, TimeStat, i);
        x = padding + ((double)(ts->timestamp - first->timestamp) / time_range) * available_width;
        y = height - padding - ((double)ts->stat_value / graph->max_stat) * available_height;

        if (!isnan(x) && !isnan(y) && isfinite(x) && isfinite(y)) {
            cairo_save(cr);
            cairo_arc(cr, x, y, 3, 0, 2 * G_PI);
            gdk_cairo_set_source_rgba(cr, &graph->line_color);
            cairo_fill(cr); // Только заливка, без обводки
            cairo_restore(cr);
        }
    }
    
    // Подпись точки монтирования УДАЛЕНА
}

// Обработчик нажатия кнопки подключения
static void on_connect_clicked(GtkButton *button, SingleGraphData *graph) {
    g_print("Connecting filesystem: %s at %s\n", graph->fs_name, graph->mount_point);
    // Здесь должна быть логика подключения файловой системы
    
    // После подключения меняем статус и обновляем интерфейс
    graph->load_status = 1;
    
    // Получаем родительский контейнер кнопки и скрываем его
    GtkWidget *connect_box = gtk_widget_get_parent(GTK_WIDGET(button));
    gtk_widget_set_visible(connect_box, FALSE);
    
    // Показываем область рисования
    gtk_widget_set_visible(graph->drawing_area, TRUE);
    
    // Обновляем график
    gtk_widget_queue_draw(graph->drawing_area);
}


// Обновление или создание графиков
static void update_all_graphs(MultiGraphManager *manager) {
    struct fs_stat new_stats[30];
    if (bpf_get_fs_stat_map(new_stats, 30) != 0) {
        g_warning("Failed to get BPF map data");
        return;
    }
    
    time_t current_time = time(NULL);
    if (manager->start_time == 0) {
        manager->start_time = current_time;
    }
    
    for (int i = 0; i < 30; i++) {
        if (strlen(new_stats[i].fs) == 0) continue;
        
        SingleGraphData *graph = NULL;
        GList *iter;
        for (iter = manager->graphs; iter != NULL; iter = iter->next) {
            SingleGraphData *g = (SingleGraphData *)iter->data;
            if (strcmp(g->fs_name, new_stats[i].fs) == 0) {
                graph = g;
                break;
            }
        }
        
        if (!graph) {
            graph = g_new0(SingleGraphData, 1);
            graph->fs_name = g_strdup(new_stats[i].fs);
            graph->mount_point = g_strdup(new_stats[i].mount_point);
            graph->load_status = new_stats[i].load_status;
            graph->time_stats = g_array_new(FALSE, FALSE, sizeof(TimeStat));
            
            // Создаем основной вертикальный контейнер
            GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
            gtk_widget_set_margin_top(box, 10);
            gtk_widget_set_margin_bottom(box, 10);
            gtk_box_append(GTK_BOX(manager->container), box);
            
            // Область рисования графика
            graph->drawing_area = gtk_drawing_area_new();
            gtk_widget_set_size_request(graph->drawing_area, 600, 200);
            gtk_widget_set_hexpand(graph->drawing_area, TRUE);
            gtk_widget_set_vexpand(graph->drawing_area, TRUE);
            
            // Стиль области графика
            GtkCssProvider *provider = gtk_css_provider_new();
            gtk_css_provider_load_from_data(provider,
                ".graph-area {"
                "   background-color: white;"
                "   border: 1px solid #d3d3d3;"
                "   border-radius: 5px;"
                "   margin: 5px;"
                "}", -1);
            gtk_style_context_add_provider(gtk_widget_get_style_context(graph->drawing_area),
                                         GTK_STYLE_PROVIDER(provider),
                                         GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            gtk_widget_add_css_class(graph->drawing_area, "graph-area");
            gtk_box_append(GTK_BOX(box), graph->drawing_area);
            
            // Контейнер для информации о ФС и кнопки (только для неактивных ФС)
            if (graph->load_status == 0) {
                GtkWidget *connect_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                gtk_widget_set_margin_top(connect_box, 5);
                gtk_widget_set_halign(connect_box, GTK_ALIGN_CENTER);
                
                // Метка с названием ФС и точкой монтирования
                GtkWidget *fs_label = gtk_label_new(NULL);
                gchar *label_text = g_strdup_printf("<b>%s</b> (%s)", 
                                                  graph->fs_name,
                                                  graph->mount_point);
                gtk_label_set_markup(GTK_LABEL(fs_label), label_text);
                g_free(label_text);
                gtk_widget_set_margin_end(fs_label, 10);
                
                // Стиль метки
                GtkCssProvider *label_provider = gtk_css_provider_new();
                gtk_css_provider_load_from_data(label_provider,
                    ".fs-label {"
                    "   padding: 5px 10px;"
                    "   background: #f5f5f5;"
                    "   border-radius: 5px;"
                    "}", -1);
                gtk_style_context_add_provider(gtk_widget_get_style_context(fs_label),
                                             GTK_STYLE_PROVIDER(label_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
                gtk_widget_add_css_class(fs_label, "fs-label");
                
                gtk_box_append(GTK_BOX(connect_box), fs_label);
                
                // Кнопка подключения
                graph->connect_btn = gtk_button_new_with_label("Подключить");
                gtk_box_append(GTK_BOX(connect_box), graph->connect_btn);
                g_signal_connect(graph->connect_btn, "clicked", 
                                G_CALLBACK(on_connect_clicked), graph);
                
                // Добавляем контейнер в основной бокс
                gtk_box_append(GTK_BOX(box), connect_box);
            }
            
            // Настраиваем функцию рисования
            gtk_drawing_area_set_draw_func(
                GTK_DRAWING_AREA(graph->drawing_area),
                draw_single_graph,
                graph,
                NULL);
                
            // Настраиваем цвет линии
            gdk_rgba_parse(&graph->line_color, 
                          graph_colors[g_list_length(manager->graphs) % G_N_ELEMENTS(graph_colors)]);
            
            manager->graphs = g_list_append(manager->graphs, graph);
            
            // Начальная видимость элементов
            gtk_widget_set_visible(graph->drawing_area, graph->load_status != 0);
        }
        
        // Обновляем данные только для активных графиков
        if (graph->load_status != 0) {
            TimeStat ts = { current_time, new_stats[i].stat };
            g_array_append_val(graph->time_stats, ts);
            
            if (new_stats[i].stat > graph->max_stat) {
                graph->max_stat = new_stats[i].stat;
            }
            
            // Удаляем старые данные (старше 1 часа)
            if (graph->time_stats->len > 0) {
                TimeStat *first = &g_array_index(graph->time_stats, TimeStat, 0);
                if (current_time - first->timestamp > 3600) {
                    g_array_remove_index(graph->time_stats, 0);
                }
            }
            
            gtk_widget_queue_draw(graph->drawing_area);
        } else {
            // Обновляем информацию для неактивных ФС (если нужно)
            // Можно добавить обновление статистики или другой информации
        }
    }
}

// Функция периодического обновления
static gboolean update_graphs_periodic(gpointer user_data) {
    MultiGraphManager *manager = (MultiGraphManager *)user_data;
    update_all_graphs(manager);
    return G_SOURCE_CONTINUE;
}

// Инициализация менеджера графиков
static MultiGraphManager* setup_graphs_container(GtkWidget *container) {
    MultiGraphManager *manager = g_new0(MultiGraphManager, 1);
    manager->container = container;
    manager->graphs = NULL;
    manager->start_time = 0;
    
    // Настраиваем контейнер
    gtk_widget_set_vexpand(container, TRUE);
    
    // Запускаем периодическое обновление (каждую секунду)
    g_timeout_add_seconds(1, (GSourceFunc)update_graphs_periodic, manager);
    
    return manager;
}


static void
on_kernel_button_clicked(GtkButton *button, GtkStack *stack)
{
  const char *page_name = g_object_get_data(G_OBJECT(button), "page-name");
  adw_view_stack_set_visible_child_name(ADW_VIEW_STACK(stack), page_name);
}

static void
on_start_button_clicked(GtkButton *button, BuildDialogData *build_dialog_data)
{ 
  GtkBuilder *builder = build_dialog_data->builder;
  GtkWidget *window = GTK_WIDGET(build_dialog_data->window);

  GenKernelData gen_kernel_data;

  GObject *virt_check_button = gtk_builder_get_object(builder, "virt_check_button");
  GObject *sec_check_button = gtk_builder_get_object(builder, "sec_check_button");
  GObject *perf_check_button = gtk_builder_get_object(builder, "perf_check_button");
  gen_kernel_data.virt_status = gtk_check_button_get_active(GTK_CHECK_BUTTON(virt_check_button));
  gen_kernel_data.sec_status = gtk_check_button_get_active(GTK_CHECK_BUTTON(sec_check_button));
  gen_kernel_data.perf_status = gtk_check_button_get_active(GTK_CHECK_BUTTON(perf_check_button));

  GObject *kernel_format = gtk_builder_get_object(builder, "kernel_format");
  gen_kernel_data.kernel_format = adw_combo_row_get_selected(ADW_COMBO_ROW(kernel_format));

  GObject *kernel_module_perf = gtk_builder_get_object(builder, "kernel_module_perf");
  gen_kernel_data.kernel_module_perf_status = adw_switch_row_get_active(ADW_SWITCH_ROW(kernel_module_perf));
  
  if (!gen_kernel_data2) {
    gen_kernel_data2 = g_new(GenKernelData2, 1);
  }
  gen_kernel_data2->gen_kernel_data = gen_kernel_data;
  gen_kernel_data2->build_dialog_data = build_dialog_data;
  gen_kernel_data2->pid = g_new(GPid, 1);
  start_compile_kernel(gen_kernel_data2);

}

static void 
response_int_cb(AdwDialog* build_alert) {
  GPid pid = *(gen_kernel_data2->pid);
  printf("CHILD PID: %i\n", pid);
  if (kill(pid, SIGINT) == 0) {
    g_print("Sent SIGTERM to process with PID: %d\n", pid);
  } else {
    g_warning("Failed to send SIGTERM to process with PID: %d", pid);
  }
}

static void
on_build_dialog_close(AdwDialog *build_dialog)
{
  AdwDialog *build_alert = adw_alert_dialog_new("Прервать сборку?", NULL);
  adw_alert_dialog_format_body(ADW_ALERT_DIALOG(build_alert),
                              "Часть прогресса сборки будет сброшена.");
  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (build_alert),
                                "cancel",  "Отмена",
                                "interupt", "Прервать", 
                                NULL);
  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (build_alert),
                                          "interupt",
                                          ADW_RESPONSE_DESTRUCTIVE);
  adw_alert_dialog_set_close_response(ADW_ALERT_DIALOG (build_alert), "cancel");
  g_signal_connect (build_alert, "response::interupt", G_CALLBACK (response_int_cb), NULL);
  adw_dialog_present(build_alert, GTK_WIDGET(build_dialog));
}

static void
activate_cb(GtkApplication *app, gpointer user_data)
{ 
  bpf_trace_run();

  GtkBuilder *builder;
  builder = gtk_builder_new_from_file ("kf.ui");

  GObject *window = gtk_builder_get_object (builder, "MainWindow");
  gtk_window_set_application (GTK_WINDOW (window), app);
  
  GObject *start_button = gtk_builder_get_object(builder, "start_button");
  BuildDialogData *build_dialog_data = g_new(BuildDialogData, 1);
  build_dialog_data->builder = builder;
  build_dialog_data->window = window;
  g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_button_clicked), build_dialog_data);

  GObject *build_dialog = gtk_builder_get_object(builder, "build_dialog");
  g_signal_connect(build_dialog, "close-attempt", G_CALLBACK(on_build_dialog_close), NULL);

  GObject *optimization_box = gtk_builder_get_object(builder, "optimization_box");
  MultiGraphManager *manager = setup_graphs_container(GTK_WIDGET(optimization_box));
  g_object_set_data_full(G_OBJECT(optimization_box), "graph-manager", 
                         manager, free_graph_manager);

  GObject *optimization2_pref_group = gtk_builder_get_object(builder, "optimization2_pref_group");
  struct mod_stat mod_stats[50];
  if (bpf_get_modules_stat_map(mod_stats, 50) != 0) {
      g_warning("Failed to get BPF map data");
      return;
  }
  for (int i = 0; i < 50; i++) {
    if (strlen(mod_stats[i].mod) > 0) {
        GtkWidget *action_row = adw_action_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(action_row), mod_stats[i].mod);
        char *stat_str = g_strdup_printf("%llu", mod_stats[i].stat);

        if (mod_stats[i].load_status == 1) {
            adw_action_row_set_subtitle(ADW_ACTION_ROW(action_row), "Модуль выключен");
            GtkWidget *load_button = gtk_button_new();
            gtk_button_set_label(GTK_BUTTON(load_button), "включить");
            adw_action_row_add_suffix(ADW_ACTION_ROW(action_row), load_button);
        }

        GtkWidget *mod_stat = gtk_label_new(stat_str);
        adw_action_row_add_suffix(ADW_ACTION_ROW(action_row), mod_stat);
        adw_preferences_group_add(ADW_PREFERENCES_GROUP(optimization2_pref_group), action_row);
    }
  }

  gtk_widget_set_visible(GTK_WIDGET(window), TRUE);
  // g_object_unref (builder);
}

int
main (int argc, char *argv[])
{
  g_autoptr (AdwApplication) app = NULL;
  app = adw_application_new ("org.gtk.kf", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate_cb), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
