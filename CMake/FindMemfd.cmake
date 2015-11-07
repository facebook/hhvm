
INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(memfd_create HAVE_MEMFD_CREATE)

if (NOT HAVE_MEMFD_CREATE)
  INCLUDE(CheckCSourceCompiles)
  CHECK_C_SOURCE_COMPILES("#include <unistd.h>
#include <asm/unistd.h>
int main() {
  return syscall(__NR_memfd_create, \"/dummy\", 0);
}" HAVE_DECL___NR_MEMFD_CREATE)
endif (NOT HAVE_MEMFD_CREATE)
