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

cmake_minimum_required(VERSION 3.0.2)

if (NOT DEFINED PACKAGE_VERSION)
  set(PACKAGE_VERSION "1.0.0")
endif()

project("wangle" VERSION ${PACKAGE_VERSION} LANGUAGES CXX C)

if (NOT DEFINED CPACK_GENERATOR)
  set(CPACK_GENERATOR "RPM")
endif()
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
include(CPack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
  # for in-fbsource builds
  "${CMAKE_CURRENT_SOURCE_DIR}/../opensource/fbcode_builder/CMake"
  # For shipit-transformed builds
  "${CMAKE_CURRENT_SOURCE_DIR}/../build/fbcode_builder/CMake"
  ${CMAKE_MODULE_PATH})

include(FBBuildOptions)
fb_activate_static_library_option()

# When installing Folly & Wangle in a non-default prefix, this will let
# projects linking against libwangle.so to find libfolly.so automatically.
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(INCLUDE_INSTALL_DIR include CACHE STRING
    "The subdirectory where header files should be installed")
set(LIB_INSTALL_DIR lib CACHE STRING
    "The subdirectory where libraries should be installed")
set(CMAKE_INSTALL_DIR lib/cmake/wangle CACHE STRING
    "The subdirectory where CMake package config files should be installed")

# Try finding folly via its installed CMake configuration file.
# This will also automatically find several libraries that we depend on through
# folly.
find_package(folly CONFIG REQUIRED)

find_package(fizz CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Glog REQUIRED)
find_package(gflags CONFIG QUIET)
if (gflags_FOUND)
  message(STATUS "Found gflags from package config")
  message(STATUS "gflags_CONFIG=${gflags_CONFIG}")
else()
  find_package(Gflags REQUIRED)
endif()
find_package(LibEvent MODULE REQUIRED)
find_package(DoubleConversion REQUIRED)
find_package(Threads REQUIRED)
if (UNIX AND NOT APPLE)
  find_package(Librt)
endif()

include(CheckAtomic)

set(WANGLE_HEADER_DIRS
  acceptor
  bootstrap
  channel
  client
  codec
  service
  ssl
  util
)

foreach(dir ${WANGLE_HEADER_DIRS})
  file(GLOB_RECURSE headers ${dir}/*.h)
  set(WANGLE_HEADERS
    ${WANGLE_HEADERS}
    ${headers})
endforeach()

set(WANGLE_SOURCES
  acceptor/Acceptor.cpp
  acceptor/AcceptorHandshakeManager.cpp
  acceptor/ConnectionManager.cpp
  acceptor/EvbHandshakeHelper.cpp
  acceptor/FizzAcceptorHandshakeHelper.cpp
  acceptor/FizzConfigUtil.cpp
  acceptor/LoadShedConfiguration.cpp
  acceptor/ManagedConnection.cpp
  acceptor/SecureTransportType.cpp
  acceptor/SocketOptions.cpp
  acceptor/SSLAcceptorHandshakeHelper.cpp
  acceptor/TLSPlaintextPeekingCallback.cpp
  acceptor/TransportInfo.cpp
  bootstrap/ServerBootstrap.cpp
  channel/FileRegion.cpp
  channel/Pipeline.cpp
  client/persistence/FilePersistenceLayer.cpp
  client/persistence/PersistentCacheCommon.cpp
  client/ssl/SSLSessionCacheData.cpp
  client/ssl/SSLSessionCacheUtils.cpp
  client/ssl/SSLSessionCallbacks.cpp
  codec/LengthFieldBasedFrameDecoder.cpp
  codec/LengthFieldPrepender.cpp
  codec/LineBasedFrameDecoder.cpp
  ssl/PasswordInFileFactory.cpp
  ssl/ServerSSLContext.cpp
  ssl/SSLContextManager.cpp
  ssl/SSLSessionCacheManager.cpp
  ssl/SSLUtil.cpp
  ssl/TLSTicketKeyManager.cpp
  ssl/TLSCredProcessor.cpp
  util/FilePoller.cpp
)

add_library(wangle
  ${WANGLE_HEADERS}
  ${WANGLE_SOURCES}
)

include(CheckLibraryExists)
check_library_exists(ssl SSL_SESSION_dup "${OPENSSL_SSL_LIBRARY}" WANGLE_HAVE_SSL_SESSION_DUP)
if(WANGLE_HAVE_SSL_SESSION_DUP)
  target_compile_definitions(wangle PRIVATE WANGLE_HAVE_SSL_SESSION_DUP)
endif()

if (BUILD_SHARED_LIBS)
  set_target_properties(wangle
    PROPERTIES VERSION ${PACKAGE_VERSION})
endif()

target_include_directories(
  wangle
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
    ${FIZZ_INCLUDE_DIR}
    ${FOLLY_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${GLOG_INCLUDE_DIRS}
    ${GFLAGS_INCLUDE_DIRS}
    ${LIBEVENT_INCLUDE_DIR}
    ${DOUBLE_CONVERSION_INCLUDE_DIR}
)
target_link_libraries(wangle PUBLIC
  ${FOLLY_LIBRARIES}
  ${FIZZ_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${LIBEVENT_LIB}
  ${DOUBLE_CONVERSION_LIBRARY}
  ${CMAKE_DL_LIBS}
  ${LIBRT_LIBRARIES}
  Threads::Threads)

install(
  TARGETS wangle
  EXPORT wangle-exports
  DESTINATION ${LIB_INSTALL_DIR}
)
# We unfortunately cannot install wangle's headers with the install()
# statement above.  install(TARGETS) appears to only support installing
# PUBLIC_HEADER in a flat include directory, and not a deeper tree.
foreach(dir ${WANGLE_HEADER_DIRS})
  install(DIRECTORY ${dir} DESTINATION "${INCLUDE_INSTALL_DIR}/wangle"
          FILES_MATCHING PATTERN "*.h")
endforeach()

# Install CMake package configuration files for wangle
include(CMakePackageConfigHelpers)
configure_package_config_file(
  cmake/wangle-config.cmake.in
  wangle-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DIR}
  PATH_VARS
    INCLUDE_INSTALL_DIR
    CMAKE_INSTALL_DIR
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/wangle-config.cmake
  DESTINATION ${CMAKE_INSTALL_DIR}
)
install(EXPORT wangle-exports
        FILE wangle-targets.cmake
        NAMESPACE wangle::
        DESTINATION ${CMAKE_INSTALL_DIR})

IF(CMAKE_CROSSCOMPILING)
   option(BUILD_TESTS "BUILD_TESTS" OFF)
ELSE(CMAKE_CROSSCOMPILING)
   option(BUILD_TESTS "BUILD_TESTS" ON)
ENDIF(CMAKE_CROSSCOMPILING)

if(BUILD_TESTS)
  enable_testing()

  find_package(GMock MODULE)
  if(NOT LIBGMOCK_FOUND)
    include(ExternalProject)

    # Download and install GoogleMock
    ExternalProject_Add(
        gtest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.8.0
        PREFIX gtest
        # Disable install step
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        LOG_UPDATE 1
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_TEST 1
        LOG_INSTALL 1
    )

    # Create a libgmock target to be used as a dependency by test programs
    add_library(libgmock IMPORTED STATIC GLOBAL)
    add_dependencies(libgmock gtest)
    add_library(libgmock_main IMPORTED STATIC GLOBAL)
    add_dependencies(libgmock_main gtest)

    # Set gmock properties
    ExternalProject_Get_Property(gtest source_dir binary_dir)
    set_target_properties(libgmock PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
    )
    set_target_properties(libgmock_main PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock_main.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
    )
    set(LIBGMOCK_LIBRARIES libgmock libgmock_main)
    set(LIBGMOCK_INCLUDE_DIR "${source_dir}/googlemock/include")
    set(LIBGTEST_INCLUDE_DIR "${source_dir}/googletest/include")
  endif()

  macro(add_gtest test_source test_name)
  add_executable(${test_name} ${test_source} test/TestMain.cpp)
  target_include_directories(
    ${test_name} PUBLIC ${LIBGMOCK_INCLUDE_DIR} ${LIBGTEST_INCLUDE_DIR}
  )
  target_link_libraries(
    ${test_name}
    wangle
    ${LIBGMOCK_LIBRARIES}
    ${LIBEVENT_LIB}
  )
  add_test(${test_name} bin/${test_name})
  endmacro(add_gtest)

  # this test segfaults
  add_gtest(acceptor/test/ConnectionManagerTest.cpp ConnectionManagerTest)
  add_gtest(acceptor/test/LoadShedConfigurationTest.cpp LoadShedConfigurationTest)
  add_gtest(acceptor/test/PeekingAcceptorHandshakeHelperTest.cpp PeekingAcceptorHandshakeHelperTest)
  add_gtest(bootstrap/test/BootstrapTest.cpp BootstrapTest)
  add_gtest(channel/broadcast/test/BroadcastHandlerTest.cpp BroadcastHandlerTest)
  add_gtest(channel/broadcast/test/BroadcastPoolTest.cpp BroadcastPoolTest)
  add_gtest(channel/broadcast/test/ObservingHandlerTest.cpp ObservingHandlerTest)
  add_gtest(channel/test/AsyncSocketHandlerTest.cpp AsyncSocketHandlerTest)
  add_gtest(channel/test/OutputBufferingHandlerTest.cpp OutputBufferingHandlerTest)
  add_gtest(channel/test/PipelineTest.cpp PipelineTest)
  add_gtest(codec/test/CodecTest.cpp CodecTest)
  # this test fails with an exception
  #  add_gtest(service/test/ServiceTest.cpp ServiceTest)
  # this test requires arguments?
  #  add_gtest(ssl/test/SSLCacheTest.cpp SSLCacheTest)
  add_gtest(ssl/test/SSLContextManagerTest.cpp SSLContextManagerTest)
  add_gtest(ssl/test/TLSCredProcessorTest.cpp TLSCredProcessorTest)
  add_gtest(util/test/FilePollerTest.cpp FilePollerTest)
endif()

option(BUILD_EXAMPLES "BUILD_EXAMPLES" OFF)

if(BUILD_EXAMPLES)
  add_executable(EchoClient example/echo/EchoClient.cpp)
  target_link_libraries(EchoClient wangle)
  add_executable(EchoServer example/echo/EchoServer.cpp)
  target_link_libraries(EchoServer wangle)
  add_executable(TelnetClient example/telnet/TelnetClient.cpp)
  target_link_libraries(TelnetClient wangle)
  add_executable(TelnetServer example/telnet/TelnetServer.cpp)
  target_link_libraries(TelnetServer wangle)
  add_executable(ProxyServer example/proxy/Proxy.cpp)
  target_link_libraries(ProxyServer wangle)
  add_executable(AcceptServer example/accept_steering/accept_steering_server.cpp)
  target_link_libraries(AcceptServer wangle)
  add_executable(BroadcastProxy example/broadcast/BroadcastProxy.cpp)
  target_link_libraries(BroadcastProxy wangle)
endif()
