# When updating:
# - verify the gpg signature (foo.tar.gz.asc) against key
#   C13466B7E169A085188632165CB4A9347B3B09DC
# - generate the sha256 with `openssl dgst -sha256 foo.tar.gz`
#
# We separately store the sha256 to be sure we're getting the exact same
# build, not just any tarball.
#
# This also avoids the need to depend on gpg in the installation.

include(HPHPFunctions)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  RUST_DOWNLOAD_ARGS
  Linux_URL
  "https://static.rust-lang.org/dist/rust-1.39.0-x86_64-unknown-linux-gnu.tar.gz"
  Darwin_URL
  "https://static.rust-lang.org/dist/rust-1.39.0-x86_64-apple-darwin.tar.gz"
  Linux_HASH
  "SHA256=b10a73e5ba90034fe51f0f02cb78f297ed3880deb7d3738aa09dc5a4d9704a25"
  Darwin_HASH
  "SHA256=3736d49c5e9592844e1a5d5452883aeaf8f1e25d671c1bc8f01e81c1766603b5"
)

set(RUST_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/rust-prefix")

include(ExternalProject)
ExternalProject_Add(
  rustInstall
  ${RUST_DOWNLOAD_ARGS}
  PREFIX "${RUST_PREFIX}"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND
  "${RUST_PREFIX}/src/rustInstall/install.sh" "--prefix=${RUST_PREFIX}"
)

add_executable(rustc IMPORTED GLOBAL)
add_executable(cargo IMPORTED GLOBAL)
add_dependencies(rustc rustInstall)
add_dependencies(cargo rustInstall)
set_property(TARGET rustc PROPERTY IMPORTED_LOCATION "${RUST_PREFIX}/bin/rustc")
set_property(TARGET cargo PROPERTY IMPORTED_LOCATION "${RUST_PREFIX}/bin/cargo")
