#include "libnotify/notification.h"
#include <libnotify/notify.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define APP_NAME "Brightness-Notifyd"

int main(void) {
  bool init_success = notify_init(APP_NAME);
  if (!init_success) {
    fprintf(stderr, "libnotify initialization failed");
    exit(EXIT_FAILURE);
  }
  NotifyNotification *notification = notify_notification_new("Summary", "Body", NULL);
  notify_notification_show(notification, NULL);
}
