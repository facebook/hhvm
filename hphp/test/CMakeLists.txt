set(CXX_SOURCES)

set(RECURSIVE_SOURCE_SUBDIRS ext)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})
	auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND CXX_SOURCES ${files})
endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})

list(APPEND CXX_SOURCES ${files})
list(APPEND CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../hhvm/process-init.cpp")
list(APPEND CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../hhvm/global-variables.cpp")

add_executable(test ${CXX_SOURCES})
target_link_libraries(test hphp_analysis hphp_runtime_static ext_hhvm_static hphp_system
                      -Wl,-uregister_libevent_server)
embed_systemlib(test "${CMAKE_CURRENT_SOURCE_DIR}/test" ${HPHP_HOME}/bin/systemlib.php)
add_dependencies(test systemlib)
