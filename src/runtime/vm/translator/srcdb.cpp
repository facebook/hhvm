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

// The current fallback translation is the translation we first jump to if the
//   current translation's type checks fail. At this writing, this will be the previous
//   most recently created translation (as the current translation is being
//   constructed, and thus is not in translations[] yet). We may ultimately
//   revisit this with various heuristics going forward.
TCA SrcRec::getFallbackTranslation() const {
  ASSERT(translations.size() > 0);
  return translations[translations.size() - 1];
}

// The top translation is our first target, a translation whose type
//   checks properly chain through all other translations. Usually this will
//   be the most recently created translation.
TCA SrcRec::getTopTranslation() const {
  return getFallbackTranslation();
}

void SrcRec::chainFrom(Asm& a, IncomingBranch::BranchType type, TCA src) {
  TCA destAddr = getTopTranslation();
  if (src == NULL) {
    src = a.code.frontier;
  }
  IncomingBranch incoming;
  incoming.type = type;
  incoming.src = src;
  incomingBranches.push_back(incoming);
  TRACE(1, "SrcRec(%p)::chainFrom %p -> %p; %zd incoming branches\n",
        this,
        a.code.frontier, destAddr, incomingBranches.size());
  patch(a, incoming, destAddr);
}

void SrcRec::emitFallbackJump(Asm &a, IncomingBranch::BranchType type) {
  TCA destAddr = getFallbackTranslation();

  IncomingBranch incoming;
  incoming.type = type;
  incoming.src = a.code.frontier;

  // emit dummy jump to be smashed via patch()
  if (type == IncomingBranch::JMP) {
    a.jmp(a.code.frontier);
  } else {
    a.jnz(a.code.frontier);
  }

  patch(a, incoming, destAddr);
}

void SrcRec::newTranslation(Asm& a, TCA newStart)
{
  ASSERT(translations.size() <= kMaxTranslations);

  translations.push_back(newStart);
  TRACE(1, "SrcRec(%p)::newTranslation @%p, "
        "%zd incoming branches to rechain\n",
        this, newStart, incomingBranches.size());

  // We have a new tracelet.
  //
  // Rewrite all jumps to this source key to point to the new tracelet.
  // Hopefully the new tracelet already contains jumps to the previously
  // newest tracelet if a type check fails.
  vector<IncomingBranch> &change = incomingBranches;

  for (unsigned i = 0; i < change.size(); ++i) {
    TRACE(1, "SrcRec(%p)::newTranslation rechaining @%p -> %p\n",
          this, change[i].src, newStart);
    patch(a, change[i], newStart);
  }
}

void SrcRec::patch(Asm& a, IncomingBranch branch, TCA dest)
{
  CodeCursor cg(a, branch.src);

  // modifying reachable code
  switch(branch.type) {
  case IncomingBranch::JMP:
    TranslatorX64::smash(a, branch.src, dest);
    break;
  case IncomingBranch::JZ:
    ASSERT(TranslatorX64::isSmashable(a,
                                      TranslatorX64::kJmpccLen));
    a.jz(dest);
    break;
  case IncomingBranch::JNZ:
    ASSERT(TranslatorX64::isSmashable(a,
                                      TranslatorX64::kJmpccLen));
    a.jnz(dest);
    break;
  default:
    not_implemented();
  }
}

} } } // HPHP::VM::Transl
