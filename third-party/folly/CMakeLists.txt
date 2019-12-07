string(REPLACE "-fno-operator-names" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(FOLLY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(FOLLY_DIR "${FOLLY_ROOT}/folly")

include(CMakePushCheckState)
include(CheckCXXSourceCompiles)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)

include(HPHPFunctions)

# Check if the given source compiles and set RETURN_VALUE based on the result.
# Also sets RETURN_VALUE=<result> as a preprocessor macro.
function (folly_check_source_compiles SOURCE RETURN_VALUE)
  check_cxx_source_compiles("${SOURCE}" ${RETURN_VALUE})
  if (${RETURN_VALUE})
    target_compile_definitions(folly PUBLIC "${RETURN_VALUE}=1")
  else()
    target_compile_definitions(folly PUBLIC "${RETURN_VALUE}=0")
  endif()
endfunction(folly_check_source_compiles)

# Needed for fcntl w/ F_SETPIPE_SZ
add_definitions("-D_GNU_SOURCE")

if (NOT EXISTS "${FOLLY_DIR}/Portability.h")
  message(FATAL_ERROR "${FOLLY_DIR}/Portability.h missing, did you forget to "
                      "run `git submodule update --init --recursive`?")
endif()

# Main folly library files
auto_sources(files "*.cpp" "RECURSE" "${FOLLY_DIR}")
auto_sources(cfiles "*.c" "RECURSE" "${FOLLY_DIR}")
auto_sources(hfiles "*.h" "RECURSE" "${FOLLY_DIR}")

# No need for tests, Benchmarks, Utils, or most experimental stuff
HHVM_REMOVE_MATCHES_FROM_LISTS(files cfiles hfiles
 MATCHES
 "/bench/"
 "/experimental/logging/example/"
 "/experimental/pushmi/"
 "/experimental/exception_tracer/"
 "/futures/exercises/"
 "/python/"
 "/test/"
 "Test.cpp$"
)
if (NOT LINUX)
  HHVM_REMOVE_MATCHES_FROM_LISTS(files cfiles hfiles
   MATCHES
   "/experimental/symbolizer/"
  )
endif()
list(REMOVE_ITEM files
  ${FOLLY_DIR}/Benchmark.cpp
  ${FOLLY_DIR}/SingletonStackTrace.cpp
  ${FOLLY_DIR}/build/GenerateFingerprintTables.cpp
  ${FOLLY_DIR}/experimental/DynamicParser.cpp
  ${FOLLY_DIR}/experimental/JSONSchemaTester.cpp
  ${FOLLY_DIR}/experimental/RCUUtils.cpp
  ${FOLLY_DIR}/experimental/ProgramOptions.cpp
  ${FOLLY_DIR}/experimental/Select64.cpp
  ${FOLLY_DIR}/experimental/TestUtil.cpp
  ${FOLLY_DIR}/experimental/exception_tracer/ExceptionCounterLib.cpp
  ${FOLLY_DIR}/experimental/exception_tracer/ExceptionStackTraceLib.cpp
  ${FOLLY_DIR}/experimental/exception_tracer/ExceptionTracer.cpp
  ${FOLLY_DIR}/experimental/exception_tracer/ExceptionTracerBenchmark.cpp
  ${FOLLY_DIR}/experimental/exception_tracer/ExceptionTracerLib.cpp
  ${FOLLY_DIR}/experimental/exception_tracer/StackTrace.cpp
  ${FOLLY_DIR}/experimental/io/AsyncIO.cpp
  ${FOLLY_DIR}/experimental/io/HugePageUtil.cpp
  ${FOLLY_DIR}/experimental/symbolizer/StackTrace.cpp
  ${FOLLY_DIR}/experimental/symbolizer/SignalHandler.cpp
  ${FOLLY_DIR}/init/Init.cpp
  # io/Compression requires snappy library
  # Don't add dep until we need it
  ${FOLLY_DIR}/io/Compression.cpp
  ${FOLLY_DIR}/python/GILAwareManualExecutor.cpp
)
list(REMOVE_ITEM hfiles
  ${FOLLY_DIR}/Benchmark.h
  ${FOLLY_DIR}/experimental/DynamicParser.h
  ${FOLLY_DIR}/experimental/DynamicParser-inl.h
  ${FOLLY_DIR}/experimental/RCUUtils.h
  ${FOLLY_DIR}/experimental/ProgramOptions.h
  ${FOLLY_DIR}/experimental/TestUtil.h
  ${FOLLY_DIR}/experimental/exception_tracer/ExceptionAbi.h
  ${FOLLY_DIR}/experimental/exception_tracer/ExceptionCounterLib.h
  ${FOLLY_DIR}/experimental/exception_tracer/ExceptionTracer.h
  ${FOLLY_DIR}/experimental/exception_tracer/StackTrace.h
  ${FOLLY_DIR}/experimental/io/AsyncIO.h
  ${FOLLY_DIR}/experimental/symbolizer/StackTrace.h
  ${FOLLY_DIR}/experimental/symbolizer/ElfCache.h
  ${FOLLY_DIR}/experimental/symbolizer/SignalHandler.h
  ${FOLLY_DIR}/init/Init.h
  # io/Compression requires snappy library
  # Don't add dep until we need it
  ${FOLLY_DIR}/io/Compression.h
  ${FOLLY_DIR}/python/AsyncioExecutor.h
  ${FOLLY_DIR}/python/GILAwareManualExecutor.h
)

CHECK_CXX_SOURCE_COMPILES("#include <bits/functexcept.h>
int main() {
  return 0;
}" FOLLY_FUNCTEXCEPT)
if (FOLLY_FUNCTEXCEPT)
  list(REMOVE_ITEM files ${FOLLY_DIR}/detail/FunctionalExcept.cpp)
  list(REMOVE_ITEM hfiles ${FOLLY_DIR}/detail/FunctionalExcept.h)
endif()

if (LINUX)
  list(REMOVE_ITEM files ${FOLLY_DIR}/detail/Clock.cpp)
  list(REMOVE_ITEM hfiles ${FOLLY_DIR}/detail/Clock.h)
elseif(WINDOWS)
  list(REMOVE_ITEM files ${FOLLY_DIR}/Subprocess.cpp)
  list(REMOVE_ITEM hfiles ${FOLLY_DIR}/Subprocess.h)
endif()

CHECK_CXX_SOURCE_COMPILES("
#ifndef __x86_64__
#error Not x64
#endif
int main() { return 0; }" IS_X64)
if (IS_X64)
set_source_files_properties(${FOLLY_DIR}/hash/detail/ChecksumDetail.cpp PROPERTIES COMPILE_FLAGS -mpclmul)
set_source_files_properties(${FOLLY_DIR}/hash/detail/Crc32CombineDetail.cpp PROPERTIES COMPILE_FLAGS -mpclmul)
set_source_files_properties(${FOLLY_DIR}/hash/detail/Crc32cDetail.cpp PROPERTIES COMPILE_FLAGS -mpclmul)
endif()

add_library(folly STATIC ${files} ${genfiles} ${cfiles} ${hfiles})
auto_source_group(folly ${FOLLY_DIR} ${files} ${genfiles} ${cfiles} ${hfiles})

# Ensure that we are either getting malloc functions
# like malloc_usable_size() from either malloc.h
# or stdlib.h. Default is stdlib.h
folly_check_source_compiles("
#include <malloc.h>
int main() {
  return 0;
}" FOLLY_HAVE_MALLOC_H)

# This is so clients of folly can #include <folly/blah>
target_include_directories(folly PUBLIC ${FOLLY_ROOT})

add_dependencies(folly boostMaybeBuild)
target_link_libraries(folly boost)

add_dependencies(folly libsodiumMaybeBuild)
target_include_directories(folly PUBLIC "${LIBSODIUM_INCLUDE_DIRS}")
target_link_libraries(folly libsodium)

find_package(Glog REQUIRED)
target_include_directories(folly PUBLIC ${LIBGLOG_INCLUDE_DIR})
target_link_libraries(folly ${LIBGLOG_LIBRARY})

find_package(PThread REQUIRED)
target_include_directories(folly PUBLIC ${LIBPTHREAD_INCLUDE_DIRS})
target_link_libraries(folly ${LIBPTHREAD_LIBRARIES})

if (DOUBLE_CONVERSION_FOUND)
  target_link_libraries(folly ${DOUBLE_CONVERSION_LIBRARY})
else()
  target_link_libraries(folly double-conversion)
endif()

if (JEMALLOC_ENABLED)
  target_link_libraries(folly ${JEMALLOC_LIB})
endif()

# For some reason we aren't making a folly-config.h and this is in there.
# Please fix properly!
add_definitions("-DFOLLY_VERSION=\"0.1\"")

# folly-config.h is a generated file from autotools
# We need to do the equivalent checks here and use
# add_definitions as needed
target_compile_definitions(folly PUBLIC "FOLLY_NO_CONFIG=1")
target_compile_definitions(folly PUBLIC "FOLLY_HAVE_PTHREAD=1")

# Check for the function FNNAME and set RETURN_VALUE based on the result.  Also
# sets RETURN_VALUE=<result> as a preprocessor macro.
function (folly_check_function_exists FNNAME RETURN_VALUE)
  check_function_exists(${FNNAME} ${RETURN_VALUE})
  if (${RETURN_VALUE})
    target_compile_definitions(folly PUBLIC "${RETURN_VALUE}=1")
  else()
    target_compile_definitions(folly PUBLIC "${RETURN_VALUE}=0")
  endif()
endfunction(folly_check_function_exists)

cmake_push_check_state()
find_library(HAS_LIBRT "rt")
if (HAS_LIBRT)
  set(CMAKE_REQUIRED_LIBRARIES rt ${LIBPTHREAD_LIBRARIES})
else()
  set(CMAKE_REQUIRED_LIBRARIES ${LIBPTHREAD_LIBRARIES})
endif()

folly_check_function_exists("clock_gettime" FOLLY_HAVE_CLOCK_GETTIME)
folly_check_function_exists("pthread_atfork" FOLLY_HAVE_PTHREAD_ATFORK)
folly_check_function_exists("pthread_spin_lock" FOLLY_HAVE_PTHREAD_SPINLOCK_T)

cmake_pop_check_state()

find_path(FEATURES_H_INCLUDE_DIR NAMES features.h)
if (FEATURES_H_INCLUDE_DIR)
  target_include_directories(folly PUBLIC "${FEATURES_H_INCLUDE_DIR}")
  target_compile_definitions(folly PUBLIC "FOLLY_HAVE_FEATURES_H=1")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if(NOT CLANG_FORCE_LIBSTDCXX)
    target_compile_definitions(folly PUBLIC "FOLLY_USE_LIBCPP=1")
  endif()
endif()

find_package(LibDwarf REQUIRED)
folly_check_source_compiles("
#include <libdwarf/dwarf.h>
int main(int argc, char** argv) { }
" FOLLY_HAVE_LIBDWARF_DWARF_H)

folly_check_source_compiles("
int main(int argc, char** argv) {
  unsigned size = argc;
  char data[size];
  return 0;
}
" FOLLY_HAVE_VLA)

folly_check_source_compiles("
extern \"C\" void (*test_ifunc(void))() { return 0; }
void func() __attribute__((ifunc(\"test_ifunc\")));
" FOLLY_HAVE_IFUNC)

check_include_file_cxx("bits/functexcept.h" FOLLY_HAVE_BITS_FUNCTEXCEPT_H)
if (FOLLY_HAVE_BITS_FUNCTEXCEPT_H)
  target_compile_definitions(folly PUBLIC "FOLLY_HAVE_BITS_FUNCTEXCEPT_H=1")
else()
  target_compile_definitions(folly PUBLIC "FOLLY_HAVE_BITS_FUNCTEXCEPT_H=0")
endif()

check_include_file("linux/membarrier.h" FOLLY_HAVE_LINUX_MEMBARRIER_H)
if (FOLLY_HAVE_LINUX_MEMBARRIER_H)
  target_compile_definitions(folly PUBLIC "FOLLY_HAVE_LINUX_MEMBARRIER_H=1")
else()
  target_compile_definitions(folly PUBLIC "FOLLY_HAVE_LINUX_MEMBARRIER_H=0")
endif()

# Weak linking on Linux, Windows, and OS X all work somewhat differently. The following test
# works well on Linux and Windows, but fails for annoying reasons on OS X, and even works
# differently on different releases of OS X, cf. http://glandium.org/blog/?p=2764. Getting
# the test to work properly on OS X would require an APPLE check anyways, so just hardcode
# OS X as "we know weak linking works".
if(APPLE)
  target_compile_definitions(folly PUBLIC "FOLLY_HAVE_WEAK_SYMBOLS=1")
else()
  # check for weak symbols
  folly_check_source_compiles("
      extern \"C\" void configure_link_extern_weak_test() __attribute__((weak));
      int main(int argc, char** argv) {
          return configure_link_extern_weak_test == nullptr;
      }
  " FOLLY_HAVE_WEAK_SYMBOLS)
endif()

folly_check_function_exists("memrchr" FOLLY_HAVE_MEMRCHR)
folly_check_function_exists("preadv" FOLLY_HAVE_PREADV)
folly_check_function_exists("pwritev" FOLLY_HAVE_PWRITEV)

# We've included gflags - so we know we have it.
target_compile_definitions(folly PUBLIC "FOLLY_HAVE_LIBGFLAGS=0" "NO_LIB_GFLAGS")
# This is so clients can #include <gflags/gflags.h>
target_include_directories(folly PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(OpenSSL REQUIRED)
target_include_directories(folly PUBLIC "${OPENSSL_INCLUDE_DIR}")

if (APPLE)
  # Just assume we have sched.h
  target_compile_definitions(folly PUBLIC "FOLLY_HAVE_SCHED_H=1")
endif()
