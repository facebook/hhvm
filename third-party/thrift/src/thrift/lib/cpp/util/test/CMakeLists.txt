# Copyright (c) Meta Platforms, Inc. and affiliates and Contributors
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

add_executable(VarintUtilsBench
  VarintUtilsBench.cpp
)
target_link_libraries(VarintUtilsBench
  thriftcpp2
  Folly::folly
  Folly::follybenchmark
)

install(
  TARGETS VarintUtilsBench
  EXPORT fbthrift-exports
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

set(PROTOCOL_TESTS
   VarintUtilsTest)

# Build each test independently.
foreach (test_name ${PROTOCOL_TESTS})
  add_executable(${test_name}
     ${test_name}.cpp)
  target_compile_options(${test_name} PUBLIC
     -Wno-shadow-compatible-local)
  target_link_libraries(${test_name}
     GTest::gtest_main
     thriftcpp2
     Folly::folly
  )
  gtest_discover_tests(${test_name})
endforeach ()

