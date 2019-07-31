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

#include "hphp/runtime/vm/jit/annotation.h"

#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/repo.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(trans);

//////////////////////////////////////////////////////////////////////

namespace {

const void annotate(NormalizedInstruction* i,
                    const StringData* funcName) {
  auto const fpi      = i->func()->findFPI(i->source.offset());
  auto pc             = i->m_unit->at(fpi->m_fpushOff);
  auto const pushOp   = decode_op(pc);

  auto decode_litstr = [&] {
    Id id;
    std::memcpy(&id, pc, sizeof id);
    pc += sizeof id;
    return i->m_unit->lookupLitstrId(id);
  };

  if (funcName->empty() &&
      (pushOp == Op::FPushFuncD || pushOp == Op::FPushFuncRD)) {
    decode_iva(pc);
    funcName = decode_litstr();
  }

  auto const func = lookupImmutableFunc(i->source.unit(), funcName).func;

  if (func) {
    FTRACE(1, "found direct func ({}) for FCall\n",
           func->fullName()->data());
    i->funcd = func;
  }
}

}

//////////////////////////////////////////////////////////////////////

void annotate(NormalizedInstruction* i) {
  if (!isLegacyFCall(i->op())) return;
  annotate(i, i->m_unit->lookupLitstrId(i->imm[2].u_SA));
}

//////////////////////////////////////////////////////////////////////

}}
