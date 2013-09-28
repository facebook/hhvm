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

#ifndef incl_HPHP_CASE_INSENSITIVE_H_
#define incl_HPHP_CASE_INSENSITIVE_H_

#include "hphp/util/base.h"
#include "hphp/util/hash.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct hashi {
  size_t operator()(const char *s) const {
    return hash_string_i(s, strlen(s));
  }
};
struct eqstri {
  bool operator()(const char* s1, const char* s2) const {
    return strcasecmp(s1, s2) == 0;
  }
};

struct string_hashi {
  size_t operator()(const std::string &s) const {
    return hash_string_i(s.data(), s.size());
  }
};

template<typename S, typename S2=S>
struct stringlike_eqstri {
  bool operator()(const S &s1, const S2 &s2) const {
    return s1.size() == s2.size() &&
      strncasecmp(s1.data(), s2.data(), s1.size()) == 0;
  }
};
typedef stringlike_eqstri<std::string> string_eqstri;

struct string_lessi {
  bool operator()(const std::string &s1, const std::string &s2) const {
    return strcasecmp(s1.data(), s2.data()) < 0;
  }
};

template<typename T>
class hphp_const_char_imap :
      public hphp_hash_map<const char *, T, hashi, eqstri> {
};

class hphp_const_char_iset :
      public hphp_hash_set<const char *, hashi, eqstri> {
};

template<typename T>
class hphp_string_imap :
      public hphp_hash_map<std::string, T, string_hashi, string_eqstri> {
};

class hphp_string_iset :
      public hphp_hash_set<std::string, string_hashi, string_eqstri> {
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_CASE_INSENSITIVE_H_
