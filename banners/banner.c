#include <adwaita.h>

static void on_response(AdwAlertDialog *dialog, const char *response, gpointer user_data) {
    adw_dialog_close(ADW_DIALOG(dialog));

    GtkApplication *app = GTK_APPLICATION(user_data);
    g_application_quit(G_APPLICATION(app));
}

static void on_activate(GtkApplication *app, gpointer user_data) {
    // Создаем временное окно как родитель
    GtkWindow *parent = GTK_WINDOW(gtk_application_window_new(app));
    
    // Создаем диалог (правильное приведение типов)
    AdwAlertDialog *dialog = ADW_ALERT_DIALOG(adw_alert_dialog_new("Сообщение", NULL));
    
    adw_alert_dialog_format_body (dialog, "Обращение к отключенному модулю ФС. Включение. Повторите попытку.");

    // Добавляем кнопки
    adw_alert_dialog_add_response(dialog, "ok", "OK");
    
    // Подключаем обработчик
    g_signal_connect(dialog, "response", G_CALLBACK(on_response), app);
    
    // Показываем диалог (новый метод)
    adw_dialog_present(ADW_DIALOG(dialog), GTK_WIDGET(parent));
    
    // Закрываем родительское окно (диалог останется)
    gtk_window_close(parent);
}

int main(int argc, char **argv) {
    g_autoptr (AdwApplication) app = NULL;
    app = adw_application_new("com.kf.banner", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}