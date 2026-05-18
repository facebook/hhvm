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
find_program(SYSTEM_OPAM opam)
if(SYSTEM_OPAM)
  message(STATUS "Using system opam: ${SYSTEM_OPAM}")
  add_executable(opam IMPORTED GLOBAL)
  set_property(TARGET opam PROPERTY IMPORTED_LOCATION "${SYSTEM_OPAM}")
  return()
endif()

include(HPHPFunctions)

if (NOT "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
  message(FATAL_ERROR "Bundled opam in the OSS build is only supported on Linux")
endif()

if (IS_X64)
  set(OPAM_ARCH "x86_64")
  set(OPAM_LINUX_HASH "03c6a85f13a452749fdb2271731f3624a3993498ff2b304123231a8f2b26ccf1182d12119466e9a85f4de370fca51bd61d0eefe6280d3ca087cf4620fdc59a22")
elseif (IS_AARCH64)
  set(OPAM_ARCH "arm64")
  set(OPAM_LINUX_HASH "216185106deb81db0e9cb329dd7f01d097173e1e7a055a1af8525cdb4dde6d443e4bf4ef8377f1cbd4c9fecdc7ea03e6f294dad30b10a0e83959476018e24972")
endif()

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  OPAM_DOWNLOAD_ARGS
  SOURCE_URL
  "https://github.com/ocaml/opam/releases/download/2.1.0/opam-2.1.0-${OPAM_ARCH}-linux"
  SOURCE_HASH
  "SHA512=${OPAM_LINUX_HASH}"
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
