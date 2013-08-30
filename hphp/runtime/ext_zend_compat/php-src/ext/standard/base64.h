/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

/* $Id$ */

#ifndef BASE64_H
#define BASE64_H

#include "hphp/runtime/ext/ext_url.h"

PHPAPI inline unsigned char *php_base64_decode_ex(const unsigned char *str, int length, int *ret_length, zend_bool strict) {
  HPHP::Variant ret = f_base64_decode(HPHP::String(str, length, HPHP::CopyString), strict);
  HPHP::String stringRet = ret.toString();
  *ret_length = stringRet.size();
  return stringRet.data();
}
PHPAPI inline unsigned char *php_base64_decode(const unsigned char *str, int length, int *ret_length) {
  return php_base64_decode_ex(str, length, ret_length, 0);
}

#endif /* BASE64_H */
