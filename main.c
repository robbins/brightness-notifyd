#include "libnotify/notification.h"
#include "systemd/sd-event.h"
#include <libnotify/notify.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-device.h>

#define APP_NAME "Brightness-Notifyd"
#define SUBSYSTEM_BACKLIGHT "backlight"
#define ATTR_BRIGHTNESS "brightness"

#define MAX_BRIGHTNESS 255
#define MAX_BRIGHTNESS_PERCENT_STRING_LEN 5 // 3-digit percent + % + null terminator

#define INCLUDE_MATCH !EXCLUDE_MATCH
#define EXCLUDE_MATCH 0

typedef struct brightness_notification {
  char *brightness_percent_str;
  NotifyNotification *notification;
} brightness_notification;

char *raw_brightness_to_percent(const char *raw_brightness) {
  float normalized_brightness = (float)strtol(raw_brightness, NULL, 10) / MAX_BRIGHTNESS;
  int brightness_percent = normalized_brightness * 100;
  char *brightness_percent_str = malloc(MAX_BRIGHTNESS_PERCENT_STRING_LEN * sizeof(char));
  snprintf(brightness_percent_str, 5, "%d%%", brightness_percent);
  return brightness_percent_str;
}

sd_device *get_backlight_device(sd_device_enumerator *enumerator) {
  if (sd_device_enumerator_add_match_subsystem(enumerator, SUBSYSTEM_BACKLIGHT, INCLUDE_MATCH) < 0) {
    fprintf(stderr, "Failed to set subsystem filter to %s", SUBSYSTEM_BACKLIGHT);
    exit(EXIT_FAILURE);
  }

  sd_device *backlight_device = NULL;
  backlight_device = sd_device_enumerator_get_device_first(enumerator);
  if (backlight_device == NULL) {
    fprintf(stderr, "Failed to find any udev devices for the given subsystem");
    exit(EXIT_FAILURE);
  }

  return backlight_device;
}

char *get_brightness_percent(sd_device *backlight_device) {
  const char *raw_brightness = NULL;
  if (sd_device_get_sysattr_value(backlight_device, ATTR_BRIGHTNESS, &raw_brightness) < 0) {
    fprintf(stderr, "Failed to get sysfs attribute %s", ATTR_BRIGHTNESS);
    exit(EXIT_FAILURE);
  }
  char *brightness_percent_str = raw_brightness_to_percent(raw_brightness);
  return brightness_percent_str;
}

int event_callback(sd_device_monitor *m, sd_device *device, void *userdata) {
  char *brightness_percent = get_brightness_percent(device);
  printf("Event %s\n", brightness_percent);
  brightness_notification *bn = (brightness_notification*)userdata;
  notify_notification_update(bn->notification, "Brightness", brightness_percent, NULL);
  notify_notification_show(bn->notification, NULL);
  return 0;
}


int main(void) {
  bool init_success = notify_init(APP_NAME);
  if (!init_success) {
    fprintf(stderr, "libnotify initialization failed");
    exit(EXIT_FAILURE);
  }

  sd_device_enumerator *enumerator = NULL;
  if (sd_device_enumerator_new(&enumerator) < 0) {
    fprintf(stderr, "Failed to create the device enumerator");
    exit(EXIT_FAILURE);
  }

  sd_device *backlight_device = get_backlight_device(enumerator);
  char *brightness_percent_str = get_brightness_percent(backlight_device);

  sd_device_unref(backlight_device);
  sd_device_enumerator_unref(enumerator);

  sd_device_monitor *monitor = NULL;
  if (sd_device_monitor_new(&monitor) < 0) {
    fprintf(stderr, "Failed to create the device monitor");
    exit(EXIT_FAILURE);
  }

  sd_device_monitor_filter_add_match_subsystem_devtype(monitor, SUBSYSTEM_BACKLIGHT, NULL);
  sd_device_monitor_filter_update(monitor);
  sd_device_monitor_handler_t callback = &event_callback;
  brightness_notification *bn = malloc(sizeof(brightness_notification));
  bn -> notification = notify_notification_new("Brightness", brightness_percent_str, NULL);
  sd_device_monitor_start(monitor, callback, bn);
  sd_event *event_loop = sd_device_monitor_get_event(monitor);
  sd_event_loop(event_loop);

  exit(EXIT_SUCCESS);
}
