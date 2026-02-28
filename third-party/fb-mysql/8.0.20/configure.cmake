# Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
#

INCLUDE (CheckCSourceCompiles)
INCLUDE (CheckCXXSourceCompiles)
INCLUDE (CheckStructHasMember)
INCLUDE (CheckLibraryExists)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckCCompilerFlag)
INCLUDE (CheckCSourceRuns)
INCLUDE (CheckCXXSourceRuns)
INCLUDE (CheckSymbolExists)


# System type affects version_compile_os variable 
IF(NOT SYSTEM_TYPE)
  IF(PLATFORM)
    SET(SYSTEM_TYPE ${PLATFORM})
  ELSE()
    SET(SYSTEM_TYPE ${CMAKE_SYSTEM_NAME})
  ENDIF()
ENDIF()

# Same for structs, setting HAVE_STRUCT_<name> instead
FUNCTION(MY_CHECK_STRUCT_SIZE type defbase)
  CHECK_TYPE_SIZE("struct ${type}" SIZEOF_${defbase})
  IF(SIZEOF_${defbase})
    SET(HAVE_STRUCT_${defbase} 1 PARENT_SCOPE)
  ENDIF()
ENDFUNCTION()

# Searches function in libraries
# if function is found, sets output parameter result to the name of the library
# if function is found in libc, result will be empty 
FUNCTION(MY_SEARCH_LIBS func libs result)
  IF(${${result}})
    # Library is already found or was predefined
    RETURN()
  ENDIF()
  CHECK_FUNCTION_EXISTS(${func} HAVE_${func}_IN_LIBC)
  IF(HAVE_${func}_IN_LIBC)
    SET(${result} "" PARENT_SCOPE)
    RETURN()
  ENDIF()
  FOREACH(lib  ${libs})
    CHECK_LIBRARY_EXISTS(${lib} ${func} "" HAVE_${func}_IN_${lib}) 
    IF(HAVE_${func}_IN_${lib})
      SET(${result} ${lib} PARENT_SCOPE)
      SET(HAVE_${result} 1 PARENT_SCOPE)
      RETURN()
    ENDIF()
  ENDFOREACH()
ENDFUNCTION()

# Find out which libraries to use.

# Figure out threading library
# Defines CMAKE_USE_PTHREADS_INIT and CMAKE_THREAD_LIBS_INIT.
FIND_PACKAGE (Threads)

IF(UNIX)
  IF(FREEBSD)
    MYSQL_CHECK_PKGCONFIG()
    PKG_CHECK_MODULES(LIBUNWIND libunwind)
  ENDIF()
  MY_SEARCH_LIBS(floor m LIBM)
  IF(NOT LIBM)
    MY_SEARCH_LIBS(__infinity m LIBM)
  ENDIF()
  IF(NOT LIBM)
    MY_SEARCH_LIBS(log m LIBM)
  ENDIF()
  MY_SEARCH_LIBS(gethostbyname_r  "nsl_r;nsl" LIBNSL)
  MY_SEARCH_LIBS(bind "bind;socket" LIBBIND)
  MY_SEARCH_LIBS(crypt crypt LIBCRYPT)
  MY_SEARCH_LIBS(setsockopt socket LIBSOCKET)
  MY_SEARCH_LIBS(dlopen dl LIBDL)
  # HAVE_dlopen_IN_LIBC
  IF(NOT LIBDL)
    MY_SEARCH_LIBS(dlsym dl LIBDL)
  ENDIF()
  MY_SEARCH_LIBS(sched_yield rt LIBRT)
  IF(NOT LIBRT)
    MY_SEARCH_LIBS(clock_gettime rt LIBRT)
  ENDIF()
  MY_SEARCH_LIBS(timer_create rt LIBRT)
  MY_SEARCH_LIBS(atomic_thread_fence atomic LIBATOMIC)
  MY_SEARCH_LIBS(backtrace execinfo LIBEXECINFO)
  MY_SEARCH_LIBS(cap_get_proc cap LIBCAP)

  LIST(APPEND CMAKE_REQUIRED_LIBRARIES
    ${LIBM} ${LIBNSL} ${LIBBIND} ${LIBCRYPT} ${LIBSOCKET} ${LIBDL}
    ${CMAKE_THREAD_LIBS_INIT} ${LIBRT} ${LIBATOMIC} ${LIBEXECINFO} ${LIBCAP}
  )
  # Need explicit pthread for gcc -fsanitize=address
  IF(CMAKE_C_FLAGS MATCHES "-fsanitize=")
    SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} pthread)
  ENDIF()

  # https://bugs.llvm.org/show_bug.cgi?id=16404
  IF(LINUX AND HAVE_UBSAN AND MY_COMPILER_IS_CLANG)
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG
      "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -rtlib=compiler-rt -lgcc_s")
    SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO
      "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -rtlib=compiler-rt -lgcc_s")
  ENDIF()

  IF(WITH_ASAN)
    SET(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -fsanitize=address")
  ENDIF()

  IF(WITH_ASAN OR WITH_LSAN OR WITH_TSAN)
    IF(CMAKE_USE_PTHREADS_INIT AND NOT CMAKE_THREAD_LIBS_INIT)
      MESSAGE(STATUS "No CMAKE_THREAD_LIBS_INIT ??")
      SET(CMAKE_THREAD_LIBS_INIT "-lpthread")
    ENDIF()
  ENDIF()

  LIST(LENGTH CMAKE_REQUIRED_LIBRARIES required_libs_length)
  IF(${required_libs_length} GREATER 0)
    LIST(REMOVE_DUPLICATES CMAKE_REQUIRED_LIBRARIES)
  ENDIF()  
  LINK_LIBRARIES(${CMAKE_THREAD_LIBS_INIT})
  
  OPTION(WITH_LIBWRAP "Compile with tcp wrappers support" OFF)
  IF(WITH_LIBWRAP)
    SET(SAVE_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} wrap)
    CHECK_C_SOURCE_COMPILES(
    "
    #include <tcpd.h>
    int allow_severity = 0;
    int deny_severity  = 0;
    int main()
    {
      hosts_access(0);
    }"
    HAVE_LIBWRAP)

    IF(HAVE_LIBWRAP)
      CHECK_CXX_SOURCE_COMPILES(
      "
      #include <tcpd.h>
      int main()
      {
        struct request_info req;
        if (req.sink)
          (req.sink)(req.fd);
      }"
      HAVE_LIBWRAP_PROTOTYPES)
    ENDIF()

    SET(CMAKE_REQUIRED_LIBRARIES ${SAVE_CMAKE_REQUIRED_LIBRARIES})
    IF(HAVE_LIBWRAP)
      SET(LIBWRAP "wrap")
    ELSE()
      MESSAGE(FATAL_ERROR 
      "WITH_LIBWRAP is defined, but can not find a working libwrap. "
      "Make sure both the header files (tcpd.h) "
      "and the library (libwrap) are installed.")
    ENDIF()
  ENDIF()
ENDIF()

#
# Tests for header files
#
INCLUDE (CheckIncludeFiles)

IF(FREEBSD)
  # On FreeBSD some includes, e.g. sasl/sasl.h, is in /usr/local/include
  LIST(APPEND CMAKE_REQUIRED_INCLUDES "/usr/local/include")
ENDIF()

CHECK_INCLUDE_FILES (alloca.h HAVE_ALLOCA_H)
CHECK_INCLUDE_FILES (arpa/inet.h HAVE_ARPA_INET_H)
CHECK_INCLUDE_FILES (dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILES (endian.h HAVE_ENDIAN_H)
CHECK_INCLUDE_FILES (execinfo.h HAVE_EXECINFO_H)
CHECK_INCLUDE_FILES (fpu_control.h HAVE_FPU_CONTROL_H)
CHECK_INCLUDE_FILES (grp.h HAVE_GRP_H)
CHECK_INCLUDE_FILES (langinfo.h HAVE_LANGINFO_H)
CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILES (netinet/in.h HAVE_NETINET_IN_H)
CHECK_INCLUDE_FILES (poll.h HAVE_POLL_H)
CHECK_INCLUDE_FILES (pwd.h HAVE_PWD_H)
CHECK_INCLUDE_FILES (strings.h HAVE_STRINGS_H) # Used by NDB
CHECK_INCLUDE_FILES (sys/ioctl.h HAVE_SYS_IOCTL_H)
CHECK_INCLUDE_FILES (sys/mman.h HAVE_SYS_MMAN_H)
CHECK_INCLUDE_FILES (sys/prctl.h HAVE_SYS_PRCTL_H)
CHECK_INCLUDE_FILES (sys/resource.h HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_FILES (sys/select.h HAVE_SYS_SELECT_H)
CHECK_INCLUDE_FILES (sys/socket.h HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILES ("curses.h;term.h" HAVE_TERM_H)
CHECK_INCLUDE_FILES (termios.h HAVE_TERMIOS_H)
CHECK_INCLUDE_FILES (termio.h HAVE_TERMIO_H)
CHECK_INCLUDE_FILES (unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES (sys/wait.h HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILES (sys/param.h HAVE_SYS_PARAM_H) # Used by NDB/libevent
CHECK_INCLUDE_FILES (fnmatch.h HAVE_FNMATCH_H)
CHECK_INCLUDE_FILES (sys/un.h HAVE_SYS_UN_H)
# Cyrus SASL 2.1.26 on Solaris 11.4 has a bug that requires sys/types.h
# to be included before checking if sasl/sasl.h exists
CHECK_INCLUDE_FILES ("sys/types.h;sasl/sasl.h" HAVE_SASL_SASL_H)

# For LIBCAP
CHECK_INCLUDE_FILES (sys/capability.h HAVE_SYS_CAPABILITY_H)

#
# Tests for functions
#
IF(WITH_ASAN)
  CHECK_SYMBOL_EXISTS (__lsan_do_recoverable_leak_check
    "sanitizer/lsan_interface.h" HAVE_LSAN_DO_RECOVERABLE_LEAK_CHECK)
ENDIF()
CHECK_FUNCTION_EXISTS (_aligned_malloc HAVE_ALIGNED_MALLOC)
CHECK_FUNCTION_EXISTS (backtrace HAVE_BACKTRACE)
CHECK_FUNCTION_EXISTS (index HAVE_INDEX)
CHECK_FUNCTION_EXISTS (chown HAVE_CHOWN)
CHECK_FUNCTION_EXISTS (cuserid HAVE_CUSERID)
CHECK_FUNCTION_EXISTS (directio HAVE_DIRECTIO)
CHECK_FUNCTION_EXISTS (ftruncate HAVE_FTRUNCATE)
CHECK_FUNCTION_EXISTS (fchmod HAVE_FCHMOD)
CHECK_FUNCTION_EXISTS (fcntl HAVE_FCNTL)
CHECK_FUNCTION_EXISTS (fdatasync HAVE_FDATASYNC)
CHECK_SYMBOL_EXISTS(fdatasync "unistd.h" HAVE_DECL_FDATASYNC)
CHECK_FUNCTION_EXISTS (fedisableexcept HAVE_FEDISABLEEXCEPT)
CHECK_FUNCTION_EXISTS (fsync HAVE_FSYNC)
CHECK_FUNCTION_EXISTS (gethrtime HAVE_GETHRTIME)
CHECK_FUNCTION_EXISTS (getpass HAVE_GETPASS)
CHECK_FUNCTION_EXISTS (getpassphrase HAVE_GETPASSPHRASE)
CHECK_FUNCTION_EXISTS (getpwnam HAVE_GETPWNAM)
CHECK_FUNCTION_EXISTS (getpwuid HAVE_GETPWUID)
CHECK_FUNCTION_EXISTS (getrusage HAVE_GETRUSAGE)
CHECK_FUNCTION_EXISTS (initgroups HAVE_INITGROUPS)
CHECK_FUNCTION_EXISTS (issetugid HAVE_ISSETUGID)
CHECK_FUNCTION_EXISTS (getuid HAVE_GETUID)
CHECK_FUNCTION_EXISTS (geteuid HAVE_GETEUID)
CHECK_FUNCTION_EXISTS (getgid HAVE_GETGID)
CHECK_FUNCTION_EXISTS (getegid HAVE_GETEGID)
CHECK_FUNCTION_EXISTS (madvise HAVE_MADVISE)
CHECK_FUNCTION_EXISTS (malloc_info HAVE_MALLOC_INFO)
CHECK_FUNCTION_EXISTS (mlock HAVE_MLOCK)
CHECK_FUNCTION_EXISTS (mlockall HAVE_MLOCKALL)
CHECK_FUNCTION_EXISTS (mmap64 HAVE_MMAP64)
CHECK_FUNCTION_EXISTS (poll HAVE_POLL)
CHECK_FUNCTION_EXISTS (posix_fallocate HAVE_POSIX_FALLOCATE)
CHECK_FUNCTION_EXISTS (posix_memalign HAVE_POSIX_MEMALIGN)
CHECK_FUNCTION_EXISTS (pthread_condattr_setclock HAVE_PTHREAD_CONDATTR_SETCLOCK)
CHECK_FUNCTION_EXISTS (pthread_getaffinity_np HAVE_PTHREAD_GETAFFINITY_NP)
CHECK_FUNCTION_EXISTS (pthread_sigmask HAVE_PTHREAD_SIGMASK)
CHECK_FUNCTION_EXISTS (sleep HAVE_SLEEP)
CHECK_FUNCTION_EXISTS (stpcpy HAVE_STPCPY)
CHECK_FUNCTION_EXISTS (stpncpy HAVE_STPNCPY)
CHECK_FUNCTION_EXISTS (strlcpy HAVE_STRLCPY)
CHECK_FUNCTION_EXISTS (strndup HAVE_STRNDUP) # Used by libbinlogevents
CHECK_FUNCTION_EXISTS (strlcat HAVE_STRLCAT)
CHECK_FUNCTION_EXISTS (strsignal HAVE_STRSIGNAL)
CHECK_FUNCTION_EXISTS (tell HAVE_TELL)
CHECK_FUNCTION_EXISTS (vasprintf HAVE_VASPRINTF)
CHECK_FUNCTION_EXISTS (memalign HAVE_MEMALIGN)
CHECK_FUNCTION_EXISTS (nl_langinfo HAVE_NL_LANGINFO)
CHECK_FUNCTION_EXISTS (ntohll HAVE_HTONLL)

CHECK_FUNCTION_EXISTS (epoll_create HAVE_EPOLL)

#--------------------------------------------------------------------
# Support for WL#2373 (Use cycle counter for timing)
#--------------------------------------------------------------------

CHECK_INCLUDE_FILES(sys/time.h HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES(sys/times.h HAVE_SYS_TIMES_H)

CHECK_FUNCTION_EXISTS(times HAVE_TIMES)
CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)


#
# Tests for symbols
#

CHECK_SYMBOL_EXISTS(lrand48 "stdlib.h" HAVE_LRAND48)
CHECK_SYMBOL_EXISTS(TIOCGWINSZ "sys/ioctl.h" GWINSZ_IN_SYS_IOCTL)
CHECK_SYMBOL_EXISTS(FIONREAD "sys/ioctl.h" FIONREAD_IN_SYS_IOCTL)
CHECK_SYMBOL_EXISTS(FIONREAD "sys/filio.h" FIONREAD_IN_SYS_FILIO)
CHECK_SYMBOL_EXISTS(MADV_DONTDUMP "sys/mman.h" HAVE_MADV_DONTDUMP)
CHECK_CXX_SOURCE_COMPILES(
"#include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
int main() {
  long long int foo = O_TMPFILE;
}" HAVE_O_TMPFILE)

# The results of these four checks are only needed here, not in code.
CHECK_FUNCTION_EXISTS (timer_create HAVE_TIMER_CREATE)
CHECK_FUNCTION_EXISTS (timer_settime HAVE_TIMER_SETTIME)
CHECK_FUNCTION_EXISTS (kqueue HAVE_KQUEUE)

# Check whether the setns() API function supported by a target platform
CHECK_C_SOURCE_RUNS("
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>
int main()
{
  (void)setns(0, 0);
  return 0;
}" HAVE_SETNS)

CHECK_SYMBOL_EXISTS(EVFILT_TIMER "sys/types.h;sys/event.h;sys/time.h" HAVE_EVFILT_TIMER)
IF(HAVE_KQUEUE AND HAVE_EVFILT_TIMER)
  SET(HAVE_KQUEUE_TIMERS 1 CACHE INTERNAL "Have kqueue timer-related filter")
ELSEIF(HAVE_TIMER_CREATE AND HAVE_TIMER_SETTIME)
  SET(HAVE_POSIX_TIMERS 1 CACHE INTERNAL "Have POSIX timer-related functions")
ENDIF()

IF(NOT HAVE_POSIX_TIMERS AND NOT HAVE_KQUEUE_TIMERS AND NOT WIN32)
  MESSAGE(FATAL_ERROR "No mysys timer support detected!")
ENDIF()

#
# Test for endianess
#
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

# The header for glibc versions less than 2.9 will not
# have the endian conversion macros defined.
IF(HAVE_ENDIAN_H)
  CHECK_SYMBOL_EXISTS(le64toh endian.h HAVE_LE64TOH)
  CHECK_SYMBOL_EXISTS(le32toh endian.h HAVE_LE32TOH)
  CHECK_SYMBOL_EXISTS(le16toh endian.h HAVE_LE16TOH)
  CHECK_SYMBOL_EXISTS(htole64 endian.h HAVE_HTOLE64)
  CHECK_SYMBOL_EXISTS(htole32 endian.h HAVE_HTOLE32)
  CHECK_SYMBOL_EXISTS(htole16 endian.h HAVE_HTOLE16)
  IF(HAVE_LE32TOH AND HAVE_LE16TOH AND HAVE_LE64TOH AND
      HAVE_HTOLE64 AND HAVE_HTOLE32 AND HAVE_HTOLE16)
    # Used by libbinlogevents and libmysqlgcs.
    SET(HAVE_ENDIAN_CONVERSION_MACROS 1)
  ENDIF()
ENDIF()

#
# Tests for type sizes (and presence)
#
INCLUDE (CheckTypeSize)

LIST(APPEND CMAKE_REQUIRED_DEFINITIONS
  -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
  -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS
  )

IF(SOLARIS)
  LIST(APPEND CMAKE_REQUIRED_DEFINITIONS
    -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT -D_PTHREADS
    )
ENDIF()

SET(CMAKE_EXTRA_INCLUDE_FILES stdint.h stdio.h sys/types.h time.h)

CHECK_TYPE_SIZE("void *"    SIZEOF_VOIDP)
CHECK_TYPE_SIZE("char *"    SIZEOF_CHARP)
CHECK_TYPE_SIZE("long"      SIZEOF_LONG)
CHECK_TYPE_SIZE("short"     SIZEOF_SHORT)
CHECK_TYPE_SIZE("int"       SIZEOF_INT)
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE("time_t"    SIZEOF_TIME_T)

CHECK_STRUCT_HAS_MEMBER("struct tm"
 tm_gmtoff "time.h" HAVE_TM_GMTOFF)

# If finds the size of a type, set SIZEOF_<type> and HAVE_<type>
FUNCTION(MY_CHECK_TYPE_SIZE type defbase)
  CHECK_TYPE_SIZE("${type}" SIZEOF_${defbase})
  IF(SIZEOF_${defbase})
    SET(HAVE_${defbase} 1 PARENT_SCOPE)
  ENDIF()
ENDFUNCTION()

# We are only interested in presence for these
MY_CHECK_TYPE_SIZE(ulong ULONG)
MY_CHECK_TYPE_SIZE(u_int32_t U_INT32_T)

# Support for tagging symbols with __attribute__((visibility("hidden")))
MY_CHECK_CXX_COMPILER_FLAG("-fvisibility=hidden" HAVE_VISIBILITY_HIDDEN)

#
# Code tests
#

CHECK_C_SOURCE_RUNS("
#include <time.h>
int main()
{
  struct timespec ts;
  return clock_gettime(CLOCK_MONOTONIC, &ts);
}" HAVE_CLOCK_GETTIME)

CHECK_C_SOURCE_RUNS("
#include <time.h>
int main()
{
  struct timespec ts;
  return clock_gettime(CLOCK_REALTIME, &ts);
}" HAVE_CLOCK_REALTIME)

IF(NOT STACK_DIRECTION)
  IF(CMAKE_CROSSCOMPILING)
   MESSAGE(FATAL_ERROR 
   "STACK_DIRECTION is not defined.  Please specify -DSTACK_DIRECTION=1 "
   "or -DSTACK_DIRECTION=-1 when calling cmake.")
  ELSE()
    TRY_RUN(STACKDIR_RUN_RESULT STACKDIR_COMPILE_RESULT    
     ${CMAKE_BINARY_DIR} 
     ${CMAKE_SOURCE_DIR}/cmake/stack_direction.c
     )
     # Test program returns 0 (down) or 1 (up).
     # Convert to -1 or 1
     IF(STACKDIR_RUN_RESULT EQUAL 0)
       SET(STACK_DIRECTION -1 CACHE INTERNAL "Stack grows direction")
     ELSE()
       SET(STACK_DIRECTION 1 CACHE INTERNAL "Stack grows direction")
     ENDIF()
     MESSAGE(STATUS "Checking stack direction : ${STACK_DIRECTION}")
   ENDIF()
ENDIF()

CHECK_INCLUDE_FILES("time.h;sys/time.h" TIME_WITH_SYS_TIME)
CHECK_SYMBOL_EXISTS(O_NONBLOCK "unistd.h;fcntl.h" HAVE_FCNTL_NONBLOCK)
IF(NOT HAVE_FCNTL_NONBLOCK)
 SET(NO_FCNTL_NONBLOCK 1)
ENDIF()

IF(NOT CMAKE_CROSSCOMPILING AND NOT MSVC)
  STRING(TOLOWER ${CMAKE_SYSTEM_PROCESSOR}  processor)
  IF(processor MATCHES "86" OR
      processor MATCHES "amd64" OR
      processor MATCHES "x64")
    IF(NOT SOLARIS)
      # The loader in some Solaris versions has a bug due to which it refuses to
      # start a binary that has been compiled by GCC and uses __asm__("pause")
      # with the error:
      # $ ./mysqld
      # ld.so.1: mysqld: fatal: hardware capability unsupported: 0x2000 [ PAUSE ]
      # Killed
      # $
      # Even though the CPU does have support for the instruction.
      # Binaries that have been compiled by GCC and use __asm__("pause")
      # on a non-buggy Solaris get flagged with a "uses pause" flag and
      # thus they are unusable if copied on buggy Solaris version. To
      # circumvent this we explicitly disable __asm__("pause") when
      # compiling on Solaris. Subsequently the tests here will enable
      # HAVE_FAKE_PAUSE_INSTRUCTION which will use __asm__("rep; nop")
      # which currently generates the same code as __asm__("pause") - 0xf3 0x90
      # but without flagging the binary as "uses pause".
      CHECK_C_SOURCE_RUNS("
      int main()
      {
        __asm__ __volatile__ (\"pause\");
        return 0;
      }"  HAVE_PAUSE_INSTRUCTION)
    ENDIF()
  ENDIF()
  IF (NOT HAVE_PAUSE_INSTRUCTION)
    CHECK_C_SOURCE_COMPILES("
    int main()
    {
     __asm__ __volatile__ (\"rep; nop\");
     return 0;
    }
   " HAVE_FAKE_PAUSE_INSTRUCTION)
  ENDIF()
  IF (NOT HAVE_PAUSE_INSTRUCTION)
    CHECK_C_SOURCE_COMPILES("
    int main()
    {
     __asm__ __volatile__ (\"or 1,1,1\");
     __asm__ __volatile__ (\"or 2,2,2\");
     return 0;
    }
    " HAVE_HMT_PRIORITY_INSTRUCTION)
  ENDIF()
ENDIF()
  
INCLUDE (CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX(cxxabi.h HAVE_CXXABI_H)
IF(HAVE_CXXABI_H)
CHECK_CXX_SOURCE_COMPILES("
 #include <cxxabi.h>
 int main(int argc, char **argv) 
  {
    char *foo= 0; int bar= 0;
    foo= abi::__cxa_demangle(foo, foo, 0, &bar);
    return 0;
  }"
  HAVE_ABI_CXA_DEMANGLE)
ENDIF()

CHECK_C_SOURCE_COMPILES("
int main()
{
  __builtin_unreachable();
  return 0;
}" HAVE_BUILTIN_UNREACHABLE)

CHECK_C_SOURCE_COMPILES("
int main()
{
  long l= 0;
  __builtin_expect(l, 0);
  return 0;
}" HAVE_BUILTIN_EXPECT)

# Only check for __builtin_stpcpy() if stpcpy() is available.
# Oracle Developer Studio requires <string.h> to be included in order
# to use __builtin_stpcpy.
IF(HAVE_STPCPY)
  CHECK_SYMBOL_EXISTS(__builtin_stpcpy "string.h" HAVE_BUILTIN_STPCPY)
ENDIF()

CHECK_CXX_SOURCE_COMPILES("
  int main()
  {
    int foo= -10; int bar= 10;
    long long int foo64= -10; long long int bar64= 10;
    if (!__sync_fetch_and_add(&foo, bar) || foo)
      return -1;
    bar= __sync_lock_test_and_set(&foo, bar);
    if (bar || foo != 10)
      return -1;
    bar= __sync_val_compare_and_swap(&bar, foo, 15);
    if (bar)
      return -1;
    if (!__sync_fetch_and_add(&foo64, bar64) || foo64)
      return -1;
    bar64= __sync_lock_test_and_set(&foo64, bar64);
    if (bar64 || foo64 != 10)
      return -1;
    bar64= __sync_val_compare_and_swap(&bar64, foo, 15);
    if (bar64)
      return -1;
    return 0;
  }"
  HAVE_GCC_SYNC_BUILTINS)

IF(WITH_VALGRIND)
  SET(VALGRIND_HEADERS "valgrind/memcheck.h;valgrind/valgrind.h")
  CHECK_INCLUDE_FILES("${VALGRIND_HEADERS}" HAVE_VALGRIND_HEADERS)
  IF(HAVE_VALGRIND_HEADERS)
    SET(HAVE_VALGRIND 1)
  ELSE()
    MESSAGE(FATAL_ERROR "Unable to find Valgrind header files ${VALGRIND_HEADERS}. Make sure you have them in your include path.")
  ENDIF()
ENDIF()

# Check for gettid() system call
CHECK_C_SOURCE_COMPILES("
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(int ac, char **av)
{
  unsigned long long tid = syscall(SYS_gettid);
  return (tid != 0 ? 0 : 1);
}"
HAVE_SYS_GETTID)

# Check for pthread_getthreadid_np()
CHECK_C_SOURCE_COMPILES("
#include <pthread_np.h>
int main(int ac, char **av)
{
  unsigned long long tid = pthread_getthreadid_np();
  return (tid != 0 ? 0 : 1);
}"
HAVE_PTHREAD_GETTHREADID_NP)

# Check for pthread_threadid_np()
CHECK_C_SOURCE_COMPILES("
#include <pthread.h>
int main(int ac, char **av)
{
  unsigned long long tid64;
  pthread_threadid_np(NULL, &tid64);
  return (tid64 != 0 ? 0 : 1);
}"
HAVE_PTHREAD_THREADID_NP)

# Check for pthread_self() returning an integer type
CHECK_C_SOURCE_COMPILES("
#include <sys/types.h>
#include <pthread.h>
int main(int ac, char **av)
{
  unsigned long long tid = pthread_self();
  return (tid != 0 ? 0 : 1);
}"
HAVE_INTEGER_PTHREAD_SELF
FAIL_REGEX "warning: incompatible pointer to integer conversion"
)

#--------------------------------------------------------------------
# Check for IPv6 support
#--------------------------------------------------------------------
CHECK_INCLUDE_FILE(netinet/in6.h HAVE_NETINET_IN6_H) # Used by libevent (never true)
MY_CHECK_STRUCT_SIZE("in6_addr" IN6_ADDR) # Used by libevent

IF(UNIX)
  SET(CMAKE_EXTRA_INCLUDE_FILES sys/types.h netinet/in.h sys/socket.h)
  IF(HAVE_NETINET_IN6_H)
    SET(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES} netinet/in6.h)
  ENDIF()
ELSEIF(WIN32)
  SET(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES} winsock2.h ws2ipdef.h)
ENDIF()

#
# Check if struct sockaddr_in::sin_len is available.
#

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in" sin_len
  "${CMAKE_EXTRA_INCLUDE_FILES}" HAVE_SOCKADDR_IN_SIN_LEN)

#
# Check if struct sockaddr_in6::sin6_len is available.
#

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in6" sin6_len
  "${CMAKE_EXTRA_INCLUDE_FILES}" HAVE_SOCKADDR_IN6_SIN6_LEN)

SET(CMAKE_EXTRA_INCLUDE_FILES)

CHECK_INCLUDE_FILES(numa.h HAVE_NUMA_H)
CHECK_INCLUDE_FILES(numaif.h HAVE_NUMAIF_H)

IF(HAVE_NUMA_H AND HAVE_NUMAIF_H)
  SET(SAVE_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
  SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} numa)
  CHECK_C_SOURCE_COMPILES(
    "
    #include <numa.h>
    #include <numaif.h>
    int main()
    {
       struct bitmask *all_nodes= numa_all_nodes_ptr;
       set_mempolicy(MPOL_DEFAULT, 0, 0);
       return all_nodes != NULL;
    }"
    HAVE_LIBNUMA)
  SET(CMAKE_REQUIRED_LIBRARIES ${SAVE_CMAKE_REQUIRED_LIBRARIES})
ELSE()
  SET(HAVE_LIBNUMA 0)
ENDIF()

IF(NOT HAVE_LIBNUMA)
  MESSAGE(STATUS "NUMA library missing or required version not available")
ENDIF()

IF(HAVE_LIBNUMA AND HAVE_NUMA_H AND HAVE_NUMAIF_H)
  OPTION(WITH_NUMA "Explicitly set NUMA memory allocation policy" ON)
ELSE()
  OPTION(WITH_NUMA "Explicitly set NUMA memory allocation policy" OFF)
ENDIF()

IF(WITH_NUMA AND NOT HAVE_LIBNUMA)
  # Forget it in cache, abort the build.
  UNSET(WITH_NUMA CACHE)
  MESSAGE(FATAL_ERROR "Could not find numa headers/libraries")
ENDIF()

IF(HAVE_LIBNUMA AND NOT WITH_NUMA)
  SET(HAVE_LIBNUMA 0)
  MESSAGE(STATUS "Disabling NUMA on user's request")
ENDIF()
