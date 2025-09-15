{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default";
    flake-utils = {
      url = "github:numtide/flake-utils";
      inputs.systems.follows = "systems";
    };
  };

  outputs =
    { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.clangStdenv.mkDerivation {
          name = "monitor-wake";
          src = builtins.path {
            name = "monitor-wake";
            path = ./.;
          };

          nativeBuildInputs = [ pkgs.pkg-config ];
          buildInputs = [ pkgs.dbus ];

          phases = [ "buildPhase" ];
          buildPhase = ''
            mkdir -p $out/bin/
            make -C $src OUTPUT_FILE=$out/bin/monitor-wake
          '';
          outputs = [ "out" ];
        };

        devShells.default = pkgs.mkShell {
          nativeBuildInputs = [ pkgs.pkg-config ];
          buildInputs = [ pkgs.dbus ];

          packages = [
            # pkgs.clang-tools should come before pkgs.clang or else clangd can't detect headers
            # https://github.com/NixOS/nixpkgs/issues/76486
            pkgs.clang-tools
            pkgs.clang
            pkgs.gnumake
            pkgs.gdb

            # manpaths dont appear in devshells
            # https://github.com/NixOS/nixpkgs/pull/234367
            # workaround here:
            # https://discourse.nixos.org/t/how-to-get-postgres-man-pages-in-a-devshell/47321/2?u=fqidz
            (pkgs.buildEnv {
              name = "devShell";
              paths = [
                pkgs.man-pages-posix
                pkgs.man-pages
                pkgs.clang-manpages
              ];
            })
          ];
        };
      }
    );
}
