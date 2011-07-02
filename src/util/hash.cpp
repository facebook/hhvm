#include <string.h>
#include <util/hash.h>

namespace HPHP {

long long hash_string_i(const char *arKey, int nKeyLength) {
  return hash_string_i_inline(arKey, nKeyLength);
}

long long hash_string(const char *arKey, int nKeyLength) {
  return hash_string_i_inline(arKey, nKeyLength);
}

}
