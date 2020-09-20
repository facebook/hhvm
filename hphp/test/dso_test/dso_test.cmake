#
# Build a DSO in dso_test, only to be used for testing
# on linux of zend extensions loaded dynamically through a DSO.
#
# See http://www.cmake.org/cmake/help/v3.0/command/add_custom_command.html
# See http://www.cmake.org/cmake/help/v3.0/command/add_custom_target.html
#
if (LINUX)
  if (NOT "${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    # As we're generating cmake files (when calling hphpize), we can't do a
    # standard out-of-source build.
    #
    # Also, as this is included from the root cmake, rather than added with
    # add_directory(), the '_CURRENT' vars are set to the root dir, so not
    # useful
    set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/hphp/test/dso_test")
    FILE(
      GLOB FILE_LIST
0
      "${SOURCE_DIR}/*.cpp"
      "${SOURCE_DIR}/config.cmake"
      "${SOURCE_DIR}/*.php"
    )
    FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/hphp/test/dso_test")
    FILE(COPY ${FILE_LIST} DESTINATION "${CMAKE_BINARY_DIR}/hphp/test/dso_test")
  endif()
  set(HPHPIZE_EXE "${CMAKE_BINARY_DIR}/hphp/tools/hphpize/hphpize")
  set(HPHPIZE_CMAKE "${CMAKE_BINARY_DIR}/hphp/tools/hphpize/hphpize.cmake")
  set(HPHPIZE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
  add_custom_target(
    dso_test
    DEPENDS dso_test.so hhvm
  )

  add_custom_command(
    OUTPUT dso_test.so

    #
    # The output from hphpize and cmake is just discarded;
    # the presumption is that it has been debugged and "just works".
    #
    COMMAND chmod 755 ${HPHPIZE_EXE}
    COMMAND HHVM_HPHPIZE_CMAKE=${HPHPIZE_CMAKE}
            /bin/sh ${HPHPIZE_EXE} > /dev/null
    COMMAND cmake -DHHVM_DSO_TEST_MODE=On . > /dev/null

    COMMAND make

    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/hphp/test/dso_test"

    COMMENT "Making dso_test.so"
    DEPENDS ${HPHPIZE_CMAKE} ${HPHPIZE_EXE}
  )

endif()
