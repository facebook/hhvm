{ bison
, boost
, brotli
, bzip2
, cacert
, cmake
, curl
, darwin
, double-conversion
, editline
, expat
, flex
, fmt
, freetype
, fribidi
, gd
, gdb
, gettext
, git
, glog
, gmp
, gperf
, gperftools
, hostPlatform
, icu
, imagemagick6
, jemalloc
, lastModifiedDate
, lib
, libcap
, libdwarf
, libedit
, libelf
, libevent
, libgccjit
, libkrb5
, libmcrypt
, libmemcached
, libpng
, libsodium
, libunwind
, libvpx
, libxml2
, libxslt
, libzip
, linux-pam
, lz4
, numactl
, oniguruma
, openldap
, openssl
, patchelf
, pcre
, perl
, pkg-config
, python3
, re2
, re2c
, stdenv
, sqlite
, tbb
, tzdata
, unixtools
, unzip
, uwimap
, which
, writeTextDir
, zlib
, zstd
}:
let
  versionParts =
    builtins.match
      ''
        .*
        #[[:blank:]]*define[[:blank:]]+HHVM_VERSION_MAJOR[[:blank:]]+([[:digit:]]+)
        #[[:blank:]]*define[[:blank:]]+HHVM_VERSION_MINOR[[:blank:]]+([[:digit:]]+)
        #[[:blank:]]*define[[:blank:]]+HHVM_VERSION_PATCH[[:blank:]]+([[:digit:]]+)
        #[[:blank:]]*define[[:blank:]]+HHVM_VERSION_SUFFIX[[:blank:]]+"([^"]*)"
        .*
      ''
      (builtins.readFile ./hphp/runtime/version.h);
  makePName = major: minor: patch: suffix:
    if suffix == "-dev" then "hhvm_nightly" else "hhvm";
  makeVersion = major: minor: patch: suffix:
    if suffix == "-dev" then "${major}.${minor}.${patch}-${lastModifiedDate}" else "${major}.${minor}.${patch}";
in
stdenv.mkDerivation rec {
  pname = builtins.foldl' lib.trivial.id makePName versionParts;
  version = builtins.foldl' lib.trivial.id makeVersion versionParts;
  src = ./.;
  nativeBuildInputs =
    [
      bison
      cacert
      cmake
      flex
      patchelf
      pkg-config
      python3
      unixtools.getconf
      which
    ];
  buildInputs =
    [
      boost
      brotli
      bzip2
      curl
      double-conversion
      editline
      expat
      fmt
      freetype
      fribidi
      gd
      gdb
      gettext
      git
      glog
      gmp
      gperf
      gperftools
      icu
      imagemagick6
      jemalloc
      libdwarf
      libedit
      libelf
      libevent
      libgccjit
      libkrb5
      libmcrypt
      libmemcached
      libpng
      libsodium
      libunwind
      libvpx
      libxml2
      libxslt
      libzip
      lz4
      oniguruma
      openldap
      openssl
      pcre
      perl
      re2
      re2c
      sqlite
      tbb
      tzdata
      unzip
      zlib
      zstd
    ]
    ++ lib.optionals hostPlatform.isLinux [
      libcap
      linux-pam
      numactl
      uwimap
    ]
    ++ lib.optionals hostPlatform.isMacOS [
      darwin.apple_sdk.frameworks.CoreFoundation
      darwin.apple_sdk.frameworks.CoreServices
    ];

  NIX_CFLAGS_COMPILE =
    [
      "-DFOLLY_MOBILE=0"
    ]
    ++ lib.optionals hostPlatform.isMacOS [
      # Workaround for dtoa.0.3.2
      "-Wno-error=unused-command-line-argument"
    ];

  cmakeInitCache =
    let
      # Use writeTextDir instead of writeTextFile as a workaround of https://github.com/xtruder/nix-devcontainer/issues/9
      dir = writeTextDir "init-cache.cmake"
        ''
          set(CAN_USE_SYSTEM_ZSTD ON CACHE BOOL "Use system zstd" FORCE)
          set(HAVE_SYSTEM_TZDATA_PREFIX "${tzdata}/share/zoneinfo" CACHE STRING "The zoneinfo directory" FORCE)
          set(HAVE_SYSTEM_TZDATA ON CACHE BOOL "Use system zoneinfo" FORCE)
          set(MYSQL_UNIX_SOCK_ADDR "/run/mysqld/mysqld.sock" CACHE STRING "The MySQL unix socket" FORCE)
          ${
            lib.optionalString hostPlatform.isMacOS ''
              set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Targeting macOS version" FORCE)
            ''
          }
        '';
    in
    dir + "/init-cache.cmake";

  cmakeFlags = [ "-C" cmakeInitCache ];

  prePatch = ''
    patchShebangs .
  '';

  preBuild =
    ''
      set -e
      make \
        -f third-party/proxygen/CMakeFiles/bundled_proxygen.dir/build.make \
        third-party/proxygen/bundled_proxygen-prefix/src/bundled_proxygen-stamp/bundled_proxygen-patch
      patchShebangs \
        third-party/proxygen/bundled_proxygen-prefix/src/bundled_proxygen

      make \
        -f third-party/rustc/CMakeFiles/bundled_rust.dir/build.make \
        third-party/rustc/bundled_rust-prefix/src/bundled_rust-stamp/bundled_rust-patch
      patchShebangs \
        third-party/rustc/bundled_rust-prefix/src/bundled_rust
    ''
    # Prebuilt rustc and cargo needs patch if HHVM is built either
    # on NixOS or in a Nix sandbox
    + lib.optionalString hostPlatform.isLinux ''
      make \
        -f third-party/rustc/CMakeFiles/bundled_rust.dir/build.make \
        third-party/rustc/bundled_rust-prefix/src/bundled_rust-stamp/bundled_rust-install
      patchelf \
        --set-interpreter ${stdenv.cc.bintools.dynamicLinker} \
        --add-needed ${zlib}/lib/libz.so.1 \
        --add-rpath "${lib.makeLibraryPath [zlib stdenv.cc.cc.lib]}" \
        third-party/rustc/bundled_rust-prefix/bin/rustc
      patchelf \
        --set-interpreter ${stdenv.cc.bintools.dynamicLinker} \
        --add-needed ${zlib}/lib/libz.so.1 \
        --add-rpath "${lib.makeLibraryPath [zlib stdenv.cc.cc.lib]}" \
        third-party/rustc/bundled_rust-prefix/bin/cargo
      patchelf \
        --set-interpreter ${stdenv.cc.bintools.dynamicLinker} \
        --add-needed ${zlib}/lib/libz.so.1 \
        --add-rpath "${lib.makeLibraryPath [zlib stdenv.cc.cc.lib]}" \
        third-party/rustc/bundled_rust-prefix/bin/rustfmt
    '';

  meta = {
    description = "High-performance JIT compiler for PHP/Hack";
    platforms = [
      "x86_64-darwin"
      "x86_64-linux"
    ];
    homepage = "https://hhvm.com";
    license = "PHP/Zend";
  };
}
