set(CXX_SOURCES)

set(RECURSIVE_SOURCE_SUBDIRS ext)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})
	auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND CXX_SOURCES ${files})
endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})

list(APPEND CXX_SOURCES ${files})
list(APPEND CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../hhvm/process-init.cpp")
list(APPEND CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../hhvm/global-variables.cpp")

if(NOT HAVE_CUSTOM_LIBEVENT)
	list(REMOVE_ITEM CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ext/test_ext_curl.cpp")
endif()

include_directories("${HPHP_HOME}/hphp/third_party")

add_executable(test ${CXX_SOURCES})
target_link_libraries(test ${HHVM_LINK_LIBRARIES})
embed_systemlib(test "${CMAKE_CURRENT_SOURCE_DIR}/test" ${HPHP_HOME}/hphp/system/systemlib.php)
add_dependencies(test systemlib)
