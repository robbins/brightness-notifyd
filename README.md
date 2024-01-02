## Brightness-libnotify
Sends notifications using Libnotify to any running notification daemon that implements the Freedesktop Notifications Specification when the screen brightness changes, displaying the current brightness level.

![image](https://github.com/robbins/brightness-notifyd/assets/31457698/ac0e50bb-61d7-4fbe-9053-edb2fd463ee4)


## Limitations
Only gets the first device in the `backlight` subsystem found by Udev, so it won't display notifications for multiple monitors.

## Compilation (with Nix)
nix build .#default

## Installation (imperatively, with Nix)
nix profile install .#default
