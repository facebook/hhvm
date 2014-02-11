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

  int64_t key() const { return m_key; }
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
    m_key++;
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
    m_key = -1;
    next();
    return true;
  }

private:
  icu::StringEnumeration *m_enum = nullptr;
  int64_t m_key = -1;
  Variant m_current = null_string;
};

#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 42
// Proxy StringEnumeration for consistent behavior
class BugStringCharEnumeration : public icu::StringEnumeration
{
public:
  explicit BugStringCharEnumeration(UEnumeration* _uenum) : uenum(_uenum) {}
  ~BugStringCharEnumeration() { uenum_close(uenum); }

  int32_t count(UErrorCode& status) const {
    return uenum_count(uenum, &status);
  }

  const UnicodeString* snext(UErrorCode& status) override {
    int32_t length;
    const UChar* str = uenum_unext(uenum, &length, &status);
    if (str == 0 || U_FAILURE(status)) {
      return 0;
    }
    return &unistr.setTo(str, length);
  }

  const char* next(int32_t *resultLength, UErrorCode &status) override {
    int32_t length = -1;
    const char* str = uenum_next(uenum, &length, &status);
    if (str == 0 || U_FAILURE(status)) {
      return 0;
    }
    if (resultLength) {
      //the bug is that uenum_next doesn't set the length
      *resultLength = (length == -1) ? strlen(str) : length;
    }

    return str;
  }

  void reset(UErrorCode& status) {
    uenum_reset(uenum, &status);
  }

  // Defined by UOBJECT_DEFINE_RTTI_IMPLEMENTATION
  UClassID getDynamicClassID() const override;
  static UClassID U_EXPORT2 getStaticClassID();

 private:
  UEnumeration *uenum;
};
#endif // icu >= 4.2

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_ITERATOR_H
