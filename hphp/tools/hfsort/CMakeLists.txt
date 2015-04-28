find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    add_executable(hfsort "hfsort.cpp" "hfutil.cpp")
    target_link_libraries(hfsort folly hphp_util ${ZLIB_LIBRARIES})
    ADD_LIBRARY(jit_sort STATIC
                "jitsort.cpp" "hfutil.cpp")
    hphp_link(jit_sort)
endif(ZLIB_FOUND)
