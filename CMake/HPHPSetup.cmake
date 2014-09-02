include(Options)

# Do this until cmake has a define for ARMv8
INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
#ifndef __AARCH64EL__
#error Not ARMv8
#endif
int main() { return 0; }" IS_AARCH64)

set(HHVM_WHOLE_ARCHIVE_LIBRARIES
    hphp_runtime_static
    hphp_runtime_ext
   )

add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

if (ENABLE_ZEND_COMPAT)
  add_definitions("-DENABLE_ZEND_COMPAT=1")
  list(APPEND HHVM_WHOLE_ARCHIVE_LIBRARIES hphp_ext_zend_compat)
endif()

if (APPLE)
  set(ENABLE_FASTCGI 1)
  set(HHVM_ANCHOR_SYMS
    -Wl,-u,_register_fastcgi_server
    -Wl,-all_load ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
elseif (IS_AARCH64)
  set(HHVM_ANCHOR_SYMS
    -Wl,--whole-archive ${HHVM_WHOLE_ARCHIVE_LIBRARIES} -Wl,--no-whole-archive)
elseif(CYGWIN)
  set(ENABLE_FASTCGI 0)
  set(HHVM_ANCHOR_SYMS
  -Wl,--whole-archive ${HHVM_WHOLE_ARCHIVE_LIBRARIES} -Wl,--no-whole-archive)
else()
  set(ENABLE_FASTCGI 1)
  set(HHVM_ANCHOR_SYMS
    -Wl,-uregister_libevent_server,-uregister_fastcgi_server
    -Wl,--whole-archive ${HHVM_WHOLE_ARCHIVE_LIBRARIES} -Wl,--no-whole-archive)
endif()

set(HHVM_LINK_LIBRARIES
  ${HHVM_ANCHOR_SYMS}
  hphp_analysis
  ext_hhvm_static
  hphp_system
  hphp_parser
  hphp_zend
  hphp_util
  hphp_hhbbc
  vixl neo)

if(ENABLE_FASTCGI)
  LIST(APPEND HHVM_LINK_LIBRARIES hphp_thrift)
  LIST(APPEND HHVM_LINK_LIBRARIES hphp_proxygen)
endif()

if(NOT CMAKE_BUILD_TYPE)
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

add_definitions(-D_REENTRANT=1 -D_PTHREADS=1 -D__STDC_FORMAT_MACROS)

if (LINUX)
  add_definitions(-D_GNU_SOURCE)
endif()

# cygwin headers are easily confused
if(CYGWIN)
  add_definitions(-D_GLIBCXX_USE_C99=1)
endif()

if(MSVC OR CYGWIN OR MINGW)
  add_definitions(-DNOUSER=1 -DGLOG_NO_ABBREVIATED_SEVERITIES)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
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

if(EXECUTION_PROFILER)
  add_definitions(-DEXECUTION_PROFILER=1)
endif()

if(ENABLE_FULL_SETLINE)
  add_definitions(-DENABLE_FULL_SETLINE=1)
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

if(DISABLE_HARDWARE_COUNTERS)
  add_definitions(-DNO_HARDWARE_COUNTERS=1)
endif ()

if(PACKED_TV)
  # Allows a packed tv build
  add_definitions(-DPACKED_TV=1)
endif()

# enable the OSS options if we have any
add_definitions(-DHPHP_OSS=1)

# later versions of binutils don't play well without automake
add_definitions(-DPACKAGE=hhvm -DPACKAGE_VERSION=Release)

if (NOT LIBSQLITE3_LIBRARY)
  include_directories("${TP_DIR}/libsqlite3")
endif()

if (NOT DOUBLE_CONVERSION_LIBRARY)
  include_directories("${TP_DIR}/double-conversion/src")
endif()

if (NOT LZ4_LIBRARY)
  include_directories("${TP_DIR}/lz4")
endif()

if (NOT LIBZIP_INCLUDE_DIR_ZIP)
  include_directories("${TP_DIR}/libzip")
endif()

if (NOT PCRE_LIBRARY)
  include_directories("${TP_DIR}/pcre")
endif()

include_directories("${TP_DIR}/fastlz")
include_directories("${TP_DIR}/timelib")
include_directories("${TP_DIR}/libafdt/src")
include_directories("${TP_DIR}/libmbfl")
include_directories("${TP_DIR}/libmbfl/mbfl")
include_directories("${TP_DIR}/libmbfl/filter")
include_directories("${TP_DIR}/folly")
include_directories(${TP_DIR})

include_directories(${HPHP_HOME}/hphp)
include_directories(${HPHP_HOME})
