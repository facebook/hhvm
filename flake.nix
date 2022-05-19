{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    flake-compat.url = "github:edolstra/flake-compat";
    flake-compat.flake = false;
  };
  outputs =
    { self, nixpkgs, flake-utils, flake-compat }:
    flake-utils.lib.eachSystem [
      "x86_64-darwin"
      "x86_64-linux"
    ]
      (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        rec {
          packages.hhvm = pkgs.callPackage ./hhvm.nix {
            lastModifiedDate = self.lastModifiedDate;
          };
          packages.default = packages.hhvm;

          checks.quick = pkgs.runCommand
            "hhvm-quick-test"
            {
              buildInputs = pkgs.lib.optionals pkgs.hostPlatform.isMacOS [
                # `system_cmds` provides `sysctl`, which is used in hphp/test/run.php on macOS
                pkgs.darwin.system_cmds
              ];
            }
            ''
              set -ex
              cd ${./.}
              HHVM_BIN="${packages.hhvm}/bin/hhvm" "${packages.hhvm}/bin/hhvm" hphp/test/run.php quick
              mkdir $out
            '';

          devShells.default =
            pkgs.callPackage "${nixpkgs.outPath}/pkgs/build-support/mkshell/default.nix"
              { stdenv = packages.hhvm.stdenv; }
              {
                inputsFrom = [
                  packages.hhvm
                ];
                packages = [
                  pkgs.rnix-lsp
                ];
                NIX_CFLAGS_COMPILE = packages.hhvm.NIX_CFLAGS_COMPILE;
                CMAKE_INIT_CACHE = packages.hhvm.cmakeInitCache;
              };

        }
      );
}
