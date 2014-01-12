#ifndef incl_HPHP_INTL_CALENDAR_H
#define incl_HPHP_INTL_CALENDAR_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/calendar.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////

// TODO: Add IntlCalendar class (subclass of IntlResourceData)
class IntlCalendar {
 public:
  static const icu::Calendar* ParseArg(CVarRef cal, const icu::Locale &locale,
                                       const String &funcname, intl_error &err,
                                       int64_t &calType, bool &calOwned);
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_INTL_CALENDAR_H
