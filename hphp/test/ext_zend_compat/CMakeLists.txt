#
# See http://www.cmake.org/cmake/help/v3.0/command/add_custom_command.html
# See http://www.cmake.org/cmake/help/v3.0/command/add_custom_target.html
#
set(HPHPIZE_EXE "../../tools/hphpize/hphpize")

if (LINUX)
  #
  # See http://www.cmake.org/Wiki/CMake_FAQ#How_do_I_generate_an_executable.2C_then_use_the_executable_to_generate_a_file.3F
  #
  add_custom_target(
    build_test_zend_dso
    ALL
    DEPENDS hello/hello.so
  )

  add_custom_command(
    OUTPUT hello/hello.so

    #
    # The output from hphpize and cmake is just discarded;
    # the presumption is that it has been debugged and "just works".
    #
    COMMAND /bin/sh ../${HPHPIZE_EXE} > /dev/null 2>&1
    COMMAND cmake .                   > /dev/null 2>&1
    COMMAND make

    WORKING_DIRECTORY hello

    COMMENT "Making test/ext_zend_compat/hello/hello.so"
    DEPENDS "${HPHPIZE_EXE}"
  )

endif()
