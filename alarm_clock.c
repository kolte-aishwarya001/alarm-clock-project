#include <gtk/gtk.h>
#include <windows.h> // For PlaySound in Windows
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h> // For system time

// Define the Alarm structure
struct Alarm {
    int hour;
    int minute;
    int second;
    struct Alarm *next;
};

// Global variables
GtkWidget *time_label, *alarm_label;
const char *sound_path ="C:\\Users\\user5\\Downloads\\alarm_ringtone.wav"; // Replace with your sound file path
struct Alarm *alarm_list = NULL; // Global alarm list

// Function prototypes
void trigger_alarm(struct Alarm *alarm);
void add_alarm(GtkWidget *widget, gpointer data);
void view_alarms(GtkWidget *widget, gpointer data);
void delete_alarm(GtkWidget *widget, gpointer data);
void snooze_alarm(struct Alarm *alarm);
gboolean on_timeout(gpointer data);

// Function to trigger alarm
void trigger_alarm(struct Alarm *alarm) {
    GtkWidget *dialog;
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Alarm!");

    // Play sound
    #ifdef _WIN32
        PlaySound(TEXT(sound_path), NULL, SND_FILENAME | SND_ASYNC);
    #else
        char sound_command[150];
        snprintf(sound_command, sizeof(sound_command), "aplay %s &", sound_path);
        system(sound_command);
    #endif

    // Create a dialog with snooze and dismiss options
    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_NONE, buffer);

    // Create buttons directly
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Snooze", 1);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Dismiss", 2);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == 1) {
        snooze_alarm(alarm);
    } else if (response == 2) {
        delete_alarm(NULL, alarm); // Assuming delete_alarm can handle it
    }
    gtk_widget_destroy(dialog);
}

// Function to add a new alarm
void add_alarm(GtkWidget *widget, gpointer data) {
    GtkWidget **entries = (GtkWidget **)data;
    const char *hour_text = gtk_entry_get_text(GTK_ENTRY(entries[0]));
    const char *minute_text = gtk_entry_get_text(GTK_ENTRY(entries[1]));
    const char *second_text = gtk_entry_get_text(GTK_ENTRY(entries[2]));

    struct Alarm *new_alarm = (struct Alarm *)malloc(sizeof(struct Alarm));
    new_alarm->hour = atoi(hour_text);
    new_alarm->minute = atoi(minute_text);
    new_alarm->second = atoi(second_text);
    new_alarm->next = alarm_list; // Add to the front of the list
    alarm_list = new_alarm;

    // Update the label with new alarm
    char alarm_info[256];
    snprintf(alarm_info, sizeof(alarm_info), "Alarm set at %02d:%02d:%02d", new_alarm->hour, new_alarm->minute, new_alarm->second);
    gtk_label_set_text(GTK_LABEL(alarm_label), alarm_info);

    printf("Alarm added: %02d:%02d:%02d\n", new_alarm->hour, new_alarm->minute, new_alarm->second);
}

// Function to view all alarms
void view_alarms(GtkWidget *widget, gpointer data) {
    struct Alarm *current = alarm_list;
    char alarm_info[256] = "Alarms:\n";

    while (current != NULL) {
        char alarm_str[100];
        snprintf(alarm_str, sizeof(alarm_str), "%02d:%02d:%02d\n", current->hour, current->minute, current->second);
        strcat(alarm_info, alarm_str);
        current = current->next;
    }

    gtk_label_set_text(GTK_LABEL(alarm_label), alarm_info);
}

// Function to delete an alarm
void delete_alarm(GtkWidget *widget, gpointer data) {
    struct Alarm *alarm_to_delete = (struct Alarm *)data;

    if (alarm_list == NULL) return;

    if (alarm_list == alarm_to_delete) {
        alarm_list = alarm_list->next;
        free(alarm_to_delete);
        return;
    }

    struct Alarm *current = alarm_list;
    while (current->next != NULL && current->next != alarm_to_delete) {
        current = current->next;
    }

    if (current->next == NULL) return; // Alarm not found
    current->next = current->next->next;
    free(alarm_to_delete);
}

// Function to snooze the alarm
void snooze_alarm(struct Alarm *alarm) {
    alarm->minute += 5; // Snooze for 5 minutes
    if (alarm->minute >= 60) {
        alarm->minute -= 60;
        alarm->hour += 1;
        if (alarm->hour >= 24) {
            alarm->hour = 0;
        }
    }

    // Update the alarm label
    char alarm_info[256];
    snprintf(alarm_info, sizeof(alarm_info), "Snooze Alarm! Now at %02d:%02d:%02d", alarm->hour, alarm->minute, alarm->second);
    gtk_label_set_text(GTK_LABEL(alarm_label), alarm_info);

    printf("Alarm snoozed: %02d:%02d:%02d\n", alarm->hour, alarm->minute, alarm->second);
}

// Timeout function to periodically check time and trigger alarms
gboolean on_timeout(gpointer data) {
    struct Alarm *current = alarm_list;
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);

    // Update the current time label
    char time_info[100];
    snprintf(time_info, sizeof(time_info), "Current Time: %02d:%02d:%02d", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
    gtk_label_set_text(GTK_LABEL(time_label), time_info);

    // Check if any alarm time matches the system time
    while (current != NULL) {
        if (current->hour == local_time->tm_hour &&
            current->minute == local_time->tm_min &&
            current->second == local_time->tm_sec) {
            trigger_alarm(current); // Trigger the alarm
        }
        current = current->next;
    }

    return TRUE; // Continue calling on_timeout every second
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Alarm Clock");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Layout grid
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Current time label
    time_label = gtk_label_new("Current Time:");
    gtk_grid_attach(GTK_GRID(grid), time_label, 0, 0, 2, 1);

    // Alarm label
    alarm_label = gtk_label_new("Alarm Messages:");
    gtk_grid_attach(GTK_GRID(grid), alarm_label, 0, 1, 2, 1);

    // Alarm time entries
    GtkWidget *hour_entry = gtk_entry_new();
    GtkWidget *minute_entry = gtk_entry_new();
    GtkWidget *second_entry = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Hour"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), hour_entry, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Minute"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), minute_entry, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Second"), 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), second_entry, 1, 4, 1, 1);

    // Buttons for adding, viewing, and deleting alarms
    GtkWidget *add_button = gtk_button_new_with_label("Add Alarm");
    GtkWidget *view_button = gtk_button_new_with_label("View Alarms");
    GtkWidget *delete_button = gtk_button_new_with_label("Delete Alarm");

    gtk_grid_attach(GTK_GRID(grid), add_button, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), view_button, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), delete_button, 0, 7, 2, 1);

    // Connect button signals to corresponding functions
    gpointer add_entry_data[] = {hour_entry, minute_entry, second_entry};
    g_signal_connect(add_button, "clicked", G_CALLBACK(add_alarm), add_entry_data);

    g_signal_connect(view_button, "clicked", G_CALLBACK(view_alarms), NULL);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(delete_alarm), NULL);

    // Set a timeout to update time and check alarms every second
    g_timeout_add(1000, on_timeout, NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}