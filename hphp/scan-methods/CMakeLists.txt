# Generate all-scan.h and all-scan-decl.h header files.
add_custom_target(ALL_SCAN_HEADER ALL
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} -E touch all-scan.h
  COMMAND ${CMAKE_COMMAND} -E touch all-scan-decl.h
  VERBATIM)
