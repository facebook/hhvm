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
  mvfst::mvfst_fizz_client
  mvfst::mvfst_server
  mvfst::mvfst_codec_types
  mvfst::mvfst_state_machine
)

add_library(
    proxygen
    healthcheck/ServerHealthCheckerCallback.cpp
    http/HTTP3ErrorCode.cpp
    http/Window.cpp
    http/codec/CodecProtocol.cpp
    http/codec/CodecUtil.cpp
    http/codec/compress/HeaderIndexingStrategy.cpp
    http/codec/compress/HeaderTable.cpp
    http/codec/compress/HPACKCodec.cpp
    http/codec/compress/HPACKContext.cpp
    http/codec/compress/HPACKDecodeBuffer.cpp
    http/codec/compress/HPACKDecoderBase.cpp
    http/codec/compress/HPACKDecoder.cpp
    http/codec/compress/HPACKEncodeBuffer.cpp
    http/codec/compress/HPACKEncoderBase.cpp
    http/codec/compress/HPACKEncoder.cpp
    http/codec/compress/HPACKHeader.cpp
    http/codec/compress/Huffman.cpp
    http/codec/compress/Logging.cpp
    http/codec/compress/NoPathIndexingStrategy.cpp
    http/codec/compress/QPACKCodec.cpp
    http/codec/compress/QPACKContext.cpp
    http/codec/compress/QPACKDecoder.cpp
    http/codec/compress/QPACKEncoder.cpp
    http/codec/compress/QPACKHeaderTable.cpp
    http/codec/compress/QPACKStaticHeaderTable.cpp
    http/codec/compress/StaticHeaderTable.cpp
    http/codec/DefaultHTTPCodecFactory.cpp
    http/codec/ErrorCode.cpp
    http/codec/FlowControlFilter.cpp
    http/codec/HeaderDecodeInfo.cpp
    http/codec/HTTP1xCodec.cpp
    http/codec/HTTP2Codec.cpp
    http/codec/HTTP2Constants.cpp
    http/codec/HTTP2Framer.cpp
    http/codec/HTTPChecks.cpp
    http/codec/HTTPCodecFactory.cpp
    http/codec/HTTPCodecFilter.cpp
    http/codec/HTTPCodecPrinter.cpp
    http/codec/HTTPParallelCodec.cpp
    http/codec/HTTPSettings.cpp
    http/codec/TransportDirection.cpp
    http/codec/RateLimitFilter.cpp
    http/connpool/ServerIdleSessionController.cpp
    http/connpool/SessionHolder.cpp
    http/connpool/SessionPool.cpp
    http/connpool/ThreadIdleSessionController.cpp
    http/experimental/RFC1867.cpp
    http/HeaderConstants.cpp
    http/HTTPConnector.cpp
    http/HTTPConnectorWithFizz.cpp
    http/HTTPConstants.cpp
    http/HTTPException.cpp
    http/HTTPHeaders.cpp
    http/HTTPMessage.cpp
    http/HTTPMessageFilters.cpp
    http/HTTPMethod.cpp
    http/HTTPPriorityFunctions.cpp
    http/StatusTypeEnum.cpp
    http/ProxygenErrorEnum.cpp
    http/ProxyStatus.cpp
    http/RFC2616.cpp
    http/sink/HTTPTransactionSink.cpp
    http/observer/HTTPSessionObserverInterface.cpp
    http/session/ByteEvents.cpp
    http/session/ByteEventTracker.cpp
    http/session/CodecErrorResponseHandler.cpp
    http/session/HTTP2PriorityQueue.cpp
    http/session/HTTPDefaultSessionCodecFactory.cpp
    http/session/HTTPDirectResponseHandler.cpp
    http/session/HTTPDownstreamSession.cpp
    http/session/HTTPErrorPage.cpp
    http/session/HTTPEvent.cpp
    http/session/HTTPSessionAcceptor.cpp
    http/session/HTTPSessionActivityTracker.cpp
    http/session/HTTPSessionBase.cpp
    http/session/HTTPSession.cpp
    http/session/HTTPTransaction.cpp
    http/session/HTTPTransactionEgressSM.cpp
    http/session/HTTPTransactionIngressSM.cpp
    http/session/HTTPUpstreamSession.cpp
    http/session/SecondaryAuthManager.cpp
    http/session/SimpleController.cpp
    http/structuredheaders/StructuredHeadersBuffer.cpp
    http/structuredheaders/StructuredHeadersDecoder.cpp
    http/structuredheaders/StructuredHeadersEncoder.cpp
    http/structuredheaders/StructuredHeadersUtilities.cpp
    pools/generators/FileServerListGenerator.cpp
    pools/generators/ServerListGenerator.cpp
    sampling/Sampling.cpp
    services/RequestWorkerThread.cpp
    services/RequestWorkerThreadNoExecutor.cpp
    services/Service.cpp
    services/WorkerThread.cpp
    stats/ResourceStats.cpp
    transport/PersistentFizzPskCache.cpp
    utils/AsyncTimeoutSet.cpp
    utils/CryptUtil.cpp
    utils/Exception.cpp
    utils/HTTPTime.cpp
    utils/Logging.cpp
    utils/ParseURL.cpp
    utils/RendezvousHash.cpp
    utils/Time.cpp
    utils/TraceEventContext.cpp
    utils/TraceEvent.cpp
    utils/WheelTimerInstance.cpp
    utils/ZlibStreamCompressor.cpp
    utils/ZlibStreamDecompressor.cpp
    utils/ZstdStreamCompressor.cpp
    utils/ZstdStreamDecompressor.cpp
    ${HTTP3_SOURCES}
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/http/HTTPCommonHeaders.cpp
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceEventType.cpp
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceFieldType.cpp
    $<TARGET_OBJECTS:proxygen_http_parser>
)
add_dependencies(
    proxygen
    proxygen-generated
    proxygen_http_parser
)
target_include_directories(
    proxygen PUBLIC
    $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
    $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
    $<INSTALL_INTERFACE:include/>
)
target_compile_options(
    proxygen PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)

if (BUILD_SHARED_LIBS)
    set_property(TARGET proxygen PROPERTY POSITION_INDEPENDENT_CODE ON)
    if (DEFINED PACKAGE_VERSION)
      set_target_properties(proxygen PROPERTIES VERSION ${PACKAGE_VERSION})
    endif()
endif()

target_link_libraries(
    proxygen PUBLIC
    Folly::folly
    fizz::fizz
    wangle::wangle
    zstd
    Boost::boost
    Boost::iostreams
    -lz
    ${HTTP3_DEPEND_LIBS}
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
add_subdirectory(http/test)
add_subdirectory(http/structuredheaders/test)
add_subdirectory(http/connpool/test)
add_subdirectory(http/codec/test)
add_subdirectory(http/codec/compress/test)
add_subdirectory(http/session/test)
add_subdirectory(http/webtransport/test)
add_subdirectory(sampling/test)
add_subdirectory(services/test)
add_subdirectory(transport/test)
add_subdirectory(utils/test)
