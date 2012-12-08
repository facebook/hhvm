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
#ifndef incl_TRANSLATOR_X64_INTERNAL_H_
#define incl_TRANSLATOR_X64_INTERNAL_H_

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include <runtime/vm/translator/abi-x64.h>

/*
 * Please don't include this unless your file implements methods of
 * TranslatorX64; you won't like it. It pollutes the namespace, makes
 * "KindOfString" #error, makes your TRACEMOD tx64, and tortures a kitten.
 */

using namespace HPHP::VM::Transl::reg;
using namespace HPHP::Util;
using namespace HPHP::Trace;
using std::max;

namespace HPHP {
namespace VM {
namespace Transl {

static const Trace::Module TRACEMOD = Trace::tx64;
static const DataType BitwiseKindOfString = KindOfString;

#define KindOfString \
#error You probably do not mean to use KindOfString in this file.

// RAII aids to machine code.

// In shared stubs, we've already made the stack odd by calling
// from a to astubs. Calls from a are on an even rsp.
typedef PhysRegSaverParity<0> PhysRegSaverStub;
typedef PhysRegSaverParity<1> PhysRegSaver;

// Put FreezeRegs in a scope around any emit calls where the caller needs
// to be sure the callee will not modify the state of the register
// map.  (Arises in situations with conditional code often.)
struct FreezeRegs : private boost::noncopyable {
  explicit FreezeRegs(RegAlloc& regs) : m_regs(regs) { m_regs.freeze(); }
  ~FreezeRegs() { m_regs.defrost(); }

private:
  RegAlloc& m_regs;
};

class RedirectSpillFill : boost::noncopyable {
  X64Assembler* const m_oldSpf;
public:
  explicit RedirectSpillFill(X64Assembler* newCode)
    : m_oldSpf(tx64->m_spillFillCode)
  {
    tx64->m_spillFillCode = newCode;
  }
  ~RedirectSpillFill() {
    tx64->m_spillFillCode = m_oldSpf;
  }
};

struct Call {
  enum { Direct, Virtual } m_kind;
  union {
    void* m_fptr;
    int   m_offset;
  };

  explicit Call(void *p) : m_kind(Direct), m_fptr(p) {}
  explicit Call(int off) : m_kind(Virtual), m_offset(off) {}
  Call(Call const&) = default;

  void emit(X64Assembler& a, PhysReg scratch) const {
    if (m_kind == Direct) {
      a.    call (TCA(m_fptr));
    } else {
      a.    loadq  (*rdi, scratch);
      a.    call   (scratch[m_offset]);
    }
  }
};

// DiamondGuard is a scoped way to protect register allocator state around
// control flow. When we enter some optional code that may affect the state
// of the register file, we copy the register file's state, and redirect any
// spills and fills to the branch's body.
//
// When we're ready to rejoin the main control flow, we bring the registers
// back into the state they were in, and restore the old spill/fill
// destinations.
class DiamondGuard : boost::noncopyable {
  RedirectSpillFill m_spfChanger;
public:
  explicit DiamondGuard(X64Assembler& a) : m_spfChanger(&a) {
    tx64->m_savedRegMaps.push(
      TranslatorX64::SavedRegState(this, tx64->m_regMap));
  }
  ~DiamondGuard() {
    ASSERT(!tx64->m_savedRegMaps.empty());
    ASSERT(tx64->m_savedRegMaps.top().saver == this);

    // Bring the register state back to its state in the main body.
    //
    // Note: it's important that tx64->m_regMap is the branch
    // RegAlloc during this.  See RegAlloc::reconcile.
    tx64->m_savedRegMaps.top().savedState.reconcile(tx64->m_regMap);
    tx64->m_regMap = tx64->m_savedRegMaps.top().savedState;
    tx64->m_savedRegMaps.pop();
  }
};

// Helper for use with UnlikelyIfBlock when you have a complex else
// branch that needs to make changes to the register file.
//
// For an example usage see UnlikelyIfBlock.
class DiamondReturn : boost::noncopyable {
  X64Assembler* m_branchA;
  X64Assembler* m_mainA;
  TCA m_branchJmp;
  TCA m_finishBranchFrontier;

private:
  friend class UnlikelyIfBlock;

  void initBranch(X64Assembler* branchA, X64Assembler* mainA) {
    /*
     * DiamondReturn must be used with branches going to different
     * code regions.
     */
    ASSERT(branchA != mainA);

    m_branchA = branchA;
    m_mainA = mainA;
    tx64->m_savedRegMaps.push(
      TranslatorX64::SavedRegState(this, tx64->m_regMap));
  }

  void finishBranch(TCA jmp, TCA frontier) {
    m_branchJmp = jmp;
    m_finishBranchFrontier = frontier;

    // If there's some reason to do something other than this we have
    // to change the way this class works.
    const int UNUSED kJumpSize = 5;
    ASSERT(m_finishBranchFrontier == m_branchJmp + kJumpSize);

    // We're done with the branch, so save the branch's state and
    // switch back to the main line's state.
    swapRegMaps();
  }

  bool finishedBranch() const { return m_branchJmp != 0; }

  void swapRegMaps() {
    ASSERT(!tx64->m_savedRegMaps.empty());
    ASSERT(tx64->m_savedRegMaps.top().saver == this);
    std::swap(tx64->m_savedRegMaps.top().savedState, tx64->m_regMap);
  }

  void emitReconciliation() {
    ASSERT(!tx64->m_savedRegMaps.empty());
    ASSERT(tx64->m_savedRegMaps.top().saver == this);

    RedirectSpillFill spfRedir(m_branchA);

    if (finishedBranch()) {
      // We need tx64->m_regMap to point at the branch during
      // reconciliation.  See RegAlloc::reconcile().
      swapRegMaps();
    }
    RegAlloc& branchState = tx64->m_regMap;
    RegAlloc& currentState = tx64->m_savedRegMaps.top().savedState;
    currentState.reconcile(branchState);
    tx64->m_regMap = currentState;
    tx64->m_savedRegMaps.pop();
  }

public:
  explicit DiamondReturn()
    : m_branchA(0)
    , m_branchJmp(0)
  {}

  void kill() {
    m_mainA = NULL;
  }

  ~DiamondReturn() {
    ASSERT(m_branchA &&
      "DiamondReturn was created without being passed to UnlikelyIfBlock");

    if (!m_mainA) {
      /*
       * We were killed. eg the UnlikelyIfBlock took a side exit, so
       * no reconciliation/branch back to a is required.
       */
      return;
    }

    if (!finishedBranch()) {
      /*
       * In this case, we're reconciling the branch even though it
       * isn't really finished (no one ever called finishBranch()), so
       * we just need to emit spills/fills now and not be as clever as
       * below.  See UnlikelyIfBlock::reconcileEarly.
       */
      emitReconciliation();
      return;
    }

    const TCA currentBranchFrontier = m_branchA->code.frontier;
    const bool branchFrontierMoved =
      currentBranchFrontier != m_finishBranchFrontier;

    /*
     * If the branch frontier hasn't moved since the branch was
     * finished, we don't need the jmp that was already emitted
     * anymore, so rewind so we can potentially overwrite it with
     * spills/fills.
     */
    if (!branchFrontierMoved) {
      m_branchA->code.frontier = m_branchJmp;
    }

    // Send out reconciliation code to the branch area.  We want to
    // bring the state of the branch's register file into sync with
    // the main-line.
    const TCA spfStart = m_branchA->code.frontier;
    emitReconciliation();
    const bool hadAnySpf = spfStart != m_branchA->code.frontier;

    if (branchFrontierMoved) {
      /*
       * In this case, more than one DiamondReturn is being used and
       * there are multiple unlikely branches.
       *
       * If there was no reconciliation code it's not big deal, we'll
       * just patch the existing jmp to go to the return in m_mainA.
       * But if we needed reconciliation code, we'll instead want to
       * patch it to jump there.
       */
      if (hadAnySpf) {
        m_branchA->patchJmp(m_branchJmp, spfStart);
        m_branchA->jmp(m_mainA->code.frontier);
      } else {
        m_branchA->patchJmp(m_branchJmp, m_mainA->code.frontier);
      }
    } else {
      ASSERT(spfStart == m_branchJmp);
      m_branchA->jmp(m_mainA->code.frontier);
    }
  }
};

// Code to profile how often our UnlikelyIfBlock branches are taken in
// practice. Enable with TRACE=unlikely:1
struct JmpHitRate {
  litstr key;
  uint64_t check;
  uint64_t take;
  JmpHitRate() : key(nullptr), check(0), take(0) {}

  float rate() const {
    return 100.0 * take / check;
  }
  bool operator<(const JmpHitRate& b) const {
    return rate() > b.rate();
  }
};
typedef hphp_hash_map<litstr, JmpHitRate, pointer_hash<const char>> JmpHitMap;
extern __thread JmpHitMap* tl_unlikelyHits;
extern __thread JmpHitMap* tl_jccHits;

template<Trace::Module mod>
static void recordJmpProfile(litstr key, int64 take) {
  JmpHitMap& map = mod == Trace::unlikely ? *tl_unlikelyHits : *tl_jccHits;
  JmpHitRate& r = map[key];
  r.key = key;
  r.check++;
  if (take) r.take++;
}

template<Trace::Module mod>
void emitJmpProfile(X64Assembler& a, ConditionCode cc) {
  if (!Trace::moduleEnabledRelease(mod)) return;
  const ssize_t sz = 1024;
  char key[sz];

  // Clean up filename
  std::string file =
    boost::filesystem::path(tx64->m_curFile).filename().string();

  // Get instruction if wanted
  const NormalizedInstruction* ni = tx64->m_curNI;
  std::string inst;
  if (Trace::moduleEnabledRelease(mod, 2)) {
    inst = std::string(", ") + (ni ? opcodeToName(ni->op()) : "<none>");
  }
  const char* fmt = Trace::moduleEnabledRelease(mod, 3) ?
    "%-25s:%-5d, %-28s%s" :
    "%-25s:%-5d (%-28s%s)";
  if (snprintf(key, sz, fmt,
               file.c_str(), tx64->m_curLine, tx64->m_curFunc,
               inst.c_str()) >= sz) {
    key[sz-1] = '\0';
  }
  litstr data = StringData::GetStaticString(key)->data();

  RegSet allRegs = kAllX64Regs;
  allRegs.remove(rsi);

  a.  pushf  ();
  a.  push   (rsi);
  a.  setcc  (cc, sil);
  a.  movzbl (sil, esi);
  {
    PhysRegSaver regs(a, allRegs);
    a.emitImmReg((intptr_t)data, rdi);
    if (false) {
      recordJmpProfile<mod>("", 0);
    }
    a.call   ((TCA)recordJmpProfile<mod>);
  }
  a.  pop    (rsi);
  a.  popf   ();
}

inline void initJmpProfile() {
  if (Trace::moduleEnabledRelease(Trace::unlikely)) {
    tl_unlikelyHits = new JmpHitMap();
  }
  if (Trace::moduleEnabledRelease(Trace::jcc)) {
    tl_jccHits = new JmpHitMap();
  }
}

inline void dumpProfileImpl(Trace::Module mod) {
  JmpHitMap*& table = mod == Trace::jcc ? tl_jccHits : tl_unlikelyHits;
  if (!table) return;

  std::vector<JmpHitRate> hits;
  JmpHitRate overall;
  overall.key = "total";
  for (auto& item : *table) {
    overall.check += item.second.check;
    overall.take += item.second.take;
    hits.push_back(item.second);
  }
  if (hits.empty()) return;
  auto cmp = [&](const JmpHitRate& a, const JmpHitRate& b) {
    return a.take > b.take ? true
    : a.take == b.take ? a.check > b.check
    : false;
  };
  std::sort(hits.begin(), hits.end(), cmp);
  Trace::traceRelease("%s hit rates for %s:\n",
                      mod == Trace::jcc ? "JccBlock" : "UnlikelyIfBlock",
                      g_context->getRequestUrl(50).c_str());
  const char* fmt = Trace::moduleEnabledRelease(mod, 3) ?
    "%6.2f, %8llu, %8llu, %5.1f, %s\n" :
    "%6.2f%% (%8llu / %8llu, %5.1f%% of total): %s\n";
  auto printRate = [&](const JmpHitRate& hr) {
    Trace::traceRelease(fmt,
                        hr.rate(), hr.take, hr.check, hr.key,
                        100.0 * hr.take / overall.take);
  };
  printRate(overall);
  std::for_each(hits.begin(), hits.end(), printRate);
  Trace::traceRelease("\n");

  delete table;
  table = nullptr;
}

inline void dumpJmpProfile() {
  if (!Trace::moduleEnabledRelease(Trace::unlikely) &&
      !Trace::moduleEnabledRelease(Trace::jcc)) {
    return;
  }
  dumpProfileImpl(Trace::unlikely);
  dumpProfileImpl(Trace::jcc);
}

struct Label : private boost::noncopyable {
  explicit Label()
    : m_a(nullptr)
    , m_address(nullptr)
  {}

  ~Label() {
    if (!m_toPatch.empty()) {
      ASSERT(m_a && m_address && "Label had jumps but was never set");
    }
    for (auto& ji : m_toPatch) {
      switch (ji.type) {
      case Branch::Jmp:   ji.a->patchJmp(ji.addr, m_address);  break;
      case Branch::Jmp8:  ji.a->patchJmp8(ji.addr, m_address); break;
      case Branch::Jcc:   ji.a->patchJcc(ji.addr, m_address);  break;
      case Branch::Jcc8:  ji.a->patchJcc8(ji.addr, m_address); break;
      case Branch::Call:  ji.a->patchCall(ji.addr, m_address); break;
      }
    }
  }

  void jmp(X64Assembler& a) {
    addJump(&a, Branch::Jmp);
    a.jmp(m_address ? m_address : a.code.frontier);
  }

  void jmp8(X64Assembler& a) {
    addJump(&a, Branch::Jmp8);
    a.jmp8(m_address ? m_address : a.code.frontier);
  }

  void jcc(X64Assembler& a, ConditionCode cc) {
    addJump(&a, Branch::Jcc);
    a.jcc(cc, m_address ? m_address : a.code.frontier);
  }

  void jcc8(X64Assembler& a, ConditionCode cc) {
    addJump(&a, Branch::Jcc8);
    a.jcc8(cc, m_address ? m_address : a.code.frontier);
  }

  void call(X64Assembler& a) {
    addJump(&a, Branch::Call);
    a.call(m_address ? m_address : a.code.frontier);
  }

  void jmpAuto(X64Assembler& a) {
    ASSERT(m_address);
    auto delta = m_address - (a.code.frontier + 2);
    if (deltaFits(delta, sz::byte)) {
      jmp8(a);
    } else {
      jmp(a);
    }
  }

  void jccAuto(X64Assembler& a, ConditionCode cc) {
    ASSERT(m_address);
    auto delta = m_address - (a.code.frontier + 2);
    if (deltaFits(delta, sz::byte)) {
      jcc8(a, cc);
    } else {
      jcc(a, cc);
    }
  }

  friend void asm_label(X64Assembler& a, Label& l) {
    ASSERT(!l.m_address && !l.m_a && "Label was already set");
    l.m_a = &a;
    l.m_address = a.code.frontier;
  }

private:
  enum class Branch {
    Jcc,
    Jcc8,
    Jmp,
    Jmp8,
    Call
  };

  struct JumpInfo {
    Branch type;
    X64Assembler* a;
    TCA addr;
  };

private:
  void addJump(X64Assembler* a, Branch type) {
    if (m_address) return;
    JumpInfo info;
    info.type = type;
    info.a = a;
    info.addr = a->code.frontier;
    m_toPatch.push_back(info);
  }

private:
  X64Assembler* m_a;
  TCA m_address;
  std::vector<JumpInfo> m_toPatch;
};

// UnlikelyIfBlock:
//
//  Branch to distant code (that we presumably don't expect to
//  take). This helps keep hot paths compact.
//
//  A common pattern using this involves patching the jump in astubs
//  to jump past the normal control flow in a (as in the following
//  example).  Do this using DiamondReturn so the register allocator
//  state will be properly maintained.  (Spills/fills to keep the
//  states in sync will be emitted on the unlikely path.)
//
// Example:
//
//  {
//    PhysReg inputParam = i.getReg(i.inputs[0]->location);
//    a.   test_reg_reg(inputParam, inputParam);
//    DiamondReturn retFromStubs;
//    {
//      UnlikelyIfBlock ifNotRax(CC_Z, a, astubs, &retFromStubs);
//      EMIT_CALL(a, TCA(launch_nuclear_missiles));
//    }
//    // The inputParam was non-zero, here is the likely branch:
//    m_regMap.allocOutputRegs(i);
//    emitMovRegReg(inputParam, m_regMap.getReg(i.outLocal->location));
//    // ~DiamondReturn patches the jump, and reconciles the branch
//    // with the main line.  (In this case it will fill the outLocal
//    // register since the main line thinks it is dirty.)
//  }
//  // The two cases are joined here.  We can do logic that was
//  // independent of whether the branch was taken, if necessary.
//  emitMovRegReg(i.outLocal, m_regMap.getReg(i.outStack->location));
//
// Note: it is ok to nest UnlikelyIfBlocks, as long as their
// corresponding DiamondReturns are correctly destroyed in reverse
// order.  But also note that this can lead to more jumps on the
// unlikely branch (see ~DiamondReturn).
struct UnlikelyIfBlock {
  X64Assembler& m_likely;
  X64Assembler& m_unlikely;
  TCA m_likelyPostBranch;

  DiamondReturn* m_returnDiamond;
  bool m_externalDiamond;
  boost::optional<FreezeRegs> m_ice;

  explicit UnlikelyIfBlock(ConditionCode cc,
                           X64Assembler& likely,
                           X64Assembler& unlikely,
                           DiamondReturn* returnDiamond = 0)
    : m_likely(likely)
    , m_unlikely(unlikely)
    , m_returnDiamond(returnDiamond ? returnDiamond : new DiamondReturn())
    , m_externalDiamond(!!returnDiamond)
  {
    emitJmpProfile<Trace::unlikely>(m_likely, cc);
    m_likely.jcc(cc, m_unlikely.code.frontier);
    m_likelyPostBranch = m_likely.code.frontier;
    m_returnDiamond->initBranch(&unlikely, &likely);
    tx64->m_spillFillCode = &unlikely;
  }

  ~UnlikelyIfBlock() {
    TCA jmpAddr = m_unlikely.code.frontier;
    m_unlikely.jmp(m_likelyPostBranch);
    if (m_returnDiamond) {
      m_returnDiamond->finishBranch(jmpAddr, m_unlikely.code.frontier);
      if (!m_externalDiamond) {
        delete m_returnDiamond;
      }
    }
    tx64->m_spillFillCode = &m_likely;
  }

  /*
   * Force early reconciliation between the branch and main line.
   * Using this has some tricky cases, part of which is that you can't
   * allocate registers anymore in the branch if you do this (so we'll
   * freeze until ~UnlikelyIfBlock).
   *
   * It's also almost certainly error if we have m_externalDiamond, so
   * for now that's an assert.
   */
  void reconcileEarly() {
    ASSERT(!m_externalDiamond);
    delete m_returnDiamond;
    m_returnDiamond = 0;
    m_ice = boost::in_place<FreezeRegs>(boost::ref(tx64->m_regMap));
  }
};

#define UnlikelyIfBlock                                                 \
  m_curFile = __FILE__; m_curFunc = __FUNCTION__; m_curLine = __LINE__; \
  UnlikelyIfBlock

// Helper structs for jcc vs. jcc8.
struct Jcc8 {
  static void branch(X64Assembler& a, ConditionCode cc, TCA dest) {
    a.   jcc8(cc, dest);
  }
  static void patch(X64Assembler& a, TCA site, TCA newDest) {
    a.patchJcc8(site, newDest);
  }
};

struct Jcc32 {
  static void branch(X64Assembler& a, ConditionCode cc, TCA dest) {
    a.   jcc(cc, dest);
  }
  static void patch(X64Assembler& a, TCA site, TCA newDest) {
    a.patchJcc(site, newDest);
  }
};

// JccBlock --
//   A raw condition-code block; assumes whatever comparison or ALU op
//   that sets the Jcc has already executed.
template <ConditionCode Jcc, typename J=Jcc8>
struct JccBlock {
  mutable X64Assembler* m_a;
  TCA m_jcc;
  mutable DiamondGuard* m_dg;

  explicit JccBlock(X64Assembler& a)
    : m_a(&a),
      m_dg(new DiamondGuard(a)) {
    emitJmpProfile<Trace::jcc>(a, Jcc);
    m_jcc = a.code.frontier;
    J::branch(a, Jcc, m_a->code.frontier);
  }

  ~JccBlock() {
    if (m_a) {
      delete m_dg;
      J::patch(*m_a, m_jcc, m_a->code.frontier);
    }
  }

private:
  JccBlock(const JccBlock&);
  JccBlock& operator=(const JccBlock&);
};

#define JccBlock                                                        \
  m_curFile = __FILE__; m_curFunc = __FUNCTION__; m_curLine = __LINE__; \
  JccBlock

template<class Lambda>
void guardDiamond(X64Assembler& a, Lambda body) {
  DiamondGuard dg(a);
  body();
}

template<ConditionCode Jcc, class Lambda>
void jccBlock(X64Assembler& a, Lambda body) {
  Label exit;

  guardDiamond(a, [&] {
    exit.jcc8(a, Jcc);
    body();
  });
asm_label(a, exit);
}

/*
 * semiLikelyIfBlock is a conditional block of code that is expected
 * to be unlikely, but not so unlikely that we should shove it into
 * astubs.
 *
 * Usage example:
 *
 * a. test_reg64_reg64(*rFoo, *rFoo);
 * semiLikelyIfBlock(CC_Z, a, [&]{
 *   EMIT_CALL(a, some_helper);
 *   emitMovRegReg(a, rax, *rFoo);
 * });
 */
template<class Lambda>
void semiLikelyIfBlock(ConditionCode Jcc, X64Assembler& a, Lambda body) {
  Label likely;
  Label unlikely;

  guardDiamond(a, [&] {
    unlikely.jcc8(a, Jcc);
    likely.jmp8(a);
  asm_label(a, unlikely);
    body();
  });
asm_label(a, likely);
}

// A CondBlock is an RAII structure for emitting conditional code. It
// compares the source register at fieldOffset with fieldValue, and
// conditionally branches over the enclosing block of assembly on the
// passed-in condition-code.
//
//  E.g.:
//    {
//      RefCountedOnly ifRefCounted(a, rdi, 0);
//      emitIncRef(rdi);
//    }
//
// will only execute emitIncRef if we find at runtime that rdi points at
// a ref-counted cell.
//
// It's ok to do reconcilable register operations in the body.
template<int FieldOffset, int FieldValue, ConditionCode Jcc>
struct CondBlock {
  X64Assembler& m_a;
  int m_off;
  TCA m_jcc8;
  DiamondGuard* m_dg;

  CondBlock(X64Assembler& a, PhysReg reg, int offset = 0)
      : m_a(a), m_off(offset), m_dg(new DiamondGuard(a)) {
    int typeDisp = m_off + FieldOffset;
    a.   cmp_imm32_disp_reg32(FieldValue, typeDisp, reg);
    m_jcc8 = a.code.frontier;
    a.   jcc8(Jcc, m_jcc8);
    // ...
  }

  ~CondBlock() {
    delete m_dg;
    m_a.patchJcc8(m_jcc8, m_a.code.frontier);
  }
};

// IfRefCounted --
//   Emits if (IS_REFCOUNTED_TYPE()) { ... }
typedef CondBlock <TVOFF(m_type),
                   KindOfRefCountThreshold,
                   CC_LE> IfRefCounted;

typedef CondBlock <TVOFF(m_type),
                   KindOfRef,
                   CC_NZ> IfVariant;

typedef CondBlock <TVOFF(m_type),
                   KindOfUninit,
                   CC_Z> UnlessUninit;

/*
 * locToRegDisp --
 *
 * Helper code for stack frames. The struct is a "template" in the
 * non-C++ sense: we don't build source-level stack frames in C++
 * for the most part, but its offsets tell us where to find fields
 * in assembly.
 *
 * If we were physically pushing stack frames, we would push them
 * in reverse order to what you see here.
 */
static void
locToRegDisp(const Location& l, PhysReg *outbase, int *outdisp,
             const Func* f = NULL) {
  assert_not_implemented((l.space == Location::Stack ||
                          l.space == Location::Local ||
                          l.space == Location::Iter));
  *outdisp = cellsToBytes(Translator::locPhysicalOffset(l, f));
  *outbase = l.space == Location::Stack ? rVmSp : rVmFp;
}

// Common code emission patterns.

// emitStoreImm --
//   Try to use a nice encoding for the size and value.
static void
emitStoreImm(X64Assembler& a, uint64_t imm, PhysReg r, int off,
             int size = sz::qword, RegAlloc* regAlloc = NULL) {
  if (size == sz::qword) {
    PhysReg immReg = regAlloc ? regAlloc->getImmReg(imm) : InvalidReg;
    if (immReg == InvalidReg) {
      if (deltaFits(imm, sz::dword)) {
        a. store_imm64_disp_reg64(imm, off, r);
        return;
      }
      emitImmReg(a, imm, rScratch);
      immReg = rScratch;
    }
    a.   store_reg64_disp_reg64(immReg, off, r);
  } else if (size == sz::dword) {
    a.   store_imm32_disp_reg(imm, off, r);
  } else {
    not_implemented();
  }
}

// vstackOffset --
// emitVStackStore --
//
//   Store to the virtual stack at a normalized instruction boundary.
//   Since rVmSp points to the stack at entry to the current BB, we need to
//   adjust stack references relative to it.
//
//   For all parameters, offsets and sizes are in bytes. Sizes are expected
//   to be a hardware size: 1, 2, 4, or 8 bytes.
static inline int
vstackOffset(const NormalizedInstruction& ni, COff off) {
  return off - cellsToBytes(ni.stackOff);
}

static inline void
emitVStackStoreImm(X64Assembler &a, const NormalizedInstruction &ni,
                   uint64_t imm, int off, int size = sz::qword,
                   RegAlloc *regAlloc = NULL) {
  int hwOff = vstackOffset(ni, off);
  emitStoreImm(a, imm, rVmSp, hwOff, size, regAlloc);
}

static inline void
emitVStackStore(X64Assembler &a, const NormalizedInstruction &ni,
                PhysReg src, int off, int size = sz::qword) {
  int hwOff = vstackOffset(ni, off);
  if (size == sz::qword) {
    a.    store_reg64_disp_reg64(src, hwOff, rVmSp);
  } else if (size == sz::dword) {
    a.    store_reg32_disp_reg64(src, hwOff, rVmSp);
  } else {
    not_implemented();
  }
}

static inline const StringData*
local_name(const Location& l) {
  ASSERT(l.isLocal());
  const StringData* ret = curFunc()->localNames()[l.offset];
  ASSERT(ret->isStatic());
  return ret;
}

// emitDispDeref --
// emitDeref --
// emitStoreTypedValue --
// emitStoreUninitNull --
// emitStoreNull --
//
//   Helpers for common cell operations.
//
//   Dereference the var in the cell whose address lives in src into
//   dest.
static void
emitDispDeref(X64Assembler &a, Reg64 src, int disp, Reg64 dest) {
  a.    loadq (src[disp + TVOFF(m_data)], dest);
}

static void
emitDeref(X64Assembler &a, PhysReg src, PhysReg dest) {
  emitDispDeref(a, src, 0, dest);
}

static void
emitDerefIfVariant(X64Assembler &a, PhysReg reg) {
  if (RuntimeOption::EvalJitCmovVarDeref) {
    a.cmp_imm32_disp_reg32(KindOfRef, TVOFF(m_type), reg);
    a.cload_reg64_disp_reg64(CC_Z, reg, TVOFF(m_data), reg);
  } else {
    IfVariant ifVar(a, reg);
    emitDeref(a, reg, reg);
  }
}

// NB: leaves count field unmodified. Does not store to m_data if type
// is a null type.
static inline void
emitStoreTypedValue(X64Assembler& a, DataType type, PhysReg val,
                    int disp, PhysReg dest, bool writeType = true) {
  if (writeType) {
    a.  store_imm32_disp_reg(type, disp + TVOFF(m_type), dest);
  }
  if (!IS_NULL_TYPE(type)) {
    ASSERT(val != reg::noreg);
    a.  store_reg64_disp_reg64(val, disp + TVOFF(m_data), dest);
  }
}

static inline void
emitStoreInvalid(X64Assembler& a, int disp, PhysReg dest) {
  a.    storeq (0xfacefacefaceface,     dest[disp + TVOFF(m_data)]);
  a.    storel ((signed int)0xfaceface, dest[disp + TVOFF(_count)]);
  a.    storel (KindOfInvalid,          dest[disp + TVOFF(m_type)]);
}

static inline void
emitStoreUninitNull(X64Assembler& a,
                    int disp,
                    PhysReg dest) {
  // OK to leave garbage in m_data, _count.
  a.    store_imm32_disp_reg(KindOfUninit, disp + TVOFF(m_type), dest);
}

static inline void
emitStoreNull(X64Assembler& a,
              int disp,
              PhysReg dest) {
  a.    store_imm32_disp_reg(KindOfNull, disp + TVOFF(m_type), dest);
  // It's ok to leave garbage in m_data, _count for KindOfNull.
}

static inline void
emitStoreNull(X64Assembler& a, const Location& where) {
  PhysReg base;
  int disp;
  locToRegDisp(where, &base, &disp);
  emitStoreNull(a, disp, base);
}

static inline void
emitCopyTo(X64Assembler& a,
           Reg64 src,
           int srcOff,
           Reg64 dest,
           int destOff,
           PhysReg scratch) {
  ASSERT(src != scratch);
  // This is roughly how gcc compiles this.
  // Blow off _count.
  auto s64 = r64(scratch);
  auto s32 = r32(scratch);
  a.    loadq  (src[srcOff + TVOFF(m_data)], s64);
  a.    storeq (s64, dest[destOff + TVOFF(m_data)]);
  a.    loadl  (src[srcOff + TVOFF(m_type)], s32);
  a.    storel (s32, dest[destOff + TVOFF(m_type)]);
}

/*
 * Version of emitCopyTo where both the source and dest are known to
 * be 16-byte aligned.  In this case we can use xmm.
 */
inline void emitCopyToAligned(X64Assembler& a,
                              Reg64 src,
                              int srcOff,
                              Reg64 dest,
                              int destOff) {
  static_assert(sizeof(TypedValue) == 16,
                "emitCopyToAligned assumes sizeof(TypedValue) is 128 bits");
  a.    movdqa  (src[srcOff], xmm0);
  a.    movdqa  (xmm0, dest[destOff]);
}

// ArgManager -- support for passing VM-level data to helper functions.
class ArgManager {
  typedef HPHP::VM::Transl::X64Assembler& A;
public:
  ArgManager(TranslatorX64 &tx64, A& a) : m_tx64(tx64), m_a(a) { }

  void addImm(uint64_t imm) {
    TRACE(6, "ArgManager: push arg %zd imm:%lu\n",
          m_args.size(), imm);
    m_args.push_back(ArgContent(ArgContent::ArgImm, InvalidReg, imm));
  }

  void addLoc(const Location &loc) {
    TRACE(6, "ArgManager: push arg %zd loc:(%s, %lld)\n",
          m_args.size(), loc.spaceName(), loc.offset);
    m_args.push_back(ArgContent(ArgContent::ArgLoc, loc));
  }

  void addDeref(const Location &loc) {
    TRACE(6, "ArgManager: push arg %zd deref:(%s, %lld)\n",
          m_args.size(), loc.spaceName(), loc.offset);
    m_args.push_back(ArgContent(ArgContent::ArgDeref, loc));
  }

  void addReg(PhysReg reg) {
    TRACE(6, "ArgManager: push arg %zd reg:r%d\n",
          m_args.size(), int(reg));
    m_args.push_back(ArgContent(ArgContent::ArgReg, reg, 0));
  }

  void addRegPlus(PhysReg reg, int32_t off) {
    TRACE(6, "ArgManager: push arg %zd regplus:r%d+%d\n",
          m_args.size(), int(reg), off);
    m_args.push_back(ArgContent(ArgContent::ArgRegPlus, reg, off));
  }

  void addReg(const LazyScratchReg& l) { addReg(r(l)); }
  void addRegPlus(const LazyScratchReg& l, int32_t off) {
    addRegPlus(r(l), off);
  }

  void addLocAddr(const Location &loc) {
    TRACE(6, "ArgManager: push arg %zd addr:(%s, %lld)\n",
          m_args.size(), loc.spaceName(), loc.offset);
    ASSERT(!loc.isLiteral());
    m_args.push_back(ArgContent(ArgContent::ArgLocAddr, loc));
  }

  void emitArguments() {
    size_t n = m_args.size();
    ASSERT((int)n <= kNumRegisterArgs);
    cleanLocs();
    std::map<PhysReg, size_t> used;
    std::vector<PhysReg> actual(n, InvalidReg);
    computeUsed(used, actual);
    shuffleRegisters(used, actual);
    emitValues(actual);
  }

private:
  struct ArgContent {
    enum ArgKind {
      ArgImm, ArgLoc, ArgDeref, ArgReg, ArgRegPlus, ArgLocAddr
    } m_kind;
    PhysReg m_reg;
    const Location *m_loc;
    uint64_t m_imm;

    ArgContent(ArgKind kind, PhysReg reg, uint64_t imm) :
      m_kind(kind), m_reg(reg), m_loc(NULL), m_imm(imm) { }
    ArgContent(ArgKind kind, const Location &loc) :
      m_kind(kind), m_reg(InvalidReg), m_loc(&loc), m_imm(0) { }
  };

  TranslatorX64& m_tx64;
  A& m_a;
  std::vector<ArgContent> m_args;

  ArgManager(); // Don't build without reference to translator

  void cleanLocs();
  void computeUsed(std::map<PhysReg, size_t> &used,
                   std::vector<PhysReg> &actual);
  void shuffleRegisters(std::map<PhysReg, size_t> &used,
                     std::vector<PhysReg> &actual);
  void emitValues(std::vector<PhysReg> &actual);
};

// Some macros to make writing calls palatable. You have to "type" the
// arguments
#define IMM(i)       _am.addImm(i)
#define V(loc)       _am.addLoc(loc)
#define DEREF(loc)   _am.addDeref(loc)
#define R(r)         _am.addReg(r)
#define RPLUS(r,off) _am.addRegPlus(r, off)
#define A(loc)       _am.addLocAddr(loc)
#define IE(cond, argIf, argElse) \
  ((cond) ? (argIf) : (argElse))
static inline void voidFunc() {}
#define ID(argDbg) IE(debug, (argDbg), voidFunc())

#define EMIT_CALL_PROLOGUE(a) do { \
  SpaceRecorder sr("_HCallInclusive", a);  \
  ArgManager _am(*this, a);       \
  prepareCallSaveRegs();

#define EMIT_CALL_EPILOGUE(a, dest) \
  _am.emitArguments();           \
  { \
    SpaceRecorder sr("_HCallExclusive", a); \
    emitCall(a, (TCA)(dest), true);         \
  } \
} while(0)

#define EMIT_CALL(a, dest, ...) \
  EMIT_CALL_PROLOGUE(a)         \
  __VA_ARGS__ ;                 \
  EMIT_CALL_EPILOGUE(a, dest)

#define EMIT_RCALL(a, ni, dest, ...) \
  EMIT_CALL(a, dest, __VA_ARGS__);   \
  recordReentrantCall(a, ni);

// typeReentersOnRelease --
//   Returns whether the release helper for a given type can
//   reenter.
static bool typeReentersOnRelease(DataType type) {
  return IS_REFCOUNTED_TYPE(type) && type != BitwiseKindOfString;
}

// supportedPlan --
// nativePlan --
// simplePlan --
//   Some helpers for analyze* methods.
static inline TXFlags
plan(bool cond, TXFlags successFlags, TXFlags fallbackFlags=Interp) {
  return cond ? successFlags : fallbackFlags;
}

static inline TXFlags simplePlan(bool cond) { return plan(cond, Simple); }
static inline TXFlags simpleOrSupportedPlan(bool cond) {
  return plan(cond, Simple, Supported);
}
static inline TXFlags supportedPlan(bool cond) { return plan(cond, Supported); }
static inline TXFlags nativePlan(bool cond) { return plan(cond, Native); }

static inline TXFlags planHingesOnRefcounting(DataType type) {
  return !IS_REFCOUNTED_TYPE(type) ? Native :
         !typeReentersOnRelease(type) ? Simple :
         Supported;
}

static inline const char* getContextName() {
  Class* ctx = arGetContextClass(curFrame());
  return ctx ? ctx->name()->data() : ":anonymous:";
}

template<class T>
struct Nuller : private boost::noncopyable {
  explicit Nuller(const T** p) : p(p) {}
  ~Nuller() { *p = 0; }
  T const** const p;
};

template<class T>
struct Deleter : private boost::noncopyable {
  explicit Deleter(T** p) : p(p) {}
  ~Deleter() {
    delete *p;
    *p = NULL;
  }
  T** p;
};

}}}

#endif

