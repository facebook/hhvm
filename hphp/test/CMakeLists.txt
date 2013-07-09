set(CXX_SOURCES)

set(RECURSIVE_SOURCE_SUBDIRS vm)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})
	auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND CXX_SOURCES ${files})
endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})

auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})
list(APPEND CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../hhvm/process_init.cpp")
list(APPEND CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../hhvm/global_variables.cpp")

add_executable(test ${CXX_SOURCES})

if (APPLE)
	target_link_libraries(test hphp_analysis hphp_runtime_static ext_hhvm_static hphp_system
	                      -Wl,-sectcreate,__text,systemlib,${HPHP_HOME}/bin/systemlib.php)
else()
	target_link_libraries(test hphp_analysis hphp_runtime_static ext_hhvm_static hphp_system
	                      -Wl,-uregister_libevent_server)

	add_custom_command(TARGET test POST_BUILD
	                   COMMAND "objcopy"
	                   ARGS "--add-section" "systemlib=${HPHP_HOME}/bin/systemlib.php"
	                        "${CMAKE_CURRENT_SOURCE_DIR}/test"
	                   COMMENT "Embedding systemlib.php in test")
endif()

add_dependencies(test systemlib)
