set(CXX_SOURCES)
auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})

add_executable(hhvm ${CXX_SOURCES})
target_link_libraries(hhvm ${HHVM_LINK_LIBRARIES})
embed_systemlib(hhvm "${CMAKE_CURRENT_SOURCE_DIR}/hhvm" ${HPHP_HOME}/hphp/system/systemlib.php)
add_dependencies(hhvm systemlib)

HHVM_INSTALL(hhvm bin)
