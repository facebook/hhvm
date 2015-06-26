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
#include "hphp/runtime/vm/jit/srcdb.h"

#include <stdint.h>
#include <stdarg.h>
#include <string>

#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/recycle-tc.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/service-requests-x64.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/trace.h"

#include <folly/MoveWrapper.h>

namespace HPHP { namespace jit {

TRACE_SET_MOD(trans)

void IncomingBranch::relocate(RelocationInfo& rel) {
  // compute adjustedTarget before altering the smash address,
  // because it might be a 5-byte nop
  TCA adjustedTarget = rel.adjustedAddressAfter(target());

  if (TCA adjusted = rel.adjustedAddressAfter(toSmash())) {
    m_ptr.set(m_ptr.tag(), adjusted);
  }

  if (adjustedTarget) {
    FTRACE_MOD(Trace::mcg, 1, "Patching: 0x{:08x} from 0x{:08x} to 0x{:08x}\n",
               (uintptr_t)toSmash(), (uintptr_t)target(),
               (uintptr_t)adjustedTarget);

    patch(adjustedTarget);
  }
}

void IncomingBranch::patch(TCA dest) {
  switch (type()) {
    case Tag::JMP: {
      mcg->backEnd().smashJmp(toSmash(), dest);
      mcg->getDebugInfo()->recordRelocMap(toSmash(), dest, "Arc-2");
      break;
    }

    case Tag::JCC: {
      mcg->backEnd().smashJcc(toSmash(), dest);
      mcg->getDebugInfo()->recordRelocMap(toSmash(), dest, "Arc-1");
      break;
    }

    case Tag::ADDR: {
      // Note that this effectively ignores a
      TCA* addr = reinterpret_cast<TCA*>(toSmash());
      assert_address_is_atomically_accessible(addr);
      *addr = dest;
      break;
    }
  }
}

TCA IncomingBranch::target() const {
  switch (type()) {
    case Tag::JMP:
      return mcg->backEnd().jmpTarget(toSmash());

    case Tag::JCC:
      return mcg->backEnd().jccTarget(toSmash());

    case Tag::ADDR:
      return *reinterpret_cast<TCA*>(toSmash());
  }
  always_assert(false);
}

TCA TransLoc::mainStart()   const { return mcg->code.base() + m_mainOff; }
TCA TransLoc::coldStart()   const { return mcg->code.base() + m_coldOff; }
TCA TransLoc::frozenStart() const { return mcg->code.base() + m_frozenOff; }

void TransLoc::setMainStart(TCA newStart) {
  assert(mcg->code.base() <= newStart &&
         newStart - mcg->code.base() < std::numeric_limits<uint32_t>::max());

  m_mainOff = newStart - mcg->code.base();
}

void TransLoc::setColdStart(TCA newStart) {
  assert(mcg->code.base() <= newStart &&
         newStart - mcg->code.base() < std::numeric_limits<uint32_t>::max());

  m_coldOff = newStart - mcg->code.base();
}

void TransLoc::setFrozenStart(TCA newStart) {
  assert(mcg->code.base() <= newStart &&
         newStart - mcg->code.base() < std::numeric_limits<uint32_t>::max());

  m_frozenOff = newStart - mcg->code.base();
}

void SrcRec::setFuncInfo(const Func* f) {
  m_unitMd5 = f->unit()->md5();
}

/*
 * The fallback translation is where to jump to if the
 * currently-translating translation's checks fail.
 *
 * The current heuristic we use for translation chaining is to assume
 * the most common cases are probably translated first, so we chain
 * new translations on the end.  This means if we have to fallback
 * from the currently-translating translation we jump to the "anchor"
 * translation (which just is a REQ_RETRANSLATE).
 */
TCA SrcRec::getFallbackTranslation() const {
  assertx(m_anchorTranslation);
  return m_anchorTranslation;
}

FPInvOffset SrcRec::nonResumedSPOff() const {
  return serviceReqSPOff(getFallbackTranslation());
}

void SrcRec::chainFrom(IncomingBranch br) {
  assertx(br.type() == IncomingBranch::Tag::ADDR ||
         mcg->code.isValidCodeAddress(br.toSmash()));
  TCA destAddr = getTopTranslation();
  m_incomingBranches.push_back(br);
  TRACE(1, "SrcRec(%p)::chainFrom %p -> %p (type %d); %zd incoming branches\n",
        this,
        br.toSmash(), destAddr, static_cast<int>(br.type()),
        m_incomingBranches.size());
  br.patch(destAddr);

  if (RuntimeOption::EvalEnableReusableTC) {
    recordJump(br.toSmash(), this);
  }
}

void SrcRec::emitFallbackJump(CodeBlock& cb, ConditionCode cc /* = -1 */) {
  // This is a spurious platform dependency. TODO(2990497)
  mcg->backEnd().prepareForSmash(
    cb,
    cc == CC_None ? x64::kJmpLen : x64::kJmpccLen
  );

  auto from = cb.frontier();
  TCA destAddr = getFallbackTranslation();
  mcg->backEnd().emitSmashableJump(cb, destAddr, cc);
  registerFallbackJump(from, cc);
}

void SrcRec::registerFallbackJump(TCA from, ConditionCode cc /* = -1 */) {
  auto incoming = cc < 0 ? IncomingBranch::jmpFrom(from)
                         : IncomingBranch::jccFrom(from);

  // We'll need to know the location of this jump later so we can
  // patch it to new translations added to the chain.
  mcg->cgFixups().m_inProgressTailJumps.push_back(incoming);
}

void SrcRec::emitFallbackJumpCustom(CodeBlock& cb,
                                    CodeBlock& frozen,
                                    SrcKey sk,
                                    TransFlags trflags,
                                    ConditionCode cc) {
  // Another platform dependency (the same one as above). TODO(2990497)
  auto optSPOff = folly::Optional<FPInvOffset>{};
  if (!sk.resumed()) optSPOff = nonResumedSPOff();
  auto toSmash = x64::emitRetranslate(cb, frozen, cc, sk, optSPOff, trflags);
  registerFallbackJump(toSmash, cc);
}

void SrcRec::newTranslation(TransLoc loc,
                            GrowableVector<IncomingBranch>& tailBranches) {
  // When translation punts due to hitting limit, will generate one
  // more translation that will call the interpreter.
  assertx(m_translations.size() <= RuntimeOption::EvalJitMaxTranslations);

  TRACE(1, "SrcRec(%p)::newTranslation @%p, ", this, loc.mainStart());

  m_translations.push_back(loc);
  if (!m_topTranslation.load(std::memory_order_acquire)) {
    m_topTranslation.store(loc.mainStart(), std::memory_order_release);
    patchIncomingBranches(loc.mainStart());
  }

  /*
   * Link all the jumps from the current tail translation to this new
   * guy.
   *
   * It's (mostly) ok if someone is running in this code while we do
   * this: we hold the write lease, they'll instead jump to the anchor
   * and do REQ_RETRANSLATE and failing to get the write lease they'll
   * interp.  FIXME: Unfortunately, right now, in an unlikely race
   * another thread could create another translation with the same
   * type specialization that we just created in this case.  (If we
   * happen to release the write lease after they jump but before they
   * get into REQ_RETRANSLATE, they'll acquire it and generate a
   * translation possibly for this same situation.)
   */
  for (auto& br : m_tailFallbackJumps) {
    br.patch(loc.mainStart());
  }

  // This is the new tail translation, so store the fallback jump list
  // in case we translate this again.
  m_tailFallbackJumps.swap(tailBranches);
}

void SrcRec::relocate(RelocationInfo& rel) {
  if (auto adjusted = rel.adjustedAddressAfter(m_anchorTranslation)) {
    m_anchorTranslation = adjusted;
  }

  if (auto adjusted = rel.adjustedAddressAfter(m_topTranslation.load())) {
    m_topTranslation.store(adjusted);
  }

  for (auto &t : m_translations) {
    if (TCA adjusted = rel.adjustedAddressAfter(t.mainStart())) {
      t.setMainStart(adjusted);
    }

    if (TCA adjusted = rel.adjustedAddressAfter(t.coldStart())) {
      t.setColdStart(adjusted);
    }

    if (TCA adjusted = rel.adjustedAddressAfter(t.frozenStart())) {
      t.setFrozenStart(adjusted);
    }
  }

  for (auto &ib : m_tailFallbackJumps) {
    ib.relocate(rel);
  }

  for (auto &ib : m_incomingBranches) {
    ib.relocate(rel);
  }
}

void SrcRec::addDebuggerGuard(TCA dbgGuard, TCA dbgBranchGuardSrc) {
  assertx(!m_dbgBranchGuardSrc);

  TRACE(1, "SrcRec(%p)::addDebuggerGuard @%p, "
        "%zd incoming branches to rechain\n",
        this, dbgGuard, m_incomingBranches.size());

  patchIncomingBranches(dbgGuard);

  // Set m_dbgBranchGuardSrc after patching, so we don't try to patch
  // the debug guard.
  m_dbgBranchGuardSrc = dbgBranchGuardSrc;
  m_topTranslation.store(dbgGuard, std::memory_order_release);
}

void SrcRec::patchIncomingBranches(TCA newStart) {
  if (hasDebuggerGuard()) {
    // We have a debugger guard, so all jumps to us funnel through
    // this.  Just smash m_dbgBranchGuardSrc.
    TRACE(1, "smashing m_dbgBranchGuardSrc @%p\n", m_dbgBranchGuardSrc);
    mcg->backEnd().smashJmp(m_dbgBranchGuardSrc, newStart);
    return;
  }

  TRACE(1, "%zd incoming branches to rechain\n", m_incomingBranches.size());

  for (auto &br : m_incomingBranches) {
    TRACE(1, "SrcRec(%p)::newTranslation rechaining @%p -> %p\n",
          this, br.toSmash(), newStart);
    br.patch(newStart);
  }
}

void SrcRec::removeIncomingBranch(TCA toSmash) {
  auto end = std::remove_if(
    m_incomingBranches.begin(),
    m_incomingBranches.end(),
    [toSmash] (const IncomingBranch& ib) { return ib.toSmash() == toSmash; }
  );
  assertx(end != m_incomingBranches.end());
  m_incomingBranches.setEnd(end);
}

void SrcRec::replaceOldTranslations() {
  // Everyone needs to give up on old translations; send them to the anchor,
  // which is a REQ_RETRANSLATE.
  auto translations = std::move(m_translations);
  m_tailFallbackJumps.clear();
  m_topTranslation.store(nullptr, std::memory_order_release);

  /*
   * It may seem a little weird that we're about to point every
   * incoming branch at the anchor, since that's going to just
   * unconditionally retranslate this SrcKey and never patch the
   * incoming branch to do something else.
   *
   * The reason this is ok is this mechanism is only used in
   * non-RepoAuthoritative mode, and the granularity of code
   * invalidation there is such that we'll only have incoming branches
   * like this basically within the same file since we don't have
   * whole program analysis.
   *
   * This means all these incoming branches are about to go away
   * anyway ...
   *
   * If we ever change that we'll have to change this to patch to
   * some sort of rebind requests.
   */
  assertx(!RuntimeOption::RepoAuthoritative || RuntimeOption::EvalJitPGO);
  patchIncomingBranches(m_anchorTranslation);

  // Now that we've smashed all the IBs for these translations they should be
  // unreachable-- to prevent a race we treadmill here and then reclaim their
  // associated TC space
  if (RuntimeOption::EvalEnableReusableTC) {
    auto trans = folly::makeMoveWrapper(std::move(translations));
    Treadmill::enqueue([trans]() mutable {
      for (auto& loc : *trans) {
        reclaimTranslation(loc);
      }
      trans->clear();
    });
    return;
  }

  translations.clear();
}

} } // HPHP::jit
