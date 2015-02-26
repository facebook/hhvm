/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/type-constraint.h"

#include "hphp/runtime/base/datatype.h"

#include <folly/Format.h>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

std::string TypeConstraint::toString() const {
  std::string ret = "<" + typeCategoryName(category);

  if (category == DataTypeSpecialized) {
    if (wantArrayKind()) ret += ",ArrayKind";
    if (wantArrayShape()) ret += ",ArrayShape";
    if (wantClass()) {
      folly::toAppend("Cls:", desiredClass()->name()->data(), &ret);
    }
  }

  if (weak) ret += ",weak";

  return ret + '>';
}

///////////////////////////////////////////////////////////////////////////////
}}
