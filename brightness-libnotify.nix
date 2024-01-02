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
    clang main.c $(pkg-config --cflags --libs libnotify) $(pkg-config --cflags --libs libsystemd) -g -Og -ggdb
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp a.out $out/bin/brightness-libnotify
  '';

  dontStrip = true;

  meta = {
    description = "";
    homepage = "";
    maintainers = with lib.maintainers; [  ];
  };
})


