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

set(RUST_NIGHTLY_VERSION "2021-11-29")

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  RUST_DOWNLOAD_ARGS
  Linux_URL
  "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-unknown-linux-gnu.tar.gz"
  Darwin_URL
  "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-apple-darwin.tar.gz"
  Linux_HASH
  "SHA512=a7ec879851bd1bb8bf57b77860c4d99b0b76702182782520ab5f5053b6b113f3a0890c1e8210f8c010f43ea0804abdeeb90422534498e47178cad3643ddcca32"
  Darwin_HASH
  "SHA512=b5655f92605e9a69b3ce49e73b92a960c4bc9bcb34edcef0d6b4829cec7258b0d00f398f41b0ce1fc399fe3cc35c91ca347eb2e0fe6b853aa94167f6dcf58697"
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
