# folly-config.h is a generated file from autotools
# We need to do the equivalent checks here and use
# add_definitions as needed
add_definitions(-DFOLLY_NO_CONFIG=1)

INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
extern \"C\" void (*test_ifunc(void))() { return 0; }
void func() __attribute__((ifunc(\"test_ifunc\")));
" FOLLY_IFUNC)
if (FOLLY_IFUNC)
  add_definitions("-DHAVE_IFUNC=1")
endif()

if (LINUX OR FREEBSD)
  set(CMAKE_REQUIRED_LIBRARIES rt ${LIBPTHREAD_LIBRARIES})
else()
  set(CMAKE_REQUIRED_LIBRARIES ${LIBPTHREAD_LIBRARIES})
endif()

include(CheckFunctionExists)
CHECK_FUNCTION_EXISTS("clock_gettime" HAVE_CLOCK_GETTIME)
CHECK_FUNCTION_EXISTS("pthread_atfork" HAVE_PTHREAD_ATFORK)
CHECK_FUNCTION_EXISTS("pthread_spin_lock" HAVE_PTHREAD_SPINLOCK)

if (HAVE_CLOCK_GETTIME)
  add_definitions("-DFOLLY_HAVE_CLOCK_GETTIME=1")
endif()
if (HAVE_PTHREAD_ATFORK)
  add_definitions("-DFOLLY_HAVE_PTHREAD_ATFORK=1")
endif()
if (HAVE_PTHREAD_SPINLOCK)
  add_definitions("-DFOLLY_HAVE_PTHREAD_SPINLOCK_T=1")
endif()
set(CMAKE_REQUIRED_LIBRARIES)

find_path(FEATURES_H_INCLUDE_DIR NAMES features.h)
if (FEATURES_H_INCLUDE_DIR)
  include_directories("${FEATURES_H_INCLUDE_DIR}")
  add_definitions("-DFOLLY_HAVE_FEATURES_H=1")
endif()

if(CYGWIN)
# cygwin has c99 issues with cxx compiler and headers
  add_definitions("-D_GLIBCXX_USE_C99_DYNAMIC")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions("-DFOLLY_USE_LIBCPP=1")
endif()
