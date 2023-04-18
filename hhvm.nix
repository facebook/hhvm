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
, fmt_8
, freetype
, fribidi
, gcc-unwrapped
, gd
, gdb
, gettext
, gflags
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
, libdwarf_20210528
, libedit
, libelf
, libevent
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
, openssl_1_1
, pcre
, perl
, pkg-config
, python3
, re2
, re2c
, rustChannelOf
, setupCompilerCache
, stdenv
, sqlite
, tbb
, tzdata
, unixtools
, unzip
, uwimap
, which
, writeTextFile
, zlib
, zstd
}:
let
  # TODO(https://github.com/NixOS/nixpkgs/pull/193086): Use stdenv.cc.libcxx once it is available
  isDefaultStdlib =
    builtins.match ".*-stdlib=\+\+.*" (builtins.readFile "${stdenv.cc}/nix-support/libcxx-ldflags") == null;
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
  makeVersion = major: minor: patch: suffix:
    if suffix == "-dev" then "${major}.${minor}.${patch}-dev${lastModifiedDate}" else "${major}.${minor}.${patch}";

  rustChannel = rustChannelOf {

    # When the date attribute changes, sha256 should be updated accordingly.
    #
    # 1. Export your diff to GitHub;
    # 2. Wait for an error message about sha256 mismatch from the GitHub
    #    Actions;
    # 3. Copy the new sha256 from the error message and paste it here;
    # 4. Submit the diff and export the diff to GitHub, again.
    # 5. Ensure no error message about sha256 mismatch from the GitHub Actions.
    sha256 = "wVnIzrnpYGqiCBtc3k55tw4VW8YLA3WZY0mSac+2yl0=";

    date = "2022-08-11";
    channel = "nightly";
  };
in
stdenv.mkDerivation rec {
  rust = rustChannel.rust;
  pname = "hhvm";
  version = builtins.foldl' lib.trivial.id makeVersion versionParts;
  src = ./.;
  nativeBuildInputs =
    [
      bison
      cacert
      cmake
      flex
      pkg-config
      python3
      unixtools.getconf
      which
    ] ++ lib.optionals hostPlatform.isMacOS [
      # `system_cmds` provides `sysctl`, which is used in hphp/test/run.php on macOS
      darwin.system_cmds
    ];
  buildInputs =
    [
      (if isDefaultStdlib then boost else boost.override { inherit stdenv; })
      brotli
      bzip2
      (curl.override { openssl = openssl_1_1; })
      (
        if isDefaultStdlib then
          double-conversion
        else
          double-conversion.override { inherit stdenv; }
      )
      editline
      expat
      (if isDefaultStdlib then fmt_8 else fmt_8.override { inherit stdenv; })
      freetype
      fribidi
      # Workaround for https://github.com/NixOS/nixpkgs/issues/192665
      gcc-unwrapped.lib
      gd
      gdb
      gettext
      git
      (
        if isDefaultStdlib then
          glog
        else
          (glog.override {
            inherit stdenv;
            gflags = gflags.override { inherit stdenv; };
          }).overrideAttrs
            (finalAttrs: previousAttrs: {
              # Workaround for https://github.com/google/glog/issues/709
              doCheck = !stdenv.cc.isClang;
            })
      )
      gmp
      (if isDefaultStdlib then gperf else gperf.override { inherit stdenv; })
      (
        if isDefaultStdlib then
          gperftools
        else
          gperftools.override { inherit stdenv; }
      )
      (if isDefaultStdlib then icu else icu.override { inherit stdenv; })
      imagemagick6
      jemalloc
      libdwarf_20210528
      libedit
      libelf
      libevent
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
      openssl_1_1
      pcre
      perl
      re2
      re2c
      sqlite
      (if isDefaultStdlib then tbb else tbb.override { inherit stdenv; })
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
    lib.optionals stdenv.cc.isClang [
      # Workaround for dtoa.0.3.2
      "-Wno-error=unused-command-line-argument"
    ];

  CMAKE_TOOLCHAIN_FILE = writeTextFile {
    name = "toolchain.cmake";
    text = ''
      set(ENABLE_SYSTEM_LOCALE_ARCHIVE ON CACHE BOOL "Use system locale archive as the default LOCALE_ARCHIVE for nix patched glibc" FORCE)
      set(CAN_USE_SYSTEM_ZSTD ON CACHE BOOL "Use system zstd" FORCE)
      set(HAVE_SYSTEM_TZDATA_PREFIX "${tzdata}/share/zoneinfo" CACHE PATH "The zoneinfo directory" FORCE)
      set(HAVE_SYSTEM_TZDATA ON CACHE BOOL "Use system zoneinfo" FORCE)
      set(MYSQL_UNIX_SOCK_ADDR "/run/mysqld/mysqld.sock" CACHE FILEPATH "The MySQL unix socket" FORCE)
      set(CARGO_EXECUTABLE "${rust}/bin/cargo" CACHE FILEPATH "The nightly cargo" FORCE)
      set(RUSTC_EXECUTABLE "${rust}/bin/rustc" CACHE FILEPATH "The nightly rustc" FORCE)
      set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Enable verbose output from Makefile builds" FORCE)
      ${
        lib.optionalString hostPlatform.isMacOS ''
          set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Targeting macOS version" FORCE)
        ''
      }
    '';
  };

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
    '';

  doCheck = true;

  checkPhase =
    ''
      set -e
      runHook preCheck
      export HHVM_BIN="$PWD/hphp/hhvm/hhvm"
      (cd ${./.} && "$HHVM_BIN" hphp/test/run.php quick)
      runHook postCheck
    '';

  meta = {
    description = "High-performance JIT compiler for PHP/Hack";
    platforms = [
      "x86_64-darwin"
      "x86_64-linux"
    ];
    homepage = "https://hhvm.com";
    license = [
      lib.licenses.php301
      {
        spdxId = "Zend-2.0";
        fullName = "Zend License v2.0";
        url = "https://www.zend.com/sites/zend/files/pdfs/2_00.txt";
      }
    ];
    maintainers = [{
      email = "hhvm-oss@fb.com";
      github = "hhvm";
      githubId = 4553654;
      name = "HHVM/Hack Open Source";
    }];
  };
}
