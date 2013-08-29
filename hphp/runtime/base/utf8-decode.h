/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ZEND_UTF8_DECODE_H_
#define incl_HPHP_ZEND_UTF8_DECODE_H_

#define UTF8_END   -1
#define UTF8_ERROR -2

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct json_utf8_decode {
  int the_index;
  const char *the_input;
  int the_length;
  int the_char;
  int the_byte;
};

class UTF8To16Decoder {
public:
  UTF8To16Decoder(const char *utf8, int length, bool loose);
  int decode();

private:
  json_utf8_decode m_decode;
  int m_loose; // Faceook: json_utf8_loose
  int m_low_surrogate;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_UTF8_DECODE_H_
