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

set(RUST_NIGHTLY_VERSION "2021-06-15")

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  RUST_DOWNLOAD_ARGS
  Linux_URL
  "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-unknown-linux-gnu.tar.gz"
  Darwin_URL
  "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-apple-darwin.tar.gz"
  Linux_HASH
  "SHA512=bff0d9c6e4e718784b6fd3598b86da0053a4126979f5a4b855239ef6c66f61cae0117945dfbcafa623519e631931a98dc34d223e2e02f1942e6c12d3fefc0cf0"
  Darwin_HASH
  "SHA512=1e91deac08e473330b095512b8333915052d9cb6e9101809164aac52b9f913fd451cf8b9b385240b296082a26cf97f3b3b13a96dd837d6cb8abeee5281b72363"
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
