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
/* utf8_decode.h */

#pragma once

#include <folly/Likely.h>

#define UTF8_END   -1
#define UTF8_ERROR -2

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct UTF8To16Decoder {
  UTF8To16Decoder(const char *utf8, int length, bool loose)
    : m_str(utf8), m_strlen(length), m_cursor(0), m_loose(loose),
      m_low_surrogate(0) {}

  int decodeTail();
  int decodeAsUTF8();

  int decode() {
    if (UNLIKELY(m_low_surrogate)) {
      int ret = m_low_surrogate;
      m_low_surrogate = 0;
      return ret;
    }

    int pos = m_cursor;
    if (UNLIKELY(pos >= m_strlen)) {
      m_cursor = pos + 1;
      return UTF8_END;
    }

    unsigned char c = m_str[pos];
    if (LIKELY(c < 0x80)) {
      m_cursor++;
      return c;
    }
    return decodeTail();
  }

private:
  int getNext();
  unsigned int getNextChar();
  const char *m_str;
  int m_strlen;
  int m_cursor;
  int m_loose; // Faceook: json_utf8_loose
  union {
    int m_low_surrogate; // used for decode
    int m_index; // used for decodeAsUTF8
  };
};

///////////////////////////////////////////////////////////////////////////////
}

