# When updating:
# - verify the gpg signature (foo.tar.gz.asc) against key with fingerprint
#   108F 6620 5EAE B0AA A8DD  5E1C 85AB 96E6 FA1B E5FE
#   (link to raw key: https://static.rust-lang.org/rust-key.gpg.ascii)
# - generate the sha512 with `openssl dgst -sha512 foo.tar.gz`
#
# We separately store the sha512 to be sure we're getting the exact same
# build, not just any tarball.
#
# This also avoids the need to depend on gpg in the installation.

include(HPHPFunctions)

option(RUSTC_EXECUTABLE "The full path of rustc executable")
option(CARGO_EXECUTABLE "The full path of cargo executable")

add_executable(rustc IMPORTED GLOBAL)
add_executable(cargo IMPORTED GLOBAL)

if(RUSTC_EXECUTABLE)
  if(CARGO_EXECUTABLE)
    message(STATUS "Using specified Rust")
    set_property(TARGET rustc PROPERTY IMPORTED_LOCATION "${RUSTC_EXECUTABLE}")
    set_property(TARGET cargo PROPERTY IMPORTED_LOCATION "${CARGO_EXECUTABLE}")
  else()
    message(FATAL_ERROR "CARGO_EXECUTABLE must be defined if RUSTC_EXECUTABLE is defined")
  endif()
else()
  message(STATUS "Using bundled Rust")

  set(RUST_NIGHTLY_VERSION "2024-10-13")

  SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
    RUST_DOWNLOAD_ARGS
    Linux_URL
    "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-unknown-linux-gnu.tar.gz"
    Darwin_URL
    "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-apple-darwin.tar.gz"
    Linux_HASH
    "SHA512=120f7eb6b55951785b22b6ec6dd4456873f4fd1a78387e61a56784d5130be82fe97d4e644459c92f8c79766e083dff6eb8365562bf7202c7aa891977d94ef127"
    Darwin_HASH
    "SHA512=7f58dc3ccc399c9fe969ba586e7cad907842d3b66e80dad62a733ce6dbb3daf922f7371a3042e4d521ea9521e7e8153c5aa32ddd1d5a84bef84ccc008fea028a"
    # The original filename doesn't contain any version information, so add the version information as a prefix to avoid cache collisions when updating later
    FILENAME_PREFIX "rustc-${RUST_NIGHTLY_VERSION}-"
  )

  include(ExternalProject)
  ExternalProject_Add(
    bundled_rust
    ${RUST_DOWNLOAD_ARGS}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND
    # rust-docs aren't needed, and installing them takes more time than installing
    # everything else and building the rust parts of hack combined
    "<SOURCE_DIR>/install.sh" "--prefix=<INSTALL_DIR>" --without=rust-docs
  )

  add_dependencies(rustc bundled_rust)
  add_dependencies(cargo bundled_rust)

  ExternalProject_Get_Property(bundled_rust INSTALL_DIR)
  set_property(TARGET rustc PROPERTY IMPORTED_LOCATION "${INSTALL_DIR}/bin/rustc")
  set_property(TARGET cargo PROPERTY IMPORTED_LOCATION "${INSTALL_DIR}/bin/cargo")
endif()
