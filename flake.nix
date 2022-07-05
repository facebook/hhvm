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
          packages.default = pkgs.callPackage ./hhvm.nix {
            lastModifiedDate = self.lastModifiedDate;
          };

          devShells.default =
            pkgs.callPackage "${nixpkgs.outPath}/pkgs/build-support/mkshell/default.nix"
              { stdenv = packages.default.stdenv; }
              {
                inputsFrom = [
                  packages.default
                ];
                packages = [
                  pkgs.rnix-lsp
                ];
              };

        }
      );
}
