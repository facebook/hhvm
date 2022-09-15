{ callPackage
, stdenv
}:
lastModifiedDate:
let
  common = callPackage ./common.nix { } lastModifiedDate;
in
stdenv.mkDerivation (common // {
  # Current HHVM' CMake build files will access internet, while Nix by
  # default would build a package in a sandbox that prevents internet access.
  #
  # Therefore, HHVM must be built either as an impure derivation, or without
  # a sandbox.
  __impure = true;
})
