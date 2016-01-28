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

#ifndef incl_HPHP_H_
#define incl_HPHP_H_

#include "hphp/util/deprecated/declare-boost-types.h"
#include "hphp/util/functional.h"
#include "hphp/util/hash-map-typedefs.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ClassScope;
class FunctionScope;

// ClassScope and FunctionScope are compared case-insensitively.

template<class V>
struct hphp_string_hash_map<V, ClassScope> :
  hphp_hash_map<std::string, V, string_hashi, string_eqstri> {};

template<class V>
struct hphp_string_hash_map<V, FunctionScope> :
  hphp_hash_map<std::string, V, string_hashi, string_eqstri> {};

///////////////////////////////////////////////////////////////////////////////
}

#endif
