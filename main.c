#include "libnotify/notification.h"
#include "systemd/sd-event.h"
#include <libnotify/notify.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-device.h>

#define APP_NAME "Brightness-Notifyd"
#define NOTIFICATION_TITLE "Brightness"

#define SUBSYSTEM_BACKLIGHT "backlight"
#define ATTR_BRIGHTNESS "brightness"

#define MAX_BRIGHTNESS 255
#define MAX_BRIGHTNESS_PERCENT_STRING_LEN 5 // 3-digit percent + % + null terminator

#define INCLUDE_MATCH !EXCLUDE_MATCH
#define EXCLUDE_MATCH 0

#define UNUSED(x) (void)(x)

typedef struct brightness_notification {
  char *brightness;
  NotifyNotification *notification;
} brightness_notification;

char *raw_brightness_to_percent(const char *raw_brightness);
char *get_brightness_percent(sd_device *backlight_device);
int event_callback(sd_device_monitor *m, sd_device *device, void *userdata);
sd_device_monitor *setup_udev_device_monitor(void);

char *raw_brightness_to_percent(const char *raw_brightness) {
  float normalized_brightness = (float)strtol(raw_brightness, NULL, 10) / MAX_BRIGHTNESS;
  int brightness_percent = (int)(normalized_brightness * 100);
  char *brightness_percent_str = malloc(MAX_BRIGHTNESS_PERCENT_STRING_LEN * sizeof(char));
  if (brightness_percent_str == NULL) {
    return NULL;
  }
  snprintf(brightness_percent_str, 5, "%d%%", brightness_percent);
  return brightness_percent_str;
}

char *get_brightness_percent(sd_device *backlight_device) {
  const char *raw_brightness = NULL;
  if (sd_device_get_sysattr_value(backlight_device, ATTR_BRIGHTNESS, &raw_brightness) < 0) {
    fprintf(stderr, "Failed to get sysfs attribute %s", ATTR_BRIGHTNESS);
    return NULL;
  }
  char *brightness_percent = raw_brightness_to_percent(raw_brightness);
  return brightness_percent;
}

int event_callback(sd_device_monitor *m, sd_device *device, void *userdata) {
  UNUSED(m);
  char *brightness_percent = get_brightness_percent(device);
  if (brightness_percent == NULL) {
    return -1;
  }
  brightness_notification *notification_data = userdata;
  notify_notification_update(notification_data->notification, "Brightness", brightness_percent, NULL);
  notify_notification_show(notification_data->notification, NULL);
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

int main(void) {
  bool init_success = notify_init(APP_NAME);
  if (!init_success) {
    fprintf(stderr, "libnotify initialization failed");
    exit(EXIT_FAILURE);
  }

  sd_device_monitor *monitor = setup_udev_device_monitor();
  if (monitor == NULL) {
    fprintf(stderr, "Failed to create the device monitor");
    exit(EXIT_FAILURE);
  }

  brightness_notification *notification_data = malloc(sizeof(brightness_notification));
  if (notification_data == NULL) {
    fprintf(stderr, "Out of memory initializing notification data");
    exit(EXIT_FAILURE);
  }

  NotifyNotification *notification = notify_notification_new(NOTIFICATION_TITLE, NULL, NULL);
  if (notification == NULL) {
    fprintf(stderr, "Failed to initialize notification");
    exit(EXIT_FAILURE);
  }

  notification_data -> notification = notification;
  notification_data -> brightness = NULL;

  sd_device_monitor_start(monitor, &event_callback, notification_data);
  sd_event *event_loop = sd_device_monitor_get_event(monitor);
  sd_event_loop(event_loop);

  exit(EXIT_SUCCESS);
}
