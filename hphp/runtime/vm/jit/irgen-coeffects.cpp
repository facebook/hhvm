/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/vm/coeffects.h"

#include "hphp/runtime/vm/jit/ssa-tmp.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

jit::SSATmp* CoeffectRule::emitJit() const {
  switch (m_type) {
    case Type::ConditionalReactiveImplements:
    case Type::ConditionalReactiveArgImplements:
    case Type::FunParam:
    case Type::CCThis:
    case Type::CCParam:
      return nullptr;
    case Type::Invalid:
      always_assert(false);
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
