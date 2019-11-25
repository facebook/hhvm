include(HHVMRustVersion)
set(RUST_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/rust-opt")

function(download_and_extract OUT_DIR_VARIABLE URL SHA256)
  get_filename_component(TARBALL "${URL}" NAME)
  string(REPLACE ".tar.gz" "" DIR "${TARBALL}")
  set(TARBALL "${CMAKE_CURRENT_BINARY_DIR}/${TARBALL}")
  set(OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${DIR}")
  set(${OUT_DIR_VARIABLE} "${OUT_DIR}" PARENT_SCOPE)

  set(STAMP_FILE "${OUT_DIR}/${SHA256}.stamp")
  if (EXISTS "${STAMP_FILE}")
    return()
  endif()

  message(STATUS "Fetching ${URL}...")
  file(DOWNLOAD "${URL}" "${TARBALL}" EXPECTED_HASH "SHA256=${SHA256}")
  message(STATUS "... extracting ${DIR}/")
  execute_process(
    COMMAND "tar" "zxf" "${TARBALL}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )
  message(STATUS "... extracted ${DIR}/")

  execute_process(COMMAND cmake -E touch "${STAMP_FILE}")
endfunction()

download_and_extract(RUST_DIR "${RUST_URL}" "${RUST_SHA256}")
set(STAMP_FILE "${CMAKE_CURRENT_BINARY_DIR}/install-${RUST_SHA256}.stamp")
if(NOT EXISTS "${STAMP_FILE}")
  message(STATUS "Running rust install.sh...")
  execute_process(
    COMMAND "${RUST_DIR}/install.sh" "--prefix=${RUST_PREFIX}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )
  execute_process(COMMAND cmake -E touch "${STAMP_FILE}")
else()
  message(STATUS "Not re-installing rust, stamp file present.")
endif()

set(RUSTC_EXECUTABLE "${RUST_PREFIX}/bin/rustc" CACHE FILEPATH "rustc executable")
set(CARGO_EXECUTABLE "${RUST_PREFIX}/bin/cargo" CACHE FILEPATH "cargo executable")
