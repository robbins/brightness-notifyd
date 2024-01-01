## Brightness-libnotify

## Clangd LSP
bear -- clang main.c $(pkg-config --cflags --libs libnotify)

## Compilation (with Nix)
nix build .#default
