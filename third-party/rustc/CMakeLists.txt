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
  "https://static.rust-lang.org/dist/rust-1.43.0-x86_64-unknown-linux-gnu.tar.gz"
  Darwin_URL
  "https://static.rust-lang.org/dist/rust-1.43.0-x86_64-apple-darwin.tar.gz"
  Linux_HASH
  "SHA512=f2118652dc4f5d4353a693ec277bfcce649c493ffd0cff4a4992ca0bbddeb49de5f1cb99b4f93b6d271edfa0dae40dd052a52a548dda3a32addbed738cdb86cc"
  Darwin_HASH
  "SHA512=e1b3b1c41131af65f0487c742c0085b589e7c0eb73ce7ca4a38d30c9ce1af4d04686a141dde8cbaa7afc56bcb670e4d6f9b8b2dbad4700b89c5db371829a4ec4"
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
