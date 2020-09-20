#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/ucol.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_Collator;

struct Collator : IntlError {
  Collator() {}
  Collator(const Collator&) = delete;
  Collator& operator=(const Collator& src) {
    IntlError::operator =(src);
    char stack[U_COL_SAFECLONE_BUFFERSIZE];
    int32_t stack_size = sizeof(stack);
    UErrorCode error = U_ZERO_ERROR;
    m_collator = ucol_safeClone(src.m_collator, stack, &stack_size, &error);
    if (U_FAILURE(error)) {
      throwException("Something went wrong cloning Collator: %d",
                     (int)error);
    }
    return *this;
  }
  ~Collator() { setCollator(nullptr); }

  static Collator* Get(ObjectData* obj) {
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

