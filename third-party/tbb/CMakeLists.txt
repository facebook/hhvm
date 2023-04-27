include(ExternalProject)

set(TBB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")

# TBB recommends using `tbb_build` but the whole logic seems pretty
# fragile and opaque so we just call Make directly. This way we get
# to control our arguments.

ExternalProject_Add(
  bundled_tbb
  EXCLUDE_FROM_ALL
  SOURCE_DIR "${TBB_ROOT}"
  CONFIGURE_COMMAND ""
  PATCH_COMMAND
    patch -p4 < "${CMAKE_CURRENT_SOURCE_DIR}/pic_compile.patch"
  BUILD_COMMAND
    # We only depend on TBB so we only build this target.
    cd <SOURCE_DIR> && make
      work_dir=<INSTALL_DIR>
      tbb_root=${TBB_ROOT}
      CC=${CMAKE_C_COMPILER}
      CXX=${CMAKE_CXX_COMPILER}
      # TODO: We should pass CFLAGS/CXXFLAGS.
      tbb
  INSTALL_COMMAND ""
)

add_library(tbb INTERFACE)
add_dependencies(tbb bundled_tbb)

ExternalProject_Get_Property(bundled_tbb INSTALL_DIR)
target_include_directories(tbb INTERFACE "${TBB_ROOT}/include")

set(TBB_STATIC_LIB_PATH ${INSTALL_DIR}$<IF:$<CONFIG:Debug>,_debug,_release>/${CMAKE_STATIC_LIBRARY_PREFIX}tbb$<$<CONFIG:Debug>:_debug>${CMAKE_STATIC_LIBRARY_SUFFIX})

target_link_libraries(
  tbb
  INTERFACE
  "${TBB_STATIC_LIB_PATH}"
)
