#include "libnotify/notification.h"
#include "sys/syslog.h"
#include "systemd/sd-event.h"
#include <libnotify/notify.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-device.h>
#include <systemd/sd-journal.h>
#include <unistd.h>

#define APP_NAME "Brightness-Notifyd"
#define NOTIFICATION_TITLE "Brightness"
#define NOTIFICATION_TIMEOUT_MS 2000
#define HINT_TRANSIENT "transient"
#define HINT_PROGRESS "value"
#define BACKLIGHT_TITLE "Backlight: "
#define BACKLIGHT_TITLE_LEN 11

#define PROPERTY_KERNEL "KERNEL"
#define SUBSYSTEM_BACKLIGHT "backlight"
#define ATTR_BRIGHTNESS "brightness"

#define MAX_BRIGHTNESS 255
#define MAX_BRIGHTNESS_PERCENT_STRING_LEN 5 // 3-digit percent + percent symbol + null terminator

#define INCLUDE_MATCH !EXCLUDE_MATCH
#define EXCLUDE_MATCH 0

#define NOCHDIR 0
#define NOCLOSE 0

#define UNUSED(x) (void)(x)

typedef struct brightness_notification {
  char *brightness;
  NotifyNotification *notification;
} brightness_notification_data;

char *raw_brightness_to_percent_formatted(const char *raw_brightness);
int raw_brightness_to_percent(const char *raw_brightness);
const char *get_raw_brightness(sd_device *backlight_device);
const char *get_kernel(sd_device *backlight_device);
int event_callback(sd_device_monitor *m, sd_device *device, void *userdata);
int raw_brightness_to_percent(const char *raw_brightness);
sd_device_monitor *setup_udev_device_monitor(void);
NotifyNotification *create_brightness_notification(void);
brightness_notification_data *create_notification_data(void);

char *raw_brightness_to_percent_formatted(const char *raw_brightness) {
  char *brightness_percent_str = malloc(MAX_BRIGHTNESS_PERCENT_STRING_LEN * sizeof(char));
  if (brightness_percent_str == NULL) {
    return NULL;
  }
  snprintf(brightness_percent_str, MAX_BRIGHTNESS_PERCENT_STRING_LEN, "%d%%", raw_brightness_to_percent(raw_brightness));
  return brightness_percent_str;
}

int raw_brightness_to_percent(const char *raw_brightness) {
  float normalized_brightness = (float)strtol(raw_brightness, NULL, 10) / MAX_BRIGHTNESS;
  int brightness_percent = (int)(normalized_brightness * 100);
  return brightness_percent;
}

const char *get_raw_brightness(sd_device *backlight_device) {
  const char *raw_brightness = NULL;
  if (sd_device_get_sysattr_value(backlight_device, ATTR_BRIGHTNESS, &raw_brightness) < 0) {
    sd_journal_print(LOG_ERR, "Failed to get sysfs %s attribute", ATTR_BRIGHTNESS);
    return NULL;
  }
  return raw_brightness;
}

const char *get_kernel(sd_device *backlight_device) {
  const char *kernel = NULL;
  if (sd_device_get_sysname(backlight_device, &kernel) < 0) {
    sd_journal_print(LOG_ERR, "Failed to get sysname");
    return NULL;
  }
  return kernel;
}

int event_callback(sd_device_monitor *m, sd_device *device, void *userdata) {
  UNUSED(m);

  const char *raw_brightness = get_raw_brightness(device);
  char *body_brightness_percent = raw_brightness_to_percent_formatted(raw_brightness);
  if (body_brightness_percent == NULL) {
    return -1;
  }

  const char *kernel = get_kernel(device);
  char summary[BUFSIZ] = BACKLIGHT_TITLE;
  strncat(summary, kernel, BUFSIZ - BACKLIGHT_TITLE_LEN - 1);

  brightness_notification_data *notification_data = userdata;
  notify_notification_update(notification_data->notification, summary, body_brightness_percent, NULL);
  notify_notification_set_hint(notification_data->notification, HINT_PROGRESS, g_variant_new_int32(raw_brightness_to_percent(raw_brightness)));
  notify_notification_show(notification_data->notification, NULL);

  free(body_brightness_percent);
  return 0;
}

sd_device_monitor *setup_udev_device_monitor(void) {
  sd_device_monitor *monitor = NULL;
  if (sd_device_monitor_new(&monitor) < 0) {
    return NULL;
  }

  if (sd_device_monitor_filter_add_match_subsystem_devtype(monitor, SUBSYSTEM_BACKLIGHT, NULL) < 0) {
    return NULL;
  }

  if (sd_device_monitor_filter_update(monitor) < 0) {
    return NULL;
  }

  return monitor;
}

NotifyNotification *create_brightness_notification(void) {
  NotifyNotification *notification = notify_notification_new(NOTIFICATION_TITLE, NULL, NULL);
  notify_notification_set_timeout(notification, NOTIFICATION_TIMEOUT_MS);
  notify_notification_set_hint(notification, HINT_TRANSIENT, g_variant_new_boolean(TRUE));
  return notification;
}

brightness_notification_data *create_notification_data(void) {
  NotifyNotification *notification = create_brightness_notification();

  brightness_notification_data *notification_data = malloc(sizeof(brightness_notification_data));
  if (notification_data == NULL) {
    return NULL;
  }

  notification_data->notification = notification;
  notification_data->brightness = NULL;
  return notification_data;
}

int main(int argc, char *argv[]) {
  UNUSED(argv);

  if (daemon(NOCHDIR, NOCLOSE) < 0) {
    sd_journal_perror("daemon:");
  }

  if (argc != 1) {
    sd_journal_print(LOG_CRIT, "Brightness-Notifyd doesn't take any parameters");
    exit(EXIT_FAILURE);
  }

  bool init_success = notify_init(APP_NAME);
  if (!init_success) {
    sd_journal_print(LOG_ERR, "libnotify initialization failed");
    exit(EXIT_FAILURE);
  }

  sd_device_monitor *monitor = setup_udev_device_monitor();
  if (monitor == NULL) {
    sd_journal_print(LOG_ERR, "Failed to create the udev device monitor");
    exit(EXIT_FAILURE);
  }

  brightness_notification_data *notification_data = create_notification_data();
  if (notification_data == NULL) {
    sd_journal_print(LOG_ERR, "Out of memory initializing notification data");
    exit(EXIT_FAILURE);
  }
  if (notification_data->notification == NULL) {
    sd_journal_print(LOG_ERR, "Failed to create a notification");
    exit(EXIT_FAILURE);
  }

  sd_device_monitor_start(monitor, &event_callback, notification_data);
  sd_event *event_loop = sd_device_monitor_get_event(monitor);
  sd_event_set_signal_exit(event_loop, TRUE);
  sd_event_loop(event_loop);

  sd_device_monitor_stop(monitor);
  sd_device_monitor_unref(monitor);
  g_object_unref(notification_data->notification);
  free(notification_data);
  notify_uninit();

  exit(EXIT_SUCCESS);
}
