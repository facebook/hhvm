set(CXX_SOURCES)
auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})

add_executable(hphp ${CXX_SOURCES})
find_library(DL_LIBRARY dl)
if (DL_LIBRARY)
    target_link_libraries(hphp ${DL_LIBRARY})
endif(DL_LIBRARY)

target_link_libraries(hphp hphp_analysis hphp_runtime_static)

add_custom_command(TARGET hphp
        POST_BUILD
        COMMAND hphp ARGS -t cpp -f exe --input-dir . -i hphpi.php -o gen --config hphpi_build.hdf --log=1
	COMMAND mv ARGS gen/program hphpi
        WORKING_DIRECTORY ${HPHP_HOME}/src/hphpi
        COMMENT "Building hphpi"
)

