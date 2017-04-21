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

#ifndef incl_HPHP_ZEND_URL_H_
#define incl_HPHP_ZEND_URL_H_

#include "hphp/runtime/base/type-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Helper struct storing parsed result from url_parse().
 */
struct Url {
  String scheme;
  String user;
  String pass;
  String host;
  unsigned short port = 0;
  String path;
  String query;
  String fragment;
};

bool url_parse(Url &output, const char *str, size_t length);

/**
 * raw_ versions ignore "+" or " ".
 */
String url_encode(const char *s, size_t len);
String url_decode(const char *s, size_t len);
size_t url_decode_ex(char *value, size_t len);
String url_raw_encode(const char *s, size_t len);
String url_raw_decode(const char *s, size_t len);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_URL_H_
