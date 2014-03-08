/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ZEND_PACK_H_
#define incl_HPHP_ZEND_PACK_H_

#include "hphp/runtime/base/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// pack/unpack

/**
 * Implemented formats are A, a, h, H, c, C, s, S, i, I, l, L, n, N, f, d,
 * x, X, @.
 */
class ZendPack {
public:
  ZendPack();

  /**
   * Takes one or more arguments and packs them into a binary string according
   * to the format argument. pack() idea stolen from Perl (implemented formats
   * behave the same as there).
   */
  Variant pack(const String& fmt, const Array& argv);

  /**
   * Unpack binary string into named array elements according to format
   * argument.
   *
   * unpack() is based on Perl's unpack(), but is modified a bit from there.
   * Rather than depending on error-prone ordered lists or syntactically
   * unpleasant pass-by-reference, we return an object with named paramters
   * (like *_fetch_object()). Syntax is "f[repeat]name/...", where "f" is the
   * formatter char (like pack()), "[repeat]" is the optional repeater
   * argument, and "name" is the name of the variable to use.
   * Example: "c2chars/nints" will return an object with fields
   * chars1, chars2, and ints.
   * Numeric pack types will return numbers, a and A will return strings,
   * f and d will return doubles.
   */
  Variant unpack(const String& fmt, const String& data);

private:
  // Whether machine is little endian
  char machine_little_endian;

  // Mapping of byte from char (8bit) to int32 for machine endian
  int byte_map[1];

  // Mappings of bytes from int (machine dependant) to int for machine endian
  int int_map[sizeof(int)];

  // Mappings of bytes from shorts (16bit) for all endian environments
  int machine_endian_short_map[2];
  int big_endian_short_map[2];
  int little_endian_short_map[2];

  // Mappings of bytes from int32s (32bit) for all endian environments
  int machine_endian_int32_map[4];
  int big_endian_int32_map[4];
  int little_endian_int32_map[4];

  void pack(const Variant& val, int size, int *map, char *output);
  int32_t unpack(const char *data, int size, int issigned, int *map);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_PACK_H_
