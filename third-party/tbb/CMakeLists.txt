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

ExternalProject_Add(
  bundled_tbb
  EXCLUDE_FROM_ALL
  SOURCE_DIR "${TBB_ROOT}"
  CONFIGURE_COMMAND ""
  PATCH_COMMAND
    patch -p4 < "${CMAKE_CURRENT_SOURCE_DIR}/pic_compile.patch"
  BUILD_COMMAND
    cd <SOURCE_DIR> && make
      work_dir=<INSTALL_DIR>
      tbb_root=${TBB_ROOT}
      CC=${CMAKE_C_COMPILER}
      CXX=${CMAKE_CXX_COMPILER}
      CXXFLAGS=${CMAKE_CXX_FLAGS}
      tbb
  INSTALL_COMMAND ""
)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
 execute_process(COMMAND "patch" "-p1" "-f" INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/clang_fix.patch")
endif()

add_dependencies(tbb bundled_tbb)

ExternalProject_Get_Property(bundled_tbb INSTALL_DIR)
target_include_directories(tbb INTERFACE "${TBB_ROOT}/include")

set(TBB_STATIC_LIB_PATH ${INSTALL_DIR}$<IF:$<CONFIG:Debug>,_debug,_release>/${CMAKE_STATIC_LIBRARY_PREFIX}tbb$<$<CONFIG:Debug>:_debug>${CMAKE_STATIC_LIBRARY_SUFFIX})

target_link_libraries(
  tbb
  INTERFACE
  "${TBB_STATIC_LIB_PATH}"
)
