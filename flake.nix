{
  outputs = {
    self,
    nixpkgs,
  }: {
    packages =
      nixpkgs.lib.genAttrs [
        "x86_64-darwin"
        "x86_64-linux"
      ] (
        system: let
          pkgs = import nixpkgs {
            inherit system;
            overlays = [
              (final: prev: {
                # A customized libdwarf.nix is provided because the libdwarf
                # package in nixpkgs-unstable does not support macOS yet.
                libdwarf = final.callPackage ./libdwarf.nix {};
              })
            ];
          };
        in {
          default = pkgs.callPackage ./hhvm.nix {};
        }
      );
  };
}
