# Only the new test is wired up for now; the other *.cpp files in this directory
# are not yet built.  Add them here as they are brought into the target.
set(CXX_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/perf-cputime-ns-test.cpp
)

add_executable(hphp_util_test ${CXX_SOURCES})
link_object_libraries(hphp_util_test ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
target_link_libraries(hphp_util_test ${HHVM_LINK_LIBRARIES} GTest::gtest_main)

auto_source_group("hphp_util_test" "${CMAKE_CURRENT_SOURCE_DIR}" ${CXX_SOURCES})

# Benchmark: perf cputime vs folly::chrono::clock_gettime_ns
add_executable(hphp_util_bench
  ${CMAKE_CURRENT_SOURCE_DIR}/perf-cputime-ns-bench.cpp
)
link_object_libraries(hphp_util_bench ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
target_link_libraries(hphp_util_bench ${HHVM_LINK_LIBRARIES} Folly::follybenchmark)
