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

#ifndef incl_HPHP_ZEND_URL_H_
#define incl_HPHP_ZEND_URL_H_

#include "hphp/util/base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Helper struct storing parsed result from url_parse().
 */
class Url {
public:
  ~Url();

  char *scheme;
  char *user;
  char *pass;
  char *host;
  unsigned short port;
  char *path;
  char *query;
  char *fragment;
};

bool url_parse(Url &output, const char *str, int length);

/**
 * raw_ versions ignore "+" or " ".
 */
char *url_encode(const char *s, int &len);
char *url_decode(const char *s, int &len);
int url_decode(char *value); // in-place version, also assuming C-string
int url_decode_ex(char *value, int len);
char *url_raw_encode(const char *s, int &len);
char *url_raw_decode(const char *s, int &len);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_URL_H_
