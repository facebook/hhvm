include(Options)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

IF(NOT DEFINED CMAKE_PREFIX_PATH)
  message(STATUS "CMAKE_PREFIX_PATH was missing, proceeding anyway")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	INCLUDE(CheckCSourceCompiles)
	CHECK_C_SOURCE_COMPILES("#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40300
#error Need GCC 4.3.0+
#endif
int main() { return 0; }" HAVE_GCC_43)

	if(NOT HAVE_GCC_43)
		message(FATAL_ERROR "Need at least GCC 4.3")
	endif()

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

add_definitions(-D_GNU_SOURCE -D_REENTRANT=1 -D_PTHREADS=1)

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	add_definitions(-DRELEASE=1)
endif()

if(INFINITE_LOOP_DETECTION)
	add_definitions(-DINFINITE_LOOP_DETECTION=1)
endif()

if(INFINITE_RECURSION_DETECTION)
	add_definitions(-DINFINITE_RECURSION_DETECTION=1)
endif()

if(REQUEST_TIMEOUT_DETECTION)
	add_definitions(-DREQUEST_TIMEOUT_DETECTION=1)
endif()

if(ENABLE_LATE_STATIC_BINDING)
	add_definitions(-DENABLE_LATE_STATIC_BINDING=1)
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

if(HOTPROFILER_NO_BUILTIN)
	add_definitions(-DHOTPROFILER_NO_BUILTIN=1)
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

# enable the OSS options if we have any
add_definitions(-DHPHP_OSS=1)

execute_process(COMMAND git describe --all --long --abbrev=40 --always
    OUTPUT_VARIABLE _COMPILER_ID OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

if (_COMPILER_ID)
	add_definitions(-DCOMPILER_ID="${_COMPILER_ID}")
endif()

IF($ENV{CXX} MATCHES "icpc")
	set(CMAKE_C_FLAGS "-no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -w")
	set(CMAKE_CXX_FLAGS "-no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -fno-omit-frame-pointer -ftemplate-depth-60 -Wall -Woverloaded-virtual -Wno-deprecated -w1 -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names")
else()
	set(CMAKE_C_FLAGS "-w")
	set(CMAKE_CXX_FLAGS "-fno-gcse -fno-omit-frame-pointer -ftemplate-depth-60 -Wall -Woverloaded-virtual -Wno-deprecated -Wno-parentheses -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names")
endif()

IF(CMAKE_COMPILER_IS_GNUCC)
	SET (CMAKE_C_FLAGS_RELEASE "-O3")
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
	SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
ENDIF()

include_directories(${HPHP_HOME}/src)
include_directories(${HPHP_HOME}/src/lib/system/gen)
