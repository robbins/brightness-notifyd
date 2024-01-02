## Brightness-libnotify
Sends notifications using Libnotify to any running notification daemon that implements the Freedesktop Notifications Specification when the screen brightness changes, displaying the current brightness level.

## Limitations
Only gets the first device in the `backlight` subsystem found by Udev, so it won't display notifications for multiple monitors.

## Compilation (with Nix)
nix build .#default

## Installation (imperatively, with Nix)
nix profile install .#default
