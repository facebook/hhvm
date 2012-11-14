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

#include <stdint.h>
#include <stdarg.h>
#include <string>

#include "util/base.h"
#include "util/trace.h"
#include "runtime/vm/translator/translator-x64.h"
#include "srcdb.h"

namespace HPHP {
namespace VM {
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
  ASSERT(m_anchorTranslation);
  return m_anchorTranslation;
}

void SrcRec::chainFrom(Asm& a, IncomingBranch br) {
  TCA destAddr = getTopTranslation();
  m_incomingBranches.push_back(br);
  TRACE(1, "SrcRec(%p)::chainFrom %p -> %p; %zd incoming branches\n",
        this,
        a.code.frontier, destAddr, m_incomingBranches.size());
  patch(&a, br, destAddr);
}

void SrcRec::emitFallbackJump(Asm &a, TCA from, int cc /* = -1 */) {
  TCA destAddr = getFallbackTranslation();
  IncomingBranch incoming(cc < 0 ? IncomingBranch::JMP : IncomingBranch::JCC,
                          from);
  // emit dummy jump to be smashed via patch()
  if (cc < 0) {
    a.jmp(a.code.frontier);
  } else {
    ASSERT(incoming.m_type == IncomingBranch::JCC);
    a.jcc((ConditionCode)cc, a.code.frontier);
  }

  patch(&a, incoming, destAddr);

  // We'll need to know the location of this jump later so we can
  // patch it to new translations added to the chain.
  m_inProgressTailJumps.push_back(incoming);
}

void SrcRec::newTranslation(Asm& a, Asm &astubs, TCA newStart) {
  // When translation punts due to hitting limit, will generate one
  // more translation that will call the interpreter.
  ASSERT(m_translations.size() <= kMaxTranslations);

  TRACE(1, "SrcRec(%p)::newTranslation @%p, ", this, newStart);

  m_translations.push_back(newStart);
  if (!m_topTranslation) {
    atomic_release_store(&m_topTranslation, newStart);
    patchIncomingBranches(a, astubs, newStart);
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
    auto& as = asmChoose(m_tailFallbackJumps[i].m_src, a, astubs);
    patch(&as, m_tailFallbackJumps[i], newStart);
  }

  // This is the new tail translation, so store the fallback jump list
  // in case we translate this again.
  m_tailFallbackJumps.swap(m_inProgressTailJumps);
  m_inProgressTailJumps.clear();
}

void SrcRec::addDebuggerGuard(Asm& a, Asm &astubs, TCA dbgGuard,
                              TCA dbgBranchGuardSrc) {
  ASSERT(!m_dbgBranchGuardSrc);

  TRACE(1, "SrcRec(%p)::addDebuggerGuard @%p, "
        "%zd incoming branches to rechain\n",
        this, dbgGuard, m_incomingBranches.size());

  patchIncomingBranches(a, astubs, dbgGuard);

  // Set m_dbgBranchGuardSrc after patching, so we don't try to patch
  // the debug guard.
  m_dbgBranchGuardSrc = dbgBranchGuardSrc;
  atomic_release_store(&m_topTranslation, dbgGuard);
}

void SrcRec::patchIncomingBranches(Asm& a, Asm &astubs, TCA newStart) {
  if (hasDebuggerGuard()) {
    // We have a debugger guard, so all jumps to us funnel through
    // this.  Just smash m_dbgBranchGuardSrc.
    TRACE(1, "smashing m_dbgBranchGuardSrc @%p\n", m_dbgBranchGuardSrc);
    TranslatorX64::smashJmp(a, m_dbgBranchGuardSrc, newStart);
    return;
  }

  TRACE(1, "%zd incoming branches to rechain\n", m_incomingBranches.size());

  vector<IncomingBranch>& change = m_incomingBranches;
  for (unsigned i = 0; i < change.size(); ++i) {
    TRACE(1, "SrcRec(%p)::newTranslation rechaining @%p -> %p\n",
          this, change[i].m_src, newStart);
    Asm *as = change[i].m_type == IncomingBranch::ADDR ?
      NULL : &asmChoose(change[i].m_src, a, astubs);
    patch(as, change[i], newStart);
  }
}

void SrcRec::replaceOldTranslations(Asm& a, Asm& astubs) {
  // Everyone needs to give up on old translations; send them to the anchor,
  // which is a REQ_RETRANSLATE
  m_translations.clear();
  m_tailFallbackJumps.clear();
  atomic_release_store(&m_topTranslation, static_cast<TCA>(0));
  patchIncomingBranches(a, astubs, m_anchorTranslation);
}

void SrcRec::patch(Asm* a, IncomingBranch branch, TCA dest) {
  if (branch.m_type == IncomingBranch::ADDR) {
    // Note that this effectively ignores a
    atomic_release_store(branch.m_addr, dest);
    return;
  }

  // modifying reachable code
  switch(branch.m_type) {
    case IncomingBranch::JMP: {
      CodeCursor cg(*a, branch.m_src);
      TranslatorX64::smashJmp(*a, branch.m_src, dest);
      break;
    }
    case IncomingBranch::JCC: {
      // patch destination, but preserve the condition code
      int32_t delta = safe_cast<int32_t>((dest - branch.m_src) -
                                         TranslatorX64::kJmpccLen);
      int32_t* addr = (int32_t*)(branch.m_src + TranslatorX64::kJmpccLen - 4);
      atomic_release_store(addr, delta);
      break;
    }
    default:
      not_implemented();
  }
}

void SrcDB::recordDependencyWork(const Eval::PhpFile* file, const SrcKey& sk) {
  if (RuntimeOption::RepoAuthoritative) return;
  ASSERT(Translator::WriteLease().amOwner());
  std::pair<FileDepMap::iterator, bool> insRet =
    m_deps.insert(FileDepMap::value_type(file, NULL));
  if (insRet.second) {
    insRet.first->second = new GrowableVector<SrcKey>();
  }
  TRACE(1, "SrcDB::recordDependencyWork: file %p\n", file);
  insRet.first->second = insRet.first->second->push_back(sk);
}

/*
 * Returns number of destroyed references to file.
 */
size_t SrcDB::invalidateCode(const Eval::PhpFile* file) {
  ASSERT(currentRank() == RankBase);
  /*
   * Hold the write lease; otherwise some other thread may have started
   * translating this very file.
   */
  BlockingLeaseHolder writer(Translator::WriteLease());

  ASSERT(!RuntimeOption::RepoAuthoritative);
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

} } } // HPHP::VM::Transl
