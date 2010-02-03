/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#define UTF8_END   -1
#define UTF8_ERROR -2

#ifdef __cplusplus
extern "C" {
#endif
  typedef struct json_utf8_decode {
    int the_index;
    char *the_input;
    int the_length;
    int the_char;
    int the_byte;
  } json_utf8_decode;

  int  utf8_decode_at_byte(json_utf8_decode *utf8);
  int  utf8_decode_at_character(json_utf8_decode *utf8);
  void utf8_decode_init(json_utf8_decode *utf8, char p[], int length);
  int  utf8_decode_next(json_utf8_decode *utf8);
#ifdef __cplusplus
}
#endif
