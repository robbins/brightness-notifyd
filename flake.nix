{
  description = "brightness-libnotify";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs = { self, nixpkgs }: 
  let
      pkgs = import nixpkgs {
        system = "x86_64-linux";
      };
  in {
    packages.x86_64-linux.brightness-libnotify = pkgs.callPackage ./brightness-libnotify.nix { stdenv = pkgs.clangStdenv; };
    packages.x86_64-linux.default = self.packages.x86_64-linux.brightness-libnotify;
    devShells."x86_64-linux".default = pkgs.mkShell {
      packages = with pkgs; [
        clang-tools
        clang
        glib.dev
        gdk-pixbuf.dev
        pkg-config
        libnotify.dev
      ];
    };
  };
}
