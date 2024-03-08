#  Copyright (c) 2018, Facebook, Inc.
#  All rights reserved.

cmake_minimum_required(VERSION 3.1)

if (NOT DEFINED PACKAGE_VERSION)
  set(PACKAGE_VERSION "1.0.0")
endif()

project("fizz" VERSION ${PACKAGE_VERSION} LANGUAGES CXX C)

if (NOT DEFINED CPACK_GENERATOR)
  set(CPACK_GENERATOR "RPM")
endif()
include(CPack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  # for in-fbsource builds
  "${CMAKE_CURRENT_SOURCE_DIR}/../opensource/fbcode_builder/CMake"
  # For shipit-transformed builds
  "${CMAKE_CURRENT_SOURCE_DIR}/../build/fbcode_builder/CMake"
  ${CMAKE_MODULE_PATH})

include(FBBuildOptions)
fb_activate_static_library_option()

# When installing Folly & Fizz in a non-default prefix, this will let
# projects linking against libfizz.so to find libfolly.so automatically.
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(INCLUDE_INSTALL_DIR include CACHE STRING
    "The subdirectory where header files should be installed")
set(LIB_INSTALL_DIR lib CACHE STRING
    "The subdirectory where libraries should be installed")
set(BIN_INSTALL_DIR bin CACHE STRING
    "The subdirectory where binaries should be installed")
set(CMAKE_INSTALL_DIR lib/cmake/fizz CACHE STRING
    "The subdirectory where CMake package config files should be installed")

find_package(folly CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

find_package(OpenSSL REQUIRED)
find_package(Glog REQUIRED)
find_package(DoubleConversion REQUIRED)
find_package(Threads REQUIRED)
find_package(Zstd REQUIRED)
if (UNIX AND NOT APPLE)
  find_package(Librt)
endif()

include(CheckAtomic)

find_package(Sodium REQUIRED)

SET(FIZZ_SHINY_DEPENDENCIES "")
SET(FIZZ_LINK_LIBRARIES "")
SET(FIZZ_INCLUDE_DIRECTORIES "")

find_package(gflags CONFIG QUIET)
if (gflags_FOUND)
  message(STATUS "Found gflags from package config")
  if (TARGET gflags-shared)
    list(APPEND FIZZ_SHINY_DEPENDENCIES gflags-shared)
  elseif (TARGET gflags)
    list(APPEND FIZZ_SHINY_DEPENDENCIES gflags)
  else()
    message(FATAL_ERROR "Unable to determine the target name for the GFlags package.")
  endif()
  list(APPEND CMAKE_REQUIRED_LIBRARIES ${GFLAGS_LIBRARIES})
  list(APPEND CMAKE_REQUIRED_INCLUDES ${GFLAGS_INCLUDE_DIR})
else()
  find_package(Gflags REQUIRED MODULE)
  list(APPEND FIZZ_LINK_LIBRARIES ${LIBGFLAGS_LIBRARY})
  list(APPEND FIZZ_INCLUDE_DIRECTORIES ${LIBGFLAGS_INCLUDE_DIR})
  list(APPEND CMAKE_REQUIRED_LIBRARIES ${LIBGFLAGS_LIBRARY})
  list(APPEND CMAKE_REQUIRED_INCLUDES ${LIBGFLAGS_INCLUDE_DIR})
endif()

find_package(ZLIB REQUIRED)

find_package(Libevent CONFIG QUIET)
if(TARGET event)
  message(STATUS "Found libevent from package config")
  list(APPEND FIZZ_SHINY_DEPENDENCIES event)
else()
  find_package(Libevent MODULE REQUIRED)
  list(APPEND FIZZ_LINK_LIBRARIES ${LIBEVENT_LIB})
  list(APPEND FIZZ_INCLUDE_DIRECTORIES ${LIBEVENT_INCLUDE_DIR})
endif()

# Fizz build time options
include(FizzOptions)

configure_file(fizz-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/fizz/fizz-config.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/fizz/fizz-config.h DESTINATION ${INCLUDE_INSTALL_DIR}/fizz/)

set(FIZZ_HEADER_DIRS
  client
  compression
  crypto
  crypto/aead
  crypto/exchange
  crypto/hpke
  crypto/signature
  crypto/openssl
  experimental/ktls
  experimental/util
  extensions/delegatedcred
  extensions/exportedauth
  extensions/tokenbinding
  protocol
  protocol/clock
  protocol/ech
  record
  server
  util
  tool
  third-party
)

set(FIZZ_TEST_HEADER_DIRS
  client/test
  compression/test
  crypto/aead/test
  crypto/exchange/test
  crypto/test
  crypto/hpke/test
  protocol/test
  protocol/clock/test
  record/test
  server/test
  tool/test
  util/test
  test
)

foreach(dir ${FIZZ_HEADER_DIRS})
  file(GLOB_RECURSE headers ${dir}/*.h)
  set(FIZZ_HEADERS
    ${FIZZ_HEADERS}
    ${headers})
endforeach()

foreach(dir ${FIZZ_TEST_HEADER_DIRS})
  file(GLOB_RECURSE headers ${dir}/*.h)
  set(FIZZ_TEST_HEADERS
    ${FIZZ_TEST_HEADERS}
    ${headers})
endforeach()

if(FIZZ_BUILD_AEGIS)
    set(FIZZ_AESNI_SOURCES
      third-party/libsodium-aegis/aegis128l/aesni/aead_aegis128l_aesni.c
      third-party/libsodium-aegis/aegis256/aesni/aead_aegis256_aesni.c
    )
    set_source_files_properties(${FIZZ_AESNI_SOURCES} PROPERTIES COMPILE_OPTIONS "-maes;-mssse3")

    set(FIZZ_AEGIS_SOURCES
      ${FIZZ_AESNI_SOURCES}
      third-party/libsodium-aegis/aegis128l/aead_aegis128l.c
      third-party/libsodium-aegis/aegis128l/soft/aead_aegis128l_soft.c
      third-party/libsodium-aegis/aegis256/aead_aegis256.c
      third-party/libsodium-aegis/aegis256/soft/aead_aegis256_soft.c
      third-party/libsodium-aegis/private/softaes.c
    )
endif()

set(FIZZ_SOURCES
  compression/CertDecompressionManager.cpp
  compression/ZlibCertificateCompressor.cpp
  compression/ZlibCertificateDecompressor.cpp
  compression/ZstdCertificateCompressor.cpp
  compression/ZstdCertificateDecompressor.cpp
  crypto/Utils.cpp
  crypto/exchange/X25519.cpp
  crypto/aead/OpenSSLEVPCipher.cpp
  crypto/aead/IOBufUtil.cpp
  crypto/aead/AEGISCipher.cpp
  crypto/signature/Signature.cpp
  crypto/Hkdf.cpp
  crypto/KeyDerivation.cpp
  crypto/Sha256.cpp
  crypto/Sha384.cpp
  crypto/hpke/Context.cpp
  crypto/hpke/DHKEM.cpp
  crypto/hpke/Hkdf.cpp
  crypto/hpke/Hpke.cpp
  crypto/hpke/Utils.cpp
  crypto/openssl/OpenSSLKeyUtils.cpp
  record/Types.cpp
  record/RecordLayer.cpp
  record/EncryptedRecordLayer.cpp
  record/PlaintextRecordLayer.cpp
  record/BufAndPaddingPolicy.cpp
  server/AeadTokenCipher.cpp
  server/AeadCookieCipher.cpp
  server/FizzServerContext.cpp
  server/ServerProtocol.cpp
  server/CertManager.cpp
  server/State.cpp
  server/FizzServer.cpp
  server/TicketCodec.cpp
  server/CookieCipher.cpp
  server/ReplayCache.cpp
  server/SlidingBloomReplayCache.cpp
  protocol/AsyncFizzBase.cpp
  protocol/Types.cpp
  protocol/Exporter.cpp
  protocol/DefaultCertificateVerifier.cpp
  protocol/Events.cpp
  protocol/KeyScheduler.cpp
  protocol/Certificate.cpp
  protocol/CertUtils.cpp
  protocol/OpenSSLFactory.cpp
  protocol/Params.cpp
  protocol/clock/SystemClock.cpp
  protocol/ech/Decrypter.cpp
  protocol/ech/Encryption.cpp
  extensions/delegatedcred/DelegatedCredentialCertManager.cpp
  extensions/delegatedcred/DelegatedCredentialClientExtension.cpp
  extensions/delegatedcred/DelegatedCredentialFactory.cpp
  extensions/delegatedcred/DelegatedCredentialUtils.cpp
  extensions/delegatedcred/Types.cpp
  extensions/exportedauth/ExportedAuthenticator.cpp
  extensions/tokenbinding/Types.cpp
  extensions/tokenbinding/TokenBindingConstructor.cpp
  extensions/tokenbinding/TokenBindingClientExtension.cpp
  extensions/tokenbinding/Validator.cpp
  experimental/client/BatchSignaturePeerCert.cpp
  experimental/protocol/BatchSignatureTypes.cpp
  experimental/ktls/AsyncFizzBaseKTLS.cpp
  experimental/ktls/FizzKTLSCallback.cpp
  experimental/ktls/AsyncKTLSSocket.cpp
  experimental/ktls/KTLS.cpp
  client/FizzClientContext.cpp
  client/State.cpp
  client/ClientProtocol.cpp
  client/PskSerializationUtils.cpp
  client/SynchronizedLruPskCache.cpp
  client/EarlyDataRejectionPolicy.cpp
  tool/FizzCommandCommon.cpp
  util/FizzUtil.cpp
  util/Tracing.cpp
  ${FIZZ_AEGIS_SOURCES}
)


add_library(fizz
  ${FIZZ_HEADERS}
  ${FIZZ_SOURCES}
)

if (BUILD_SHARED_LIBS)
  set_target_properties(fizz
    PROPERTIES VERSION ${PACKAGE_VERSION})
endif()

get_filename_component(FIZZ_BASE_DIR ${CMAKE_SOURCE_DIR}/.. ABSOLUTE)

target_include_directories(
  fizz
  PUBLIC
    $<BUILD_INTERFACE:${FIZZ_BASE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
    ${FOLLY_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${sodium_INCLUDE_DIR}
    ${ZSTD_INCLUDE_DIR}
  PRIVATE
    ${GLOG_INCLUDE_DIRS}
    ${FIZZ_INCLUDE_DIRECTORIES}
    ${DOUBLE_CONVERSION_INCLUDE_DIR}
)


target_link_libraries(fizz
  PUBLIC
    ${FOLLY_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    sodium
    Threads::Threads
    ZLIB::ZLIB
    ${ZSTD_LIBRARY}
  PRIVATE
    ${GLOG_LIBRARIES}
    ${GFLAGS_LIBRARIES}
    ${FIZZ_LINK_LIBRARIES}
    ${DOUBLE_CONVERSION_LIBRARY}
    ${CMAKE_DL_LIBS}
    ${LIBRT_LIBRARIES})

if ($FIZZ_SHINY_DEPENDENCIES)
  add_dependencies(fizz ${FIZZ_SHINY_DEPENDENCIES})
endif()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  # Work around C1128: number of sections exceeded object file format limit.
  target_compile_options(fizz PUBLIC /bigobj)
endif()

install(
  TARGETS fizz
  EXPORT fizz-exports
  DESTINATION ${LIB_INSTALL_DIR}
)

# We unfortunately cannot install fizz's headers with the install()
# statement above.  install(TARGETS) appears to only support installing
# PUBLIC_HEADER in a flat include directory, and not a deeper tree.
foreach(dir ${FIZZ_HEADER_DIRS})
  get_filename_component(PARENT_DIR "/${dir}" DIRECTORY)
  install(DIRECTORY ${dir} DESTINATION "${INCLUDE_INSTALL_DIR}/fizz${PARENT_DIR}"
          FILES_MATCHING PATTERN "*.h"
          PATTERN "test" EXCLUDE)
endforeach()

# Install CMake package configuration files for fizz
include(CMakePackageConfigHelpers)
configure_package_config_file(
  cmake/fizz-config.cmake.in
  fizz-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DIR}
  PATH_VARS
    INCLUDE_INSTALL_DIR
    CMAKE_INSTALL_DIR
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/fizz-config.cmake
  DESTINATION ${CMAKE_INSTALL_DIR}
)
install(EXPORT fizz-exports
        FILE fizz-targets.cmake
        NAMESPACE fizz::
        DESTINATION ${CMAKE_INSTALL_DIR})

IF(CMAKE_CROSSCOMPILING)
   option(BUILD_TESTS "BUILD_TESTS" OFF)
ELSE(CMAKE_CROSSCOMPILING)
   option(BUILD_TESTS "BUILD_TESTS" ON)
ENDIF(CMAKE_CROSSCOMPILING)

SET(FIZZ_TEST_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

if(BUILD_TESTS)
  find_package(GMock 1.8.0 MODULE REQUIRED)
  find_package(GTest 1.8.0 MODULE REQUIRED)
endif()

add_library(fizz_test_support
  crypto/aead/test/TestUtil.cpp
  crypto/test/TestUtil.cpp
  protocol/ech/test/TestUtil.cpp
  ${FIZZ_TEST_HEADERS}
)

target_link_libraries(fizz_test_support
  PUBLIC
    fizz
    ${LIBGMOCK_LIBRARIES}
    ${GLOG_LIBRARY}
)

target_compile_definitions(fizz_test_support
  PUBLIC
    ${LIBGMOCK_DEFINES}
)

target_include_directories(fizz_test_support
  SYSTEM
  PUBLIC
    ${LIBGMOCK_INCLUDE_DIR}
    ${LIBGTEST_INCLUDE_DIRS}
)

# export fizz headers and targets for unit tests utils
# since other projects such as mvfst and proxygen use them
install(
  TARGETS fizz_test_support
  EXPORT fizz-exports
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
)

foreach(dir ${FIZZ_TEST_HEADER_DIRS})
  get_filename_component(PARENT_DIR "/${dir}" DIRECTORY)
  install(
    DIRECTORY ${dir}
    DESTINATION "${FIZZ_TEST_INSTALL_PREFIX}/include/fizz${PARENT_DIR}"
    FILES_MATCHING PATTERN "*.h"
  )
endforeach()

macro(add_gtest test_source test_name)
  add_executable(${test_name} ${test_source} test/CMakeTestMain.cpp)

  set_property(TARGET ${test_name} PROPERTY ENABLE_EXPORTS true)
  target_include_directories(
    ${test_name} PUBLIC ${LIBGMOCK_INCLUDE_DIR} ${LIBGTEST_INCLUDE_DIR})
  target_compile_definitions(${test_name} PUBLIC ${LIBGMOCK_DEFINES})
  target_link_libraries(
    ${test_name}
    fizz
    fizz_test_support
    ${LIBGMOCK_LIBRARIES})

  if(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    # GMOCK_MOCK_METHOD() will complain otherwise
    target_compile_options(${test_name}
      PRIVATE "-Wno-inconsistent-missing-override")
  endif()

  add_test(${test_name} bin/${test_name})
endmacro(add_gtest)

if(BUILD_TESTS)
  enable_testing()
  add_gtest(client/test/SynchronizedLruPskCacheTest.cpp SyncronizedLruPskCacheTest)
  add_gtest(client/test/AsyncFizzClientTest.cpp AsyncFizzClientTest)
  add_gtest(client/test/ClientProtocolTest.cpp ClientProtocolTest)
  add_gtest(client/test/FizzClientTest.cpp FizzClientTest)
  add_gtest(compression/test/CertDecompressionManagerTest.cpp CertDecompressionManagerTest)
  add_gtest(compression/test/ZlibCertificateCompressorTest.cpp ZlibCertificateCompressorTest)
  add_gtest(crypto/aead/test/EVPCipherTest.cpp EVPCipherTest)
  add_gtest(crypto/aead/test/IOBufUtilTest.cpp IOBufUtilTest)
  add_gtest(crypto/exchange/test/X25519KeyExchangeTest.cpp X25519KeyExchangeTest)
  add_gtest(crypto/exchange/test/ECKeyExchangeTest.cpp ECKeyExchangeTest)
  add_gtest(crypto/hpke/test/ContextTest.cpp ContextTest)
  add_gtest(crypto/hpke/test/DHKEMTest.cpp DHKEMTest)
  add_gtest(crypto/hpke/test/HpkeTest.cpp HpkeTest)
  add_gtest(crypto/openssl/test/OpenSSLKeyUtilsTest.cpp OpenSSLKeyUtilsTest)
  add_gtest(crypto/signature/test/RSAPSSSignatureTest.cpp RSAPSSSignatureTest)
  add_gtest(crypto/signature/test/ECSignatureTest.cpp ECSignatureTest)
  add_gtest(crypto/test/HkdfTest.cpp HkdfTest)
  add_gtest(crypto/test/KeyDerivationTest.cpp KeyDerivationTest)
  add_gtest(crypto/test/RandomGeneratorTest.cpp RandomGeneratorTest)
  add_gtest(crypto/test/UtilsTest.cpp UtilsTest)
  add_gtest(extensions/delegatedcred/test/DelegatedCredTypesTest.cpp DelegatedCredTypesTest)
  add_gtest(extensions/delegatedcred/test/DelegatedCredentialFactoryTest.cpp DelegatedCredentialFactoryTest)
  add_gtest(extensions/delegatedcred/test/DelegatedCredentialCertManagerTest.cpp DelegatedCredentialCertManagerTest)
  add_gtest(extensions/delegatedcred/test/PeerDelegatedCredentialTest.cpp PeerDelegatedCredentialTest)
  add_gtest(extensions/delegatedcred/test/SelfDelegatedCredentialTest.cpp SelfDelegatedCredentialTest)
  add_gtest(extensions/tokenbinding/test/TokenBindingConstructorTest.cpp TokenBindingConstructorTest)
  add_gtest(extensions/tokenbinding/test/ValidatorTest.cpp ValidatorTest)
  add_gtest(extensions/tokenbinding/test/TokenBindingServerExtensionTest.cpp TokenBindingServerExtensionTest)
  add_gtest(extensions/tokenbinding/test/TokenBindingTest.cpp TokenBindingTest)
  add_gtest(extensions/tokenbinding/test/TokenBindingClientExtensionTest.cpp TokenBindingClientExtensionTest)
  add_gtest(experimental/client/test/BatchSignaturePeerCertTest.cpp BatchSignaturePeerCertTest)
  add_gtest(experimental/crypto/test/BatchSignatureTest.cpp BatchSignatureTest)
  add_gtest(experimental/crypto/test/MerkleTreeTest.cpp MerkleTreeTest)
  add_gtest(experimental/protocol/test/BatchSignatureTypesTest.cpp BatchSignatureTypesTest)
  add_gtest(experimental/server/test/BatchSignatureAsyncSelfCertTest.cpp BatchSignatureAsyncSelfCertTest)
  add_gtest(protocol/ech/test/DecrypterTest.cpp DecrypterTest)
  add_gtest(protocol/ech/test/ECHTest.cpp ECHTest)
  add_gtest(protocol/ech/test/EncryptionTest.cpp EncryptionTest)
  add_gtest(protocol/test/CertTest.cpp CertTest)
  add_gtest(protocol/test/FizzBaseTest.cpp FizzBaseTest)
  add_gtest(protocol/test/KeySchedulerTest.cpp KeySchedulerTest)
  add_gtest(protocol/test/DefaultCertificateVerifierTest.cpp DefaultCertificateVerifierTest)
  add_gtest(protocol/test/HandshakeContextTest.cpp HandshakeContextTest)
  add_gtest(protocol/test/ExporterTest.cpp ExporterTest)
  add_gtest(record/test/ExtensionsTest.cpp ExtensionsTest)
  add_gtest(record/test/EncryptedRecordTest.cpp EncryptedRecordTest)
  add_gtest(record/test/TypesTest.cpp TypesTest)
  add_gtest(record/test/HandshakeTypesTest.cpp HandshakeTypesTest)
  add_gtest(record/test/RecordTest.cpp RecordTest)
  add_gtest(record/test/PlaintextRecordTest.cpp PlaintextRecordTest)
  add_gtest(server/test/CertManagerTest.cpp CertManagerTest)
  add_gtest(server/test/CookieCipherTest.cpp CookieCipherTest)
  add_gtest(server/test/DualTicketCipherTest.cpp DualTicketCipherTest)
  add_gtest(server/test/AeadTicketCipherTest.cpp AeadTicketCipherTest)
  add_gtest(server/test/AsyncFizzServerTest.cpp AsyncFizzServerTest)
  add_gtest(server/test/AeadCookieCipherTest.cpp AeadCookieCipherTest)
  add_gtest(server/test/TicketCodecTest.cpp TicketCodecTest)
  add_gtest(server/test/ServerProtocolTest.cpp ServerProtocolTest)
  add_gtest(server/test/NegotiatorTest.cpp NegotiatorTest)
  add_gtest(server/test/FizzServerTest.cpp FizzServerTest)
  add_gtest(server/test/SlidingBloomReplayCacheTest.cpp SlidingBloomReplayCacheTest)
  add_gtest(tool/test/FizzCommandCommonTest.cpp FizzCommandCommonTest)
  add_gtest(util/test/FizzUtilTest.cpp FizzUtilTest)
  add_gtest(util/test/FizzVariantTest.cpp FizzVariantTest)
  add_gtest(util/test/KeyLogWriterTest.cpp KeyLogWriterTest)
  add_gtest(test/AsyncFizzBaseTest.cpp AsyncFizzBaseTest)
  add_gtest(test/HandshakeTest.cpp HandshakeTest)
endif()

option(BUILD_EXAMPLES "BUILD_EXAMPLES" ON)

if(BUILD_EXAMPLES)
  add_executable(BogoShim test/BogoShim.cpp)
  target_link_libraries(BogoShim fizz sodium)
  set_target_properties(BogoShim PROPERTIES OUTPUT_NAME fizz-bogoshim)
  install(
    TARGETS BogoShim
    DESTINATION ${BIN_INSTALL_DIR}
  )
  add_executable(FizzTool
      tool/Main.cpp
      tool/FizzClientCommand.cpp
      tool/FizzClientLoadGenCommand.cpp
      tool/FizzCommandCommon.cpp
      tool/FizzGenerateDelegatedCredentialCommand.cpp
      tool/FizzServerBenchmarkCommand.cpp
      tool/FizzServerCommand.cpp)
  target_link_libraries(FizzTool fizz sodium)
  set_target_properties(FizzTool PROPERTIES OUTPUT_NAME fizz)
  install(
    TARGETS FizzTool
    DESTINATION ${BIN_INSTALL_DIR}
  )
endif()
