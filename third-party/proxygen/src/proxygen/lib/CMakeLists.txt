# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

file(
    MAKE_DIRECTORY
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/http
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils
)

add_custom_command(
    OUTPUT ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/http/HTTPCommonHeaders.h
    OUTPUT ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/http/HTTPCommonHeaders.cpp
    COMMAND
        ${CMAKE_CURRENT_SOURCE_DIR}/http/gen_HTTPCommonHeaders.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/http/HTTPCommonHeaders.txt
        ${PROXYGEN_FBCODE_ROOT}
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/http
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/http/gen_HTTPCommonHeaders.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/http/HTTPCommonHeaders.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/gen_perfect_hash_table.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/perfect_hash_table_template.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/perfect_hash_table_template.cpp.gperf
    COMMENT "Generating HTTPCommonHeaders.h and HTTPCommonHeaders.cpp"
)

add_custom_command(
    OUTPUT ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/stats/StatsWrapper.h
    COMMAND
        ${CMAKE_CURRENT_SOURCE_DIR}/stats/gen_StatsWrapper.sh
        ${PROXYGEN_FBCODE_ROOT}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/stats/BaseStats.h
    COMMENT "Generating StatsWrapper.h"
)

add_custom_command(
    OUTPUT
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceEventType.h
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceEventType.cpp
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceFieldType.h
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceFieldType.cpp
    COMMAND
        ${PROXYGEN_PYTHON}
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/gen_trace_event_constants.py
        --output_type=cpp
        --input_files=samples/TraceEventType.txt,samples/TraceFieldType.txt
        --output_scope=proxygen
        --header_path=proxygen/lib/utils
        --install_dir=${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils
        --fbcode_dir=${PROXYGEN_FBCODE_ROOT}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/
    COMMENT "Generating TraceEventType and TraceFieldType"
)

add_custom_target(
    proxygen-generated
    DEPENDS
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/http/HTTPCommonHeaders.h
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/http/HTTPCommonHeaders.cpp
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceEventType.h
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceEventType.cpp
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceFieldType.h
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceFieldType.cpp
        ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/stats/StatsWrapper.h
)

set(
    HTTP3_SOURCES
    ${HTTP3_SOURCES}
    http/SynchronizedLruQuicPskCache.cpp
    http/HQConnector.cpp
    http/codec/HTTPBinaryCodec.cpp
    http/codec/HQControlCodec.cpp
    http/codec/HQFramedCodec.cpp
    http/codec/HQFramer.cpp
    http/codec/HQStreamCodec.cpp
    http/codec/HQUnidirectionalCodec.cpp
    http/codec/HQUtils.cpp
    http/session/HQByteEventTracker.cpp
    http/session/HQDownstreamSession.cpp
    http/session/HQSession.cpp
    http/session/HQStreamBase.cpp
    http/session/HQStreamDispatcher.cpp
    http/session/HQUpstreamSession.cpp
    transport/H3DatagramAsyncSocket.cpp
    transport/PersistentQuicPskCache.cpp
    transport/PersistentQuicTokenCache.cpp
)
set(
  HTTP3_DEPEND_LIBS
  ${HTTP3_DEPEND_LIBS}
  mvfst::mvfst_transport
  mvfst::mvfst_client
  mvfst::mvfst_client_cached_server_tp_serialization
  mvfst::mvfst_fizz_client
  mvfst::mvfst_server
  mvfst::mvfst_codec_types
  mvfst::mvfst_state_machine
)

# =============================================================================
# Granular library for generated code (HTTPCommonHeaders, TraceEventType, etc.)
# This library compiles the generated sources and provides them to other targets
# =============================================================================
proxygen_add_library(proxygen_http_http_common_headers
  SRCS
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/http/HTTPCommonHeaders.cpp
  EXPORTED_DEPS
    proxygen-generated
    Folly::folly_range
)
target_include_directories(proxygen_http_http_common_headers PUBLIC
    $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
    $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
    $<INSTALL_INTERFACE:include/>
)

proxygen_add_library(proxygen_utils_trace_event_types
  SRCS
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceEventType.cpp
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceFieldType.cpp
  EXPORTED_DEPS
    proxygen-generated
    Folly::folly_conv
)
target_include_directories(proxygen_utils_trace_event_types PUBLIC
    $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
    $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
    $<INSTALL_INTERFACE:include/>
)

# Granular library for http_parser
proxygen_add_library(proxygen_external_http_parser
  SRCS
    ${PROXYGEN_FBCODE_ROOT}/proxygen/external/http_parser/http_parser_cpp.cpp
  EXPORTED_DEPS
    Folly::folly_portability_string
)
# http_parser requires strict URL checking
target_compile_definitions(proxygen_external_http_parser_obj PRIVATE HTTP_PARSER_STRICT_URL=1)

# =============================================================================
# Granular library subdirectories
# Auto-generated CMakeLists.txt using proxygen_add_library()
# These include their own test subdirectories via BUILD_TESTS conditional
# =============================================================================
add_subdirectory(dns)
add_subdirectory(healthcheck)
add_subdirectory(http)
add_subdirectory(pools/generators)
add_subdirectory(sampling)
add_subdirectory(services)
add_subdirectory(ssl)
add_subdirectory(transport)
add_subdirectory(utils)

# =============================================================================
# Create monolithic proxygen library from all OBJECT targets
# =============================================================================
proxygen_create_monolithic_library()

# Add mvfst/HTTP3 dependencies to the monolithic library
target_link_libraries(proxygen
  PUBLIC
    mvfst::mvfst_transport
    mvfst::mvfst_client
    mvfst::mvfst_fizz_client
    mvfst::mvfst_server
    mvfst::mvfst_codec_types
    mvfst::mvfst_state_machine
)

# Install the headers, excluding unit testing related headers
file(
    GLOB_RECURSE PROXYGEN_HEADERS_TOINSTALL
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.h
)
list(FILTER PROXYGEN_HEADERS_TOINSTALL EXCLUDE REGEX test/)
list(FILTER PROXYGEN_HEADERS_TOINSTALL EXCLUDE REGEX utils/TestUtils.h)
list(FILTER PROXYGEN_HEADERS_TOINSTALL EXCLUDE REGEX .template.h)
# some exceptions
list(APPEND PROXYGEN_HEADERS_TOINSTALL http/webtransport/test/FakeSharedWebTransport.h)

# cmake doesn't provide a way to install a list of relative paths to the correct
# location (it will flatten them all into DESTINATION), so we have to manually
# do this for PROXYGEN_HEADERS_TOINSTALL
foreach(header ${PROXYGEN_HEADERS_TOINSTALL})
    get_filename_component(header_dir ${header} DIRECTORY)
    install(FILES ${header} DESTINATION include/proxygen/lib/${header_dir})
endforeach()

install(
    DIRECTORY ${PROXYGEN_GENERATED_ROOT}/proxygen/
    DESTINATION include/proxygen/
)
install(
    TARGETS proxygen
    EXPORT proxygen-exports
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

add_subdirectory(test)
add_subdirectory(http/coro/client/samples/cocurl)
