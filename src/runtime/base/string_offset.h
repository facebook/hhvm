/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_STRING_OFFSET_H__
#define __HPHP_STRING_OFFSET_H__

#include <runtime/base/types.h>
#include <runtime/base/string_data.h>
#include <runtime/base/util/exceptions.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Handles sub-string expressions. This class should delegate all real work to
 * StringData.
 */
class StringOffset {
 public:
  /**
   * Constructor. "offset" is where we are at in the string.
   */
  StringOffset(StringData *data, int offset)
    : m_data(data), m_offset(offset) {
    ASSERT(m_data);
  }

  /**
   * Get r-value of this offset object.
   */
  operator String() const;

  /**
   * Get l-value of this offset object. Well, not quite, since this is illegal.
   */
  String &lval() const {
    throw InvalidOperandException("taking l-value of a string offset");
  }

  /**
   * Assignement operator. Almost the whole purpose for having this offset
   * class.
   */
  StringOffset &operator=(CVarRef v);

 private:
  StringData *m_data;
  int m_offset;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_STRING_OFFSET_H__
