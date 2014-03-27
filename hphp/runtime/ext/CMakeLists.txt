HHVM_SELECT_SOURCES("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${HPHP_HOME}/hphp/third_party")

ADD_LIBRARY(hphp_runtime_ext STATIC
            ${CXX_SOURCES} ${C_SOURCES} ${ASM_SOURCES})
hphp_link(hphp_runtime_ext)
