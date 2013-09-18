set(CXX_SOURCES)
auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})

add_executable(hhvm ${CXX_SOURCES})
target_link_libraries(hhvm hphp_analysis hphp_runtime_static ext_hhvm_static hphp_system
    -Wl,-u,_register_libevent_server)
embed_systemlib(hhvm "${CMAKE_CURRENT_SOURCE_DIR}/hhvm" ${HPHP_HOME}/bin/systemlib.php)
add_dependencies(hhvm systemlib)

install(TARGETS hhvm DESTINATION bin)
