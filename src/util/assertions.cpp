#include "util/assertions.h"

namespace HPHP {

// In builds without NDEBUG, we don't have __assert_fail from the GNU
// library, so we implement it here for always_assert().
void impl_assert_fail(const char* e, const char* file,
                      unsigned int line, const char* func) {
  fprintf(stderr, "%s:%d: %s: assertion `%s' failed.", file, line, func, e);
  std::abort();
}

}
