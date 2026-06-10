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

#include "hphp/runtime/ext/hsl/ext_hsl_locale.h"

namespace HPHP {
  struct HSLLocale::Ops {
    virtual ~Ops() {}
    virtual int64_t strlen(const OptString&) const = 0;
    virtual OptString uppercase(const OptString&) const = 0;
    virtual OptString lowercase(const OptString&) const = 0;
    virtual OptString foldcase(const OptString&) const = 0;
    virtual int64_t strcoll(const OptString&, const OptString&) const = 0;
    virtual int64_t strcasecmp(const OptString&, const OptString&) const = 0;

    // Special-case each of these instead of adding `substrcoll`/`substrcasecmp`
    // as:
    // - these are all that are needed for the HSL
    // - they can be more performant, especially for the HSL; for example,
    //   calculating offsets and lengths may require an encoding conversion
    //   which we're going to have to do here anyway, let's not do it twice
    virtual bool starts_with(const OptString& str, const OptString& prefix) const = 0;
    virtual bool starts_with_ci(const OptString& str, const OptString& prefix) const = 0;
    virtual bool ends_with(const OptString& str, const OptString& prefix) const = 0;
    virtual bool ends_with_ci(const OptString& str, const OptString& prefix) const = 0;

    virtual OptString strip_prefix(const OptString& str, const OptString& prefix) const = 0;
    virtual OptString strip_suffix(const OptString& str, const OptString& suffix) const = 0;

    virtual int64_t strpos(const OptString& haystack, const OptString& needle, int64_t offset) const = 0;
    virtual int64_t strrpos(const OptString& haystack, const OptString& needle, int64_t offset) const = 0;
    virtual int64_t stripos(const OptString& haystack, const OptString& needle, int64_t offset) const = 0;
    virtual int64_t strripos(const OptString& haystack, const OptString& needle, int64_t offset) const = 0;

    virtual Array chunk(const OptString&, int64_t chunk_size) const = 0;
    virtual OptString slice(const OptString& str, int64_t offset, int64_t length) const = 0;
    virtual OptString splice(const OptString& str, const OptString& replacement, int64_t offset, int64_t length) const = 0;
    virtual Array split(const OptString& str, const OptString& delimiter, int64_t limit = -1) const = 0;

    virtual OptString reverse(const OptString& str) const = 0;

    virtual OptString pad_left(const OptString& str, int64_t len, const OptString& pad) const = 0;
    virtual OptString pad_right(const OptString& str, int64_t len, const OptString& pad) const = 0;

    enum class TrimSides: uint8_t {
      LEFT = 1,
      RIGHT = 2,
      BOTH = LEFT | RIGHT
    };

    virtual OptString trim(const OptString& str, TrimSides sides) const = 0;
    virtual OptString trim(const OptString& str, const OptString& what, TrimSides sides) const = 0;

    virtual OptString replace(const OptString& haystack, const OptString& needle, const OptString& replacement) const = 0;
    virtual OptString replace_ci(const OptString& haystack, const OptString& needle, const OptString& replacement) const = 0;
    virtual OptString replace_every(const OptString& haystack, const Array& replacements) const = 0;
    virtual OptString replace_every_ci(const OptString& haystack, const Array& replacements) const = 0;
    virtual OptString replace_every_nonrecursive(const OptString& haystack, const Array& replacements) const = 0;
    virtual OptString replace_every_nonrecursive_ci(const OptString& haystack, const Array& replacements) const = 0;

    static int64_t normalize_offset(int64_t offset, int64_t length);
  };
} // namespace HPHP
