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
    : m_cursor(utf8), m_end(utf8 + length), m_index(utf8), m_loose(loose) {}

  int decodeTail();
  int decodeAsUTF8();

  int decode() {
    const char *pos = m_cursor;
    if (UNLIKELY(pos >= m_end)) {
      if (m_low_surrogate != 0) {
        int ret = m_low_surrogate;
        m_low_surrogate = 0;
        m_cursor = m_surrogate_saved_cursor;
        return ret;
      }
      m_cursor = pos + 1;
      return UTF8_END;
    }

    unsigned char c = *pos;
    if (LIKELY(c < 0x80)) {
      m_cursor = pos + 1;
      return c;
    }
    return decodeTail();
  }

private:
  int getNext();
  unsigned int getNextChar();
  const char *m_cursor;
  const char *m_end;
  const char *m_index; // used for decodeAsUTF8
  const char *m_surrogate_saved_cursor;
  int m_low_surrogate = 0; // used for decode
  bool m_loose; // Facebook: json_utf8_loose
};

///////////////////////////////////////////////////////////////////////////////
}

