{ callPackage
, llvmPackages_14
}:
lastModifiedDate:
let
  common = callPackage ./common.nix { } lastModifiedDate;
in
llvmPackages_14.libcxxStdenv.mkDerivation common
