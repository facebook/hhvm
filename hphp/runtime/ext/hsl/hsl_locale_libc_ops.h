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
  // Extended by HSLLocaleByteOps for `C` locale
  struct HSLLocaleLibcOps : public HSLLocale::Ops {
    explicit HSLLocaleLibcOps(const Locale& locale);
    virtual ~HSLLocaleLibcOps() override;

    virtual int64_t strlen(const String&) const override;
    virtual String uppercase(const String&) const override;
    virtual String lowercase(const String&) const override;
    virtual String foldcase(const String&) const override;

    virtual Array chunk(const String&, int64_t) const override;

    virtual int64_t strcoll(const String&, const String&) const override;
    virtual int64_t strcasecmp(const String&, const String&) const override;
    virtual bool starts_with(const String& str, const String& prefix) const override;
    virtual bool starts_with_ci(const String& str, const String& prefix) const override;
    virtual bool ends_with(const String& str, const String& suffix) const override;
    virtual bool ends_with_ci(const String& str, const String& suffix) const override; 

    virtual int64_t strpos(const String& haystack, const String& needle, int64_t offset) const override;
    virtual int64_t strrpos(const String& haystack, const String& needle, int64_t offset) const override;
    virtual int64_t stripos(const String& haystack, const String& needle, int64_t offset) const override;
    virtual int64_t strripos(const String& haystack, const String& needle, int64_t offset) const override;

    virtual String slice(const String& str, int64_t offset, int64_t length) const override;

    virtual String reverse(const String& str) const override;
    private:
      locale_t m_loc;
  };
} // namespace HPHP
