/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
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
