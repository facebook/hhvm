set(CXX_SOURCES)

set(RECURSIVE_SOURCE_SUBDIRS vm)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})
	auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND CXX_SOURCES ${files})
endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})

auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})
list(REMOVE_ITEM CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/genCmpTest.cpp")
list(APPEND CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../hhvm/process_init.cpp")

add_definitions(-DHPHPI_PATH="hphpi/hphpi")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_base_suite.inc ${CMAKE_CURRENT_SOURCE_DIR}/test_suite.inc COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_base_fast.inc ${CMAKE_CURRENT_SOURCE_DIR}/test_fast.inc COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dummy_mysql_info.inc ${CMAKE_CURRENT_SOURCE_DIR}/test_mysql_info.inc COPYONLY)

add_executable(test ${CXX_SOURCES})
target_link_libraries(test hphp_analysis hphp_runtime_static)
