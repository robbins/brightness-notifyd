{ lib
, stdenv
, pkg-config
, libnotify
, glib
, gdk-pixbuf
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "brightness-libnotify";
  version = "git";

  src = ./.;

  nativeBuildInputs = [
    libnotify.dev
    glib.dev
    gdk-pixbuf.dev
  ];

  buildInputs = [
    libnotify
    pkg-config
  ];

  buildPhase = ''
    clang main.c $(pkg-config --cflags --libs libnotify)
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp a.out $out/bin/brightness-libnotify
  '';

  meta = {
    description = "";
    homepage = "";
    maintainers = with lib.maintainers; [  ];
  };
})


