# Generate all-scan.h and all-scan-decl.h header files.
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/all-scan.h ${CMAKE_CURRENT_SOURCE_DIR}/all-scan-decl.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} -E touch all-scan.h
  COMMAND ${CMAKE_COMMAND} -E touch all-scan-decl.h
  VERBATIM)

add_custom_target(
  all-scan-header
  ALL
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/all-scan.h ${CMAKE_CURRENT_SOURCE_DIR}/all-scan-decl.h)
