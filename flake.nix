{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    flake-compat.url = "github:edolstra/flake-compat";
    flake-compat.flake = false;
    nixpkgs-mozilla.url = "github:mozilla/nixpkgs-mozilla";
  };
  outputs =
    { self, nixpkgs, flake-utils, flake-compat, nixpkgs-mozilla }:
    flake-utils.lib.eachSystem [
      "x86_64-darwin"
      "x86_64-linux"
    ]
      (
        system:
        let
          pkgs = import nixpkgs {
            inherit system;
            overlays = [
              nixpkgs-mozilla.overlays.rust
            ];
            config.permittedInsecurePackages = [
              # It's OK to depend on libdwarf 20210528, because we did not call
              # the particular vulnerable function in libdwarf
              "libdwarf-20210528"
            ];
          };
          devShellForPackage = hhvm:
            pkgs.mkShell.override
              { stdenv = hhvm.stdenv; }
              {
                inputsFrom = [
                  hhvm
                ];
                packages = [
                  pkgs.rnix-lsp
                  pkgs.fpm
                  pkgs.rpm
                ];
                inherit (hhvm)
                  NIX_CFLAGS_COMPILE
                  CMAKE_INIT_CACHE
                  CCACHE_DIR
                  CCACHE_UMASK;
              };
        in
        rec {
          packages.hhvm = pkgs.callPackage ./hhvm.nix {
            lastModifiedDate = self.lastModifiedDate;
            isDefaultStdlib = true;
          };
          packages.hhvm_ccache = (packages.hhvm.override {
            stdenv = pkgs.ccacheStdenv;
          }).overrideAttrs (finalAttrs: previousAttrs: {
            CCACHE_DIR = "/nix/var/cache/ccache";
            CCACHE_UMASK = 007;
          });
          packages.hhvm_clang = packages.hhvm.override {
            stdenv = pkgs.llvmPackages_14.stdenv;
          };
          packages.default = packages.hhvm;

          devShells.clang = devShellForPackage packages.hhvm_clang;
          devShells.default = devShellForPackage packages.hhvm_ccache;

          ${if pkgs.hostPlatform.isLinux then "bundlers" else null} =
            let
              fpmScript =
                outputType: pkg:
                ''
                  # Copy to a temporary directory as a workaround to https://github.com/jordansissel/fpm/issues/807
                  while read LINE
                  do
                    mkdir -p "$(dirname "./$LINE")"
                    cp -r "/$LINE" "./$LINE"
                    chmod --recursive u+w "./$LINE"
                    FPM_INPUTS+=("./$LINE")
                  done < ${pkgs.lib.strings.escapeShellArg (pkgs.referencesByPopularity pkg)}

                  ${pkgs.lib.strings.escapeShellArg pkgs.fpm}/bin/fpm \
                    --verbose \
                    --package "$out" \
                    --input-type dir \
                    --output-type ${outputType} \
                    --name ${pkgs.lib.strings.escapeShellArg pkg.pname} \
                    --version ${
                      pkgs.lib.strings.escapeShellArg
                        (builtins.replaceStrings ["-"] ["~"] pkg.version)
                    } \
                    --description ${pkgs.lib.strings.escapeShellArg pkg.meta.description} \
                    --url ${pkgs.lib.strings.escapeShellArg pkg.meta.homepage} \
                    --maintainer ${pkgs.lib.strings.escapeShellArg (pkgs.lib.strings.concatStringsSep ", " (map ({name, email, ...}: "\"${name}\" <${email}>") pkg.meta.maintainers))} \
                    --license ${pkgs.lib.strings.escapeShellArg (pkgs.lib.strings.concatStringsSep " AND " (map ({spdxId, ...}: spdxId) (pkgs.lib.lists.toList pkg.meta.license)))} \
                    --after-install ${
                      pkgs.writeScript "after-install.sh" ''
                        for EXECUTABLE in ${pkgs.lib.strings.escapeShellArg pkg}/bin/*
                        do
                          NAME=$(basename "$EXECUTABLE")
                          update-alternatives --install "/usr/bin/$NAME" "$NAME" "$EXECUTABLE" 1
                        done
                      ''
                    } \
                    --before-remove ${
                      pkgs.writeScript "before-remove.sh" ''
                        for EXECUTABLE in ${pkgs.lib.strings.escapeShellArg pkg}/bin/*
                        do
                          NAME=$(basename "$EXECUTABLE")
                          update-alternatives --remove "$NAME" "$EXECUTABLE"
                        done
                      ''
                    } \
                    -- \
                    "''${FPM_INPUTS[@]}"
                '';
            in
            {
              rpm = pkg: pkgs.runCommand
                "bundle.rpm"
                { nativeBuildInputs = [ pkgs.rpm ]; }
                (fpmScript "rpm" pkg);
              deb = pkg: pkgs.runCommand
                "bundle.deb"
                { nativeBuildInputs = [ pkg.stdenv.cc ]; }
                (fpmScript "deb" pkg);
            };

        }
      );
}
