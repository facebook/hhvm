{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs =
    { self, nixpkgs, flake-utils }:
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
              (final: prev: {
                # A customized libdwarf.nix is provided because the libdwarf
                # package in nixpkgs-unstable does not support macOS yet.
                libdwarf = final.callPackage ./libdwarf.nix { };
              })
            ];
          };
        in
        rec {
          packages.default = pkgs.callPackage ./hhvm.nix { };

          devShells.default = pkgs.mkShell {
            buildInputs = packages.default.nativeBuildInputs ++ packages.default.buildInputs ++ [
              pkgs.rnix-lsp
            ];
          };
        }
      );
}
