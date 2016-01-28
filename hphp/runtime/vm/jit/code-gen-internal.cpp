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

#include "hphp/runtime/vm/jit/code-gen-internal.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/member-operations.h"

#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

ArrayKeyInfo checkStrictlyInteger(Type key) {
  auto ret = ArrayKeyInfo{};

  if (key <= TInt) {
    ret.type = KeyType::Int;
    return ret;
  }
  assertx(key <= TStr);
  ret.type = KeyType::Str;

  if (key.hasConstVal()) {
    int64_t i;
    if (key.strVal()->isStrictlyInteger(i)) {
      ret.converted    = true;
      ret.type         = KeyType::Int;
      ret.convertedInt = i;
    }
  } else {
    ret.checkForInt = true;
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

}}
