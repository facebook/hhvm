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

    virtual int64_t strlen(const OptString&) const override;
    virtual OptString uppercase(const OptString&) const override;
    virtual OptString lowercase(const OptString&) const override;
    virtual OptString foldcase(const OptString&) const override;

    virtual int64_t strcoll(const OptString&, const OptString&) const override;
    virtual int64_t strcasecmp(const OptString&, const OptString&) const override;
    virtual bool starts_with(const OptString& str, const OptString& prefix) const override;
    virtual bool starts_with_ci(const OptString& str, const OptString& prefix) const override;
    virtual bool ends_with(const OptString& str, const OptString& suffix) const override;
    virtual bool ends_with_ci(const OptString& str, const OptString& suffix) const override;

    virtual OptString strip_prefix(const OptString& str, const OptString& prefix) const override;
    virtual OptString strip_suffix(const OptString& str, const OptString& suffix) const override;

    virtual int64_t strpos(const OptString& haystack, const OptString& needle, int64_t offset) const override;
    virtual int64_t strrpos(const OptString& haystack, const OptString& needle, int64_t offset) const override;
    virtual int64_t stripos(const OptString& haystack, const OptString& needle, int64_t offset) const override;
    virtual int64_t strripos(const OptString& haystack, const OptString& needle, int64_t offset) const override;

    virtual Array chunk(const OptString&, int64_t) const override;
    virtual OptString slice(const OptString& str, int64_t offset, int64_t length) const override;
    virtual OptString splice(const OptString& str, const OptString& replacement, int64_t offset, int64_t length) const override;
    virtual Array split(const OptString& str, const OptString& delimiter, int64_t limit = -1) const override;

    virtual OptString reverse(const OptString& str) const override;

    virtual OptString pad_left(const OptString& str, int64_t len, const OptString& pad) const override;
    virtual OptString pad_right(const OptString& str, int64_t len, const OptString& pad) const override;

    virtual OptString trim(const OptString& str, TrimSides sides) const override;
    virtual OptString trim(const OptString& str, const OptString& what, TrimSides sides) const override;

    virtual OptString replace(const OptString& haystack, const OptString& needle, const OptString& replacement) const override;
    virtual OptString replace_ci(const OptString& haystack, const OptString& needle, const OptString& replacement) const override;

    virtual OptString replace_every(const OptString& haystack, const Array& replacements) const override;
    virtual OptString replace_every_ci(const OptString& haystack, const Array& replacements) const override;
    virtual OptString replace_every_nonrecursive(const OptString& haystack, const Array& replacements) const override;
    virtual OptString replace_every_nonrecursive_ci(const OptString& haystack, const Array& replacements) const override;
    private:
      locale_t m_loc;
  };
} // namespace HPHP
