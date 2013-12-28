#ifndef incl_HPHP_ICU_TIMEZONE_H
#define incl_HPHP_ICU_TIMEZONE_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/timezone.h>
#include <unicode/strenum.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////

class IntlTimeZone : public IntlResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(IntlTimeZone);
  CLASSNAME_IS("IntlTimeZone");
  const String& o_getClassNameHook() const override { return classnameof(); }

  explicit IntlTimeZone(icu::TimeZone *tz, bool owned = true):
    m_timezone(tz), m_owned(owned) {}

  void sweep() override {
    if (m_timezone && m_owned) {
      delete m_timezone;
    }
    m_timezone = nullptr;
  }

  bool isInvalid() const override {
    return m_timezone == nullptr;
  }

  static IntlTimeZone *Get(Object obj);
  Object wrap();

  static bool isValidStyle(int64_t style) {
    return (style == icu::TimeZone::SHORT) ||
           (style == icu::TimeZone::LONG) ||
#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 44
           (style == icu::TimeZone::SHORT_GENERIC) ||
           (style == icu::TimeZone::LONG_GENERIC) ||
           (style == icu::TimeZone::SHORT_GMT) ||
           (style == icu::TimeZone::LONG_GMT) ||
           (style == icu::TimeZone::SHORT_COMMONLY_USED) ||
           (style == icu::TimeZone::GENERIC_LOCATION) ||
#endif
           false;
  }

  icu::TimeZone* timezone() const { return m_timezone; }

 private:
  icu::TimeZone *m_timezone;
  bool m_owned;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_TIMEZONE_H
