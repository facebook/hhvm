include(Options)

set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug RelWithDebInfo)

# Do this until cmake has a define for ARMv8
INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
#ifndef __x86_64__
#error Not x64
#endif
int main() { return 0; }" IS_X64)

CHECK_CXX_SOURCE_COMPILES("
#ifndef __AARCH64EL__
#error Not ARMv8
#endif
int main() { return 0; }" IS_AARCH64)

CHECK_CXX_SOURCE_COMPILES("
#ifndef __powerpc64__
#error Not PPC64
#endif
int main() { return 0; }" IS_PPC64)

set(HHVM_WHOLE_ARCHIVE_LIBRARIES
    hphp_runtime_static
    hphp_runtime_ext
   )

if (ENABLE_ZEND_COMPAT)
  list(APPEND HHVM_WHOLE_ARCHIVE_LIBRARIES hphp_ext_zend_compat)
endif()

if (LINUX)
  set(HHVM_WRAP_SYMS -Wl,--wrap=pthread_create -Wl,--wrap=pthread_exit -Wl,--wrap=pthread_join)
else ()
  set(HHVM_WRAP_SYMS)
endif ()

set(HHVM_LINK_LIBRARIES
  ${HHVM_WRAP_SYMS}
  hphp_analysis
  hphp_system
  hphp_parser
  hphp_zend
  hphp_util
  hphp_hhbbc
  jit_sort
  ppc64-asm
  vixl neo)

if(ENABLE_FASTCGI)
  LIST(APPEND HHVM_LINK_LIBRARIES hphp_thrift)
  LIST(APPEND HHVM_LINK_LIBRARIES hphp_proxygen)
  include(CheckCXXSourceCompiles)
  CHECK_CXX_SOURCE_COMPILES("#include <pthread.h>
  int main() {
    return pthread_mutex_timedlock();
  }" PTHREAD_TIMEDLOCK)
  if (NOT PTHREAD_TIMEDLOCK)
    add_definitions(-DTHRIFT_MUTEX_EMULATE_PTHREAD_TIMEDLOCK)
  endif()
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: cmake build type Release.")
endif()

if(HHVM_DYNAMIC_EXTENSION_DIR)
  add_definitions(-DHHVM_DYNAMIC_EXTENSION_DIR="${HHVM_DYNAMIC_EXTENSION_DIR}")
else()
  if(UNIX)
    add_definitions(-DHHVM_DYNAMIC_EXTENSION_DIR="${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/hhvm/extensions")
  endif()
endif()

# Look for the chrpath tool so we can warn if it's not there
SET(FOUND_CHRPATH OFF)
IF(UNIX AND NOT APPLE)
    find_program(CHRPATH chrpath)
    IF (NOT CHRPATH STREQUAL "CHRPATH-NOTFOUND")
        SET(FOUND_CHRPATH ON)
    endif()
ENDIF(UNIX AND NOT APPLE)

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

# Ubuntu 15.10 and 14.04 have been failing to include a dependency on jemalloc
# as a these linked flags force the dependency to be recorded
if (JEMALLOC_ENABLED AND LINUX)
  LIST(APPEND HHVM_LINK_LIBRARIES -Wl,--no-as-needed ${JEMALLOC_LIB} -Wl,--as-needed)
endif()

if (HHVM_VERSION_OVERRIDE)
  parse_version("HHVM_VERSION_" ${HHVM_VERSION_OVERRIDE})
  add_definitions("-DHHVM_VERSION_OVERRIDE")
  add_definitions("-DHHVM_VERSION_MAJOR=${HHVM_VERSION_MAJOR}")
  add_definitions("-DHHVM_VERSION_MINOR=${HHVM_VERSION_MINOR}")
  add_definitions("-DHHVM_VERSION_PATCH=${HHVM_VERSION_PATCH}")
  add_definitions("-DHHVM_VERSION_SUFFIX=\"${HHVM_VERSION_SUFFIX}\"")
endif()

# Weak linking on Linux, Windows, and OS X all work somewhat differently. The following test
# works well on Linux and Windows, but fails for annoying reasons on OS X, and even works
# differently on different releases of OS X, cf. http://glandium.org/blog/?p=2764. Getting
# the test to work properly on OS X would require an APPLE check anyways, so just hardcode
# OS X as "we know weak linking works".
if(APPLE)
  set(FOLLY_HAVE_WEAK_SYMBOLS 1)
else()
  # check for weak symbols
  CHECK_CXX_SOURCE_COMPILES("
      extern \"C\" void configure_link_extern_weak_test() __attribute__((weak));
      int main(int argc, char** argv) {
          return configure_link_extern_weak_test == nullptr;
      }
  "
      FOLLY_HAVE_WEAK_SYMBOLS
  )
endif()

if(FOLLY_HAVE_WEAK_SYMBOLS)
  add_definitions(-DFOLLY_HAVE_WEAK_SYMBOLS=1)
else()
  add_definitions(-DFOLLY_HAVE_WEAK_SYMBOLS=0)
endif()

include(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(memrchr FOLLY_HAVE_MEMRCHR)
CHECK_FUNCTION_EXISTS(preadv FOLLY_HAVE_PREADV)
CHECK_FUNCTION_EXISTS(pwritev FOLLY_HAVE_PWRITEV)
if (FOLLY_HAVE_MEMRCHR)
  add_definitions("-DFOLLY_HAVE_MEMRCHR=1")
else()
  add_definitions("-DFOLLY_HAVE_MEMRCHR=0")
endif()
if (FOLLY_HAVE_PREADV)
  add_definitions("-DFOLLY_HAVE_PREADV=1")
else()
  add_definitions("-DFOLLY_HAVE_PREADV=0")
endif()
if (FOLLY_HAVE_PWRITEV)
  add_definitions("-DFOLLY_HAVE_PWRITEV=1")
else()
  add_definitions("-DFOLLY_HAVE_PWRITEV=0")
endif()
add_definitions(-DFOLLY_HAVE_LIBGFLAGS=0)

add_definitions(-D_REENTRANT=1 -D_PTHREADS=1 -D__STDC_FORMAT_MACROS)

if (LINUX)
  add_definitions(-D_GNU_SOURCE)
endif()

# cygwin headers are easily confused
if(CYGWIN)
  add_definitions(-D_GLIBCXX_USE_C99=1)
endif()

if(MSVC OR CYGWIN OR MINGW)
  add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

if(CMAKE_CONFIGURATION_TYPES)
  if(NOT MSVC)
    message(FATAL_ERROR "Adding the appropriate defines for multi-config targets using anything other than MSVC is not yet supported!")
  endif()
  if (MSVC_NO_ASSERT_IN_DEBUG)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D RELEASE=1 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D RELEASE=1 /D NDEBUG")
  else()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D DEBUG")
  endif()
  foreach(flag_var
      CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    set(${flag_var} "${${flag_var}} /D RELEASE=1 /D NDEBUG")
  endforeach()
elseif(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_definitions(-DDEBUG)
  message("Generating DEBUG build")
else()
  add_definitions(-DRELEASE=1)
  add_definitions(-DNDEBUG)
  message("Generating Release build")
endif()

if(ALWAYS_ASSERT)
  add_definitions(-DALWAYS_ASSERT=1)
endif()

if(APPLE OR FREEBSD OR CYGWIN OR MSVC OR MINGW)
  add_definitions(-DSKIP_USER_CHANGE=1)
endif()

if(ENABLE_TRACE)
    add_definitions(-DUSE_TRACE=1)
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

if(DISABLE_HARDWARE_COUNTERS OR NOT LINUX)
  add_definitions(-DNO_HARDWARE_COUNTERS=1)
endif ()

# enable the OSS options if we have any
add_definitions(-DHPHP_OSS=1)

# later versions of binutils don't play well without automake
add_definitions(-DPACKAGE=hhvm -DPACKAGE_VERSION=Release)

add_definitions(-DDEFAULT_CONFIG_DIR="${DEFAULT_CONFIG_DIR}")

add_definitions(-DHAVE_INTTYPES_H)
add_definitions(-DNO_LIB_GFLAGS)
include_directories(${TP_DIR})
if (THIRD_PARTY_INCLUDE_PATHS)
  include_directories(${THIRD_PARTY_INCLUDE_PATHS})
  add_definitions(${THIRD_PARTY_DEFINITIONS})
  include_directories(${HPHP_HOME}/hphp)
  include_directories(${HPHP_HOME})
endif()
