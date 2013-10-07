include(Options)

if (APPLE)
	set(HHVM_LINK_LIBRARIES
	    hphp_analysis
	    hphp_runtime_static
	    ext_hhvm_static
	    hphp_system
	    hphp_parser
	    vixl neo
	    -Wl,-u,_register_libevent_server)
else ()
	set(HHVM_LINK_LIBRARIES
            hphp_analysis
            hphp_runtime_static
            ext_hhvm_static
            hphp_system
	    hphp_parser
	    vixl neo
            -Wl,-uregister_libevent_server)
endif ()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

IF(NOT DEFINED CMAKE_PREFIX_PATH)
  message(STATUS "CMAKE_PREFIX_PATH was missing, proceeding anyway")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	INCLUDE(CheckCSourceCompiles)
	CHECK_C_SOURCE_COMPILES("#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40600
#error Need GCC 4.6.0+
#endif
int main() { return 0; }" HAVE_GCC_46)

	if(NOT HAVE_GCC_46)
		message(FATAL_ERROR "Need at least GCC 4.6")
	endif()

        CHECK_C_SOURCE_COMPILES("#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40700
#error Not GCC 4.7.0+
#endif
int main() { return 0; }" HAVE_GCC_47)

endif()

set(FREEBSD FALSE)
set(LINUX FALSE)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
	set(FREEBSD TRUE)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set(LINUX TRUE)
endif()

LIST(APPEND CMAKE_PREFIX_PATH "$ENV{CMAKE_PREFIX_PATH}")

if(APPLE)
	if(EXISTS "/opt/local/var/macports/")
		LIST (APPEND CMAKE_PREFIX_PATH "/opt/local")
		LIST (APPEND CMAKE_LIBRARY_PATH "/opt/local/lib/x86_64")
	endif()
endif()

include(HPHPFunctions)
include(HPHPFindLibs)

add_definitions(-D_REENTRANT=1 -D_PTHREADS=1 -D__STDC_FORMAT_MACROS)
add_definitions(-DHHVM_LIB_PATH_DEFAULT="${HPHP_HOME}/bin")

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

# enable the OSS options if we have any
add_definitions(-DHPHP_OSS=1)

# later versions of binutils don't play well without automake
add_definitions(-DPACKAGE=hhvm -DPACKAGE_VERSION=Release)

IF($ENV{CXX} MATCHES "icpc")
	set(CMAKE_C_FLAGS "-no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -w")
	set(CMAKE_CXX_FLAGS "-no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -fno-omit-frame-pointer -ftemplate-depth-120 -Wall -Woverloaded-virtual -Wno-deprecated -w1 -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names")
else()
	set(GNUCC_UNINIT_OPT "")
	if(HAVE_GCC_47)
		set(GNUCC_UNINIT_OPT "-Wno-maybe-uninitialized")
	endif()
	set(CMAKE_C_FLAGS "-w")
	set(CMAKE_CXX_FLAGS "-fno-gcse -fno-omit-frame-pointer -ftemplate-depth-120 -Wall -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -std=gnu++0x -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes ${GNUCC_UNINIT_OPT}")
endif()

IF(CMAKE_COMPILER_IS_GNUCC)
	SET (CMAKE_C_FLAGS_RELEASE "-O3")
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
	SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
ENDIF()

include_directories(${HPHP_HOME}/hphp)
include_directories(${HPHP_HOME}/hphp/lib/system/gen)
include_directories(${HPHP_HOME})
