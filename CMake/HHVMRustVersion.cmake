# When updating:
# - verify the gpg signature (foo.tar.gz.asc) against key
#   C13466B7E169A085188632165CB4A9347B3B09DC
# - generate the sha256 with `openssl dgst -sha256 foo.tar.gz`
#
# We separately store the sha256 to be sure we're getting the exact same
# build, not just any tarball.
#
# This also avoids the need to depend on gpg in the installation.

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(RUST_URL "https://static.rust-lang.org/dist/rust-1.36.0-x86_64-unknown-linux-gnu.tar.gz")
  set(RUST_SHA256 "15e592ec52f14a0586dcebc87a957e472c4544e07359314f6354e2b8bd284c55")
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  set(RUST_URL "https://static.rust-lang.org/dist/rust-1.36.0-x86_64-apple-darwin.tar.gz")
  set(RUST_SHA256 "91f151ec7e24f5b0645948d439fc25172ec4012f0584dd16c3fb1acb709aa325")
else()
  message(FATAL_ERROR "HHVM does not support this operating system")
endif()

set(RUST_URL "${RUST_URL}" PARENT_SCOPE)
set(RUST_SHA256 "${RUST_SHA256}" PARENT_SCOPE)
