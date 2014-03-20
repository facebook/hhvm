#ifndef incl_HPHP_ICU_COLLATOR_H
#define incl_HPHP_ICU_COLLATOR_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/ucol.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_Collator;

class Collator : public IntlError {
 public:
  Collator() {}
  Collator(const Collator&) = delete;
  Collator& operator=(const Collator& src) {
    *this = src;
    char stack[U_COL_SAFECLONE_BUFFERSIZE];
    int32_t stack_size = sizeof(stack);
    UErrorCode error = U_ZERO_ERROR;
    m_collator = ucol_safeClone(src.m_collator, stack, &stack_size, &error);
    if (U_FAILURE(error)) {
      throw getException("Something went wrong cloning Collator: %d",
                         (int)error);
    }
    return *this;
  }
  ~Collator() { setCollator(nullptr); }

  static Collator* Get(Object obj) {
    return GetData<Collator>(obj, s_Collator);
  }

  bool isValid() const {
    return m_collator;
  }

  UCollator* collator() const { return m_collator; }
  void setCollator(UCollator* col) {
    if (m_collator) {
      ucol_close(m_collator);
    }
    m_collator = col;
  }

 private:
  UCollator* m_collator{nullptr};
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_COLLATOR_H
