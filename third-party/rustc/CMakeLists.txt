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
  "https://static.rust-lang.org/dist/rust-1.51.0-x86_64-unknown-linux-gnu.tar.gz"
  Darwin_URL
  "https://static.rust-lang.org/dist/rust-1.51.0-x86_64-apple-darwin.tar.gz"
  Linux_HASH
  "SHA512=08230543a78082a758f43d5cd70a9c7330a1132af394e3191db9cb64cb61462ec2ce5af6b81a719a2059662f11c08586a241aa0048e5ca8f729c9daf550b57fd"
  Darwin_HASH
  "SHA512=e5241828e524cb849c0496c55df46bb51d96ef5f934f763d98b4ed723d18fc888318e3d82771970502672d24dda1c25f6af86d46b6682bea1bab3400327766d2"
)

include(ExternalProject)
ExternalProject_Add(
  bundled_rust
  ${RUST_DOWNLOAD_ARGS}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND
  "<SOURCE_DIR>/install.sh" "--prefix=<INSTALL_DIR>"
)

add_executable(rustc IMPORTED GLOBAL)
add_executable(cargo IMPORTED GLOBAL)
add_dependencies(rustc bundled_rust)
add_dependencies(cargo bundled_rust)

ExternalProject_Get_Property(bundled_rust INSTALL_DIR)
set_property(TARGET rustc PROPERTY IMPORTED_LOCATION "${INSTALL_DIR}/bin/rustc")
set_property(TARGET cargo PROPERTY IMPORTED_LOCATION "${INSTALL_DIR}/bin/cargo")
