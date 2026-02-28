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

# This CMakeLists.txt takes care of building a selection of benchmarks and tests
# in this folder. Currently, it only builds ProtocolBench with its dependencies.

# Obtain paths to other generated thrift files from the global property space.
get_property(protocol-cpp2-SOURCES GLOBAL PROPERTY
    protocol-cpp2-SOURCES)
get_property(protocol_detail-cpp2-SOURCES GLOBAL PROPERTY
    protocol_detail-cpp2-SOURCES)
get_property(frozen-cpp2-SOURCES GLOBAL PROPERTY
    frozen-cpp2-SOURCES)
get_property(field_mask-cpp2-SOURCES GLOBAL PROPERTY
   field_mask-cpp2-SOURCES)

# Generate Thrift files for this benchmark.
thrift_generate(
  "ProtocolBenchData" #file_name
  "" #services
  "cpp2" #language
  "frozen2,layouts" #options
  "${CMAKE_CURRENT_SOURCE_DIR}" #file_path
  "${CMAKE_CURRENT_BINARY_DIR}" #output_path
  "thrift/lib/cpp2/test" #include_prefix
  THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
)

set(GENERATED_THRIFT_SOURCES ${ProtocolBenchData-cpp2-SOURCES}
  ${protocol-cpp2-SOURCES}
  ${protocol_detail-cpp2-SOURCES}
  ${field_mask-cpp2-SOURCES}
  ${frozen-cpp2-SOURCES})
set(SOURCE_FILES ProtocolBench.cpp)
set(ADDITIONAL_SOURCE_FILES
    ../../thrift/detail/protocol.cpp ../../thrift/TypeToMaskAdapter.cpp)

# Generate conformance Thrift files.
foreach (conf_name "protocol" "type" "any" "serialization" "patch_data"
         "conformance")
  thrift_generate(
    "${conf_name}" #file_name
    "" #services
    "cpp2" #language
    "frozen2" #options
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../conformance/if" #file_path
    "${CMAKE_CURRENT_BINARY_DIR}/../../../conformance/if" #output_path
    "thrift/conformance/if" #include_prefix
    THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
    TARGET_NAME_BASE "${conf_name}conformance"
  )
  set(GENERATED_THRIFT_SOURCES
    ${GENERATED_THRIFT_SOURCES}
    ${${conf_name}conformance-cpp2-SOURCES})
endforeach ()

foreach (GENERATED ${GENERATED_THRIFT_SOURCES})
  set_source_files_properties(${GENERATED} PROPERTIES GENERATED TRUE)
endforeach ()

add_executable(ProtocolBench
  ${SOURCE_FILES}
  ${ADDITIONAL_SOURCE_FILES}
  ${GENERATED_THRIFT_SOURCES}
)
target_link_libraries(ProtocolBench
  thriftcpp2
  Folly::folly
  Folly::follybenchmark
)

install(
  TARGETS ProtocolBench
  EXPORT fbthrift-exports
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)
