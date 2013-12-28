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

#include "hphp/compiler/analysis/peephole.h"
#include "hphp/compiler/analysis/emitter.h"
#include "hphp/runtime/vm/preclass-emit.h"

namespace HPHP { namespace Compiler {


static void collapseJmp(Offset* offsetPtr, Op* instr, Op* start) {
  if (offsetPtr) {
    Op* dest = instr + *offsetPtr;
    while (isUnconditionalJmp(*dest) && dest != instr) {
      dest = start + instrJumpTarget(start, dest - start);
    }
    *offsetPtr = dest - instr;
  }
}

Peephole::Peephole(UnitEmitter &ue, MetaInfoBuilder& metaInfo)
    : m_ue(ue) {
  // be careful about an empty input
  if (ue.m_bclen == 0) {
    return;
  }

  // We need to know which instructions may be jumped to, since this blocks
  // certain optimizations
  buildJumpTargets();

  // Scan the bytecode linearly.
  Op* start = (Op*)ue.m_bc;
  Op* prev = start;
  Op* cur = prev + instrLen(prev);
  Op* end = start + ue.m_bclen;

  /*
   * TODO(1086005): we should try to minimize use of CGetL2/CGetL3.
   * (When they appear in an order like "CGetL; CGetL2" we can just
   * switch them to "CGetL; CGetL".)
   */

  while (cur < end) {
    if (LIKELY(!m_jumpTargets.count(cur - start))) {
      // prev and cur are always dynamically adjacent (i.e. whenever cur is
      // executed, prev was always the previous instruction), so we can optimize
      // them based on this assumption.

      // Not, JmpZ -> Nop, JmpNZ (and vice versa)
      // We replace the Not with a Nop, instead of shifting up the rest of the
      // bytecode. Shifting has a lot of costs: copying bytecode around, and
      // remapping jump targets and line numbers. This optimization is rare
      // enough that the space savings are not worthwhile.
      if (*prev == OpNot) {
        if (*cur == OpJmpZ) {
          *prev = OpNop;
          *cur = OpJmpNZ;
        } else if (*cur == OpJmpNZ) {
          *prev = OpNop;
          *cur = OpJmpZ;
        }
        metaInfo.deleteInfo(prev - start);
      }

      // IncDec* Post*,  PopC -> IncDec* Pre*,  PopC
      ArgUnion* imm = 0;
      if (*cur == OpPopC) {
        switch (*prev) {
        case OpIncDecL:
          imm = getImmPtr(prev, 1);
          goto incDecOp;
        case OpIncDecN:
        case OpIncDecG:
        case OpIncDecS:
        case OpIncDecM:
          imm = getImmPtr(prev, 0);
          // fallthrough

        incDecOp:
          if (static_cast<IncDecOp>(imm->u_OA) == IncDecOp::PostInc) {
            imm->u_OA = static_cast<unsigned char>(IncDecOp::PreInc);
          } else if (static_cast<IncDecOp>(imm->u_OA) == IncDecOp::PostDec) {
            imm->u_OA = static_cast<unsigned char>(IncDecOp::PreDec);
          }
          break;
        default:
          break;
        }
      }
    }

    // Simplify jumps. Follow a jump's target until it lands on something that
    // isn't an unconditional jump. Then rewrite the offset to cut out any
    // intermediate jumps.
    if (isSwitch(*prev)) {
      foreachSwitchTarget(prev, [&](Offset& o) {
        collapseJmp(&o, prev, start);
      });
    } else {
      collapseJmp(instrJumpOffset(prev), prev, start);
    }

    prev = cur;
    cur = cur + instrLen(cur);
  }
}

void Peephole::buildFuncTargets(FuncEmitter* fe) {
  m_jumpTargets.insert(fe->base());
  for (FuncEmitter::EHEntVec::const_iterator it = fe->ehtab().begin();
      it != fe->ehtab().end(); ++it) {
    m_jumpTargets.insert(it->m_base);
    m_jumpTargets.insert(it->m_past);
    for (EHEnt::CatchVec::const_iterator catchIt = it->m_catches.begin();
         catchIt != it->m_catches.end(); ++catchIt) {
      m_jumpTargets.insert(catchIt->second);
    }
    m_jumpTargets.insert(it->m_fault);
  }
  for (uint i = 0; i < fe->params().size(); i++) {
    const FuncEmitter::ParamInfo& pi = fe->params()[i];
    if (pi.hasDefaultValue()) {
      m_jumpTargets.insert(pi.funcletOff());
    }
  }
  for (FuncEmitter::FPIEntVec::const_iterator it = fe->fpitab().begin();
       it != fe->fpitab().end(); ++it) {
    m_jumpTargets.insert(it->m_fpushOff);
    m_jumpTargets.insert(it->m_fcallOff);
  }
}

void Peephole::buildJumpTargets() {
  // all function start locations, exception handlers, default value funclets,
  //   and FPI regions are targets
  for (UnitEmitter::FeVec::const_iterator it = m_ue.m_fes.begin();
       it != m_ue.m_fes.end(); ++it) {
    FuncEmitter *fe = *it;
    buildFuncTargets(fe);
  }
  for (UnitEmitter::PceVec::const_iterator it = m_ue.m_pceVec.begin();
       it != m_ue.m_pceVec.end(); ++it) {
    for (PreClassEmitter::MethodVec::const_iterator mit =
         (*it)->methods().begin(); mit != (*it)->methods().end(); ++mit) {
      FuncEmitter* fe = *mit;
      buildFuncTargets(fe);
    }
  }
  // all jump targets are targets
  for (Offset pos = 0; pos < (Offset)m_ue.m_bclen;
       pos += instrLen((Op*)&m_ue.m_bc[pos])) {
    Op* instr = (Op*)&m_ue.m_bc[pos];
    if (isSwitch(*instr)) {
      foreachSwitchTarget(instr, [&](Offset& o) {
        m_jumpTargets.insert(pos + o);
      });
    } else if (*instr == OpIterBreak) {
      uint32_t veclen = *(uint32_t *)(instr + 1);
      assert(veclen > 0);
      Offset target = *(Offset *)((uint32_t *)(instr + 1) + 2 * veclen + 1);
      m_jumpTargets.insert(pos + target);
    } else {
      Offset target = instrJumpTarget((Op*)m_ue.m_bc, pos);
      if (target != InvalidAbsoluteOffset) {
        m_jumpTargets.insert(target);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
