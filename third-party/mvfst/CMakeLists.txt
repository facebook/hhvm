include(ExternalProject)
include(HPHPFunctions)

get_target_property(BOOST_INCLUDE_DIR boost INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(BOOST_LIBRARIES boost INTERFACE_LINK_LIBRARIES)
list(GET BOOST_LIBRARIES 0 FIRST_LIB)
if("${FIRST_LIB}" MATCHES ".+/${CMAKE_STATIC_LIBRARY_PREFIX}boost_.+${CMAKE_STATIC_LIBRARY_SUFFIX}$")
  set(Boost_USE_STATIC_LIBS ON)
else()
  set(Boost_USE_STATIC_LIBS OFF)
endif()
get_filename_component(BOOST_LIBRARY_DIR "${FIRST_LIB}" DIRECTORY)

get_target_property(JEMALLOC_INCLUDE_DIR jemalloc INTERFACE_INCLUDE_DIRECTORIES)


ExternalProject_Add(
  bundled_mvfst
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/
  BUILD_ALWAYS ON
  CMAKE_ARGS
    "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
    -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/CMake
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include

    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}

    -DBUILD_TESTS=OFF

    -DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
    -DOPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}
    -DBoost_NO_SYSTEM_PATHS=ON
    -DBoost_NO_BOOST_CMAKE=ON
    -DBoost_USE_STATIC_LIBS=${Boost_USE_STATIC_LIBS}
    -DBoost_INCLUDE_DIR=${BOOST_INCLUDE_DIR}
    "-DBOOST_LIBRARYDIR=${BOOST_LIBRARY_DIR}"
    -DLIBGLOG_INCLUDE_DIR=${GLOG_INCLUDE_DIR}
    -DLIBGLOG_LIBRARY=${GLOG_LIBRARY}

    "-DFizz_DIR=${FIZZ_INSTALL_DIR}/lib/cmake/fizz"
    "-Dfmt_DIR=${fmt_DIR}"
    "-Dfolly_DIR=${FOLLY_INSTALL_DIR}/lib/cmake/folly"

    "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -I${JEMALLOC_INCLUDE_DIR}"
)
ExternalProject_Get_property(bundled_mvfst INSTALL_DIR)

set(
  MVFST_DEPS
  fizz
  fmt
  folly
)

set(
  MVFST_LIBRARIES
  mvfst_constants
  mvfst_exception
  mvfst_transport
  mvfst_batch_writer
  mvfst_client
  mvfst_codec_types
  mvfst_codec_decode
  mvfst_codec_pktbuilder
  mvfst_codec_pktrebuilder
  mvfst_codec_packet_number_cipher
  mvfst_codec
  mvfst_looper
  mvfst_buf_accessor
  mvfst_bufutil
  mvfst_transport_knobs
  mvfst_events
  mvfst_async_udp_socket
  mvfst_cc_algo
  mvfst_dsr_types
  mvfst_dsr_frontend
  mvfst_fizz_client
  mvfst_fizz_handshake
  mvfst_flowcontrol
  mvfst_handshake
  mvfst_happyeyeballs
  mvfst_qlogger
  mvfst_loss
  mvfst_observer
  mvfst_server
  mvfst_server_state
  mvfst_server_async_tran
  mvfst_state_machine
  mvfst_state_ack_handler
  mvfst_state_datagram_handler
  mvfst_state_stream_functions
  mvfst_state_pacing_functions
  mvfst_state_functions
  mvfst_state_simple_frame_functions
  mvfst_state_stream
  mvfst_transport_settings_functions
  mvfst_xsk
)

add_dependencies(bundled_mvfst ${MVFST_DEPS})
add_library(mvfst INTERFACE)
add_dependencies(mvfst bundled_mvfst)
target_include_directories(mvfst INTERFACE "${INSTALL_DIR}/include")

foreach(MVFST_LIBRARY_NAME ${MVFST_LIBRARIES})
  target_link_libraries(
    mvfst
    INTERFACE
    "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${MVFST_LIBRARY_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )
endforeach()

set(MVFST_INSTALL_DIR "${INSTALL_DIR}" PARENT_SCOPE)
