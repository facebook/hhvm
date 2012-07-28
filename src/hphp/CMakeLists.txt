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
else()
set(ENV{HHVM_BINARY} "1")
set(ENV{HPHP_EMITTER} "1")
set(ENV{HHVM} "1")
add_custom_command(TARGET hphp
	POST_BUILD
	COMMAND env "HHVM=1" "HHVM_BINARY=1" "HPHP_EMITTER=1" "HHVM_EXT_LIB=${HPHP_HOME}/bin/libext_hhvm.a" ${HPHP_HOME}/src/hphp/hphp ARGS -t cpp -f exe --input-dir ${HPHP_HOME}/src/hhvm/ -i hhvm.php -o ${HPHP_HOME}/src/hhvm/gen --sync-dir=${HPHP_HOME}/src/hhvm/sync -vEnableEval=2 --log=1
	COMMAND mv ARGS gen/program hhvm
	WORKING_DIRECTORY ${HPHP_HOME}/src/hhvm
	COMMENT "Building hhvm"
	VERBATIM
)
endif(NOT USE_HHVM)
