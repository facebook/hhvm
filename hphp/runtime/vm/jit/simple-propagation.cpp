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

#include "hphp/runtime/vm/jit/simple-propagation.h"

#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

void constProp(IRUnit& unit, IRInstruction* inst) {
  for (auto& src : inst->srcs()) {
    if (!src->inst()->is(DefConst) && src->type().admitsSingleVal()) {
      src = unit.cns(src->type());
    }
  }
}

void copyProp(IRInstruction* inst) {
  for (auto& src : inst->srcs()) {
    while (src->inst()->is(Mov)) src = src->inst()->src(0);
  }
  auto fp = inst->marker().fp();
  while (fp && fp->inst()->is(Mov)) fp = fp->inst()->src(0);
  if (fp != inst->marker().fp()) inst->marker() = inst->marker().adjustFP(fp);
}

///////////////////////////////////////////////////////////////////////////////

}}
