include(Options)

set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug DebugOpt RelWithDebInfo)

set(HHVM_WHOLE_ARCHIVE_LIBRARIES
    hphp_runtime_static
    hphp_runtime_ext
   )

set(HHVM_WRAP_SYMS)

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

if (HHVM_VERSION_OVERRIDE)
  parse_version("HHVM_VERSION_" ${HHVM_VERSION_OVERRIDE})
  add_definitions("-DHHVM_VERSION_OVERRIDE")
  add_definitions("-DHHVM_VERSION_MAJOR=${HHVM_VERSION_MAJOR}")
  add_definitions("-DHHVM_VERSION_MINOR=${HHVM_VERSION_MINOR}")
  add_definitions("-DHHVM_VERSION_PATCH=${HHVM_VERSION_PATCH}")
  add_definitions("-DHHVM_VERSION_SUFFIX=\"${HHVM_VERSION_SUFFIX}\"")
endif()

add_definitions(-D_REENTRANT=1 -D_PTHREADS=1 -D__STDC_FORMAT_MACROS)

if (LINUX)
  add_definitions(-D_GNU_SOURCE)
endif()

if(MSVC)
  add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

if(CMAKE_CONFIGURATION_TYPES)
  if(NOT MSVC)
    message(FATAL_ERROR "Adding the appropriate defines for multi-config targets using anything other than MSVC is not yet supported!")
  endif()
  foreach(flag_var
      CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    set(${flag_var} "${${flag_var}} /D NDEBUG")
  endforeach()
elseif(${CMAKE_BUILD_TYPE} MATCHES "Debug" OR
       ${CMAKE_BUILD_TYPE} MATCHES "DebugOpt")
  message("Generating DEBUG build")
else()
  add_definitions(-DNDEBUG)
  message("Generating Release build")
endif()

if(ALWAYS_ASSERT)
  add_definitions(-DALWAYS_ASSERT=1)
endif()

if(APPLE OR FREEBSD OR MSVC)
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

include_directories(${TP_DIR})
if (THIRD_PARTY_INCLUDE_PATHS)
  include_directories(${THIRD_PARTY_INCLUDE_PATHS})
  add_definitions(${THIRD_PARTY_DEFINITIONS})
  include_directories(${HPHP_HOME}/hphp)
  include_directories(${HPHP_HOME})
endif()
