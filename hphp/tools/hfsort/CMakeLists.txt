add_executable(hfsort "hfsort.cpp" "hfutil.cpp")
target_link_libraries(hfsort Folly::folly hphp_util zlib)
hphp_link(hfsort)
ADD_LIBRARY(jit_sort STATIC
            "jitsort.cpp" "hfutil.cpp")
target_link_libraries(jit_sort hphp_util)
hphp_link(jit_sort)
