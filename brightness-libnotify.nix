{ lib
, stdenv
, pkgconfig
, libnotify
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "brightness-libnotify";
  version = "git";

  src = ./.;

  nativeBuildInputs = [
    libnotify.dev
  ];

  buildInputs = [
    libnotify
  ];

  buildPhase = ''
    clang main.c
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


