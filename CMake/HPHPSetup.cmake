include(HPHPFunctions)
include(HPHPFindLibs)

OPTION(INFINITE_LOOP_DETECTION "Enable Infinite Loop Detection" ON)
OPTION(INFINITE_RECURSION_DETECTION "Enable Infinite Recursion Detection" ON)
OPTION(REQUEST_TIMEOUT_DETECTION "Enable Timeout Detection" ON)

add_definitions(-D_GNU_SOURCE -D_REENTRANT=1 -D_PTHREADS=1)

#should wrap this in an if
add_definitions(-DRELEASE=1)

# eable the OSS options if we have any
add_definitions(-DHPHP_OSS=1)

set(HPHP_OPT "-O3")

set(CMAKE_C_FLAGS "${HPHP_OPT} -w -fPIC")
set(CMAKE_CXX_FLAGS "${HPHP_OPT} -fPIC -fno-omit-frame-pointer -ftemplate-depth-60 -Wall -Woverloaded-virtual -Wno-deprecated -Wno-parentheses -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof")

include_directories(${HPHP_HOME}/src)
include_directories(${HPHP_HOME}/src/lib/system/gen)
