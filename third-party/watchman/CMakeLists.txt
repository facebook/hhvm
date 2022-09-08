add_library(libwatchmanclient INTERFACE)

include(ExternalProject)
include(HPHPFunctions)


get_target_property(BOOST_INCLUDE_DIR boost INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(DOUBLE_CONVERSION_INCLUDE_DIR double-conversion INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(FMT_INCLUDE_DIR fmt INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(FOLLY_INCLUDE_DIR folly INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(JEMALLOC_INCLUDE_DIR jemalloc INTERFACE_INCLUDE_DIRECTORIES)

set(WATCHMAN_DEPS folly boost fmt jemalloc double-conversion)

ExternalProject_Add(
  bundled_watchman
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/
  PATCH_COMMAND
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/cppclient-CMakeLists.txt"
    "watchman/cppclient/CMakeLists.txt"
  SOURCE_SUBDIR watchman/cppclient
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib

    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}

    "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}"
    "-DBOOST_INCLUDE_DIR=${BOOST_INCLUDE_DIR}"
    "-DDOUBLE_CONVERSION_INCLUDE_DIR=${DOUBLE_CONVERSION_INCLUDE_DIR}"
    "-DFMT_INCLUDE_DIR=${FMT_INCLUDE_DIR}"
    "-DFOLLY_INCLUDE_DIR=${FOLLY_INCLUDE_DIR}"
    "-DGLOG_INCLUDE_DIR=${GLOG_INCLUDE_DIR}"
    "-DJEMALLOC_INCLUDE_DIR=${JEMALLOC_INCLUDE_DIR}"
    "-DLIBEVENT_INCLUDE_DIR=${LIBEVENT_INCLUDE_DIR}"
    "-DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}"
  DEPENDS ${WATCHMAN_DEPS}
  EXCLUDE_FROM_ALL
)

find_package(libWatchmanClient)
if(WATCHMANCLIENT_INCLUDE_DIRS AND WATCHMANCLIENT_LIBRARIES)
  target_include_directories(libwatchmanclient INTERFACE "${WATCHMANCLIENT_INCLUDE_DIRS}")
  target_link_libraries(libwatchmanclient INTERFACE "${WATCHMANCLIENT_LIBRARIES}")
else()
  add_dependencies(libwatchmanclient bundled_watchman)
  ExternalProject_Get_Property(bundled_watchman INSTALL_DIR)
  target_include_directories(libwatchmanclient INTERFACE "${INSTALL_DIR}/include")
  add_library(watchman_deps INTERFACE)
  target_link_libraries(watchman_deps INTERFACE ${WATCHMAN_DEPS})
  target_link_libraries(
    libwatchmanclient
    INTERFACE
    "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}watchmanclient${CMAKE_STATIC_LIBRARY_SUFFIX}"
    watchman_deps
  )
endif()
