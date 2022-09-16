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
, pcre
, perl
, pkg-config
, python3
, re2
, re2c
, rustChannelOf
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
  makeVersion = major: minor: patch: suffix:
    if suffix == "-dev" then "${major}.${minor}.${patch}-dev${lastModifiedDate}" else "${major}.${minor}.${patch}";

  rustNightly = rustChannelOf {

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
      (boost.override { inherit stdenv; })
      brotli
      bzip2
      curl
      (double-conversion.override { inherit stdenv; })
      editline
      expat
      (fmt.override { inherit stdenv; })
      freetype
      fribidi
      gd
      gdb
      gettext
      git
      (
        (glog.override {
          inherit stdenv;
          gflags = gflags.override { inherit stdenv; };
        }).overrideAttrs (finalAttrs: previousAttrs: {
          # Workaround for https://github.com/google/glog/issues/709
          doCheck = !stdenv.cc.isClang;
        })
      )
      gmp
      (gperf.override { inherit stdenv; })
      (gperftools.override { inherit stdenv; })
      (icu.override { inherit stdenv; })
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
      (tbb.override { inherit stdenv; })
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
    ++ lib.optionals stdenv.cc.isClang [
      # Workaround for dtoa.0.3.2
      "-Wno-error=unused-command-line-argument"
    ];

  CMAKE_INIT_CACHE =
    let
      # Use writeTextDir instead of writeTextFile as a workaround of https://github.com/xtruder/nix-devcontainer/issues/9
      dir = writeTextDir "init-cache.cmake"
        ''
          set(CAN_USE_SYSTEM_ZSTD ON CACHE BOOL "Use system zstd" FORCE)
          set(HAVE_SYSTEM_TZDATA_PREFIX "${tzdata}/share/zoneinfo" CACHE PATH "The zoneinfo directory" FORCE)
          set(HAVE_SYSTEM_TZDATA ON CACHE BOOL "Use system zoneinfo" FORCE)
          set(MYSQL_UNIX_SOCK_ADDR "/run/mysqld/mysqld.sock" CACHE FILEPATH "The MySQL unix socket" FORCE)
          set(CARGO_EXECUTABLE "${rustNightly.cargo}/bin/cargo" CACHE FILEPATH "The nightly cargo" FORCE)
          set(RUSTC_EXECUTABLE "${rustNightly.rust}/bin/rustc" CACHE FILEPATH "The nightly rustc" FORCE)
          ${
            lib.optionalString hostPlatform.isMacOS ''
              set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Targeting macOS version" FORCE)
            ''
          }
        '';
    in
    dir + "/init-cache.cmake";

  cmakeFlags = [ "-C" CMAKE_INIT_CACHE ];

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
      set -ex
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
