# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include(GoogleTest)

# Generate required Thrift dependencies.
foreach (thrift_file Module CompactProtocolTestStructs)
  thrift_generate(
     "${thrift_file}" #file_name
     "" #services
     "cpp2" #language
     "frozen2,layouts" #options
     "${CMAKE_CURRENT_SOURCE_DIR}" #file_path
     "${CMAKE_CURRENT_BINARY_DIR}" #output_path
     "thrift/lib/cpp2/protocol/test" #include_prefix
     THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
  )
  set_source_files_properties(${thrift_file} PROPERTIES GENERATED TRUE)
  set(GENERATED_THRIFT_SOURCES ${GENERATED_THRIFT_SOURCES}
      ${${thrift_file}-cpp2-SOURCES})
endforeach ()

# Test files to compile.
set(PROTOCOL_TESTS
   BinaryProtocolTest
   CompactProtocolTest
   CompactV1ProtocolTest
   ContainerSkippingTest
   DebugProtocolTest
   F14RoundTripTest
   InterfaceTest
   JSONProtocolTest
   ProtocolReaderStructReadStateTest
   ProtocolTest
   SimpleJSONProtocolTest
   TraitsTest)

# Build each test independently.
foreach (test_name ${PROTOCOL_TESTS})
  add_executable(${test_name}
     ${test_name}.cpp
     ${GENERATED_THRIFT_SOURCES})
  target_compile_options(${test_name} PUBLIC
     -Wno-shadow-compatible-local)
  target_link_libraries(${test_name}
     GTest::gtest_main
     thriftcpp2
     Folly::folly
     fmt::fmt
  )
  gtest_discover_tests(${test_name})
endforeach ()
