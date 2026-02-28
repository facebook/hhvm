# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

function(proxygen_add_fuzzer)
    if(NOT BUILD_FUZZERS)
        return()
    endif()

    set(options)
    set(one_value_args TARGET WORKING_DIRECTORY PREFIX)
    set(multi_value_args SOURCES DEPENDS INCLUDES EXTRA_ARGS)
    cmake_parse_arguments(PARSE_ARGV 0 PROXYGEN_FUZZ "${options}" "${one_value_args}" "${multi_value_args}")

    # Remove any references to the main we have from gmock
    # So we can use the one provided by the fuzzing engine
    set(LIBGMOCK_LIBRARIES_NO_MAIN ${LIBGMOCK_LIBRARIES})
    list(REMOVE_ITEM
         LIBGMOCK_LIBRARIES_NO_MAIN
         "${GTEST_BINARY_DIR}/${CMAKE_CFG_INTDIR}/googlemock/${CMAKE_STATIC_LIBRARY_PREFIX}gmock_main${CMAKE_STATIC_LIBRARY_SUFFIX}"
         gmock_main
         gmock_maind
         GTest::gmock_main
    )

    if(NOT PROXYGEN_FUZZ_TARGET)
      message(FATAL_ERROR "The TARGET parameter is mandatory.")
    endif()

    if(NOT PROXYGEN_FUZZ_SOURCES)
      set(PROXYGEN_FUZZ_SOURCES "${PROXYGEN_FUZZ_TARGET}.cpp")
    endif()

    add_executable(${PROXYGEN_FUZZ_TARGET}
      "${PROXYGEN_FUZZ_SOURCES}"
    )
    target_link_libraries(${PROXYGEN_FUZZ_TARGET} PRIVATE
      "${PROXYGEN_FUZZ_DEPENDS}"
    )
    target_include_directories(${PROXYGEN_FUZZ_TARGET} PRIVATE
      "${PROXYGEN_FUZZ_INCLUDES}"
    )
    target_compile_options(${PROXYGEN_FUZZ_TARGET} PRIVATE
      "${_PROXYGEN_COMMON_COMPILE_OPTIONS}"
    )
    target_link_libraries(${PROXYGEN_FUZZ_TARGET} PRIVATE
      ${LIBGMOCK_LIBRARIES_NO_MAIN}
      ${LIB_FUZZING_ENGINE}
    )
    target_include_directories(${PROXYGEN_FUZZ_TARGET} SYSTEM PRIVATE
      ${LIBGMOCK_INCLUDE_DIR}
      ${LIBGTEST_INCLUDE_DIR}
    )
    target_compile_definitions(${PROXYGEN_FUZZ_TARGET} PRIVATE ${LIBGMOCK_DEFINES})
endfunction()

proxygen_add_fuzzer(TARGET ProxygenHTTP1xFuzzer
  SOURCES
    ProxygenHTTP1xFuzzer.cpp
  DEPENDS
    codectestutils
    proxygen
)
