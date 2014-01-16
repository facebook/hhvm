#ifndef incl_HPHP_ICU_NUMFMT_H
#define incl_HPHP_ICU_NUMFMT_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/unum.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////

class NumberFormatter : public IntlResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(NumberFormatter);
  CLASSNAME_IS("NumberFormatter");
  const String& o_getClassNameHook() const override { return classnameof(); }

  NumberFormatter(const String& locale,
                  int64_t style,
                  const String& pattern);
  explicit NumberFormatter(const NumberFormatter *orig);

  void sweep() override {
    if (m_formatter) {
      unum_close(m_formatter);
      m_formatter = nullptr;
    }
  }

  bool isInvalid() const override {
    return m_formatter == nullptr;
  }

  static NumberFormatter* Get(Object obj);
  Object wrap();

  UNumberFormat *formatter() const { return m_formatter; }

private:
  UNumberFormat *m_formatter;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_NUMFMT_H
