#!/bin/bash
# Build HHVM from its public GitHub source tree on Linux.
#
# Downloads and builds every non-system dependency from public internet
# sources, configures HHVM with CMake, and builds the selected target.
# Safe to re-run: completed phases are reused unless --rebuild is specified.
#
# Usage: ./build/oss_build.sh [--rebuild] [--clean] [--make]
#   --rebuild: force cmake reconfigure and rebuild (keeps deps)
#   --clean:   delete everything and start from scratch
#   --make:    skip all setup phases, just run make (for iterating on fixes)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
HHVM_OSS_WORK_ROOT="${HHVM_OSS_WORK_ROOT:-$HOME/hhvm-oss}"
HOST_OS="$(uname -s)"

if [ "$HOST_OS" != "Linux" ]; then
  echo "ERROR: build/oss_build.sh only supports Linux."
  exit 1
fi

JOBS="$(nproc 2>/dev/null || echo 4)"
BUILD_DIR="${HHVM_OSS_BUILD_DIR:-$HHVM_OSS_WORK_ROOT/hhvm}"
HHVM_CMAKE_BUILD_TYPE="${HHVM_OSS_CMAKE_BUILD_TYPE:-Release}"
HHVM_BUILD_TARGET="${HHVM_OSS_BUILD_TARGET:-hhvm}"
RUN_LOG_FILE="${HHVM_OSS_LOG_FILE:-$HHVM_OSS_WORK_ROOT/oss_build.log}"
GETDEPS_REAL_ROOT="$SRC_DIR/build/fbcode_builder"
GETDEPS="$GETDEPS_REAL_ROOT/getdeps.py"
SCRATCH_DIR="${HHVM_OSS_SCRATCH_DIR:-$HHVM_OSS_WORK_ROOT/getdeps}"
GETDEPS_SHADOW_ROOT="$SCRATCH_DIR/fbcode_builder-hhvm"
GETDEPS_SHADOW_MANIFESTS_DIR="$GETDEPS_SHADOW_ROOT/manifests"
GETDEPS_DIR="$SCRATCH_DIR/installed"
GETDEPS_PUBLIC_CWD="${HHVM_OSS_GETDEPS_CWD:-${XDG_CACHE_HOME:-$HOME/.cache}/hhvm-getdeps-cwd}"
GETDEPS_SOURCE_MODE_FILE="$SCRATCH_DIR/.hhvm_getdeps_source_mode"
GETDEPS_SOURCE_MODE_BASE="public-git-shipit-sources-shadow-manifests-hhvm-toolchain-v3"
GETDEPS_SOURCE_MODE="$GETDEPS_SOURCE_MODE_BASE"
GETDEPS_MANIFEST_OVERRIDE_DIR="$SCRATCH_DIR/manifest-overrides"
GETDEPS_GNU_URL_ROOT="https://ftpmirror.gnu.org/gnu"
GETDEPS_GNU_MIRROR_URL_ROOT="https://mirrors.kernel.org/gnu"
FORCE_REBUILD=false
MAKE_ONLY=false
DOWNLOAD_CACHE_DIR="$HHVM_OSS_WORK_ROOT/downloads"
THIRD_PARTY_SOURCE_CACHE_PREFIX="file://$DOWNLOAD_CACHE_DIR/"
FBMYSQL_OSS_BRANCH="fb-mysql-8.0.32"
FBMYSQL_OSS_URL="https://github.com/facebook/mysql-5.6/archive/refs/heads/${FBMYSQL_OSS_BRANCH}.tar.gz"
FBMYSQL_OSS_ARCHIVE="facebook-mysql-5.6-${FBMYSQL_OSS_BRANCH}.tar.gz"
FBMYSQL_OSS_SHA256="98d6114b36ce0d080040c94534926600224937e617b516922ede6c4e9fcb8051"
FBMYSQL_OSS_CACHE_DIR="$DOWNLOAD_CACHE_DIR"
FBMYSQL_OSS_SRC_DIR="$HHVM_OSS_WORK_ROOT/sources/fb-mysql"
FBMYSQL_OSS_BOOST_PACKAGE="boost_1_77_0"
FBMYSQL_OSS_BOOST_URL="https://archives.boost.io/release/1.77.0/source/${FBMYSQL_OSS_BOOST_PACKAGE}.tar.bz2"
FBMYSQL_OSS_BOOST_ARCHIVE="${FBMYSQL_OSS_BOOST_PACKAGE}.tar.bz2"
FBMYSQL_OSS_BOOST_SHA256="fc9f85fc030e233142908241af7a846e60630aa7388de9a5fafb1f3a26840854"
LIBDWARF_OSS_VERSION="20210528"
LIBDWARF_OSS_URL="https://www.prevanders.net/libdwarf-${LIBDWARF_OSS_VERSION}.tar.gz"
LIBDWARF_OSS_ARCHIVE="libdwarf-${LIBDWARF_OSS_VERSION}.tar.gz"
LIBDWARF_OSS_SHA256="b8ba0ee9b70d2052d45272489d79bf456c4d342fc8c3bba45038afc50ec6e28b"
LIBDWARF_OSS_ARCHIVE_PATH="$DOWNLOAD_CACHE_DIR/$LIBDWARF_OSS_ARCHIVE"
LIBDWARF_OSS_CACHE_DIR="$DOWNLOAD_CACHE_DIR"
LIBDWARF_OSS_SRC_DIR="$HHVM_OSS_WORK_ROOT/sources/libdwarf-${LIBDWARF_OSS_VERSION}"
LIBDWARF_OSS_PREFIX="$HHVM_OSS_WORK_ROOT/installed/libdwarf-${LIBDWARF_OSS_VERSION}"
TIMELIB_VERSION="2021.07"
TIMELIB_URL="https://github.com/derickr/timelib/archive/refs/tags/${TIMELIB_VERSION}.tar.gz"
TIMELIB_ARCHIVE="timelib-${TIMELIB_VERSION}.tar.gz"
TIMELIB_SHA512="7bc56d20360937af10f63960e443cc8bd4d24c5369f697241e54da21465d4512bd16cfa6f0efcf2b847cc19781e1cecf93c9e19a1efa4f1a7012c9fa442eeabe"
TIMELIB_CACHE_DIR="$HHVM_OSS_WORK_ROOT/downloads/timelib"
LIBURING_REVISION="liburing-2.15"
LIBURING_VERSION="2.15"
LIBURING_URL="https://github.com/axboe/liburing/archive/refs/tags/${LIBURING_REVISION}.tar.gz"
LIBURING_ARCHIVE="liburing-${LIBURING_VERSION}.tar.gz"
LIBURING_SHA256="8d052f2622dcb3678cbaee5ff582a87572672a6c0a56533cdda5b65cb636120a"
LIBURING_SRC_DIR="$HHVM_OSS_WORK_ROOT/sources/liburing"
LIBURING_PREFIX="$HHVM_OSS_WORK_ROOT/installed/liburing"
LIBURING_REVISION_STAMP="$LIBURING_PREFIX/.hhvm-revision"
PCRE1_VERSION="8.45"
PCRE1_URL="https://downloads.sourceforge.net/project/pcre/pcre/${PCRE1_VERSION}/pcre-${PCRE1_VERSION}.tar.gz"
PCRE1_ARCHIVE="pcre-${PCRE1_VERSION}.tar.gz"
PCRE1_SHA256="4e6ce03e0336e8b4a3d6c2b70b1c5e18590a5673a98186da90d4f33c23defc09"
PCRE1_SRC_DIR="$HHVM_OSS_WORK_ROOT/sources/pcre"
PCRE1_PREFIX="$HHVM_OSS_WORK_ROOT/installed/pcre"
PCRE1_JIT_STAMP="$PCRE1_PREFIX/.hhvm-jit-enabled"
export PKG_CONFIG_PATH="$PCRE1_PREFIX/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
IMAGEMAGICK6_VERSION="6.9.13-33"
IMAGEMAGICK6_ABI_VERSION="6.9.13"
IMAGEMAGICK6_URL="https://github.com/ImageMagick/ImageMagick6/archive/refs/tags/${IMAGEMAGICK6_VERSION}.tar.gz"
IMAGEMAGICK6_ARCHIVE="ImageMagick6-${IMAGEMAGICK6_VERSION}.tar.gz"
IMAGEMAGICK6_SHA256="a1a3753b616b90d342cff028eb9ab8a1fd24a65e9dec5adca8af569e3d0c5759"
IMAGEMAGICK6_SRC_DIR="$HHVM_OSS_WORK_ROOT/sources/ImageMagick6"
IMAGEMAGICK6_PREFIX="$HHVM_OSS_WORK_ROOT/installed/ImageMagick6"
IMAGEMAGICK6_INCLUDE_DIRS="$IMAGEMAGICK6_PREFIX/include/ImageMagick-6;$IMAGEMAGICK6_PREFIX/lib/ImageMagick-${IMAGEMAGICK6_ABI_VERSION}/config-Q16"
IMAGEMAGICK6_WAND_LIBRARY="$IMAGEMAGICK6_PREFIX/lib/libMagickWand-6.Q16.so"
IMAGEMAGICK6_CORE_LIBRARY="$IMAGEMAGICK6_PREFIX/lib/libMagickCore-6.Q16.so"
IMAGEMAGICK6_FORMATS_STAMP="$IMAGEMAGICK6_PREFIX/.hhvm-jpeg-png-formats"
OPAM_REPOSITORY_REVISION="588f3130c1513932f63440010a91796ce12d54fa"
OPAM_REPOSITORY_URL="https://github.com/ocaml/opam-repository/archive/${OPAM_REPOSITORY_REVISION}.tar.gz"
OPAM_REPOSITORY_ARCHIVE="opam-repository-${OPAM_REPOSITORY_REVISION}.tar.gz"
OPAM_REPOSITORY_SHA256="7221dd867475707a9e0f423dde137821863dfe6c84b86c3ba802927d74e05787"
OPAM_REPOSITORY_SRC_DIR="$HHVM_OSS_WORK_ROOT/sources/opam-repository"
export HHVM_OSS_OPAM_REPOSITORY="$OPAM_REPOSITORY_SRC_DIR"
FBMYSQL_OSS_ROCKSDB_SRC=""
CMAKE_C_COMPILER_PATH="${HHVM_OSS_C_COMPILER:-}"
CMAKE_CXX_COMPILER_PATH="${HHVM_OSS_CXX_COMPILER:-}"
CMAKE_TOOLCHAIN_FILE_PATH=""
CMAKE_COMPILER_FLAGS=()
CMAKE_COMPILER_DESCRIPTION=""
HHVM_CMAKE_ARCHITECTURE=""
NEED_XED=false
MAGIC_ENUM_VERSION="v0.9.7"
MAGIC_ENUM_DOWNLOAD_URL="https://github.com/Neargye/magic_enum/releases/download/${MAGIC_ENUM_VERSION}/magic_enum-${MAGIC_ENUM_VERSION}.tar.gz"
MAGIC_ENUM_DOWNLOAD_ARCHIVE="magic_enum-${MAGIC_ENUM_VERSION}.tar.gz"
MAGIC_ENUM_DOWNLOAD_SHA256="c047bc7ca0b76752168140e7ae9a4a30d72bf6530c196fdfbf5105a39d40cc46"
SHIPIT_SOURCE_MAPPINGS=(
  "folly:$SRC_DIR/third-party/folly/src"
  "fizz:$SRC_DIR/third-party/fizz/src"
  "wangle:$SRC_DIR/third-party/wangle/src"
  "fbthrift:$SRC_DIR/third-party/thrift/src"
  "proxygen:$SRC_DIR/third-party/proxygen/src"
  "mcrouter:$SRC_DIR/third-party/mcrouter/src"
  "fatal:$SRC_DIR/third-party/fatal"
)
SHIPIT_SOURCE_ARGS=()
for source_mapping in "${SHIPIT_SOURCE_MAPPINGS[@]}"; do
  SHIPIT_SOURCE_ARGS+=(--src-dir "$source_mapping")
done
unset source_mapping

case "$(uname -m)" in
  x86_64|amd64)
    HHVM_CMAKE_ARCHITECTURE="IS_X64"
    NEED_XED=true
    RUST_TARGET="x86_64-unknown-linux-gnu"
    RUST_SHA256="a2edc6497cc5bef5d19f75fb16792f36e5972cf8a0d1af765613041921050821"
    ;;
  aarch64|arm64)
    HHVM_CMAKE_ARCHITECTURE="IS_AARCH64"
    RUST_TARGET="aarch64-unknown-linux-gnu"
    RUST_SHA256="a76786cd5ebff9c61579c711a60d0a8af55b07204c0eff3065f2555d11bcde75"
    ;;
  *)
    echo "ERROR: Unsupported architecture: $(uname -m)"
    exit 1
    ;;
esac

for arg in "$@"; do
  case "$arg" in
    --rebuild) FORCE_REBUILD=true ;;
    --make) MAKE_ONLY=true ;;
    --clean)
      echo "Cleaning all output directories..."
      rm -rf "$HHVM_OSS_WORK_ROOT"
      echo "Done. Re-run without --clean to rebuild."
      exit 0
      ;;
    -h|--help)
      cat <<HELPEOF
HHVM OSS Build Setup

Initializes the public git submodules, builds dependencies with getdeps.py,
then configures and builds HHVM. Safe to re-run: each phase skips if its
outputs already exist.

This script supports Linux on x86-64 and AArch64.

Usage: $0 [OPTIONS]

Options:
  --rebuild   Force cmake reconfigure and rebuild (keeps deps)
  --make      Skip setup, just run make (for iterating on code fixes)
  --clean     Delete all output directories and exit
  -h, --help  Show this help

Directories:
  Source tree:   $SRC_DIR
  Build dir:     $BUILD_DIR
  getdeps cache: $SCRATCH_DIR
  HHVM build type: $HHVM_CMAKE_BUILD_TYPE
  HHVM build target: $HHVM_BUILD_TARGET

Environment overrides:
  HHVM_OSS_WORK_ROOT
  HHVM_OSS_BUILD_DIR
  HHVM_OSS_SCRATCH_DIR
  HHVM_OSS_GETDEPS_CWD
  HHVM_OSS_C_COMPILER
  HHVM_OSS_CXX_COMPILER
  HHVM_OSS_CMAKE_BUILD_TYPE
  HHVM_OSS_BUILD_TARGET
  HHVM_OSS_LOG_FILE

Requires internet access for git submodules, public archives, getdeps, and cargo.
Building Hack tool targets separately may also require opam.
HELPEOF
      exit 0
      ;;
    *)
      echo "Unknown option: $arg (use --help)"
      exit 1
      ;;
  esac
done

mkdir -p "$(dirname "$RUN_LOG_FILE")"
: > "$RUN_LOG_FILE"
exec > >(tee "$RUN_LOG_FILE") 2>&1

echo "=== HHVM OSS Build Setup ==="
echo "Source:  $SRC_DIR"
echo "Build:   $BUILD_DIR"
echo "Log:     $RUN_LOG_FILE"
echo ""

# ---------------------------------------------------------------------------
# Helper: find a supported C/C++ compiler pair
# ---------------------------------------------------------------------------
DETECTED_C_COMPILER_PATH=""
DETECTED_CXX_COMPILER_PATH=""
DETECTED_COMPILER_KIND=""
GCC_VERSIONED_COMPILER_PAIRS=(
  "gcc-16|g++-16"
  "gcc-15|g++-15"
  "gcc-14|g++-14"
)
GCC_TOOLSET_VERSIONS=(14 15 16)
CLANG_COMPILER_VERSIONS=(21 20)

detect_compiler_pair() {
  local c_name="$1" cxx_name="$2" minimum_major="$3" compiler_kind="$4"
  local c_bin cxx_bin compiler_major

  c_bin="$(command -v "$c_name" 2>/dev/null || true)"
  cxx_bin="$(command -v "$cxx_name" 2>/dev/null || true)"
  if [ -z "$c_bin" ] || [ -z "$cxx_bin" ]; then
    return 1
  fi

  compiler_major="$("$c_bin" -dumpversion 2>/dev/null | cut -d. -f1)"
  case "$compiler_major" in
    ''|*[!0-9]*) return 1 ;;
  esac
  if [ "$compiler_major" -lt "$minimum_major" ]; then
    return 1
  fi

  DETECTED_C_COMPILER_PATH="$c_bin"
  DETECTED_CXX_COMPILER_PATH="$cxx_bin"
  DETECTED_COMPILER_KIND="$compiler_kind"
}

detect_supported_compilers() {
  local pair c_name cxx_name compiler_version

  DETECTED_C_COMPILER_PATH=""
  DETECTED_CXX_COMPILER_PATH=""
  DETECTED_COMPILER_KIND=""

  if detect_compiler_pair gcc g++ 14 gcc; then
    return 0
  fi

  for pair in "${GCC_VERSIONED_COMPILER_PAIRS[@]}"; do
    IFS='|' read -r c_name cxx_name <<< "$pair"
    if detect_compiler_pair "$c_name" "$cxx_name" 14 gcc; then
      return 0
    fi
  done

  for compiler_version in "${GCC_TOOLSET_VERSIONS[@]}"; do
    c_name="/opt/rh/gcc-toolset-${compiler_version}/root/usr/bin/gcc"
    cxx_name="/opt/rh/gcc-toolset-${compiler_version}/root/usr/bin/g++"
    if detect_compiler_pair "$c_name" "$cxx_name" 14 gcc; then
      return 0
    fi
  done

  for compiler_version in "${CLANG_COMPILER_VERSIONS[@]}"; do
    if detect_compiler_pair \
      "clang-${compiler_version}" "clang++-${compiler_version}" 20 clang; then
      return 0
    fi
  done

  if detect_compiler_pair clang clang++ 20 clang; then
    return 0
  fi

  return 1
}

# ---------------------------------------------------------------------------
# Helper: query /etc/os-release without inheriting its quoting
# ---------------------------------------------------------------------------
os_release_field() {
  local field="$1"

  [ -r /etc/os-release ] || return 0
  # Values in /etc/os-release are shell-quoted (ID="rhel"), so sourcing the
  # file is the only reliable way to read them back unquoted.
  (
    . /etc/os-release
    printf '%s\n' "${!field:-}"
  )
}

is_rhel_family() {
  local id id_like word

  id="$(os_release_field ID)"
  id_like="$(os_release_field ID_LIKE)"
  # RHEL itself reports ID=rhel with ID_LIKE=fedora, so both fields matter.
  for word in $id $id_like; do
    case "$word" in
      centos|rhel|rocky|almalinux) return 0 ;;
    esac
  done
  return 1
}

# ---------------------------------------------------------------------------
# Helper: report which of the given packages are not installed yet
#
# Uses the caller's "package_query" array, which ensure_system_dependencies
# sets once for the detected package manager.
# ---------------------------------------------------------------------------
collect_missing_packages() {
  local package

  for package in "$@"; do
    "${package_query[@]}" "$package" >/dev/null 2>&1 || printf '%s\n' "$package"
  done
}

# ---------------------------------------------------------------------------
# Helper: latest major version of a package offered by the dnf repositories
# ---------------------------------------------------------------------------
dnf_available_major() {
  local name="$1" version

  version="$(dnf -q repoquery --latest-limit 1 --qf '%{version}' "$name" \
    2>/dev/null | tail -1 | cut -d. -f1)"
  printf '%s\n' "${version:-0}"
}

# ---------------------------------------------------------------------------
# Helper: install required system development packages
# ---------------------------------------------------------------------------
ensure_system_dependencies() {
  local package_manager=""
  local common_packages=(
    autoconf automake bison bzip2 ca-certificates cmake diffutils file
    flex gawk gzip libtool make ninja-build openssl patch python3 rsync
    tar unzip zip
  )
  local packages=()
  local package_query=()
  local repository_packages=()
  local missing_repository_packages=()
  local missing_packages=()
  local root_command=()
  local install_environment=()
  local install_command=()
  local compiler_pair compiler_version c_package cxx_package
  local gcc_toolset_version=""
  local compiler_package_found=false
  local supported_compiler_found=false

  if detect_supported_compilers; then
    supported_compiler_found=true
    if [[ "$DETECTED_C_COMPILER_PATH" =~ ^/opt/rh/gcc-toolset-([0-9]+)/ ]]; then
      gcc_toolset_version="${BASH_REMATCH[1]}"
    fi
  fi

  if command -v dnf >/dev/null 2>&1; then
    package_manager="dnf"
    packages=(
      "${common_packages[@]}"
      gcc gcc-c++ git-core perl-interpreter pkgconf-pkg-config procps-ng which xz
      double-conversion-devel libedit-devel brotli-devel bzip2-devel
      binutils-devel glibc-gconv-extra libaio-devel libatomic lz4-devel
      numactl-devel openssl-devel
      python3-devel libjpeg-turbo-devel
      libpng-devel freetype-devel
      libcurl-devel systemd-devel libbpf-devel libunwind-devel libcap-devel
      libzip-devel re2-devel re2c expat-devel fribidi-devel libheif-devel
      c-ares-devel gmp-devel lmdb-devel libmemcached-awesome-devel
      libxslt-devel sqlite-devel openldap-devel
    )
    package_query=(rpm -q)
  elif command -v apt-get >/dev/null 2>&1; then
    package_manager="apt-get"
    packages=(
      "${common_packages[@]}"
      build-essential git perl pkg-config procps xz-utils
      libdouble-conversion-dev libedit-dev libbrotli-dev libbz2-dev
      libaio-dev libiberty-dev liblz4-dev libnuma-dev libssl-dev
      libjpeg-dev libpng-dev
      libfreetype-dev libicu-dev libonig-dev
      libcurl4-openssl-dev libsystemd-dev libbpf-dev libunwind-dev libcap-dev
      libzip-dev libre2-dev re2c libexpat1-dev libfribidi-dev libheif-dev
      libc-ares-dev libgmp-dev liblmdb-dev libmemcached-dev libxslt1-dev
      libsqlite3-dev libldap2-dev
    )
    package_query=(dpkg-query -W)
  else
    echo "ERROR: A supported system package manager is required (dnf or apt-get)."
    exit 1
  fi

  if [ "$HHVM_CMAKE_ARCHITECTURE" = "IS_AARCH64" ]; then
    packages+=(lld)
  fi

  if ! command -v curl >/dev/null 2>&1; then
    packages+=(curl)
  fi

  if [ "$(id -u)" -eq 0 ]; then
    root_command=()
  elif command -v sudo >/dev/null 2>&1; then
    root_command=(sudo)
  else
    echo "ERROR: System dependencies must be installed, but sudo is unavailable."
    exit 1
  fi

  if [ "$package_manager" = "apt-get" ]; then
    install_environment=(DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC)
    "${root_command[@]}" apt-get update
    if apt-cache show tzdata-legacy >/dev/null 2>&1; then
      packages+=(tzdata-legacy)
    fi
  fi
  install_command=(
    "${root_command[@]}" env "${install_environment[@]}"
    "$package_manager" install -y
  )

  if [ "$package_manager" = "dnf" ] && is_rhel_family; then
    repository_packages=(dnf-plugins-core epel-release)
    readarray -t missing_repository_packages < <(
      collect_missing_packages "${repository_packages[@]}")
    if [ "${#missing_repository_packages[@]}" -gt 0 ]; then
      "${install_command[@]}" "${missing_repository_packages[@]}"
    fi
    "${root_command[@]}" dnf config-manager --set-enabled crb
    "${root_command[@]}" dnf makecache
  fi

  if [ "$supported_compiler_found" = false ]; then
    if [ "$package_manager" = "apt-get" ]; then
      for compiler_pair in "${GCC_VERSIONED_COMPILER_PAIRS[@]}"; do
        IFS='|' read -r c_package cxx_package <<< "$compiler_pair"
        if apt-cache show "$c_package" >/dev/null 2>&1 &&
           apt-cache show "$cxx_package" >/dev/null 2>&1; then
          packages+=("$c_package")
          if [ "$cxx_package" != "$c_package" ]; then
            packages+=("$cxx_package")
          fi
          compiler_package_found=true
          break
        fi
      done
      if [ "$compiler_package_found" = false ]; then
        for compiler_version in "${CLANG_COMPILER_VERSIONS[@]}"; do
          c_package="clang-${compiler_version}"
          if apt-cache show "$c_package" >/dev/null 2>&1; then
            packages+=("$c_package")
            compiler_package_found=true
            break
          fi
        done
      fi
    else
      if [ "$(dnf_available_major gcc)" -ge 14 ]; then
        compiler_package_found=true
      else
        for compiler_version in "${GCC_TOOLSET_VERSIONS[@]}"; do
          c_package="gcc-toolset-${compiler_version}-gcc"
          cxx_package="${c_package}-c++"
          if dnf -q list --available "$c_package" "$cxx_package" >/dev/null 2>&1; then
            gcc_toolset_version="$compiler_version"
            compiler_package_found=true
            break
          fi
        done
        if [ "$compiler_package_found" = false ] &&
           [ "$(dnf_available_major clang)" -ge 20 ]; then
          packages+=(clang)
          compiler_package_found=true
        fi
      fi
    fi

    if [ "$compiler_package_found" = false ]; then
      echo "ERROR: GCC >= 14 or Clang >= 20 is required."
      exit 1
    fi
  fi

  if [ "$package_manager" = "dnf" ] && [ -n "$gcc_toolset_version" ]; then
    packages+=(
      "gcc-toolset-${gcc_toolset_version}-gcc"
      "gcc-toolset-${gcc_toolset_version}-gcc-c++"
      "gcc-toolset-${gcc_toolset_version}-libatomic-devel"
    )
  fi

  readarray -t missing_packages < <(collect_missing_packages "${packages[@]}")
  [ "${#missing_packages[@]}" -gt 0 ] || return 0

  echo ">>> Installing required system development packages..."
  "${install_command[@]}" "${missing_packages[@]}"
}

# ---------------------------------------------------------------------------
# Helper: download a file
# ---------------------------------------------------------------------------
download_tarball() {
  local name="$1" url="$2" target_dir="$3"
  local filename="${4:-$(basename "$url")}"
  local expected_hash="${5:-}"
  local target="$target_dir/$filename"
  local actual_hash

  verify_hash() {
    local file="$1" expected="$2"
    [ -n "$expected" ] || return 0

    case "$expected" in
      SHA256=*)
        actual_hash="$(openssl dgst -sha256 "$file" | awk '{print $NF}')"
        ;;
      SHA512=*)
        actual_hash="$(openssl dgst -sha512 "$file" | awk '{print $NF}')"
        ;;
      *)
        echo "    ERROR: Unsupported hash format: $expected"
        return 1
        ;;
    esac

    if [ "$actual_hash" != "${expected#*=}" ]; then
      echo "    ERROR: Hash mismatch for $file"
      return 1
    fi
  }

  verify_archive() {
    local file="$1"

    case "$file" in
      *.tar.gz|*.tgz)
        tar tzf "$file" >/dev/null 2>&1
        ;;
      *.tar.bz2)
        tar tjf "$file" >/dev/null 2>&1
        ;;
      *.tar.xz)
        tar tJf "$file" >/dev/null 2>&1
        ;;
      *)
        return 0
        ;;
    esac || {
      echo "    ERROR: Downloaded file is not a valid archive: $file"
      return 1
    }
  }

  if [ -f "$target" ] && [ -s "$target" ]; then
    if verify_hash "$target" "$expected_hash" && verify_archive "$target"; then
      echo "    $name: cached"
      return
    else
      rm -f "$target"
    fi
  fi
  echo "    $name: downloading..."
  mkdir -p "$target_dir"
  rm -f "$target"
  if ! curl -fsSL \
       --retry 8 \
       --retry-all-errors \
       --retry-delay 2 \
       "$url" \
       -o "$target" ||
     [ ! -s "$target" ]; then
    rm -f "$target"
    echo "    ERROR: Failed to download $url"
    return 1
  fi
  if ! verify_hash "$target" "$expected_hash"; then
    rm -f "$target"
    return 1
  fi
  if ! verify_archive "$target"; then
    rm -f "$target"
    return 1
  fi
  echo "    Done."
}

extract_source_archive() {
  local name="$1" archive="$2" source_dir="$3" marker="$4"

  if [ "$FORCE_REBUILD" = false ] && [ -f "$source_dir/$marker" ]; then
    echo "    $name source tree: cached"
    return
  fi

  rm -rf "$source_dir"
  mkdir -p "$source_dir"
  tar --no-same-owner -xzf "$archive" --strip-components=1 -C "$source_dir"
}

autotools_install_complete() {
  local prefix="$1" library="$2" header="$3"

  [ -f "$prefix/$library" ] && [ -f "$prefix/$header" ]
}

# Report whether the freshly built ImageMagick6 picked up a delegate library.
# HHVM's imagick extension is useless without at least JPEG and PNG, and a
# missing delegate only shows up at runtime otherwise.
imagemagick6_supports_format() {
  local format="$1"

  LD_LIBRARY_PATH="$IMAGEMAGICK6_PREFIX/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
    "$IMAGEMAGICK6_PREFIX/bin/identify" -list format |
    grep -Eq "^[[:space:]]+${format}[*[:space:]]"
}

build_autotools_dependency() {
  local name="$1" source_dir="$2" prefix="$3" library="$4" header="$5"
  local extra_cflags="${6:-}"
  local extra_configure_flags="${7:-}"
  local configure_environment=(
    "CC=$CMAKE_C_COMPILER_PATH"
    "CXX=$CMAKE_CXX_COMPILER_PATH"
  )
  local configure_options=(--prefix="$prefix" --disable-shared)
  local extra_configure_options=()

  if [ "$FORCE_REBUILD" = false ] && \
     autotools_install_complete "$prefix" "$library" "$header"; then
    echo "    $name install: cached"
    return
  fi

  rm -rf "$prefix"
  if [ -n "$extra_cflags" ]; then
    configure_environment+=("CFLAGS=$extra_cflags")
  fi
  if [ -n "$extra_configure_flags" ]; then
    read -r -a extra_configure_options <<< "$extra_configure_flags"
    configure_options+=("${extra_configure_options[@]}")
  fi
  (
    cd "$source_dir"
    if [ -f Makefile ]; then
      make distclean >/dev/null 2>&1 || true
    fi
    env "${configure_environment[@]}" \
      ./configure "${configure_options[@]}"
    make -j"$JOBS" install
  )

  if autotools_install_complete "$prefix" "$library" "$header"; then
    echo "    $name: Done."
  else
    echo "ERROR: $name install is incomplete in $prefix"
    exit 1
  fi
}

show_build_result() {
  local build_dir="$1" build_status="$2"

  if [ "$build_status" -eq 0 ]; then
    echo ""
    echo "=== Build Succeeded ==="
    ls -lh "$build_dir/hphp/hhvm/hhvm" 2>/dev/null
  else
    echo ""
    echo "=== Build Failed ==="
    echo "See $build_dir/build.log"
    echo "Unique errors:"
    grep "fatal error\|: error[: ]" "$build_dir/build.log" 2>/dev/null | sort -u | head -10
  fi
}

run_hhvm_build() {
  local build_dir="$1"
  local libmbfl_eaw_table="$SRC_DIR/third-party/forks/libmbfl/mbfl/eaw_table.h"

  # The libmbfl generator writes into the source tree. An interrupted build
  # can leave a truncated header which the generator then mistakes for a
  # completed output, so remove it before retrying the build.
  if [ -f "$libmbfl_eaw_table" ] && \
     ! grep -qF 'mbfl_eaw_table' "$libmbfl_eaw_table"; then
    echo ">>> Removing an invalid generated libmbfl East Asian width table."
    rm -f "$libmbfl_eaw_table"
  fi

  cmake --build "$build_dir" --target "$HHVM_BUILD_TARGET" -j"$JOBS" 2>&1 | tee "$build_dir/build.log"
  return "${PIPESTATUS[0]}"
}

run_public_getdeps() {
  (
    mkdir -p "$GETDEPS_PUBLIC_CWD"
    cd "$GETDEPS_PUBLIC_CWD"
    CC="$CMAKE_C_COMPILER_PATH" \
    CXX="$CMAKE_CXX_COMPILER_PATH" \
    CFLAGS="${CFLAGS:+$CFLAGS }-std=gnu17" \
      python3 "$GETDEPS" "$@"
  )
}

build_public_getdeps_target() {
  local target="$1" cmake_defines="$2"
  shift 2
  local extra_args=("$@")

  if [ "$cmake_defines" != "{}" ]; then
    extra_args+=(--extra-cmake-defines "$cmake_defines")
  fi

  run_public_getdeps build "$target" \
    --allow-system-packages \
    --no-facebook-internal \
    --no-tests \
    --scratch-path "$SCRATCH_DIR" \
    --num-jobs "$JOBS" \
    "${SHIPIT_SOURCE_ARGS[@]}" \
    "${extra_args[@]}"
}

set_getdeps_source_mode() {
  local fingerprint_input fingerprint mapping source_dir source_path
  local shipit_source_versions=""

  for mapping in "${SHIPIT_SOURCE_MAPPINGS[@]}"; do
    source_dir="${mapping#*:}"
    source_path="${source_dir#"$SRC_DIR/"}"
    shipit_source_versions+="$(git -C "$SRC_DIR" rev-parse "HEAD:$source_path")"$'\n'
    shipit_source_versions+="$(git -C "$SRC_DIR" diff --binary HEAD -- "$source_path")"$'\n'
  done

  fingerprint_input="$CMAKE_C_COMPILER_PATH
$("$CMAKE_C_COMPILER_PATH" --version)
$CMAKE_CXX_COMPILER_PATH
$("$CMAKE_CXX_COMPILER_PATH" --version)
$CMAKE_TOOLCHAIN_FILE_PATH
$(sha256sum "$CMAKE_TOOLCHAIN_FILE_PATH" "$SRC_DIR/CMake/HPHPCompiler.cmake")
${shipit_source_versions}
$(find "$SRC_DIR/build/deps/github_hashes" -type f -name '*-rev.txt' -print0 | sort -z | xargs -0 -r sha256sum)"
  fingerprint="$(printf '%s' "$fingerprint_input" | sha256sum | cut -c1-16)"
  GETDEPS_SOURCE_MODE="${GETDEPS_SOURCE_MODE_BASE}-${fingerprint}"
}

ensure_public_getdeps_source_mode() {
  local previous_mode=""

  if [ -f "$GETDEPS_SOURCE_MODE_FILE" ]; then
    previous_mode="$(cat "$GETDEPS_SOURCE_MODE_FILE")"
  fi

  if [ "$previous_mode" != "$GETDEPS_SOURCE_MODE" ]; then
    echo "    Resetting getdeps scratch for $GETDEPS_SOURCE_MODE"
    rm -rf \
      "$SCRATCH_DIR/build" \
      "$SCRATCH_DIR/extracted" \
      "$SCRATCH_DIR/installed" \
      "$SCRATCH_DIR/repos"
  fi

  mkdir -p "$SCRATCH_DIR"
  printf '%s\n' "$GETDEPS_SOURCE_MODE" > "$GETDEPS_SOURCE_MODE_FILE"
}

prepare_getdeps_boost_override() {
  local base_manifest="$GETDEPS_REAL_ROOT/manifests/boost"
  local override_manifest="$GETDEPS_MANIFEST_OVERRIDE_DIR/boost"
  local user_config="$SCRATCH_DIR/boost-user-config.jam"
  local toolset

  case "$(basename "$CMAKE_TOOLCHAIN_FILE_PATH")" in
    HPHPClangToolchain.cmake) toolset="clang" ;;
    HPHPGccToolchain.cmake) toolset="gcc" ;;
    *)
      echo "ERROR: Cannot select a Boost toolset for $CMAKE_COMPILER_DESCRIPTION"
      exit 1
      ;;
  esac

  mkdir -p "$GETDEPS_MANIFEST_OVERRIDE_DIR"
  printf 'using %s : hhvm : %s ;\n' \
    "$toolset" "$CMAKE_CXX_COMPILER_PATH" > "$user_config"
  python3 - \
    "$base_manifest" "$override_manifest" "$user_config" "$toolset" <<'PY'
from pathlib import Path
import sys

base_manifest = Path(sys.argv[1])
override_manifest = Path(sys.argv[2])
user_config = sys.argv[3]
toolset = sys.argv[4]

output = []
inserted = False
for line in base_manifest.read_text().splitlines():
    output.append(line)
    if line.strip() == "[b2.args.os=linux]":
        output.append(f"--user-config={user_config}")
        output.append(f"toolset={toolset}-hhvm")
        inserted = True

if not inserted:
    raise SystemExit("ERROR: boost manifest has no Linux b2 args section")

override_manifest.write_text("\n".join(output) + "\n")
PY

  printf '%s\n' "$override_manifest"
}

prepare_getdeps_download_manifest_override() {
  local name="$1" url="$2" sha256="$3"
  local base_manifest="$GETDEPS_REAL_ROOT/manifests/$name"
  local override_manifest="$GETDEPS_MANIFEST_OVERRIDE_DIR/$name"

  if [ ! -f "$base_manifest" ]; then
    echo "ERROR: getdeps manifest not found: $base_manifest"
    exit 1
  fi

  mkdir -p "$GETDEPS_MANIFEST_OVERRIDE_DIR"
  python3 - "$base_manifest" "$override_manifest" "$url" "$sha256" <<'PY'
from pathlib import Path
import sys

base_manifest = Path(sys.argv[1])
override_manifest = Path(sys.argv[2])
url = sys.argv[3]
sha256 = sys.argv[4]

lines = base_manifest.read_text().splitlines()
output = []
in_download = False
url_done = False
sha_done = False

for line in lines:
    stripped = line.strip()
    if in_download and stripped.startswith("[") and stripped.endswith("]"):
        if not url_done:
            output.append(f"url = {url}")
            url_done = True
        if not sha_done:
            output.append(f"sha256 = {sha256}")
            sha_done = True
        in_download = False

    if in_download and stripped.startswith("url ="):
        if not url_done:
            output.append(f"url = {url}")
            url_done = True
        continue

    if in_download and stripped.startswith("sha256 ="):
        if not sha_done:
            output.append(f"sha256 = {sha256}")
            sha_done = True
        continue

    output.append(line)
    if stripped == "[download]":
        in_download = True

if in_download:
    if not url_done:
        output.append(f"url = {url}")
    if not sha_done:
        output.append(f"sha256 = {sha256}")

override_manifest.write_text("\n".join(output) + "\n")
PY

  printf '%s\n' "$override_manifest"
}

prepare_getdeps_gnu_mirror_overrides() {
  local base_manifest name override_manifest source_manifest
  local rewritten=0

  mkdir -p "$GETDEPS_MANIFEST_OVERRIDE_DIR"
  for base_manifest in "$GETDEPS_REAL_ROOT/manifests/"*; do
    grep -qF "$GETDEPS_GNU_URL_ROOT/" "$base_manifest" || continue
    name="$(basename "$base_manifest")"
    override_manifest="$GETDEPS_MANIFEST_OVERRIDE_DIR/$name"
    # Rewrite on top of an override another prepare_* function already wrote,
    # so this never discards its edits. A plain substitution also covers
    # manifests that list more than one GNU download.
    source_manifest="$base_manifest"
    if [ -f "$override_manifest" ]; then
      source_manifest="$override_manifest"
    fi
    sed "s|$GETDEPS_GNU_URL_ROOT/|$GETDEPS_GNU_MIRROR_URL_ROOT/|g" \
      "$source_manifest" > "$override_manifest.tmp"
    mv "$override_manifest.tmp" "$override_manifest"
    ((rewritten += 1))
  done

  if [ "$rewritten" -gt 0 ]; then
    echo "    Redirected $rewritten GNU getdeps downloads to mirrors.kernel.org"
  fi
}

preseed_getdeps_download() {
  local name="$1" url="$2" archive_name="$3" sha256="$4"

  mkdir -p "$SCRATCH_DIR/downloads"
  download_tarball \
    "$name" \
    "$url" \
    "$SCRATCH_DIR/downloads" \
    "${name}-${archive_name}" \
    "SHA256=${sha256}"
}

prepare_getdeps_runner_root() {
  local source_root="$GETDEPS_REAL_ROOT"
  local shadow_root="$GETDEPS_SHADOW_ROOT"
  local shadow_manifests="$GETDEPS_SHADOW_MANIFESTS_DIR"
  local shadow_deps="$SCRATCH_DIR/deps"
  local entry base

  mkdir -p "$shadow_root"

  for entry in "$source_root/"*; do
    [ -e "$entry" ] || continue
    base="$(basename "$entry")"
    [ "$base" = "manifests" ] && continue
    if [ ! -e "$shadow_root/$base" ] && [ ! -L "$shadow_root/$base" ]; then
      ln -s "$entry" "$shadow_root/$base"
    fi
  done

  rm -rf "$shadow_manifests"
  mkdir -p "$shadow_manifests"

  for entry in "$source_root/manifests/"*; do
    [ -e "$entry" ] || continue
    ln -s "$entry" "$shadow_manifests/$(basename "$entry")"
  done

  for entry in "$GETDEPS_MANIFEST_OVERRIDE_DIR/"*; do
    [ -f "$entry" ] || continue
    base="$(basename "$entry")"
    rm -f "$shadow_manifests/$base"
    cp "$entry" "$shadow_manifests/$base"
  done

  # getdeps reads ShipIt revision files from ../deps/github_hashes relative to
  # its fbcode_builder root, so expose HHVM's generated files beside the shadow.
  rm -rf "$shadow_deps"
  ln -s "$SRC_DIR/build/deps" "$shadow_deps"

  GETDEPS="$shadow_root/getdeps.py"
}

# --make: skip all setup, just run make
if [ "$MAKE_ONLY" = true ]; then
  if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "ERROR: No cmake cache found. Run without --make first."
    exit 1
  fi
  echo ">>> Building HHVM (--make mode)..."
  if run_hhvm_build "$BUILD_DIR"; then
    build_status=0
  else
    build_status="$?"
  fi
  show_build_result "$BUILD_DIR" "$build_status"
  exit "$build_status"
fi

find_getdeps_prefix() {
  local dep="$1" candidate

  for candidate in "$GETDEPS_DIR/$dep" "$GETDEPS_DIR/$dep"-*; do
    [ -d "$candidate" ] || continue
    printf '%s\n' "$candidate"
    return 0
  done

  return 1
}

find_mcrouter_install_prefix() {
  local prefix libdir

  prefix="$(find_getdeps_prefix mcrouter || true)"
  [ -n "$prefix" ] || return 1

  for libdir in "$prefix/lib" "$prefix/lib64"; do
    if [ -f "$prefix/include/mcrouter/McrouterClient.h" ] && \
       [ -f "$libdir/cmake/mcrouter/mcrouter-config.cmake" ]; then
      printf '%s\n' "$prefix"
      return 0
    fi
  done

  return 1
}

find_boost_cmake_dir() {
  local prefix="$1" candidate

  [ -n "$prefix" ] || return 1

  for candidate in "$prefix"/lib/cmake/Boost-* "$prefix"/lib64/cmake/Boost-*; do
    [ -d "$candidate" ] || continue
    if [ -f "$candidate/BoostConfig.cmake" ]; then
      printf '%s\n' "$candidate"
      return 0
    fi
  done

  return 1
}

find_boost_install_prefix() {
  local candidate cmake_dir libdir mtime
  local best_prefix=""
  local best_mtime=0

  for candidate in "$GETDEPS_DIR"/boost "$GETDEPS_DIR"/boost-*; do
    [ -d "$candidate" ] || continue
    [ -f "$candidate/include/boost/version.hpp" ] || continue
    libdir=""
    if [ -f "$candidate/lib/libboost_system.a" ] && \
       [ -f "$candidate/lib/libboost_filesystem.a" ]; then
      libdir="$candidate/lib"
    elif [ -f "$candidate/lib64/libboost_system.a" ] && \
         [ -f "$candidate/lib64/libboost_filesystem.a" ]; then
      libdir="$candidate/lib64"
    fi
    [ -n "$libdir" ] || continue
    cmake_dir="$(find_boost_cmake_dir "$candidate" || true)"
    [ -n "$cmake_dir" ] || continue
    mtime="$(stat -c '%Y' "$cmake_dir/BoostConfig.cmake" 2>/dev/null || echo 0)"
    if [ "$mtime" -ge "$best_mtime" ]; then
      best_prefix="$candidate"
      best_mtime="$mtime"
    fi
  done

  [ -n "$best_prefix" ] || return 1
  printf '%s\n' "$best_prefix"
}

find_fatal_install_prefix() {
  local prefix

  prefix="$(find_getdeps_prefix fatal || true)"
  [ -n "$prefix" ] || return 1

  if [ -f "$prefix/fatal/portability.h" ] || \
     [ -f "$prefix/include/fatal/portability.h" ]; then
    printf '%s\n' "$prefix"
    return 0
  fi

  return 1
}

find_rocksdb_source() {
  local candidate

  for candidate in "$SCRATCH_DIR"/extracted/rocksdb*/* "$SCRATCH_DIR"/extracted/rocksdb*; do
    [ -d "$candidate" ] || continue
    if [ -f "$candidate/Makefile" ] && [ -f "$candidate/include/rocksdb/env.h" ]; then
      printf '%s\n' "$candidate"
      return 0
    fi
  done

  return 1
}

have_system_jemalloc_53() {
  local compiler tmp

  compiler="${CC:-cc}"
  command -v "$compiler" >/dev/null 2>&1 || return 1

  tmp="$(mktemp)"
  if cat <<'EOF' | "$compiler" -x c - -o "$tmp" -ljemalloc >/dev/null 2>&1
#include <jemalloc/jemalloc.h>
#if JEMALLOC_VERSION_MAJOR < 5 || (JEMALLOC_VERSION_MAJOR == 5 && JEMALLOC_VERSION_MINOR < 3)
# error jemalloc version >= 5.3 required
#endif
int main(void) { return 0; }
EOF
  then
    rm -f "$tmp"
    return 0
  fi
  rm -f "$tmp"
  return 1
}

choose_cmake_compilers() {
  if [ "${#CMAKE_COMPILER_FLAGS[@]}" -gt 0 ]; then
    return 0
  fi

  if [ -n "$CMAKE_C_COMPILER_PATH" ] || [ -n "$CMAKE_CXX_COMPILER_PATH" ]; then
    if [ -z "$CMAKE_C_COMPILER_PATH" ] || [ -z "$CMAKE_CXX_COMPILER_PATH" ]; then
      echo "ERROR: Set both HHVM_OSS_C_COMPILER and HHVM_OSS_CXX_COMPILER."
      exit 1
    fi
    CMAKE_C_COMPILER_PATH="$(command -v "$CMAKE_C_COMPILER_PATH" 2>/dev/null || true)"
    CMAKE_CXX_COMPILER_PATH="$(command -v "$CMAKE_CXX_COMPILER_PATH" 2>/dev/null || true)"
    if [ -z "$CMAKE_C_COMPILER_PATH" ] || [ -z "$CMAKE_CXX_COMPILER_PATH" ]; then
      echo "ERROR: Requested OSS C/C++ compiler was not found."
      exit 1
    fi
    CMAKE_COMPILER_FLAGS=(
      "-DCMAKE_C_COMPILER=$CMAKE_C_COMPILER_PATH"
      "-DCMAKE_CXX_COMPILER=$CMAKE_CXX_COMPILER_PATH"
    )
    if "$CMAKE_CXX_COMPILER_PATH" --version | head -1 | grep -qi clang; then
      CMAKE_TOOLCHAIN_FILE_PATH="$SRC_DIR/CMake/HPHPClangToolchain.cmake"
    else
      CMAKE_TOOLCHAIN_FILE_PATH="$SRC_DIR/CMake/HPHPGccToolchain.cmake"
    fi
    CMAKE_COMPILER_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE_PATH")
    CMAKE_COMPILER_DESCRIPTION="requested toolchain: $("$CMAKE_CXX_COMPILER_PATH" --version | head -1)"
    return 0
  fi

  if detect_supported_compilers; then
    CMAKE_C_COMPILER_PATH="$DETECTED_C_COMPILER_PATH"
    CMAKE_CXX_COMPILER_PATH="$DETECTED_CXX_COMPILER_PATH"
  else
    echo "ERROR: GCC >= 14 or Clang >= 20 is required."
    exit 1
  fi

  if [ "$DETECTED_COMPILER_KIND" = "gcc" ]; then
    CMAKE_TOOLCHAIN_FILE_PATH="$SRC_DIR/CMake/HPHPGccToolchain.cmake"
    CMAKE_COMPILER_DESCRIPTION="GCC: $("$CMAKE_CXX_COMPILER_PATH" --version | head -1)"
  else
    CMAKE_TOOLCHAIN_FILE_PATH="$SRC_DIR/CMake/HPHPClangToolchain.cmake"
    CMAKE_COMPILER_DESCRIPTION="Clang: $("$CMAKE_CXX_COMPILER_PATH" --version | head -1)"
  fi

  CMAKE_COMPILER_FLAGS=(
    "-DCMAKE_C_COMPILER=$CMAKE_C_COMPILER_PATH"
    "-DCMAKE_CXX_COMPILER=$CMAKE_CXX_COMPILER_PATH"
    "-DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE_PATH"
  )
}

configure_fbmysql_source() {
  local rocksdb_dst="$FBMYSQL_OSS_SRC_DIR/rocksdb"
  local current_target expected_target

  if [ ! -d "$FBMYSQL_OSS_SRC_DIR/include" ]; then
    echo "ERROR: OSS fb-mysql source not found at $FBMYSQL_OSS_SRC_DIR"
    exit 1
  fi

  expected_target="$(readlink -f "$FBMYSQL_OSS_ROCKSDB_SRC")"
  current_target="$(readlink -f "$rocksdb_dst" 2>/dev/null || true)"
  if [ "$current_target" != "$expected_target" ]; then
    rm -rf "$rocksdb_dst"
    ln -s "$FBMYSQL_OSS_ROCKSDB_SRC" "$rocksdb_dst"
  fi

  if [ ! -f "$rocksdb_dst/Makefile" ] ||
     [ ! -f "$rocksdb_dst/include/rocksdb/env.h" ]; then
    echo "ERROR: fb-mysql requires a RocksDB source tree with Makefile and headers."
    echo "Expected at: $rocksdb_dst"
    exit 1
  fi
}

ensure_system_dependencies
# HHVM's build scripts rely on GNU awk extensions, so mawk/busybox awk on PATH
# is not a usable substitute.
GAWK_EXECUTABLE="$(command -v gawk || true)"
if [ -z "$GAWK_EXECUTABLE" ]; then
  echo "ERROR: gawk is required but was not found on PATH."
  exit 1
fi

# ---------------------------------------------------------------------------
# Phase 1: Fetch public git submodules
# ---------------------------------------------------------------------------
echo ">>> Phase 1: Initializing public git submodules..."
if ! git -C "$SRC_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "ERROR: $SRC_DIR is not a git checkout."
  echo "Clone HHVM with git before running this script."
  exit 1
fi
git -C "$SRC_DIR" submodule sync --recursive
git -C "$SRC_DIR" submodule update --init --recursive
echo "    Done."

# ---------------------------------------------------------------------------
# Phase 2: Build a recent public liburing
# ---------------------------------------------------------------------------
if [ -f "$LIBURING_PREFIX/lib/liburing.a" ] &&
   [ -f "$LIBURING_PREFIX/include/liburing/io_uring_version.h" ] &&
   [ -f "$LIBURING_REVISION_STAMP" ] &&
   [ "$(cat "$LIBURING_REVISION_STAMP")" = "$LIBURING_REVISION" ]; then
  echo ">>> Phase 2: liburing $LIBURING_VERSION already built. Skipping."
else
  echo ">>> Phase 2: Building liburing $LIBURING_VERSION..."
  choose_cmake_compilers
  echo "    Using $CMAKE_COMPILER_DESCRIPTION"
  download_tarball \
    "liburing-${LIBURING_VERSION}" \
    "$LIBURING_URL" \
    "$DOWNLOAD_CACHE_DIR" \
    "$LIBURING_ARCHIVE" \
    "SHA256=$LIBURING_SHA256"
  rm -rf "$LIBURING_SRC_DIR" "$LIBURING_PREFIX"
  mkdir -p "$LIBURING_SRC_DIR" "$LIBURING_PREFIX"
  tar --no-same-owner -xzf "$DOWNLOAD_CACHE_DIR/$LIBURING_ARCHIVE" \
    --strip-components=1 \
    -C "$LIBURING_SRC_DIR"
  (
    cd "$LIBURING_SRC_DIR"
    ./configure \
      --prefix="$LIBURING_PREFIX" \
      --cc="$CMAKE_C_COMPILER_PATH" \
      --cxx="$CMAKE_CXX_COMPILER_PATH"
    make -j"$JOBS"
    make install
  )
  printf '%s\n' "$LIBURING_REVISION" > "$LIBURING_REVISION_STAMP"
fi

# ---------------------------------------------------------------------------
# Phase 3: Build Meta dependencies with getdeps
# ---------------------------------------------------------------------------
choose_cmake_compilers
set_getdeps_source_mode
ensure_public_getdeps_source_mode
rm -rf "$GETDEPS_MANIFEST_OVERRIDE_DIR"
prepare_getdeps_boost_override >/dev/null
prepare_getdeps_download_manifest_override magic_enum "$MAGIC_ENUM_DOWNLOAD_URL" "$MAGIC_ENUM_DOWNLOAD_SHA256" >/dev/null
prepare_getdeps_gnu_mirror_overrides
prepare_getdeps_runner_root
MCROUTER_INSTALL="$(find_mcrouter_install_prefix || true)"
BOOST_INSTALL="$(find_boost_install_prefix || true)"
FATAL_INSTALL="$(find_fatal_install_prefix || true)"
BLAKE3_INSTALL="$(find_getdeps_prefix blake3 || true)"
GETDEPS_CMAKE_DEFINES="$(printf \
  '{"CMAKE_C_COMPILER":"%s","CMAKE_CXX_COMPILER":"%s","CMAKE_TOOLCHAIN_FILE":"%s","CMAKE_PREFIX_PATH":"%s","CMAKE_MODULE_PATH":"%s"}' \
  "$CMAKE_C_COMPILER_PATH" \
  "$CMAKE_CXX_COMPILER_PATH" \
  "$CMAKE_TOOLCHAIN_FILE_PATH" \
  "$LIBURING_PREFIX" \
  "$GETDEPS_REAL_ROOT/CMake")"
if [ -d "$GETDEPS_DIR/fbthrift" ] && [ -d "$GETDEPS_DIR/folly" ] && [ -d "$GETDEPS_DIR/proxygen" ] && \
   [ -n "$MCROUTER_INSTALL" ] && [ -n "$BOOST_INSTALL" ] && [ -n "$FATAL_INSTALL" ] && [ -n "$BLAKE3_INSTALL" ] && \
   [ "$FORCE_REBUILD" = false ]; then
  echo ">>> Phase 3: Meta deps already built. Skipping. (use --rebuild to force)"
else
  echo ">>> Phase 3: Building Meta dependencies with getdeps..."
  echo "    Using $CMAKE_COMPILER_DESCRIPTION"

  # Use the ShipIt-synchronized sources bundled with HHVM. Dependencies which
  # are external cells, such as mvfst, use the exported ShipIt revision files.
  for dep in boost blake3 fatal folly fizz wangle mvfst fbthrift proxygen ragel mcrouter; do
    target="$dep"
    label="$dep"
    extra_args=()
    dep_cmake_defines="$GETDEPS_CMAKE_DEFINES"

    case "$dep" in
      folly|fizz|wangle|fbthrift|proxygen|fatal)
        label="$dep (ShipIt source)"
        ;;
      mvfst)
        label="$dep (ShipIt revision)"
        extra_args+=(--no-deps)
        ;;
      mcrouter)
        label="$dep (ShipIt source)"
        BOOST_INSTALL="${BOOST_INSTALL:-$(find_boost_install_prefix || true)}"
        if [ -z "$BOOST_INSTALL" ]; then
          echo "ERROR: boost install was not found before configuring mcrouter"
          exit 1
        fi
        MCROUTER_BOOST_CMAKE_DIR="$(find_boost_cmake_dir "$BOOST_INSTALL" || true)"
        if [ -z "$MCROUTER_BOOST_CMAKE_DIR" ]; then
          echo "ERROR: BoostConfig.cmake was not found under $BOOST_INSTALL"
          exit 1
        fi
        dep_cmake_defines="$(python3 - \
          "$dep_cmake_defines" \
          "$BOOST_INSTALL" \
          "$MCROUTER_BOOST_CMAKE_DIR" <<'PY'
import json
import sys

defines = json.loads(sys.argv[1])
defines["BOOST_ROOT"] = sys.argv[2]
defines["Boost_ROOT"] = sys.argv[2]
defines["Boost_DIR"] = sys.argv[3]
defines["Boost_NO_SYSTEM_PATHS"] = "ON"
print(json.dumps(defines, separators=(",", ":")))
PY
)"
        ;;
    esac

    echo "  --- $label ---"
    build_public_getdeps_target \
      "$target" "$dep_cmake_defines" "${extra_args[@]}" \
      || { echo "ERROR: Failed to build $dep"; exit 1; }

    if [ "$dep" = "boost" ]; then
      BOOST_INSTALL="$(find_boost_install_prefix || true)"
      if [ -z "$BOOST_INSTALL" ]; then
        echo "ERROR: boost build finished but the getdeps install was not found"
        exit 1
      fi
    fi
  done

  BOOST_INSTALL="$(find_boost_install_prefix || true)"
  if [ -z "$BOOST_INSTALL" ]; then
    echo "ERROR: boost build finished but the getdeps install was not found"
    exit 1
  fi

  FATAL_INSTALL="$(find_fatal_install_prefix || true)"
  if [ -z "$FATAL_INSTALL" ]; then
    echo "ERROR: fatal build finished but the getdeps install was not found"
    exit 1
  fi

  BLAKE3_INSTALL="$(find_getdeps_prefix blake3 || true)"
  if [ -z "$BLAKE3_INSTALL" ]; then
    echo "ERROR: blake3 build finished but the getdeps install was not found"
    exit 1
  fi

  MCROUTER_INSTALL="$(find_mcrouter_install_prefix || true)"
  if [ -z "$MCROUTER_INSTALL" ]; then
    echo "ERROR: mcrouter build finished but the getdeps install was not found"
    exit 1
  fi

  echo "    boost install: $BOOST_INSTALL"
  echo "    mcrouter install: $MCROUTER_INSTALL"
fi

# ---------------------------------------------------------------------------
# Phase 3b: Build the header-only magic_enum dependency
# ---------------------------------------------------------------------------
MAGIC_ENUM_INSTALL="$(find_getdeps_prefix magic_enum || true)"
if [ -n "$MAGIC_ENUM_INSTALL" ] && [ "$FORCE_REBUILD" = false ]; then
  echo ">>> Phase 3b: magic_enum already built. Skipping."
else
  echo ">>> Phase 3b: Building magic_enum $MAGIC_ENUM_VERSION with getdeps..."
  preseed_getdeps_download \
    magic_enum \
    "$MAGIC_ENUM_DOWNLOAD_URL" \
    "$MAGIC_ENUM_DOWNLOAD_ARCHIVE" \
    "$MAGIC_ENUM_DOWNLOAD_SHA256"
  build_public_getdeps_target magic_enum "$GETDEPS_CMAKE_DEFINES" \
    || { echo "ERROR: Failed to build magic_enum"; exit 1; }
  MAGIC_ENUM_INSTALL="$(find_getdeps_prefix magic_enum || true)"
  if [ -z "$MAGIC_ENUM_INSTALL" ]; then
    echo "ERROR: magic_enum build finished but the getdeps install was not found"
    exit 1
  fi
fi


# ---------------------------------------------------------------------------
# Phase 3c: Fetch RocksDB source for fb-mysql warm-storage support
# ---------------------------------------------------------------------------
FBMYSQL_OSS_ROCKSDB_SRC="$(find_rocksdb_source || true)"
if [ -n "$FBMYSQL_OSS_ROCKSDB_SRC" ] && [ "$FORCE_REBUILD" = false ]; then
  echo ">>> Phase 3c: Using getdeps RocksDB source: $FBMYSQL_OSS_ROCKSDB_SRC"
else
  echo ">>> Phase 3c: Fetching RocksDB source with getdeps..."
  run_public_getdeps fetch rocksdb \
    --no-facebook-internal \
    --scratch-path "$SCRATCH_DIR" \
    || { echo "ERROR: Failed to fetch RocksDB source"; exit 1; }
  FBMYSQL_OSS_ROCKSDB_SRC="$(find_rocksdb_source || true)"
  if [ -z "$FBMYSQL_OSS_ROCKSDB_SRC" ]; then
    echo "ERROR: getdeps fetch completed, but RocksDB source was not found"
    exit 1
  fi
  echo "    RocksDB source: $FBMYSQL_OSS_ROCKSDB_SRC"
fi

# ---------------------------------------------------------------------------
# Phase 4: Fetch OSS fb-mysql source and Boost cache
# ---------------------------------------------------------------------------
echo ">>> Phase 4: Ensuring OSS fb-mysql source archives..."

if ! download_tarball \
  "fb-mysql-${FBMYSQL_OSS_BRANCH}" \
  "$FBMYSQL_OSS_URL" \
  "$FBMYSQL_OSS_CACHE_DIR" \
  "$FBMYSQL_OSS_ARCHIVE" \
  "SHA256=$FBMYSQL_OSS_SHA256"; then
  echo "ERROR: Failed to download OSS fb-mysql source tarball."
  echo "Manual fallback:"
  echo "  mkdir -p \"$FBMYSQL_OSS_CACHE_DIR\""
  echo "  curl -L \"$FBMYSQL_OSS_URL\" -o \"$FBMYSQL_OSS_CACHE_DIR/$FBMYSQL_OSS_ARCHIVE\""
  exit 1
fi

extract_source_archive \
  "fb-mysql" \
  "$FBMYSQL_OSS_CACHE_DIR/$FBMYSQL_OSS_ARCHIVE" \
  "$FBMYSQL_OSS_SRC_DIR" \
  "CMakeLists.txt"

if ! download_tarball \
  "$FBMYSQL_OSS_BOOST_PACKAGE" \
  "$FBMYSQL_OSS_BOOST_URL" \
  "$FBMYSQL_OSS_CACHE_DIR" \
  "$FBMYSQL_OSS_BOOST_ARCHIVE" \
  "SHA256=$FBMYSQL_OSS_BOOST_SHA256"; then
  echo "ERROR: Failed to download fb-mysql's required Boost source tarball."
  echo "Manual fallback:"
  echo "  mkdir -p \"$FBMYSQL_OSS_CACHE_DIR\""
  echo "  curl -L \"$FBMYSQL_OSS_BOOST_URL\" -o \"$FBMYSQL_OSS_CACHE_DIR/$FBMYSQL_OSS_BOOST_ARCHIVE\""
  exit 1
fi

configure_fbmysql_source

# ---------------------------------------------------------------------------
# Phase 4b: Build producer-capable OSS libdwarf
# ---------------------------------------------------------------------------
echo ">>> Phase 4b: Ensuring producer-capable OSS libdwarf..."
choose_cmake_compilers
echo "    Using $CMAKE_COMPILER_DESCRIPTION"

if ! download_tarball \
  "libdwarf-${LIBDWARF_OSS_VERSION}" \
  "$LIBDWARF_OSS_URL" \
  "$LIBDWARF_OSS_CACHE_DIR" \
  "$LIBDWARF_OSS_ARCHIVE" \
  "SHA256=$LIBDWARF_OSS_SHA256"; then
  echo "ERROR: Failed to download producer-capable OSS libdwarf source."
  echo "Manual fallback:"
  echo "  mkdir -p \"$LIBDWARF_OSS_CACHE_DIR\""
  echo "  curl -L \"$LIBDWARF_OSS_URL\" -o \"$LIBDWARF_OSS_ARCHIVE_PATH\""
  echo "  sha256sum \"$LIBDWARF_OSS_ARCHIVE_PATH\""
  exit 1
fi

extract_source_archive \
  "libdwarf" \
  "$LIBDWARF_OSS_ARCHIVE_PATH" \
  "$LIBDWARF_OSS_SRC_DIR" \
  "configure"
build_autotools_dependency \
  "libdwarf" \
  "$LIBDWARF_OSS_SRC_DIR" \
  "$LIBDWARF_OSS_PREFIX" \
  "lib/libdwarf.a" \
  "include/libdwarf.h"

# ---------------------------------------------------------------------------
# Phase 4c: Seed bundled timelib source cache
# ---------------------------------------------------------------------------
echo ">>> Phase 4c: Ensuring bundled timelib source cache..."

if ! download_tarball \
  "timelib-${TIMELIB_VERSION}" \
  "$TIMELIB_URL" \
  "$TIMELIB_CACHE_DIR" \
  "$TIMELIB_ARCHIVE" \
  "SHA512=$TIMELIB_SHA512"; then
  echo "ERROR: Failed to download bundled timelib source tarball."
  echo "Manual fallback:"
  echo "  mkdir -p \"$TIMELIB_CACHE_DIR\""
  echo "  curl -L \"$TIMELIB_URL\" -o \"$TIMELIB_CACHE_DIR/$TIMELIB_ARCHIVE\""
  exit 1
fi

# ---------------------------------------------------------------------------
# Phase 4e: Build PCRE1 for the Hack OCaml dependencies
# ---------------------------------------------------------------------------
echo ">>> Phase 4e: Ensuring OSS PCRE1..."

if ! download_tarball \
  "pcre-${PCRE1_VERSION}" \
  "$PCRE1_URL" \
  "$DOWNLOAD_CACHE_DIR" \
  "$PCRE1_ARCHIVE" \
  "SHA256=$PCRE1_SHA256"; then
  echo "ERROR: Failed to download OSS PCRE1 source."
  exit 1
fi

extract_source_archive \
  "pcre" \
  "$DOWNLOAD_CACHE_DIR/$PCRE1_ARCHIVE" \
  "$PCRE1_SRC_DIR" \
  "configure"
if [ -f "$PCRE1_PREFIX/include/pcre.h" ] && [ ! -f "$PCRE1_JIT_STAMP" ]; then
  rm -rf "$PCRE1_PREFIX"
fi
build_autotools_dependency \
  "pcre" \
  "$PCRE1_SRC_DIR" \
  "$PCRE1_PREFIX" \
  "lib/libpcre.a" \
  "include/pcre.h" \
  "-fPIC" \
  "--enable-jit --enable-utf --enable-unicode-properties"
touch "$PCRE1_JIT_STAMP"

# ---------------------------------------------------------------------------
# Phase 4f: Build ImageMagick 6 for the legacy Imagick extension API
# ---------------------------------------------------------------------------
echo ">>> Phase 4f: Ensuring OSS ImageMagick 6..."

if ! download_tarball \
  "ImageMagick6-${IMAGEMAGICK6_VERSION}" \
  "$IMAGEMAGICK6_URL" \
  "$DOWNLOAD_CACHE_DIR" \
  "$IMAGEMAGICK6_ARCHIVE" \
  "SHA256=$IMAGEMAGICK6_SHA256"; then
  echo "ERROR: Failed to download OSS ImageMagick 6 source."
  exit 1
fi

extract_source_archive \
  "ImageMagick6" \
  "$DOWNLOAD_CACHE_DIR/$IMAGEMAGICK6_ARCHIVE" \
  "$IMAGEMAGICK6_SRC_DIR" \
  "configure"
if [ -f "$IMAGEMAGICK6_PREFIX/include/ImageMagick-6/wand/MagickWand.h" ] &&
   [ ! -f "$IMAGEMAGICK6_FORMATS_STAMP" ]; then
  rm -rf "$IMAGEMAGICK6_PREFIX"
fi
build_autotools_dependency \
  "ImageMagick6" \
  "$IMAGEMAGICK6_SRC_DIR" \
  "$IMAGEMAGICK6_PREFIX" \
  "lib/libMagickWand-6.Q16.so" \
  "include/ImageMagick-6/wand/MagickWand.h" \
  "" \
  "--enable-shared --disable-static --without-magick-plus-plus --without-perl --without-x"
# The stamp is only absent when the block above wiped an unverified prefix, so
# this re-reads the delegate list exactly once per ImageMagick6 build.
if [ ! -f "$IMAGEMAGICK6_FORMATS_STAMP" ]; then
  for imagemagick6_format in JPEG PNG; do
    if ! imagemagick6_supports_format "$imagemagick6_format"; then
      echo "ERROR: ImageMagick6 was built without required" \
        "$imagemagick6_format format support."
      exit 1
    fi
  done
  touch "$IMAGEMAGICK6_FORMATS_STAMP"
fi

# ---------------------------------------------------------------------------
# Phase 4g: Pin the public opam package index used by Hack
# ---------------------------------------------------------------------------
echo ">>> Phase 4g: Ensuring pinned public opam repository..."

if ! download_tarball \
  "opam-repository-${OPAM_REPOSITORY_REVISION}" \
  "$OPAM_REPOSITORY_URL" \
  "$DOWNLOAD_CACHE_DIR" \
  "$OPAM_REPOSITORY_ARCHIVE" \
  "SHA256=$OPAM_REPOSITORY_SHA256"; then
  echo "ERROR: Failed to download the pinned public opam repository."
  exit 1
fi

extract_source_archive \
  "opam-repository" \
  "$DOWNLOAD_CACHE_DIR/$OPAM_REPOSITORY_ARCHIVE" \
  "$OPAM_REPOSITORY_SRC_DIR" \
  "repo"

# ---------------------------------------------------------------------------
# Phase 5: Build jemalloc 5.3
# ---------------------------------------------------------------------------
JEMALLOC_PREFIX="$HHVM_OSS_WORK_ROOT/installed/jemalloc"
USE_SYSTEM_JEMALLOC=false
if have_system_jemalloc_53; then
  USE_SYSTEM_JEMALLOC=true
fi

if [ "$USE_SYSTEM_JEMALLOC" = true ]; then
  echo ">>> Phase 5: Using system jemalloc >= 5.3. Skipping local build."
elif [ -f "$JEMALLOC_PREFIX/lib/libjemalloc.a" ]; then
  echo ">>> Phase 5: jemalloc 5.3 already built. Skipping."
else
  echo ">>> Phase 5: Building jemalloc 5.3..."
  JEMALLOC_BUILD_DIR="$(mktemp -d)"
  download_tarball "jemalloc-5.3.0" \
    "https://github.com/jemalloc/jemalloc/releases/download/5.3.0/jemalloc-5.3.0.tar.bz2" \
    "$JEMALLOC_BUILD_DIR" \
    "" \
    "SHA256=2db82d1e7119df3e71b7640219b6dfe84789bc0537983c3b7ac4f7189aecfeaa"
  (cd "$JEMALLOC_BUILD_DIR" && \
    tar --no-same-owner -xf jemalloc-5.3.0.tar.bz2 && \
    cd jemalloc-5.3.0 && \
    ./configure --prefix="$JEMALLOC_PREFIX" --disable-shared --enable-static --with-jemalloc-prefix= && \
    make -j"$JOBS" && \
    make install)
  rm -rf "$JEMALLOC_BUILD_DIR"
  echo "    Done."
fi

# ---------------------------------------------------------------------------
# Phase 7: Prepare Rust toolchain and crate cache
# ---------------------------------------------------------------------------
# Install rust nightly to a persistent location (not inside build dir)
RUST_NIGHTLY="2025-05-04"
RUST_PREFIX="$HHVM_OSS_WORK_ROOT/installed/rust-nightly"
if [ -x "$RUST_PREFIX/bin/rustc" ]; then
  echo ">>> Rust nightly already installed. Skipping."
else
  echo ">>> Installing Rust nightly ($RUST_NIGHTLY)..."
  RUST_CACHE="$HHVM_OSS_WORK_ROOT/downloads/rust"
  RUST_FILENAME="rust-nightly-${RUST_TARGET}.tar.gz"
  download_tarball "rust-nightly" \
    "https://static.rust-lang.org/dist/${RUST_NIGHTLY}/$RUST_FILENAME" \
    "$RUST_CACHE" \
    "$RUST_FILENAME" \
    "SHA256=$RUST_SHA256"
  RUST_TMP="$(mktemp -d)"
  tar --no-same-owner -xzf "$RUST_CACHE/$RUST_FILENAME" -C "$RUST_TMP"
  if ! "$RUST_TMP/rust-nightly-${RUST_TARGET}/install.sh" \
      --prefix="$RUST_PREFIX" --without=rust-docs 2>&1; then
    rm -rf "$RUST_TMP"
    echo "ERROR: Rust nightly installer failed"
    exit 1
  fi
  rm -rf "$RUST_TMP"
  if [ ! -x "$RUST_PREFIX/bin/rustc" ]; then
    echo "ERROR: Rust nightly install completed without rustc in $RUST_PREFIX/bin"
    exit 1
  fi
  echo "    Installed: $("$RUST_PREFIX/bin/rustc" --version 2>/dev/null)"
fi

# Pre-fetch Rust crates
RUST_FETCH_STAMP="$HHVM_OSS_WORK_ROOT/.rust-crates-v2"
if [ -f "$RUST_FETCH_STAMP" ]; then
  echo ">>> Phase 7: Rust crates already fetched. Skipping."
else
  echo ">>> Phase 7: Pre-fetching Rust crates..."
  for rust_source in \
    "$SRC_DIR/hphp/hack/src" \
    "$SRC_DIR/hphp/tools/configs"; do
    [ -f "$rust_source/Cargo.toml" ] || continue
    (
      cd "$rust_source"
      PATH="$RUST_PREFIX/bin:$PATH" \
        "$RUST_PREFIX/bin/cargo" fetch --locked --quiet
    )
  done
  touch "$RUST_FETCH_STAMP"
fi

echo ""

# ---------------------------------------------------------------------------
# Phase 8: Configure and build
# ---------------------------------------------------------------------------
PREFIX_PATH=""
if [ -d "$GETDEPS_DIR" ]; then
  for d in "$GETDEPS_DIR"/*/; do
    PREFIX_PATH="${PREFIX_PATH:+$PREFIX_PATH;}$d"
  done
fi
PREFIX_PATH="${PREFIX_PATH:+$PREFIX_PATH;}$LIBURING_PREFIX"
PREFIX_PATH="$PREFIX_PATH;$PCRE1_PREFIX"
PREFIX_PATH="$PREFIX_PATH;$IMAGEMAGICK6_PREFIX"

mkdir -p "$BUILD_DIR"

RECONFIGURE_CMAKE="$FORCE_REBUILD"
cmake_cache_matches() {
  local variable="$1" expected="$2" cache_type

  for cache_type in BOOL FILEPATH PATH STRING UNINITIALIZED; do
    grep -Fqx "$variable:$cache_type=$expected" \
      "$BUILD_DIR/CMakeCache.txt" && return 0
  done
  return 1
}

if [ "$NEED_XED" = true ] && [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   cmake_cache_matches ENABLE_XED OFF; then
  echo ">>> Phase 8: Existing cmake cache has ENABLE_XED=OFF. Reconfiguring with bundled XED."
  RECONFIGURE_CMAKE=true
fi
if [ "$NEED_XED" = true ] && [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   [ ! -f "$SRC_DIR/third-party/xed/xed/build/include/xed/xed-interface.h" ]; then
  echo ">>> Phase 8: Bundled XED headers are not generated yet. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   ! cmake_cache_matches HHVM_OSS_LIBDWARF_ROOT "$LIBDWARF_OSS_PREFIX"; then
  echo ">>> Phase 8: Existing cmake cache does not point at the producer-capable OSS libdwarf. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   grep -Eq '^LIBDWARF_LIBRARIES:FILEPATH=/usr/.*/libdwarf' "$BUILD_DIR/CMakeCache.txt"; then
  echo ">>> Phase 8: Existing cmake cache is still using the system libdwarf. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   { ! cmake_cache_matches SYSTEM_PCRE_INCLUDE_DIR "$PCRE1_PREFIX/include" ||
     ! cmake_cache_matches SYSTEM_PCRE_LIBRARY "$PCRE1_PREFIX/lib/libpcre.a"; }; then
  echo ">>> Phase 8: Existing cmake cache does not point at the JIT-enabled OSS PCRE1. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   ! cmake_cache_matches CMAKE_BUILD_TYPE "$HHVM_CMAKE_BUILD_TYPE"; then
  echo ">>> Phase 8: Existing cmake cache uses a different CMAKE_BUILD_TYPE. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   ! cmake_cache_matches "$HHVM_CMAKE_ARCHITECTURE" ON; then
  echo ">>> Phase 8: Existing cmake cache does not select the host architecture. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   { ! cmake_cache_matches CMAKE_C_COMPILER "$CMAKE_C_COMPILER_PATH" ||
     ! cmake_cache_matches CMAKE_CXX_COMPILER "$CMAKE_CXX_COMPILER_PATH" ||
     ! cmake_cache_matches CMAKE_TOOLCHAIN_FILE "$CMAKE_TOOLCHAIN_FILE_PATH"; }; then
  echo ">>> Phase 8: Existing cmake cache uses a different compiler toolchain. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   ! cmake_cache_matches AWK_EXECUTABLE "$GAWK_EXECUTABLE"; then
  echo ">>> Phase 8: Existing cmake cache does not point at GNU awk. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   ! cmake_cache_matches ENABLE_LD_GOLD OFF; then
  echo ">>> Phase 8: Existing cmake cache still enables the gold linker. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   { ! cmake_cache_matches LIBMAGICKWAND_LIBRARIES "$IMAGEMAGICK6_WAND_LIBRARY" ||
     ! cmake_cache_matches LIBMAGICKCORE_LIBRARIES "$IMAGEMAGICK6_CORE_LIBRARY"; }; then
  echo ">>> Phase 8: Existing cmake cache does not point at OSS ImageMagick 6. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   ! cmake_cache_matches \
     HHVM_THIRD_PARTY_SOURCE_CACHE_PREFIX "$THIRD_PARTY_SOURCE_CACHE_PREFIX"; then
  echo ">>> Phase 8: Existing cmake cache does not point at the local third-party source cache. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   ! cmake_cache_matches CMAKE_DISABLE_FIND_PACKAGE_libWatchmanClient ON; then
  echo ">>> Phase 8: Existing cmake cache can still use a system libWatchmanClient. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && \
   { ! cmake_cache_matches RUSTC_EXE "$RUST_PREFIX/bin/rustc" ||
     ! cmake_cache_matches CARGO_EXE "$RUST_PREFIX/bin/cargo"; }; then
  echo ">>> Phase 8: Existing cmake cache does not point at the OSS Rust toolchain. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && [ ! -f "$BUILD_DIR/Makefile" ] && \
   [ ! -f "$BUILD_DIR/build.ninja" ]; then
  echo ">>> Phase 8: Existing cmake configuration is incomplete. Reconfiguring."
  RECONFIGURE_CMAKE=true
fi

# Check if cmake already configured and we're not forcing rebuild
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && [ "$RECONFIGURE_CMAKE" = false ]; then
  echo ">>> Phase 8: cmake already configured. Use --rebuild to re-run configure."
fi

if [ ! -f "$BUILD_DIR/CMakeCache.txt" ] || [ "$RECONFIGURE_CMAKE" = true ]; then
  echo ""
  echo ">>> Running cmake configure..."
  if [ "$FORCE_REBUILD" = true ]; then
    rm -rf "${BUILD_DIR:?}/"*
  fi
  cd "$BUILD_DIR"
  choose_cmake_compilers
  echo "    Using $CMAKE_COMPILER_DESCRIPTION"
  echo "    HHVM build type: $HHVM_CMAKE_BUILD_TYPE"

  JEMALLOC_CMAKE_FLAGS=()
  if [ "$USE_SYSTEM_JEMALLOC" = false ]; then
    JEMALLOC_CMAKE_FLAGS=(
      -DJEMALLOC_INCLUDE_DIR="$JEMALLOC_PREFIX/include"
      -DJEMALLOC_LIB="$JEMALLOC_PREFIX/lib/libjemalloc_pic.a"
    )
  fi

  XED_CMAKE_FLAGS=()
  if [ "$NEED_XED" = true ]; then
    XED_CMAKE_FLAGS=(-DENABLE_XED=ON)
  fi

  cmake "$SRC_DIR" \
    -DCMAKE_BUILD_TYPE="$HHVM_CMAKE_BUILD_TYPE" \
    -D"$HHVM_CMAKE_ARCHITECTURE"=ON \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    "${CMAKE_COMPILER_FLAGS[@]}" \
    -DCMAKE_PREFIX_PATH="$PREFIX_PATH" \
    -DCMAKE_MODULE_PATH="$SRC_DIR/CMake;$GETDEPS_REAL_ROOT/CMake" \
    -DAWK_EXECUTABLE="$GAWK_EXECUTABLE" \
    -DCMAKE_DISABLE_FIND_PACKAGE_libWatchmanClient=ON \
    -DENABLE_EXTENSION_MCRYPT=OFF \
    -DENABLE_LD_GOLD=OFF \
    -DHHVM_THIRD_PARTY_SOURCE_CACHE_PREFIX="$THIRD_PARTY_SOURCE_CACHE_PREFIX" \
    -DHHVM_OSS_LIBDWARF_ROOT="$LIBDWARF_OSS_PREFIX" \
    -DHHVM_OSS_FBMYSQL_SOURCE_DIR="$FBMYSQL_OSS_SRC_DIR" \
    -DHHVM_OSS_FBMYSQL_BOOST_ROOT="$FBMYSQL_OSS_CACHE_DIR" \
    -DHHVM_OSS_TIMELIB_CACHE="$TIMELIB_CACHE_DIR/$TIMELIB_ARCHIVE" \
    -DLIBMAGICKWAND_INCLUDE_DIRS="$IMAGEMAGICK6_INCLUDE_DIRS" \
    -DLIBMAGICKWAND_LIBRARIES="$IMAGEMAGICK6_WAND_LIBRARY" \
    -DLIBMAGICKCORE_LIBRARIES="$IMAGEMAGICK6_CORE_LIBRARY" \
    -DMYSQL_UNIX_SOCK_ADDR=/dev/null \
    -DFORCE_BUNDLED_LZ4=OFF \
    "${JEMALLOC_CMAKE_FLAGS[@]}" \
    -DRUSTC_EXE="$RUST_PREFIX/bin/rustc" \
    -DCARGO_EXE="$RUST_PREFIX/bin/cargo" \
    "${XED_CMAKE_FLAGS[@]}" \
    -Wno-dev \
    2>&1 | tee "$BUILD_DIR/cmake.log"

  cmake_status="${PIPESTATUS[0]}"
  if [ "$cmake_status" -ne 0 ]; then
    echo "ERROR: cmake configure failed. See $BUILD_DIR/cmake.log"
    exit 1
  fi
fi

cd "$BUILD_DIR"

echo ""
echo ">>> Building HHVM target: $HHVM_BUILD_TARGET"
if run_hhvm_build "$BUILD_DIR"; then
  build_status=0
else
  build_status="$?"
fi
show_build_result "$BUILD_DIR" "$build_status"

exit "$build_status"
