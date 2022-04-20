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

set(RUST_NIGHTLY_VERSION "2022-02-24")

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  RUST_DOWNLOAD_ARGS
  Linux_URL
  "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-unknown-linux-gnu.tar.gz"
  Darwin_URL
  "https://static.rust-lang.org/dist/${RUST_NIGHTLY_VERSION}/rust-nightly-x86_64-apple-darwin.tar.gz"
  Linux_HASH
  "SHA512=79e4f5a81b668fe5718b5491eb9acfd363067ab0d1220af27b3aef7b6490f291a05bd29ed761f19786e87d4ff6a63b07194f2de2f81d2631c13ba4848ab7df43"
  Darwin_HASH
  "SHA512=d44aa4da10736dd43eac32794a46cd50b8aff06b2c66cb756fdccfa828773de755a11892b27a8256ec89fe65393948abfda55880c62aa3b5225ed0d5f56f7935"
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

add_executable(rustc IMPORTED GLOBAL)
add_executable(cargo IMPORTED GLOBAL)
add_dependencies(rustc bundled_rust)
add_dependencies(cargo bundled_rust)

ExternalProject_Get_Property(bundled_rust INSTALL_DIR)
set_property(TARGET rustc PROPERTY IMPORTED_LOCATION "${INSTALL_DIR}/bin/rustc")
set_property(TARGET cargo PROPERTY IMPORTED_LOCATION "${INSTALL_DIR}/bin/cargo")
