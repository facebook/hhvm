# When updating:
# - verify the gpg signature (fooasc) against key
#   1958 E634 BA58 0D63 9E4B  1EF6 AC26 8762 CB21 AF4D
#   available from https://keybase.io/altgr/pgp_keys.asc
# - generate the hash with `openssl dgst -sha512 foo'
#
# We separately store the hash to be sure we're getting the exact same
# build, not just any tarball.
#
# This also avoids the need to depend on gpg in the installation.
include(HPHPFunctions)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  OPAM_DOWNLOAD_ARGS
  Linux_URL
  "https://github.com/ocaml/opam/releases/download/2.1.0/opam-2.1.0-x86_64-linux"
  Darwin_URL
  "https://github.com/ocaml/opam/releases/download/2.1.0/opam-2.1.0-x86_64-macos"
  Linux_HASH
  "SHA512=03c6a85f13a452749fdb2271731f3624a3993498ff2b304123231a8f2b26ccf1182d12119466e9a85f4de370fca51bd61d0eefe6280d3ca087cf4620fdc59a22"
  Darwin_HASH
  "SHA512=1c9acee545c851dd3701229e3a6aa7b5650620e37e01400d797a4b1fbeeb614adc459411283684e223a72fda8b14ba6c6e5482661485f888819f6a2a02e4d279"
)

include(ExternalProject)
ExternalProject_Add(
  opamDownload
  ${OPAM_DOWNLOAD_ARGS}
  DOWNLOAD_NO_EXTRACT ON
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND
  install -m755 "<DOWNLOADED_FILE>" "<INSTALL_DIR>/opam"
)

ExternalProject_Get_Property(opamDownload INSTALL_DIR)
get_filename_component(OPAM_EXECUTABLE "${INSTALL_DIR}/opam" ABSOLUTE)

add_executable(opam IMPORTED GLOBAL)
add_dependencies(opam opamDownload)
set_property(TARGET opam PROPERTY IMPORTED_LOCATION "${OPAM_EXECUTABLE}")
