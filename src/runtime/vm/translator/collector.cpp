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

#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <string>
#include <queue>

#include "util/trace.h"
#include "util/debug.h"
#include "runtime/eval/runtime/file_repository.h"
#include "system/lib/systemlib.h"
#include "runtime/vm/treadmill.h"
#include "runtime/vm/translator/translator-deps.h"
#include "runtime/vm/translator/translator-inline.h"
#include "runtime/vm/translator/translator-x64.h"

namespace HPHP {
namespace VM {
namespace Transl {

/*
 * Code related to reclaiming translation spaces.
 */

static const Trace::Module TRACEMOD = Trace::tx64;

/*
 * Visitor that makes a best effort at touching all live Func*'s at the
 * time it is invoked. Caveat invocator: new funcs may come and go while
 * this runs.
 */
struct FuncVisitor : public UnitVisitor {
  int m_numFuncs;
  FuncVisitor() : m_numFuncs(0) { }

  virtual void visit(Func*) = 0;
  virtual void operator()(Unit* u) {
    for (MutableAllFuncs fr(u); !fr.empty();) {
      fr.front()->validate();
      m_numFuncs++;
      visit(fr.popFront());
    }
  }
  int go() {
    Eval::FileRepository::forEachUnit(*this); // user-defined
    (*this)(SystemLib::s_nativeFuncUnit); // native
    // Now all the classes. AllFuncs above only visited the Funcs hanging
    // off of preclasses.
    for (AllClasses ac; !ac.empty();) {
      for (Class::MutableMethodRange fr(ac.popFront()->mutableMethodRange());
           !fr.empty();) {
        visit(fr.popFront());
      }
    }
    return m_numFuncs;
  }
};

void verifyFuncReferencesSevered() {
  if (!debug) return;
  struct NoLivePrologues : FuncVisitor {
    void visit(Func* f) {
      for (int i = 0; i < f->numPrologues(); ++i) {
        DEBUG_ONLY TCA pro = (TCA)f->getPrologue(i);
        ASSERT(pro == (TCA)fcallHelperThunk);
      }
    }
  } verify;
  verify.go();
}

void severFuncReferences(Translator* tx) {
  DEBUG_ONLY time_t start = time(NULL);
  struct FuncPatcher : public FuncVisitor {
    FuncPatcher() { }
    void visit(Func* f) {
      TRACE(3, "Tx64: resetting func %s %p\n", f->fullName()->data(), f);
      f->resetPrologues();
    }
  } fpatch;
  DEBUG_ONLY int n = fpatch.go();
  TRACE(0, "Tx64: reset done: %d funcs %ld seconds\n", n, time(NULL) - start);
  verifyFuncReferencesSevered();
}

/*
 * This global flag prevents more than one replacement from happening at a
 * time, and prevents us from burning TCAs into external structures (e.g.,
 * Func's prologue tables) during periods where we can't resolve which TC to
 * use.
 */
volatile bool Translator::s_replaceInFlight;

bool TranslatorX64::replace() {
  if (!RuntimeOption::EvalJit) return false;

  // Only one replace can occur at a time. Replace-related data is
  // protected under the write lease.
  BlockingLeaseHolder lease(Translator::WriteLease());
  if (s_replaceInFlight) return false;

  // Maybe a replacement started and ended while we waited for the lock.
  if (this != nextTx64) return false;
  // Exclude the creation of new prologue entries while we clean up
  // old prologues; these are external references to the old code.

  s_replaceInFlight = true;
  severFuncReferences(tx64);
  TranslatorX64* n00b = new TranslatorX64();
  n00b->initGdb();
  // m_interceptsEnabled should persist across TC spaces.
  n00b->m_interceptsEnabled = m_interceptsEnabled;
  TRACE(0, "Tx64: replace %p a.code %p -> %p a.code %p complete\n",
        this, a.code.base, n00b, n00b->a.code.base);
  // Here is the changing of the guard.
  nextTx64 = n00b;
  // TxReaper: runs after a translation space becomes unreachable.
  struct TxReaper : public Treadmill::WorkItem {
    Translator* m_tx;
    TxReaper(Translator* tx) : m_tx(tx) { }
    void operator()() {
      TRACE(1, "Tx: reaping tx at %p\n", m_tx);
      ASSERT(Translator::ReplaceInFlight());
      // No prologues, old or new, should be active at treadmill
      // time.
      verifyFuncReferencesSevered();
      // Func*'s may have references directly into the old TC. The new one is
      // visble now, so there may also be new
      s_replaceInFlight = false;
      delete m_tx;
    }
  };
  // No new requests can reach us; once no old ones persist, we can tear down
  // the old translator.
  Treadmill::WorkItem::enqueue(new TxReaper(this));
  return true;
}

struct Tx64Annihilator {
  ~Tx64Annihilator() {
    if (nextTx64) {
      TRACE(2, "Goodbye, cruel world: last nextTx64 %p\n", nextTx64);
      delete nextTx64;
    }
  }
};

static Tx64Annihilator boom;

} } } // HPHP::VM::Transl
