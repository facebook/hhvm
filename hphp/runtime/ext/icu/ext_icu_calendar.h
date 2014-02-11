#ifndef incl_HPHP_INTL_CALENDAR_H
#define incl_HPHP_INTL_CALENDAR_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/calendar.h>
#include <unicode/gregocal.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////

class IntlCalendar : public IntlResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(IntlCalendar);
  CLASSNAME_IS("IntlCalendar");
  const String& o_getClassNameHook() const override { return classnameof(); }

  explicit IntlCalendar(icu::Calendar *cal) : m_cal(cal) {}

  void sweep() override {
    if (m_cal) {
      delete m_cal;
      m_cal = nullptr;
    }
  }

  bool isInvalid() const override {
    return m_cal == nullptr;
  }

  static IntlCalendar *Get(Object obj);
  Object wrap();

  icu::Calendar *calendar() const { return m_cal; }

  static const icu::Calendar* ParseArg(CVarRef cal, const icu::Locale &locale,
                                       const String &funcname, intl_error &err,
                                       int64_t &calType, bool &calOwned);
 protected:
  icu::Calendar *m_cal = nullptr;
};

class IntlGregorianCalendar : public IntlCalendar {
 public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(IntlGregorianCalendar);
  CLASSNAME_IS("IntlGregorianCalendar");
  const String& o_getClassNameHook() const override { return classnameof(); }

  explicit IntlGregorianCalendar(icu::GregorianCalendar *cal)
    : IntlCalendar(cal) {}

  bool isInvalid() const override {
    return m_cal == nullptr;
  }

  static IntlGregorianCalendar *Get(Object obj);
  Object wrap();

  icu::GregorianCalendar *calendar() const {
    return dynamic_cast<icu::GregorianCalendar*>(m_cal);
  }
};


/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_INTL_CALENDAR_H
