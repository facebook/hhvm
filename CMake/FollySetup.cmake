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

set(CMAKE_REQUIRED_LIBRARIES rt)
CHECK_CXX_SOURCE_COMPILES("#include <time.h>
int main() {
  clock_gettime((clockid_t)0, NULL);
  return 0;
}" HAVE_CLOCK_GETTIME)

if (HAVE_CLOCK_GETTIME)
	add_definitions("-DFOLLY_HAVE_CLOCK_GETTIME=1")
endif()
set(CMAKE_REQUIRED_LIBRARIES)

find_path(FEATURES_H_INCLUDE_DIR NAMES features.h)
if (FEATURES_H_INCLUDE_DIR)
	include_directories("${FEATURES_H_INCLUDE_DIR}")
	add_definitions("-DFOLLY_HAVE_FEATURES_H=1")
endif()
