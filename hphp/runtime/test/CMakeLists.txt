find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(CXX_SOURCES)
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files} "${CMAKE_CURRENT_SOURCE_DIR}/../../hhvm/process-init.cpp")

set(HEADER_SOURCES)
auto_sources(files "*.h" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND HEADER_SOURCES ${files})

add_executable(hphp_runtime_test ${CXX_SOURCES} ${HEADER_SOURCES})
link_object_libraries(hphp_runtime_test ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
target_link_libraries(hphp_runtime_test ${HHVM_LINK_LIBRARIES} gtest gmock)
embed_all_systemlibs(hphp_runtime_test "${CMAKE_CURRENT_BINARY_DIR}/../.."
                              "${CMAKE_CURRENT_BINARY_DIR}/hphp_runtime_test")

auto_source_group("hphp_runtime_test" "${CMAKE_CURRENT_SOURCE_DIR}"
  ${CXX_SOURCES} ${HEADER_SOURCES})
target_link_libraries(hphp_runtime_test hphp_system)
if (ENABLE_COTIRE)
  cotire(hphp_runtime_test)
endif()
