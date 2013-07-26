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
#include "hphp/runtime/vm/jit/srcdb.h"

#include <stdint.h>
#include <stdarg.h>
#include <string>

#include "hphp/util/base.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/translator-x64.h"

namespace HPHP {
namespace Transl {

TRACE_SET_MOD(trans)

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
  assert(m_anchorTranslation);
  return m_anchorTranslation;
}

void SrcRec::chainFrom(IncomingBranch br) {
  assert(br.type() == IncomingBranch::Tag::ADDR    ||
         tx64->a.           contains(br.toSmash()) ||
         tx64->ahot.        contains(br.toSmash()) ||
         tx64->aprof.       contains(br.toSmash()) ||
         tx64->astubs.      contains(br.toSmash()) ||
         tx64->atrampolines.contains(br.toSmash()));
  TCA destAddr = getTopTranslation();
  m_incomingBranches.push_back(br);
  TRACE(1, "SrcRec(%p)::chainFrom %p -> %p (type %d); %zd incoming branches\n",
        this,
        br.toSmash(), destAddr, br.type(), m_incomingBranches.size());
  patch(br, destAddr);
}

void SrcRec::emitFallbackJump(TCA from, int cc /* = -1 */) {
  TCA destAddr = getFallbackTranslation();
  auto incoming = cc < 0 ? IncomingBranch::jmpFrom(from)
                         : IncomingBranch::jccFrom(from);
  auto& a = tx64->getAsmFor(from);

  // emit dummy jump to be smashed via patch()
  if (cc < 0) {
    a.jmp(a.frontier());
  } else {
    assert(incoming.type() == IncomingBranch::Tag::JCC);
    a.jcc((ConditionCode)cc, a.frontier());
  }

  patch(incoming, destAddr);

  // We'll need to know the location of this jump later so we can
  // patch it to new translations added to the chain.
  m_inProgressTailJumps.push_back(incoming);
}

void SrcRec::newTranslation(TCA newStart) {
  // When translation punts due to hitting limit, will generate one
  // more translation that will call the interpreter.
  assert(m_translations.size() <= RuntimeOption::EvalJitMaxTranslations);

  TRACE(1, "SrcRec(%p)::newTranslation @%p, ", this, newStart);

  m_translations.push_back(newStart);
  if (!m_topTranslation) {
    atomic_release_store(&m_topTranslation, newStart);
    patchIncomingBranches(newStart);
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
  for (size_t i = 0; i < m_tailFallbackJumps.size(); ++i) {
    patch(m_tailFallbackJumps[i], newStart);
  }

  // This is the new tail translation, so store the fallback jump list
  // in case we translate this again.
  m_tailFallbackJumps.swap(m_inProgressTailJumps);
  m_inProgressTailJumps.clear();
}

void SrcRec::addDebuggerGuard(TCA dbgGuard, TCA dbgBranchGuardSrc) {
  assert(!m_dbgBranchGuardSrc);

  TRACE(1, "SrcRec(%p)::addDebuggerGuard @%p, "
        "%zd incoming branches to rechain\n",
        this, dbgGuard, m_incomingBranches.size());

  patchIncomingBranches(dbgGuard);

  // Set m_dbgBranchGuardSrc after patching, so we don't try to patch
  // the debug guard.
  m_dbgBranchGuardSrc = dbgBranchGuardSrc;
  atomic_release_store(&m_topTranslation, dbgGuard);
}

void SrcRec::patchIncomingBranches(TCA newStart) {
  if (hasDebuggerGuard()) {
    // We have a debugger guard, so all jumps to us funnel through
    // this.  Just smash m_dbgBranchGuardSrc.
    TRACE(1, "smashing m_dbgBranchGuardSrc @%p\n", m_dbgBranchGuardSrc);
    TranslatorX64::smashJmp(tx64->getAsmFor(m_dbgBranchGuardSrc),
                            m_dbgBranchGuardSrc,
                            newStart);
    return;
  }

  TRACE(1, "%zd incoming branches to rechain\n", m_incomingBranches.size());

  vector<IncomingBranch>& change = m_incomingBranches;
  for (unsigned i = 0; i < change.size(); ++i) {
    TRACE(1, "SrcRec(%p)::newTranslation rechaining @%p -> %p\n",
          this, change[i].toSmash(), newStart);
    patch(change[i], newStart);
  }
}

void SrcRec::replaceOldTranslations() {
  // Everyone needs to give up on old translations; send them to the anchor,
  // which is a REQ_RETRANSLATE.
  m_translations.clear();
  m_tailFallbackJumps.clear();
  atomic_release_store(&m_topTranslation, static_cast<TCA>(0));

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
  assert(!RuntimeOption::RepoAuthoritative || RuntimeOption::EvalJitPGO);
  patchIncomingBranches(m_anchorTranslation);
}

void SrcRec::patch(IncomingBranch branch, TCA dest) {
  switch (branch.type()) {
  case IncomingBranch::Tag::JMP: {
    auto toSmash = branch.toSmash();
    auto& a = tx64->getAsmFor(toSmash);
    CodeCursor cg(a, branch.toSmash());
    TranslatorX64::smashJmp(a, branch.toSmash(), dest);
    break;
  }

  case IncomingBranch::Tag::JCC: {
    // patch destination, but preserve the condition code
    int32_t delta = safe_cast<int32_t>((dest - branch.toSmash()) - kJmpccLen);
    int32_t* addr = (int32_t*)(branch.toSmash() + kJmpccLen - 4);
    atomic_release_store(addr, delta);
    break;
  }

  case IncomingBranch::Tag::ADDR:
    // Note that this effectively ignores a
    atomic_release_store(reinterpret_cast<TCA*>(branch.toSmash()), dest);
  }
}

/*
 * Returns number of destroyed references to file.
 */
size_t SrcDB::invalidateCode(const Eval::PhpFile* file) {
  assert(currentRank() == RankBase);
  /*
   * Hold the write lease; otherwise some other thread may have started
   * translating this very file.
   */
  BlockingLeaseHolder writer(Translator::WriteLease());

  assert(!RuntimeOption::RepoAuthoritative);
  unsigned i = 0;
  {
    TRACE(1, "SrcDB::invalidateCode: file %p\n", file);
    FileDepMap::iterator entry = m_deps.find(file);
    if (entry != m_deps.end()) {
      GrowableVector<SrcKey>* deferredSrcKeys = entry->second;
      for (/* already inited*/; i < deferredSrcKeys->size(); i++) {
        tx64->invalidateSrcKey((*deferredSrcKeys)[i]);
      }
      TRACE(1, "SrcDB::invalidateCode: file %p has %zd srcKeys\n", file,
            entry->second->size());
      m_deps.erase(entry);
      free(deferredSrcKeys);
    }
  }
  return i;
}

} } // HPHP::Transl
