add_library(tbb INTERFACE)

find_package(TBB QUIET)
if(TBB_FOUND AND TARGET TBB::tbb)
  message(STATUS "Using system TBB")
  target_link_libraries(tbb INTERFACE TBB::tbb)
  return()
endif()

find_library(TBB_LIBRARY tbb)
find_path(TBB_INCLUDE_DIR tbb/tbb.h PATH_SUFFIXES tbb)
if(TBB_LIBRARY AND TBB_INCLUDE_DIR)
  message(STATUS "Using system TBB (manual): ${TBB_LIBRARY}")
  target_include_directories(tbb INTERFACE "${TBB_INCLUDE_DIR}")
  target_link_libraries(tbb INTERFACE "${TBB_LIBRARY}")
  return()
endif()

message(STATUS "Using third-party bundled TBB")
include(ExternalProject)

set(TBB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TBB_STATIC_LIB_PATH $<IF:$<CONFIG:Debug>,_debug,_release>/${CMAKE_STATIC_LIBRARY_PREFIX}tbb$<$<CONFIG:Debug>:_debug>${CMAKE_STATIC_LIBRARY_SUFFIX})
set(TBB_PATCH_COMMAND
  patch -p4 < "${CMAKE_CURRENT_SOURCE_DIR}/pic_compile.patch"
)
set(TBB_COMPILER_ARG)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  list(APPEND TBB_PATCH_COMMAND
    COMMAND
      patch -p4 < "${CMAKE_CURRENT_SOURCE_DIR}/clang_fix.patch"
  )
  set(TBB_COMPILER_ARG compiler=clang)
endif()

ExternalProject_Add(
  bundled_tbb
  EXCLUDE_FROM_ALL
  SOURCE_DIR "${TBB_ROOT}"
  CONFIGURE_COMMAND ""
  PATCH_COMMAND ${TBB_PATCH_COMMAND}
  BUILD_COMMAND
    cd <SOURCE_DIR> && make
      work_dir=<INSTALL_DIR>
      tbb_root=${TBB_ROOT}
      CC=${CMAKE_C_COMPILER}
      CXX=${CMAKE_CXX_COMPILER}
      CXXFLAGS=${CMAKE_CXX_FLAGS}
      ${TBB_COMPILER_ARG}
      tbb
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS "<INSTALL_DIR>${TBB_STATIC_LIB_PATH}"
)

add_dependencies(tbb bundled_tbb)

ExternalProject_Get_Property(bundled_tbb INSTALL_DIR)
target_include_directories(tbb INTERFACE "${TBB_ROOT}/include")

target_link_libraries(
  tbb
  INTERFACE
  "${INSTALL_DIR}${TBB_STATIC_LIB_PATH}"
)
