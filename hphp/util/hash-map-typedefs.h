/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_HASH_MAP_TYPEDEFS_H_
#define incl_HPHP_HASH_MAP_TYPEDEFS_H_

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>

#include "hphp/util/functional.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Note: these deliberately change the default-constructor behavior of
 * unordered_{map,set} to allocate slightly less memory.
 *
 * Generally be careful about using these hashtables.  They're not
 * cheap, although they may be convenient.
 */

template <class T, class U,
          class V = std::hash<T>,
          class W = std::equal_to<T> >
struct hphp_hash_map : std::unordered_map<T,U,V,W> {
  hphp_hash_map() : std::unordered_map<T,U,V,W>(0) {}
};

template <class T,
          class V = std::hash<T>,
          class W = std::equal_to<T> >
struct hphp_hash_set : std::unordered_set<T,V,W> {
  hphp_hash_set() : std::unordered_set<T,V,W>(0) {}
};

//////////////////////////////////////////////////////////////////////

template<class type, class T> struct hphp_string_hash_map :
  public hphp_hash_map<std::string,type,string_hash> {
};

template<class T> using hphp_const_char_map =
  hphp_hash_map<const char*,T,cstr_hash,eqstr>;

template<typename T>
class hphp_string_map :
    public hphp_hash_map<std::string, T, string_hash> {
};

typedef hphp_hash_set<std::string, string_hash> hphp_string_set;

typedef hphp_hash_map<void*, void*, pointer_hash<void> > PointerMap;
typedef hphp_hash_map<void*, int, pointer_hash<void> > PointerCounterMap;
typedef hphp_hash_set<void*, pointer_hash<void> > PointerSet;

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

//////////////////////////////////////////////////////////////////////

}

#endif
