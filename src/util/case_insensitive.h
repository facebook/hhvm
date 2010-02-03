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

#ifndef __HPHP_CASE_INSENSITIVE_H__
#define __HPHP_CASE_INSENSITIVE_H__

#include <util/base.h>
#include <util/hash.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct eqstri {
  bool operator()(const char* s1, const char* s2) const {
    return strcasecmp(s1, s2) == 0;
  }
};
struct hashi {
  size_t operator()(const char *s) const {
    return hash_string_i(s, strlen(s));
  }
};

template<typename T>
class hphp_const_char_imap :
    public hphp_hash_map<const char *, T, hashi, eqstri> {
};

class hphp_const_char_iset :
    public hphp_hash_set<const char *, hashi, eqstri> {
};


///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_CASE_INSENSITIVE_H__
