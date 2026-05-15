# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

proxygen_add_test(TARGET StructuredHeadersTests
  SOURCES
    StructuredFieldsRfc9651Test.cpp
    StructuredHeadersBufferTest.cpp
    StructuredHeadersDecoderTest.cpp
    StructuredHeadersEncoderTest.cpp
    StructuredHeadersStandardTest.cpp
    StructuredHeadersUtilitiesTest.cpp
  DEPENDS
    proxygen
    testmain
)

if(TARGET StructuredHeadersTests)
  set(STRUCTURED_FIELD_TEST_RESOURCE_DEST
    "$<TARGET_FILE_DIR:StructuredHeadersTests>/proxygen/lib/http/structuredheaders/test"
  )
  add_custom_command(TARGET StructuredHeadersTests POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E make_directory
      "${STRUCTURED_FIELD_TEST_RESOURCE_DEST}"
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/structured-field-tests"
      "${STRUCTURED_FIELD_TEST_RESOURCE_DEST}/structured-field-tests"
    COMMENT "Copying Structured Field test fixtures"
    VERBATIM
  )
endif()
