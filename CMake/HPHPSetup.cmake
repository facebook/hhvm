include(Options)

# Do this until cmake has a define for ARMv8
INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
#ifndef __AARCH64EL__
#error Not ARMv8  
#endif
int main() { return 0; }" IS_AARCH64)

if (APPLE OR IS_AARCH64)
	set(HHVM_ANCHOR_SYMS -Wl,-u,_register_libevent_server)
else()
	set(ENABLE_FASTCGI 1)
	set(HHVM_ANCHOR_SYMS -Wl,-uregister_libevent_server,-uregister_fastcgi_server)
endif()

set(HHVM_LINK_LIBRARIES
    hphp_analysis
    hphp_runtime_static
    ext_hhvm_static
    hphp_system
    hphp_parser
    hphp_zend
    hphp_util
    hphp_hhbbc
    vixl neo
    ${HHVM_ANCHOR_SYMS})

if(ENABLE_FASTCGI)
	LIST(APPEND HHVM_LINK_LIBRARIES hphp_thrift)
	LIST(APPEND HHVM_LINK_LIBRARIES hphp_proxygen)
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

IF(NOT DEFINED CMAKE_PREFIX_PATH)
  message(STATUS "CMAKE_PREFIX_PATH was missing, proceeding anyway")
endif()

# Look for the chrpath tool so we can warn if it's not there
find_program(CHRPATH chrpath)
IF (CHRPATH STREQUAL "CHRPATH-NOTFOUND")
    SET(FOUND_CHRPATH OFF)
    message(WARNING "chrpath not found, rpath will not be stripped from installed binaries")
else()
    SET(FOUND_CHRPATH ON)
endif()

LIST(APPEND CMAKE_PREFIX_PATH "$ENV{CMAKE_PREFIX_PATH}")

if(APPLE)
	if(EXISTS "/opt/local/var/macports/")
		LIST (APPEND CMAKE_PREFIX_PATH "/opt/local")
		LIST (APPEND CMAKE_LIBRARY_PATH "/opt/local/lib/x86_64")
	endif()
endif()

include(HPHPCompiler)
include(HPHPFunctions)
include(HPHPFindLibs)

add_definitions(-D_REENTRANT=1 -D_PTHREADS=1 -D__STDC_FORMAT_MACROS)

if (LINUX)
	add_definitions(-D_GNU_SOURCE)
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	add_definitions(-DRELEASE=1)
	add_definitions(-DNDEBUG)
	message("Generating Release build")
else()
	add_definitions(-DDEBUG)
	message("Generating DEBUG build")
endif()

if(DEBUG_MEMORY_LEAK)
	add_definitions(-DDEBUG_MEMORY_LEAK=1)
endif()

if(DEBUG_APC_LEAK)
	add_definitions(-DDEBUG_APC_LEAK=1)
endif()

if(ALWAYS_ASSERT)
	add_definitions(-DALWAYS_ASSERT=1)
endif()

if(HOTPROFILER)
	add_definitions(-DHOTPROFILER=1)
endif()

if(EXECUTION_PROFILER)
	add_definitions(-DEXECUTION_PROFILER=1)
endif()

if(ENABLE_FULL_SETLINE)
	add_definitions(-DENABLE_FULL_SETLINE=1)
endif()

if(APPLE OR FREEBSD)
	add_definitions(-DSKIP_USER_CHANGE=1)
endif()

if(APPLE)
	# We have to be a little more permissive in some cases.
	add_definitions(-fpermissive)

	# Skip deprecation warnings in OpenSSL. 
	add_definitions(-DMAC_OS_X_VERSION_MIN_REQUIRED=MAC_OS_X_VERSION_10_6)

	# Just assume we have sched.h
	add_definitions(-DFOLLY_HAVE_SCHED_H=1)

	# Enable weak linking
	add_definitions(-DMACOSX_DEPLOYMENT_TARGET=10.6)
endif()

if(ENABLE_FASTCGI)
	add_definitions(-DENABLE_FASTCGI=1)
endif ()

# enable the OSS options if we have any
add_definitions(-DHPHP_OSS=1)

# later versions of binutils don't play well without automake
add_definitions(-DPACKAGE=hhvm -DPACKAGE_VERSION=Release)

include_directories(${HPHP_HOME}/hphp)
include_directories(${HPHP_HOME}/hphp/lib/system/gen)
include_directories(${HPHP_HOME})
