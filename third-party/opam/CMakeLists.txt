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
  "https://github.com/ocaml/opam/releases/download/2.0.7/opam-2.0.7-x86_64-linux"
  Darwin_URL
  "https://github.com/ocaml/opam/releases/download/2.0.7/opam-2.0.7-x86_64-macos"
  Linux_HASH
  "SHA512=da75b0cb5ad50f95d31857a7d72f1836132a1fa1cdbfdedf684342b798e7107b4add4c74c05d5ce44881309fa1e57707538dbcda874e7f74b269b1bb204f3ae3"
  Darwin_HASH
  "SHA512=de1194c8e97e53956e5e47502c28881bbf26d1beaac4f33a43a922b8ca7ce97725533cfaf65a33fc0e183eab5a95e9ecd2e20f72faeaec333dc3850b79b5fe8a"
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
