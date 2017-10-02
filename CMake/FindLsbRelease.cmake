set(LSB_RELEASE_FOUND FALSE)
mark_as_advanced(LSB_RELEASE_FOUND)

if(LINUX)
  find_program(LSB_RELEASE_EXECUTABLE lsb_release DOC "path to lsb_release")
  mark_as_advanced(LSB_RELEASE_EXECUTABLE)

  if(LSB_RELEASE_EXECUTABLE)
    set(LSB_RELEASE_FOUND TRUE)
    execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -is
      OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Found lsb_release: ${LSB_RELEASE_EXECUTABLE}")
  else()
    message(STATUS "Could not find lsb_release, consider installing it")
  endif()
endif()
