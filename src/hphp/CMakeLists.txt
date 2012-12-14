set(CXX_SOURCES)
auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})

add_executable(hphp ${CXX_SOURCES})
find_library(DL_LIBRARY dl)

ADD_DEFINITIONS("-DCOMPILERID='vm branch'")

IF(DL_LIBRARY)
    target_link_libraries(hphp ${DL_LIBRARY})
ENDIF()

target_link_libraries(hphp hphp_analysis hphp_runtime_static)

IF(EXISTS "${HPHP_HOME}/src/hphpi/gen/CMakeCache.txt")
  file(REMOVE_RECURSE "${HPHP_HOME}/src/hphpi/gen/")
ENDIF()

if (NOT USE_HHVM)
add_custom_command(TARGET hphp
        POST_BUILD
        COMMAND hphp ARGS -t cpp -f exe --input-dir . -i hphpi.php -o gen -vEnableEval=2 --log=1
        COMMAND mv ARGS gen/program hphpi
        WORKING_DIRECTORY ${HPHP_HOME}/src/hphpi
        COMMENT "Building hphpi"
)
endif()
