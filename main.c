#include "libnotify/notification.h"
#include <libnotify/notify.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-device.h>

#define APP_NAME "Brightness-Notifyd"
#define SUBSYSTEM_BACKLIGHT "backlight"
#define ATTR_BRIGHTNESS "brightness"

#define MAX_BRIGHTNESS 255

#define INCLUDE_MATCH !EXCLUDE_MATCH
#define EXCLUDE_MATCH 0

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

  const char *raw_brightness = NULL;
  if (sd_device_get_sysattr_value(backlight_device, ATTR_BRIGHTNESS, &raw_brightness) < 0) {
    fprintf(stderr, "Failed to get sysfs attribute %s", ATTR_BRIGHTNESS);
    exit(EXIT_FAILURE);
  }

  float normalized_brightness = (float)strtol(raw_brightness, NULL, 10) / MAX_BRIGHTNESS;

  sd_device_unref(backlight_device);
  sd_device_enumerator_unref(enumerator);

  int brightness_percent = normalized_brightness * 100;
  char brightness_percent_str[5];
  snprintf(brightness_percent_str, 5, "%d%%", brightness_percent);

  NotifyNotification *brightness_notification = notify_notification_new("Brightness", brightness_percent_str, NULL);
  notify_notification_show(brightness_notification, NULL);

  exit(EXIT_SUCCESS);
}
