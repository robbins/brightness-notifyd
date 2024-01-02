{ lib
, stdenv
, pkg-config
, libnotify
, glib
, gdk-pixbuf
, systemd
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "brightness-libnotify";
  version = "git";

  src = ./.;

  nativeBuildInputs = [
    libnotify.dev
    glib.dev
    gdk-pixbuf.dev
    systemd.dev
  ];

  buildInputs = [
    libnotify
    pkg-config
  ];

  buildPhase = ''
    clang -Wall -Wextra -Wfloat-equal -Wundef -Werror -fverbose-asm -Wint-to-pointer-cast -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wcast-qual -Wmissing-prototypes -Wstrict-overflow=5 -Wwrite-strings -Wconversion --pedantic-errors main.c $(pkg-config --cflags --libs libnotify) $(pkg-config --cflags --libs libsystemd) $(pkg-config --cflags --libs libsystemd-journal)
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp a.out $out/bin/brightness-libnotify
  '';

  meta = {
    description = "A simple program to send notifications when screen brightness changes";
  };
})


