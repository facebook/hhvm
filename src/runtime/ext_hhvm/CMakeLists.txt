
if (USE_HHVM)

message("Including ext_hhvm files")

set(files)
set(CXX_SOURCES)
set(C_SOURCES)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

set(RECURSIVE_SOURCE_SUBDIRS "${PARENT_DIR}/base" "${PARENT_DIR}/ext" "${CMAKE_CURRENT_SOURCE_DIR}")

foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})
        auto_sources(files "*ext_hhvm*.cpp" "RECURSE" "${dir}")
        list(APPEND CXX_SOURCES ${files})
endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})

ADD_LIBRARY(ext_hhvm_static STATIC ${CXX_SOURCES}) 
SET_TARGET_PROPERTIES(ext_hhvm_static PROPERTIES OUTPUT_NAME "ext_hhvm")
SET_TARGET_PROPERTIES(ext_hhvm_static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(ext_hhvm_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

endif(USE_HHVM)
