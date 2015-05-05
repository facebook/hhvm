# Generate all-scan.h header file
add_custom_target(ALL_SCAN_HEADER ALL
  COMMAND ${CMAKE_COMMAND} -E touch all-scan.h
  VERBATIM)
