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

set(RUST_NIGHTLY_VERSION "2021-10-21")

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  RUST_DOWNLOAD_ARGS
  Linux_URL
  "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-unknown-linux-gnu.tar.gz"
  Darwin_URL
  "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-apple-darwin.tar.gz"
  Linux_HASH
  "SHA512=84b38862809f5ff000a4ed86c8a2738ae255dc0c6ed8c9de68e5a9e8e651d046595a595641b9a8d9a5ac7711f4a04ac9f8a1cbb2723b56da6bbb972d2a844d9c"
  Darwin_HASH
  "SHA512=1ecf81b997cd200c9c11689528c507ad87a37918d5b5476129bb83c7f83b38464a9b4bb04f76b841b9a783de31b10623206fd6fdbeeb6a918df8b4d59c1f39a9"
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
  "<SOURCE_DIR>/install.sh" "--prefix=<INSTALL_DIR>"
)

add_executable(rustc IMPORTED GLOBAL)
add_executable(cargo IMPORTED GLOBAL)
add_dependencies(rustc bundled_rust)
add_dependencies(cargo bundled_rust)

ExternalProject_Get_Property(bundled_rust INSTALL_DIR)
set_property(TARGET rustc PROPERTY IMPORTED_LOCATION "${INSTALL_DIR}/bin/rustc")
set_property(TARGET cargo PROPERTY IMPORTED_LOCATION "${INSTALL_DIR}/bin/cargo")
