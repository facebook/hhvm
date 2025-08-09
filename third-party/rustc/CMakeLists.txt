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

  set(RUST_NIGHTLY_VERSION "2025-01-22")

  SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
    RUST_DOWNLOAD_ARGS
    Linux_URL
    "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-unknown-linux-gnu.tar.gz"
    Darwin_URL
    "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-apple-darwin.tar.gz"
    Linux_HASH
    "SHA512=0099c08475017f469321bfe697c2c36a099f087068e3f9c84d8b4894a08e7711e415067b17b2e44b7c2bea13c198c2d32f675c0f1e56292f4d064c2e37ffd4e0"
    Darwin_HASH
    "SHA512=16f444f63b6b74f1f4a9494a95e9a40d602761a8741139c5bcbed359a6bc0cfe46b90e79b1c62072286f6354a09c7aa75d2ce7b40af69538e9bec2b6bfb65dd9"
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
