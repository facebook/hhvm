{ callPackage
, stdenv
, lastModifiedDate
}:
stdenv.mkDerivation (callPackage ./common.nix { inherit lastModifiedDate; })
