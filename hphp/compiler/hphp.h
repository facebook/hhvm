/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/util/base.h"
#include "hphp/util/case_insensitive.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ClassScope;

template<class type> struct hphp_string_hash_map<type, ClassScope> :
      public hphp_hash_map<std::string, type, string_hashi,
                           string_eqstri> {};

///////////////////////////////////////////////////////////////////////////////
}

#endif  // incl_HPHP_H_
