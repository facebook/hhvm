/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <cstdint>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// pack/unpack

struct Variant;
struct String;
struct Array;

/**
 * Implemented formats are A, a, h, H, c, C, s, S, i, I, l, L, n, N, f, d,
 * x, X, Z and @.
 */
struct ZendPack {
  /**
   * Takes one or more arguments and packs them into a binary string according
   * to the format argument. pack() idea stolen from Perl (implemented formats
   * behave the same as there).
   */
  static Variant pack(const String& fmt, const Array& argv);

  /**
   * Unpack binary string into named array elements according to format
   * argument.
   *
   * unpack() is based on Perl's unpack(), but is modified a bit from there.
   * Rather than depending on error-prone ordered lists or syntactically
   * unpleasant pass-by-reference, we return an object with named parameters
   * (like *_fetch_object()). Syntax is "f[repeat]name/...", where "f" is the
   * formatter char (like pack()), "[repeat]" is the optional repeater
   * argument, and "name" is the name of the variable to use.
   * Example: "c2chars/nints" will return an object with fields
   * chars1, chars2, and ints.
   * Numeric pack types will return numbers, a and A will return strings,
   * f and d will return doubles.
   */
  static Variant unpack(const String& fmt, const String& data);
};

///////////////////////////////////////////////////////////////////////////////
}
