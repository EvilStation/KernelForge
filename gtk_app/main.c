#include "gtk_app.h"

GenKernelData2 *gen_kernel_data2 = NULL;



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
draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data)
{
  GdkRGBA color;

  // cairo_arc(cr,
  //            width / 2.0, height / 2.0,
  //            MIN (width, height) / 2.0,
  //            0, 2 * G_PI);
  cairo_set_line_width (cr, 0.5);
  cairo_rectangle (cr, 20, 20, 650, 80);

  gtk_widget_get_color(GTK_WIDGET(area), &color);
  gdk_cairo_set_source_rgba(cr, &color);

  cairo_stroke(cr);
}


static gboolean update_optimization_box(gpointer data) {
  GtkWidget *optimization_box = (GtkWidget *)data;
  GtkWidget *child;
  while ((child = gtk_widget_get_first_child(optimization_box)) != NULL) {
      gtk_box_remove(GTK_BOX(optimization_box), child);
  }
  struct fs_stat bpf_map[30];
  if (bpf_get_map(bpf_map, 30) != 0) {
      printf("Error getting map\n");
      return G_SOURCE_CONTINUE;
  }
  for (int i = 0; i < 30; i++) {
    if (strlen(bpf_map[i].fs) > 0) {
      char stat_str[64];
      snprintf(stat_str, sizeof(stat_str), "%llu", bpf_map[i].stat);
      GtkWidget *fs_label = gtk_label_new(bpf_map[i].fs);
      GtkWidget *stat_label = gtk_label_new(stat_str);
      gtk_box_append(GTK_BOX(optimization_box), fs_label);
      gtk_box_append(GTK_BOX(optimization_box), stat_label);
      GtkWidget *area = gtk_drawing_area_new();
      gtk_drawing_area_set_content_width(GTK_DRAWING_AREA (area), 100);
      gtk_drawing_area_set_content_height(GTK_DRAWING_AREA (area), 100);
      gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA (area), draw_function, NULL, NULL);
      gtk_box_append(GTK_BOX(optimization_box), area);
    }
  }
  return G_SOURCE_CONTINUE;
}

static void
activate_cb(GtkApplication *app, gpointer user_data)
{ 
  bpf_trace_run();

  GtkBuilder *builder;
  builder = gtk_builder_new_from_file ("kf.ui");

  GObject *window = gtk_builder_get_object (builder, "MainWindow");
  gtk_window_set_application (GTK_WINDOW (window), app);

  GObject *kernel_box = gtk_builder_get_object(builder, "kernel_box");
  GObject *kernel_stack = gtk_builder_get_object(builder, "kernel_stack");

  int count = 0;
  char** kernels_arr;
  kernels_arr = get_kernels_files_arr(&count);
  for (int i = 0; i < count; i++) {
    GtkWidget *kernel_page = gtk_label_new(kernels_arr[i]);
    GtkWidget *kernel_button = gtk_button_new_with_label(g_path_get_basename(kernels_arr[i]));
    gtk_widget_set_margin_top(kernel_button, 2);
    gtk_widget_set_margin_start(kernel_button, 7);
    gtk_widget_set_margin_end(kernel_button, 7);

    char *page_name = g_strdup_printf("page%d", i);
    g_object_set_data(G_OBJECT(kernel_button), "page-name", page_name);
    g_signal_connect(kernel_button, "clicked", G_CALLBACK(on_kernel_button_clicked), kernel_stack);

    gtk_box_append(GTK_BOX(kernel_box), kernel_button);
    adw_view_stack_add_titled(ADW_VIEW_STACK(kernel_stack), kernel_page, page_name, g_path_get_basename(kernels_arr[i]));

    free(kernels_arr[i]);
  }
  free(kernels_arr);
  
  GObject *start_button = gtk_builder_get_object(builder, "start_button");
  BuildDialogData *build_dialog_data = g_new(BuildDialogData, 1);
  build_dialog_data->builder = builder;
  build_dialog_data->window = window;
  g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_button_clicked), build_dialog_data);

  GObject *build_dialog = gtk_builder_get_object(builder, "build_dialog");
  g_signal_connect(build_dialog, "close-attempt", G_CALLBACK(on_build_dialog_close), NULL);

  GObject *optimization_box = gtk_builder_get_object(builder, "optimization_box");
  // struct fs_stat bpf_map[30];
  // if (bpf_get_map(bpf_map, 30) != 0) {
  //   printf("Error getting map");
  // }
  
  // for (int i = 0; i < 30; i++) {
  //   if (strlen(bpf_map[i].fs) > 0) {
  //     char stat_str[64];
  //     snprintf(stat_str, sizeof(stat_str), "%llu", bpf_map[i].stat);
  //     GtkWidget *fs_label = gtk_label_new(bpf_map[i].fs);
  //     GtkWidget *stat_label = gtk_label_new(stat_str);
  //     gtk_box_append(GTK_BOX(optimization_box), fs_label);
  //     gtk_box_append(GTK_BOX(optimization_box), stat_label);
  //   }
  // }
  g_timeout_add(1000, update_optimization_box, optimization_box);

  gtk_widget_set_visible (GTK_WIDGET (window), TRUE);
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
