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

#include <runtime/vm/func.h>
#include <runtime/vm/peephole.h>

namespace HPHP {
namespace VM {

using namespace std;

Peephole::Peephole(Unit &unit) : m_unit(unit) {

  // be careful about an empty input
  if (unit.m_bclen == 0) {
    return;
  }

  // We need to know which instructions may be jumped to, since this blocks
  // certain optimizations
  buildJumpTargets();

  // Scan the bytecode linearly.
  Opcode* start = unit.m_bc;
  Opcode* prev = start;
  Opcode* cur = prev + instrLen(prev);
  Opcode* end = start + unit.m_bclen;

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
      }

      // IncDec* Post*,  PopC -> IncDec* Pre*,  PopC
      if (*cur == OpPopC) {
        switch (*prev) {
        case OpIncDecH:
        case OpIncDecN:
        case OpIncDecG:
        case OpIncDecS:
        case OpIncDecM: {
          ArgUnion* imm = getImmPtr(prev, 0);
          if (imm->u_OA == PostInc) {
            imm->u_OA = PreInc;
          } else if (imm->u_OA == PostDec) {
            imm->u_OA = PreDec;
          }
          break;
          }
        default:
          break;
        }
      }
    }

    // Simplify jumps. Follow a jump's target until it lands on something that
    // isn't an unconditional jump. Then rewrite the offset to cut out any
    // intermediate jumps.
    Offset destOffset = instrJumpTarget(start, prev - start);
    if (destOffset != InvalidAbsoluteOffset) {
      Opcode* dest = start + destOffset;

      // Watch out for infinite loops
      while (*dest == OpJmp && dest != prev) {
        destOffset = instrJumpTarget(start, dest - start);
        dest = start + destOffset;
      }
      *instrJumpOffset(prev) = dest - prev;
    }

    prev = cur;
    cur = cur + instrLen(cur);
  }
}

void Peephole::buildFuncTargets(Func* f) {
  m_jumpTargets.insert(f->m_base);
  for (vector<EHEnt>::const_iterator it = f->m_ehtab.begin();
      it != f->m_ehtab.end(); ++it) {
    m_jumpTargets.insert(it->m_base);
    m_jumpTargets.insert(it->m_past);
    for (EHEnt::CatchVec::const_iterator catchIt = it->m_catches.begin();
         catchIt != it->m_catches.end(); ++catchIt) {
      m_jumpTargets.insert(catchIt->second);
    }
    m_jumpTargets.insert(it->m_fault);
  }
  for (uint i = 0; i < f->m_params.size(); i++) {
    const Func::ParamInfo& pi = f->m_params[i];
    if (pi.hasDefaultValue()) {
      m_jumpTargets.insert(pi.m_funcletOff);
    }
  }
  for (vector<FPIEnt>::const_iterator it = f->m_fpitab.begin();
       it != f->m_fpitab.end(); ++it) {
    m_jumpTargets.insert(it->m_base);
    m_jumpTargets.insert(it->m_past);
  }
}

void Peephole::buildJumpTargets() {
  // all function start locations, exception handlers, default value funclets,
  //   and FPI regions are targets
  for (vector<Func*>::const_iterator it = m_unit.m_funcs.begin();
       it != m_unit.m_funcs.end(); ++it) {
    Func *f = *it;
    buildFuncTargets(f);
  }
  for (vector<PreClassPtr>::const_iterator
       it = m_unit.m_preClasses.begin();
       it != m_unit.m_preClasses.end(); ++it) {
    for (PreClass::MethodVec::iterator mit = (*it)->m_methods.begin();
         mit != (*it)->m_methods.end(); ++mit) {
      Func* f = *mit;
      buildFuncTargets(f);
    }
  }
  // all jump targets are targets
  for (Offset pos = 0; pos < (Offset)m_unit.m_bclen;
       pos += instrLen(&m_unit.m_bc[pos])) {
    Offset target = instrJumpTarget(m_unit.m_bc, pos);
    if (target != InvalidAbsoluteOffset) {
      m_jumpTargets.insert(target);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}
