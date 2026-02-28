# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# Package information
set(PACKAGE_NAME "mcrouter")
if(NOT DEFINED PACKAGE_VERSION)
  set(PACKAGE_VERSION "0.1.0-dev")
endif()

project(${PACKAGE_NAME} CXX C)

# Directory containing mcrouter source (mcrouter/ subdirectory)
set(MCROUTER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mcrouter")

# CMake module path setup
set(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  # for in-fbsource builds
  "${CMAKE_CURRENT_SOURCE_DIR}/../opensource/fbcode_builder/CMake"
  # For shipit-transformed builds
  "${CMAKE_CURRENT_SOURCE_DIR}/build/fbcode_builder/CMake"
  ${CMAKE_MODULE_PATH}
)

include(GNUInstallDirs)

# Build options
option(BUILD_TESTS "Build mcrouter tests" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FBBuildOptions)
fb_activate_static_library_option()

include(FBThriftCppLibrary)

# Only enable PIC if building shared libs or if user wants it
if(BUILD_SHARED_LIBS)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# C++ standard
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  message(STATUS "Setting C++ standard to C++${CMAKE_CXX_STANDARD}")
endif()

# Compiler flags
add_compile_options(-fno-strict-aliasing)
add_compile_options(-Wall -Wextra -Wno-unused-parameter)
add_compile_options(
  -Wno-missing-field-initializers
  -Wno-deprecated-declarations
)
add_compile_definitions(
  LIBMC_FBTRACE_DISABLE
  DISABLE_COMPRESSION
  MCROUTER_OSS_BUILD
)

# =============================================================================
# Create header copies for OSS implementations
# =============================================================================
# These files map OSS-only implementations to the expected header names.
# The internal Buck build uses facebook/ substitutions for multi-router support.

# Generated headers go into mcrouter/ subdirectory of build tree
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mcrouter")

# config-impl.h is special - it auto-includes config.h to ensure HAVE_CONFIG_H
# is defined before any mcrouter internal headers that depend on it
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/config-impl.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/mcrouter/config-impl.h"
  COPYONLY
)
configure_file(
  "${MCROUTER_DIR}/RouterRegistry-impl.h"
  "${CMAKE_CURRENT_BINARY_DIR}/mcrouter/RouterRegistry.h"
  COPYONLY
)
configure_file(
  "${MCROUTER_DIR}/ThriftAcceptor-impl.h"
  "${CMAKE_CURRENT_BINARY_DIR}/mcrouter/ThriftAcceptor.h"
  COPYONLY
)
configure_file(
  "${MCROUTER_DIR}/mcrouter_sr_deps-impl.h"
  "${CMAKE_CURRENT_BINARY_DIR}/mcrouter/mcrouter_sr_deps.h"
  COPYONLY
)
configure_file(
  "${MCROUTER_DIR}/HostWithShard-fwd-impl.h"
  "${CMAKE_CURRENT_BINARY_DIR}/mcrouter/HostWithShard-fwd.h"
  COPYONLY
)

# =============================================================================
# Generate config.h
# =============================================================================
# This file is included as "mcrouter/config.h" so it goes in mcrouter/ subdir
# of the build tree.

set(MCROUTER_PACKAGE_STRING "${PACKAGE_VERSION} ${PACKAGE_NAME}")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/mcrouter/config.h"
  @ONLY
)

# =============================================================================
# Find Dependencies
# =============================================================================

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(folly CONFIG REQUIRED)
find_package(wangle CONFIG REQUIRED)
find_package(fizz CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(FBThrift CONFIG REQUIRED)
find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(Boost 1.69.0 REQUIRED
  COMPONENTS
    context
    filesystem
    program_options
    regex
    system
    thread
)
find_package(OpenSSL REQUIRED)
find_package(LibEvent MODULE REQUIRED)
find_package(ZLIB REQUIRED)
find_package(double-conversion CONFIG REQUIRED)

find_program(RAGEL_EXECUTABLE ragel)
if(NOT RAGEL_EXECUTABLE)
  message(FATAL_ERROR "Ragel is required to build mcrouter")
endif()
message(STATUS "Found Ragel: ${RAGEL_EXECUTABLE}")

# =============================================================================
# Create mcrouter_deps INTERFACE library for dependencies
# =============================================================================
add_library(mcrouter_deps INTERFACE)
target_link_libraries(mcrouter_deps INTERFACE
  Folly::folly
  wangle::wangle
  fizz::fizz
  FBThrift::thriftcpp2
  fmt::fmt
  gflags
  glog::glog
  Boost::context
  Boost::filesystem
  Boost::program_options
  Boost::regex
  Boost::system
  Boost::thread
  OpenSSL::SSL
  OpenSSL::Crypto
  ZLIB::ZLIB
  double-conversion::double-conversion
  Threads::Threads
  ${LIBEVENT_LIB}
  atomic
)

# Include directories
# Source tree: CMAKE_CURRENT_SOURCE_DIR contains mcrouter/
# Build tree: CMAKE_CURRENT_BINARY_DIR has generated files
target_include_directories(mcrouter_deps
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${LIBEVENT_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# =============================================================================
# Ragel Parser Generation
# =============================================================================
set(RAGEL_INPUT "${MCROUTER_DIR}/lib/network/McAsciiParser.rl")
set(RAGEL_OUTPUT
  "${CMAKE_CURRENT_BINARY_DIR}/mcrouter/lib/network/McAsciiParser-gen.cpp"
)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mcrouter/lib/network")

add_custom_command(
  OUTPUT ${RAGEL_OUTPUT}
  COMMAND ${RAGEL_EXECUTABLE} -G1 -o ${RAGEL_OUTPUT} ${RAGEL_INPUT}
  DEPENDS ${RAGEL_INPUT}
  COMMENT "Generating McAsciiParser from Ragel"
)

add_custom_target(mcrouter_ragel_gen DEPENDS ${RAGEL_OUTPUT})

# =============================================================================
# Thrift Code Generation
# =============================================================================
add_fbthrift_cpp_library(
  mcrouter_carbon_result_thrift
  mcrouter/lib/carbon/carbon_result.thrift
  OPTIONS stack_arguments sync_methods_return_try
)

add_fbthrift_cpp_library(
  mcrouter_carbon_thrift
  mcrouter/lib/carbon/carbon.thrift
  DEPENDS mcrouter_carbon_result_thrift
  OPTIONS stack_arguments sync_methods_return_try
)

add_fbthrift_cpp_library(
  mcrouter_common_thrift
  mcrouter/lib/network/gen/Common.thrift
  DEPENDS mcrouter_carbon_thrift
  OPTIONS stack_arguments sync_methods_return_try deprecated_terse_writes
)

add_fbthrift_cpp_library(
  mcrouter_memcache_thrift
  mcrouter/lib/network/gen/Memcache.thrift
  DEPENDS mcrouter_common_thrift
  OPTIONS stack_arguments sync_methods_return_try deprecated_terse_writes
)

add_fbthrift_cpp_library(
  mcrouter_memcache_service_thrift
  mcrouter/lib/network/gen/MemcacheService.thrift
  SERVICES Memcache
  DEPENDS mcrouter_memcache_thrift
  OPTIONS stack_arguments sync_methods_return_try deprecated_terse_writes
)

# Add source include directories to thrift targets
# Generated thrift code includes headers like <mcrouter/lib/carbon/Keys.h>
foreach(thrift_target
    mcrouter_carbon_result_thrift
    mcrouter_carbon_thrift
    mcrouter_common_thrift
    mcrouter_memcache_thrift
    mcrouter_memcache_service_thrift)
  target_include_directories(${thrift_target}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  )
endforeach()

# =============================================================================
# libmcrouter library (mcrouter/lib/)
# =============================================================================
set(MCROUTER_LIB_SOURCES
  # Lib root sources
  mcrouter/lib/AuxiliaryCPUThreadPool.cpp
  mcrouter/lib/AuxiliaryIOThreadPool.cpp
  mcrouter/lib/Clocks.cpp
  mcrouter/lib/Compression.cpp
  mcrouter/lib/CompressionCodecManager.cpp
  mcrouter/lib/FailoverErrorsSettingsBase.cpp
  mcrouter/lib/IOBufUtil.cpp
  mcrouter/lib/IovecCursor.cpp
  mcrouter/lib/Lz4CompressionCodec.cpp
  mcrouter/lib/Lz4Immutable.cpp
  mcrouter/lib/Lz4ImmutableCompressionCodec.cpp
  mcrouter/lib/MessageQueue.cpp
  mcrouter/lib/RendezvousHashFunc.cpp
  mcrouter/lib/RendezvousHashHelper.cpp
  mcrouter/lib/RuntimeVarsData.cpp
  mcrouter/lib/StatsReply.cpp
  mcrouter/lib/WeightedCh3HashFunc.cpp
  mcrouter/lib/WeightedCh3RvHashFunc.cpp
  mcrouter/lib/WeightedChHashFuncBase.cpp
  mcrouter/lib/WeightedRendezvousHashFunc.cpp
  mcrouter/lib/ZstdCompressionCodec.cpp

  # Carbon
  mcrouter/lib/carbon/CarbonQueueAppender.cpp
  mcrouter/lib/carbon/CarbonProtocolReader.cpp
  mcrouter/lib/carbon/ReplyCommon.cpp
  mcrouter/lib/carbon/RequestCommon.cpp
  mcrouter/lib/carbon/Result.cpp

  # Config
  mcrouter/lib/config/ConfigPreprocessor.cpp
  mcrouter/lib/config/RendezvousHash.cpp

  # Debug
  mcrouter/lib/debug/ConnectionFifo.cpp
  mcrouter/lib/debug/ConnectionFifoProtocol.cpp
  mcrouter/lib/debug/Fifo.cpp
  mcrouter/lib/debug/FifoManager.cpp

  # FBI
  mcrouter/lib/fbi/counting_sem.cpp
  mcrouter/lib/fbi/hash.c
  mcrouter/lib/fbi/network.c
  mcrouter/lib/fbi/WeightedFurcHash.cpp
  mcrouter/lib/fbi/cpp/globals.cpp
  mcrouter/lib/fbi/cpp/LogFailure.cpp
  mcrouter/lib/fbi/cpp/LowerBoundPrefixMap.cpp
  mcrouter/lib/fbi/cpp/ParsingUtil.cpp
  mcrouter/lib/fbi/cpp/util.cpp

  # MC
  mcrouter/lib/mc/msg.cpp

  # Network
  mcrouter/lib/network/AccessPoint.cpp
  mcrouter/lib/network/AsciiSerialized.cpp
  mcrouter/lib/network/AsyncMcClientImpl.cpp
  mcrouter/lib/network/AsyncMcServer.cpp
  mcrouter/lib/network/AsyncMcServerWorker.cpp
  mcrouter/lib/network/AsyncTlsToPlaintextSocket.cpp
  mcrouter/lib/network/CaretProtocol.cpp
  mcrouter/lib/network/ConnectionTracker.cpp
  mcrouter/lib/network/CpuController.cpp
  mcrouter/lib/network/FailureDomains.cpp
  mcrouter/lib/network/FizzContextProvider.cpp
  mcrouter/lib/network/McAsciiParser.cpp
  mcrouter/lib/network/McClientRequestContext.cpp
  mcrouter/lib/network/McParser.cpp
  mcrouter/lib/network/McSerializedRequest.cpp
  mcrouter/lib/network/McServerRequestContext.cpp
  mcrouter/lib/network/McServerSession.cpp
  mcrouter/lib/network/McServerThriftRequestContext.cpp
  mcrouter/lib/network/McSSLUtil.cpp
  mcrouter/lib/network/MultiOpParent.cpp
  mcrouter/lib/network/Qos.cpp
  mcrouter/lib/network/SecurityOptions.cpp
  mcrouter/lib/network/ServerLoad.cpp
  mcrouter/lib/network/SocketConnector.cpp
  mcrouter/lib/network/SocketUtil.cpp
  mcrouter/lib/network/ThreadLocalSSLContextProvider.cpp
  mcrouter/lib/network/ThriftTransport.cpp
  mcrouter/lib/network/WriteBuffer.cpp

  # Network gen
  mcrouter/lib/network/gen/CommonMessages.cpp
  mcrouter/lib/network/gen/CommonMessagesThrift.cpp
  mcrouter/lib/network/gen/MemcacheMessages.cpp
  mcrouter/lib/network/gen/MemcacheMessagesThrift.cpp
)

add_library(mcrouter_lib ${MCROUTER_LIB_SOURCES})
add_dependencies(mcrouter_lib mcrouter_ragel_gen)
target_sources(mcrouter_lib PRIVATE ${RAGEL_OUTPUT})
target_link_libraries(mcrouter_lib PUBLIC
  mcrouter_deps
  mcrouter_carbon_thrift
  mcrouter_carbon_result_thrift
  mcrouter_common_thrift
  mcrouter_memcache_thrift
  mcrouter_memcache_service_thrift
)
set_target_properties(mcrouter_lib PROPERTIES OUTPUT_NAME "mcrouter")

# =============================================================================
# libmcroutercore library
# =============================================================================
set(MCROUTER_CORE_SOURCES
  mcrouter/AsyncLog.cpp
  mcrouter/AsyncWriter.cpp
  mcrouter/CarbonRouterClient.cpp
  mcrouter/CarbonRouterClientBase.cpp
  mcrouter/CarbonRouterFactory.cpp
  mcrouter/CarbonRouterInstance.cpp
  mcrouter/CarbonRouterInstanceBase.cpp
  mcrouter/ConfigApi.cpp
  mcrouter/FileDataProvider.cpp
  mcrouter/FileObserver.cpp
  mcrouter/flavor.cpp
  mcrouter/LeaseTokenMap.cpp
  mcrouter/mcrouter_config.cpp
  mcrouter/McrouterFiberContext.cpp
  mcrouter/McrouterLogFailure.cpp
  mcrouter/McrouterLogger.cpp
  mcrouter/McrouterManager.cpp
  mcrouter/McDistributionUtils.cpp
  mcrouter/lib/invalidation/McInvalidationKvPairs.cpp
  mcrouter/options.cpp
  mcrouter/OptionsUtil.cpp
  mcrouter/PoolFactory.cpp
  mcrouter/ProxyBase.cpp
  mcrouter/ProxyConfigBuilder.cpp
  mcrouter/ProxyDestination.cpp
  mcrouter/ProxyDestinationBase.cpp
  mcrouter/ProxyDestinationKey.cpp
  mcrouter/ProxyDestinationMap.cpp
  mcrouter/ProxyRequestContext.cpp
  mcrouter/ProxyStats.cpp
  mcrouter/route.cpp
  mcrouter/RoutingPrefix.cpp
  mcrouter/ServiceInfo.cpp
  mcrouter/stats.cpp
  mcrouter/ThreadUtil.cpp
  mcrouter/ThriftAcceptor.cpp
  mcrouter/TkoLog.cpp
  mcrouter/TkoTracker.cpp
  mcrouter/ExternalStatsHandler.cpp

  # Routes
  mcrouter/routes/BigValueRoute.cpp
  mcrouter/routes/CarbonLookasideRoute.cpp
  mcrouter/routes/FailoverRateLimiter.cpp
  mcrouter/routes/HashStopAllowListRoute.cpp
  mcrouter/routes/KeyParseRoute.cpp
  mcrouter/routes/LatencyInjectionRoute.cpp
  mcrouter/routes/McBucketRoute.cpp
  mcrouter/routes/McImportResolver.cpp
  mcrouter/routes/McRouteHandleProvider.cpp
  mcrouter/routes/McRouteHandleProvider-AllFastestRoute.cpp
  mcrouter/routes/McRouteHandleProvider-CarbonLookasideRoute.cpp
  mcrouter/routes/McRouteHandleProvider-FailoverRoute.cpp
  mcrouter/routes/McRouteHandleProvider-HashRoute.cpp
  mcrouter/routes/McRouteHandleProvider-PoolRoute.cpp
  mcrouter/routes/NullRoute.cpp
  mcrouter/routes/RateLimiter.cpp
  mcrouter/routes/RendezvousRouteHelpers.cpp
  mcrouter/routes/RoutingUtils.cpp
  mcrouter/routes/ShadowSettings.cpp
  mcrouter/routes/ShardHashFunc.cpp
  mcrouter/routes/ShardSelectionRouteFactory.cpp
  mcrouter/routes/ShardSplitRoute.cpp
  mcrouter/routes/ShardSplitter.cpp
  mcrouter/routes/SlowWarmUpRouteSettings.cpp
  mcrouter/routes/StagingRoute.cpp
  mcrouter/routes/WarmUpRoute.cpp
)

add_library(mcroutercore ${MCROUTER_CORE_SOURCES})
target_link_libraries(mcroutercore PUBLIC mcrouter_lib)

# =============================================================================
# mcrouter executable
# =============================================================================
set(MCROUTER_SERVER_SOURCES
  mcrouter/main.cpp
  mcrouter/RequestAclChecker.cpp
  mcrouter/StandaloneConfig.cpp
  mcrouter/StandaloneUtils.cpp
)

add_executable(mcrouter_server ${MCROUTER_SERVER_SOURCES})
target_link_libraries(mcrouter_server PRIVATE mcroutercore)
set_target_properties(mcrouter_server PROPERTIES
  OUTPUT_NAME mcrouter
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
)

# =============================================================================
# mcpiper tool
# =============================================================================
set(MCPIPER_SOURCES
  mcrouter/tools/mcpiper/AnsiColorCodeStream.cpp
  mcrouter/tools/mcpiper/Config.cpp
  mcrouter/tools/mcpiper/FifoReader.cpp
  mcrouter/tools/mcpiper/main.cpp
  mcrouter/tools/mcpiper/McPiper.cpp
  mcrouter/tools/mcpiper/MessagePrinter.cpp
  mcrouter/tools/mcpiper/StyledString.cpp
  mcrouter/tools/mcpiper/Util.cpp
)

add_executable(mcpiper ${MCPIPER_SOURCES})
target_link_libraries(mcpiper PRIVATE mcrouter_lib)
set_target_properties(mcpiper PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
)

# =============================================================================
# Installation
# =============================================================================
install(
  TARGETS
    mcrouter_server
    mcpiper
    mcrouter_lib
    mcroutercore
    mcrouter_deps
  EXPORT mcrouter-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install thrift targets separately to preserve header directory
# structure via HEADER_INSTALL_DIR property set by FBThriftCppLibrary
set(MCROUTER_THRIFT_TARGETS
  mcrouter_carbon_result_thrift
  mcrouter_carbon_thrift
  mcrouter_common_thrift
  mcrouter_memcache_thrift
  mcrouter_memcache_service_thrift
)
foreach(thrift_target IN LISTS MCROUTER_THRIFT_TARGETS)
  install(
    TARGETS ${thrift_target}
    EXPORT mcrouter-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION
      "$<TARGET_PROPERTY:${thrift_target},HEADER_INSTALL_DIR>"
  )
endforeach()

# Install headers from mcrouter/ subdirectory
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/mcrouter/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mcrouter
  FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.tcc"
    PATTERN "facebook" EXCLUDE
    PATTERN "test" EXCLUDE
    PATTERN "build" EXCLUDE
)

# Install generated headers
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/mcrouter/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/mcrouter/config-impl.h
    ${CMAKE_CURRENT_BINARY_DIR}/mcrouter/RouterRegistry.h
    ${CMAKE_CURRENT_BINARY_DIR}/mcrouter/ThriftAcceptor.h
    ${CMAKE_CURRENT_BINARY_DIR}/mcrouter/mcrouter_sr_deps.h
    ${CMAKE_CURRENT_BINARY_DIR}/mcrouter/HostWithShard-fwd.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mcrouter
)

# CMake config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/mcrouter-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/mcrouter-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mcrouter
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/mcrouter-config.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mcrouter
)

install(
  EXPORT mcrouter-targets
  NAMESPACE mcrouter::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mcrouter
)

# =============================================================================
# Tests
# =============================================================================
if(BUILD_TESTS)
  enable_testing()
  find_package(GTest CONFIG REQUIRED)

  # Add test subdirectories when ready
  # add_subdirectory(mcrouter/lib/test)
  # add_subdirectory(mcrouter/test)
endif()
