#ifndef incl_HPHP_ICU_ITERATOR_H
#define incl_HPHP_ICU_ITERATOR_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/strenum.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////

class IntlIterator : public IntlResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(IntlIterator);
  CLASSNAME_IS("IntlIterator");
  const String& o_getClassNameHook() const override { return classnameof(); }

  explicit IntlIterator(icu::StringEnumeration *se) : m_enum(se) {}

  void sweep() override {
    if (m_enum) {
      delete m_enum;
      m_enum = nullptr;
    }
  }

  bool isInvalid() const override {
    return m_enum == nullptr;
  }

  static IntlIterator *Get(Object obj);
  Object wrap();

  Variant current() const { return m_current; }
  bool valid() const { return m_current.isString(); }

  Variant next() {
    UErrorCode error = U_ZERO_ERROR;
    int32_t len;
    const char *e = m_enum->next(&len, error);
    if (U_FAILURE(error)) {
      s_intl_error->set(error, "Error fetching next iteration element");
      m_current = uninit_null();
    } else {
      m_current = String(e, len, CopyString);
    }
    return m_current;
  }

  bool rewind() {
    UErrorCode error = U_ZERO_ERROR;
    m_enum->reset(error);
    if (U_FAILURE(error)) {
      s_intl_error->set(error, "Error resetting enumeration");
      m_current = uninit_null();
      return false;
    }
    next();
    return true;
  }

private:
  icu::StringEnumeration *m_enum = nullptr;
  Variant m_current = null_string;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_ITERATOR_H
