#
# Build a DSO in dso_test, only to be used for testing
# on linux of zend extensions loaded dynamically through a DSO.
#
# See http://www.cmake.org/cmake/help/v3.0/command/add_custom_command.html
# See http://www.cmake.org/cmake/help/v3.0/command/add_custom_target.html
#
if (LINUX)
  set(HPHPIZE_EXE "../../../tools/hphpize/hphpize")
  set(HPHPIZE_CMAKE "../../../tools/hphpize/hphpize.cmake")
  set(HPHPIZE_MODULE_PATH "../../../../CMake")
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

    WORKING_DIRECTORY dso_test

    COMMENT "Making dso_test.so"
  )

endif()
