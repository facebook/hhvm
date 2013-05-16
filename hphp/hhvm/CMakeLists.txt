set(CXX_SOURCES)
auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})

add_executable(hhvm ${CXX_SOURCES})
target_link_libraries(hhvm hphp_analysis hphp_runtime_static ext_hhvm_static)

add_custom_command(TARGET hhvm POST_BUILD
                   COMMAND "objcopy"
                   ARGS "--add-section" "systemlib=${HPHP_HOME}/bin/systemlib.php"
                        "${CMAKE_CURRENT_SOURCE_DIR}/hhvm"
                   COMMENT "Embedding systemlib.php in hhvm")

install(TARGETS hhvm DESTINATION bin)
