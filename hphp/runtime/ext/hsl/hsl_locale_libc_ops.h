/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/hsl/hsl_locale_ops.h"

namespace HPHP {
  struct HSLLocaleLibcOps final : public HSLLocale::Ops {
    explicit HSLLocaleLibcOps(const Locale& locale);
    virtual ~HSLLocaleLibcOps() override;

    virtual int64_t strlen(const String&) const override;
    virtual String uppercase(const String&) const override;
    virtual String lowercase(const String&) const override;
    virtual String foldcase(const String&) const override;
    virtual Array chunk(const String&, int64_t) const override;
    private:
      locale_t m_loc;
  };
} // namespace HPHP
