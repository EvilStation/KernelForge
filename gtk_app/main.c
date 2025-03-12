#include <adwaita.h>

G_GNUC_BEGIN_IGNORE_DEPRECATIONS

typedef struct {
    GtkWidget *main_stack;
    GtkWidget *tracing_data_label;
} TracingPageData;

gboolean 
update_label (gpointer user_data) {
    TracingPageData *data = (TracingPageData *)user_data;
    GHashTable *bpf_map = NULL;
    
    if (bpf_map == NULL || g_hash_table_size(bpf_map) == 0) {
        gtk_label_set_text(GTK_LABEL(data->tracing_data_label), "eBPF map is empty or not available");
        return TRUE;
    }

    gpointer value = g_hash_table_lookup(bpf_map, "fat_readdir");
    
    if (value != NULL) {
        gchar *value_str = g_strdup_printf("%lu", *(uint64_t *)value);
        gtk_label_set_text(GTK_LABEL(data->tracing_data_label), value_str);
        g_free(value_str);
    } else {
        gtk_label_set_text(GTK_LABEL(data->tracing_data_label), "Key 'fat_readdir' not found in map");
    }
    return TRUE;
}

void start_label_update(TracingPageData *data) {
    g_timeout_add(1500, update_label, data);
}

static void
app_activate (GApplication *application) {
    AdwApplication *app = ADW_APPLICATION (application);
    GtkWidget *win = gtk_application_window_new (GTK_APPLICATION (app));  // Явное приведение типа
    gtk_window_set_title (GTK_WINDOW (win), "KernelForge");  // Используем gtk_window_set_title
    gtk_window_set_default_size (GTK_WINDOW (win), 600, 600);

    GtkWidget *main_stack = gtk_stack_new ();
    GtkWidget *sidebar = adw_view_switcher_bar_new ();
    adw_view_switcher_bar_set_stack (ADW_VIEW_SWITCHER_BAR (sidebar), ADW_VIEW_STACK (main_stack));
    GtkWidget *content_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append (GTK_BOX (content_box), sidebar);
    gtk_box_append (GTK_BOX (content_box), main_stack);

    GtkWidget *main_page_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0); 
    gtk_widget_set_hexpand (main_page_box, TRUE);
    gtk_widget_set_vexpand (main_page_box, TRUE);

    GtkWidget *new_kernel_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0); 
    GtkWidget *check_button_kvm = gtk_switch_new ();  // Используем gtk_switch_new
    GtkWidget *check_button_selinux = gtk_switch_new ();  // Используем gtk_switch_new
    GtkWidget *check_button_apparmor = gtk_switch_new ();  // Используем gtk_switch_new
    gtk_box_append (GTK_BOX (new_kernel_box), check_button_kvm);
    gtk_box_append (GTK_BOX (new_kernel_box), check_button_selinux);
    gtk_box_append (GTK_BOX (new_kernel_box), check_button_apparmor);
    
    GtkWidget *tracing_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0); 
    GtkWidget *tracing_label = gtk_label_new ("Статистика:");
    GtkWidget *tracing_data_label = gtk_label_new ("");
    gtk_box_append (GTK_BOX (tracing_box), tracing_label);
    gtk_box_append (GTK_BOX (tracing_box), tracing_data_label);

    adw_view_stack_add_titled (ADW_VIEW_STACK (main_stack), main_page_box, "main_page", "Главная");
    adw_view_stack_add_titled (ADW_VIEW_STACK (main_stack), new_kernel_box, "new_kernel_page", "Новое ядро");
    adw_view_stack_add_titled (ADW_VIEW_STACK (main_stack), tracing_box, "tracing_page", "Трассировка");

    gtk_window_set_child (GTK_WINDOW (win), content_box);
    gtk_window_present (GTK_WINDOW (win));
}

#define APPLICATION_ID "com.KF"

int main (int argc, char **argv) {
    AdwApplication *app;
    int stat;

    app = adw_application_new (APPLICATION_ID, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);

    stat = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return stat;
}