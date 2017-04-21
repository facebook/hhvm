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

#ifndef incl_HPHP_JIT_TYPE_PROFILE_H_
#define incl_HPHP_JIT_TYPE_PROFILE_H_

#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/type-scan.h"

#include <string>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * TypeProfile keeps the union of all the types observed during profiling.
 */
struct TypeProfile {
  std::string toString() const { return type.toString(); }

  void report(TypedValue tv) {
    type |= typeFromTV(&tv, nullptr);
  }

  static void reduce(TypeProfile& a, const TypeProfile& b) {
    a.type |= b.type;
  }

  Type type; // This gets initialized with 0, which is TBottom.
  static_assert(Type::Bits::kBottom == 0, "Assuming TBottom is 0");

  // In RDS, but can't contain pointers to request-allocated data.
  TYPE_SCAN_IGNORE_ALL;
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif
