{ callPackage
, llvmPackages_12
}:
lastModifiedDate:
let
  common = callPackage ./common.nix { } lastModifiedDate;
in
llvmPackages_12.libcxxStdenv.mkDerivation (common // {
  NIX_CFLAGS_COMPILE = common.NIX_CFLAGS_COMPILE ++ [
    # Workaround for dtoa.0.3.2
    "-Wno-error=unused-command-line-argument"
  ];
})
