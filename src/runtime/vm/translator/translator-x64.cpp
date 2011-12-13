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
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <strstream>
#include <stdio.h>
#include <stdarg.h>
#include <string>

#include <util/pathtrack.h>
#include <util/trace.h>

#include <runtime/base/tv_macros.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/php_debug.h>
#include <runtime/vm/runtime.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/runtime_option.h>
#include <runtime/vm/debug/debug.h>
#include <runtime/vm/translator/targetcache.h>
#include <util/debug.h>
#include <runtime/vm/translator/log.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/translator/asm-x64.h>
#include <runtime/vm/translator/srcdb.h>

using namespace HPHP::x64;
using namespace HPHP::x64::reg;
using namespace std;
using namespace HPHP;
using HPHP::DataType;
using HPHP::Variant;
using namespace HPHP::VM::Debug;

namespace HPHP {
namespace VM {
namespace Transl {

static const DataType BitwiseKindOfString = KindOfString;
static const DataType BitwiseKindOfInt64 = KindOfInt64;
static const DataType BitwiseKindOfInt32 = KindOfInt32;

#define KindOfString \
#error You probably do not mean to use KindOfString in this file.

#define KindOfInt64 \
#error You probably do not mean to use KindOfInt64 in this file.

#define KindOfInt32 \
#error You probably do not mean to use KindOfInt32 in this file.

#define NULLCASE() \
  case KindOfUninit: case KindOfNull

#define STRINGCASE() \
  case BitwiseKindOfString: case KindOfStaticString

#define INTCASE() \
  case BitwiseKindOfInt64: case BitwiseKindOfInt32

static const Trace::Module TRACEMOD = Trace::tx64;
#ifdef DEBUG
static const bool debug = true;
#else
static const bool debug = false;
#endif

typedef const size_t COff; // Const offsets

// tx64: shared across all threads and requests.
static TranslatorX64* tx64;

// RAII logger for TC space consumption.
struct SpaceRecorder {
  const char *m_name;
  const X64Assembler& m_a;
  const uint8_t *m_start;
  SpaceRecorder(const char* name, const X64Assembler& a) :
      m_name(name), m_a(a), m_start(a.code.frontier)
    { }
  ~SpaceRecorder() {
    if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
      ptrdiff_t diff = m_a.code.frontier - m_start;
      if (diff) Trace::traceRelease("TCSpace %10s %3d\n", m_name, diff);
    }
    if (Trace::moduleEnabledRelease(Trace::tcdump, 1)) {
      Trace::traceRelease("TCDump %s", m_name);
      for (const uint8_t* p = m_start; p < m_a.code.frontier; p++) {
        Trace::traceRelease(" %x", *p);
      }
      Trace::traceRelease("\n");
    }
  }
};

// While running in the TC, the rVmAr register points to the head of the the
// VM's chain of activation records. It's handy to have it be callee-saved,
// and not in the upper half of the register file since we frequently
// manipulate it, and big registers require an extra byte.
static const PhysReg rVmSp = rbx;
static const PhysReg rVmFp = rbp;

static StaticString s___call(LITSTR_INIT("__call"));
static StaticString s___callStatic(LITSTR_INIT("__callStatic"));

// The x64 C ABI.
static const PhysReg argNumToRegName[] = {
  rdi, rsi, rdx, rcx, r8, r9
};
static const int kNumRegisterArgs = sizeof(argNumToRegName) /
  sizeof(PhysReg);

static void dumpInstrInputs(int level, const NormalizedInstruction& i) {
#ifdef DEBUG
  for (size_t ii = 0; ii < i.inputs.size(); ii++) {
    TRACE(level, Trace::prettyNode("inputs", *i.inputs[ii]) + string("\n"));
  }
#endif
}

// Initialize at most this many locals inline in function body prologue; more
// than this, and emitting a loop is more compact.  (To be precise, the actual
// crossover point in terms of code size is 6; 9 was determined by experiment to
// be the optimal point in certain benchmarks. #microoptimization
static const int kLocalsToInitializeInline = 9;

// An intentionally funny-looking-in-core-dumps constant for uninitialized
// instruction pointers.
static const uint64_t kUninitializedRIP = 0xba5eba11acc01ade;

static inline size_t
cellsToBytes(int nCells) {
  return nCells * sizeof(Cell);
}

static inline size_t
bytesToCells(int nBytes) {
  ASSERT(nBytes % sizeof(Cell) == 0);
  return nBytes / sizeof(Cell);
}

static inline void
translator_not_reached(X64Assembler &a) {
  if (debug) {
    a.  ud2();
  }
}

static inline void
translator_debug_break(X64Assembler &a) {
  if (debug) {
    a.  int3();
  }
}

/*
 * TLS access: XXX we currently only support static-style TLS directly
 * linked off of FS.
 *
 * x86 terminology review: "Virtual addresses" are subject to both
 * segmented translation and paged translation. "Linear addresses" are
 * post-segmentation address, subject only to paging. C and C++ generally
 * only have access to bitwise linear addresses.
 *
 * TLS data live at negative virtual addresses off FS: the first datum
 * is typically at VA(FS:-sizeof(datum)). Linux's x64 ABI stores the linear
 * address of the base of TLS at VA(FS:0). While this is just a convention, it
 * is firm: gcc builds binaries that assume it when, e.g., evaluating
 * "&myTlsDatum".
 *
 * The virtual addresses of TLS data are not exposed to C/C++. To figure it
 * out, we take a datum's linear address, and subtract it from the linear
 * address where TLS starts. If you use the void* variant here, it's up to
 * the programmer to ensure that it really is a TLS address.
 */
static inline void
emitTLSLoad(X64Assembler& a, const void* datum, PhysReg reg) {
  uintptr_t virtualAddress = uintptr_t(datum) - tlsBase();
  a.    fs();
  a.    load_disp32_reg64(virtualAddress, reg);
}

template<typename T>
static inline void
emitTLSLoad(X64Assembler& a, const ThreadLocalNoCheck<T>& datum, PhysReg reg) {
  emitTLSLoad(a, &datum.m_node.m_p, reg);
}


/*
 * tx64LocPhysicalOffset --
 *
 *   The translator uses the stack pointer slightly differently from
 *   VM::Stack. Consequently, the translated code accesses slightly
 *   different offsets from rVmSp than the C++ runtime.
 */
static inline int
tx64LocPhysicalOffset(const Location& l, const Func *f = NULL) {
  return Translator::locPhysicalOffset(l, f);
}

/*
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
  *outdisp = cellsToBytes(tx64LocPhysicalOffset(l, f));
  *outbase = l.space == Location::Stack ? rVmSp : rVmFp;
}

// A CondBlock is an RAII structure for emitting conditional code. It
// compares the source register at fieldOffset with fieldValue, and
// conditionally branches over the enclosing block of assembly on the
// passed-in condition-code.
//  E.g.:
//    {
//      RefCountedOnly ifRefCounted(a, rdi, 0);
//      emitIncRef(rdi);
//    }
// will only execute emitIncRef if we find at runtime that rdi points at
// a ref-counted cell.
template<int FieldOffset, int FieldValue, int Jcc>
struct CondBlock {
  X64Assembler& m_a;
  int m_off;
  TCA m_jcc8;

  CondBlock(X64Assembler& a, PhysReg reg, int offset = 0)
      : m_a(a), m_off(offset) {
    int typeDisp = m_off + FieldOffset;
    a.   cmp_imm32_disp_reg32(FieldValue, typeDisp, reg);
    m_jcc8 = a.code.frontier;
    a.   jcc8(Jcc, m_jcc8);
    // ...
  }

  ~CondBlock() {
    m_a.patchJcc8(m_jcc8, m_a.code.frontier);
  }
};

// JccBlock --
//   A raw condition-code block; assumes whatever comparison or ALU op
//   that sets the Jcc has already executed.
template <int Jcc>
struct JccBlock {
  friend class ElseBlock;
  mutable X64Assembler* m_a;
  TCA m_jcc8;
  // Default ctor for containers.
  JccBlock() : m_a(NULL) { }

  JccBlock(X64Assembler& a)
    : m_a(&a), m_jcc8(a.code.frontier) {
    a.    jcc8(Jcc, 0);
  }

  JccBlock(const JccBlock& rhs) {
    m_a = rhs.m_a;
    // When copying, make the left-hand-side the "owner."
    rhs.m_a = NULL;
    m_jcc8 = rhs.m_jcc8;
  }

  ~JccBlock() {
    if (m_a) {
      m_a->patchJcc8(m_jcc8, m_a->code.frontier);
    }
  }
};

// UnlikelyIfBlock: branch to distant code (that we presumably
//   don't expect to take). This helps keep hot paths compact.
//   TCA retFromStubs;
//   a.   test_reg_reg(rax, rax);
//   {  UnlikelyIfBlock<CC_Z> ifNotRax(a, astubs);
//      astubs.   call(launch_nuclear_missiles);
//      retFromStubs = astubs.code.frontier;
//   }
//   a.   call(continue_peace_and_prosperity);
//   astubs.patchJmp(retFromStubs, a.code.frontier);
template <int Jcc>
struct UnlikelyIfBlock {
  X64Assembler &m_likely;
  X64Assembler &m_unlikely;
  TCA m_likelyPostBranch;

  UnlikelyIfBlock(X64Assembler& likely, X64Assembler& unlikely) :
    m_likely(likely), m_unlikely(unlikely) {
    m_likely.jcc(Jcc, m_unlikely.code.frontier);
    m_likelyPostBranch = m_likely.code.frontier;
  }

  ~UnlikelyIfBlock() {
    m_unlikely.jmp(m_likelyPostBranch);
  }
};

// IfElseBlock: like CondBlock, but with an else clause.
//    a.   test_reg_reg(rax, rax);
//    {  IfElseBlock<CC_Z> ifRax(a);
//       // Code executed for rax == 0
//       ifRax.Else();
//       // Code executed for rax != 0
//    }
//
template <int Jcc>
class IfElseBlock {
  X64Assembler& m_a;
  TCA m_jcc8;
  TCA m_jmp8;
 public:
  IfElseBlock(X64Assembler& a) :
    m_a(a), m_jcc8(a.code.frontier), m_jmp8(NULL) {
    m_a.jcc8(Jcc, m_jmp8);  // 1f
  }
  void Else() {
    ASSERT(m_jmp8 == NULL);
    m_jmp8 = m_a.code.frontier;
    m_a.jmp8(m_jmp8); // 2f
    // 1:
    m_a.patchJcc8(m_jcc8, m_a.code.frontier);
  }
  ~IfElseBlock() {
    ASSERT(m_jmp8 != NULL);
    // 2:
    m_a.patchJmp8(m_jmp8, m_a.code.frontier);
  }
};

static bool
typeCanBeStatic(DataType t) {
  return t != KindOfObject && t != KindOfVariant;
}

// IfRefCounted --
//   Emits if (IS_REFCOUNTED_TYPE()) { ... }
typedef CondBlock <offsetof(TypedValue, m_type),
                   KindOfStaticString,
                   CC_LE> IfRefCounted;

typedef CondBlock <offsetof(TypedValue, m_type),
                   KindOfVariant,
                   CC_NZ> IfVariant;

// IfCountNotStatic --
//   Emits if (%reg->_count != RefCountStaticValue) { ... }.
//   May short-circuit this check if the type is known to be
//   static already.
struct IfCountNotStatic {
  typedef CondBlock<offsetof(TypedValue, _count),
                    RefCountStaticValue,
                    CC_Z> NonStaticCondBlock;
  NonStaticCondBlock *m_cb; // might be null
  IfCountNotStatic(X64Assembler& a,
                   PhysReg reg,
                   DataType t = KindOfInvalid) {
    // Objects and variants cannot be static
    if (typeCanBeStatic(t)) {
      m_cb = new NonStaticCondBlock(a, reg);
    } else {
      m_cb = NULL;
    }
  }

  ~IfCountNotStatic() {
    if (m_cb) {
      delete m_cb;
    }
  }
};

bool
TranslatorX64::Lease::amOwner() {
  return m_held && m_owner == pthread_self();
}

/*
 * Multi-threaded scenarios for lease interleaving are hard to tease out.
 * The "gremlin" is a pseudo-thread that sometimes steals our lease while
 * we're running in the TC.
 *
 * The gremlin's pthread id is the bitwise negation of our own. Yes,
 * hypothetical wacky pthread implementations can break this. It's
 * DEBUG-only, folks.
 */
void
TranslatorX64::Lease::gremlinLock() {
  if (amOwner()) {
    TRACE(2, "Lease: gremlinLock dropping lease\n");
    drop();
  }
  pthread_mutex_lock(&m_lock);
  TRACE(2, "Lease: gremlin grabbed lock\n ");
  m_held = true;
  m_owner = ~pthread_self();
}

void
TranslatorX64::Lease::gremlinUnlock() {
  if (m_held && m_owner == ~pthread_self()) {
    TRACE(2, "Lease: gremlin dropping lock\n ");
    pthread_mutex_unlock(&m_lock);
    m_held = false;
  }
}

// becomeWriter: also returns true if we are already the writer.
bool
TranslatorX64::Lease::acquire() {
  if (0 == pthread_mutex_trylock(&m_lock)) {
    TRACE(3, "thr%ld: acquired lease\n", pthread_self());
    m_held = true;
    m_owner = pthread_self();
  }
  return amOwner();
}

void
TranslatorX64::Lease::drop() {
  if (amOwner()) {
    TRACE(3, "thr%ld: dropping lease\n", pthread_self());
    m_held = false;
    pthread_mutex_unlock(&m_lock);
  }
}

// emitDispDeref --
// emitDeref --
// emitTypedValueStore --
// emitStoreUninitNull --
//
//   Helpers for common cell operations.
//
//   Dereference the var or home in the cell whose address lives in src
//   into dest.
static void
emitDispDeref(X64Assembler &a, PhysReg src, int disp,
              PhysReg dest) {
  a.    load_reg64_disp_reg64(src, disp + offsetof(Cell, m_data), dest);
}

static void
emitDeref(X64Assembler &a, PhysReg src, PhysReg dest) {
  emitDispDeref(a, src, 0, dest);
}

// Logical register move: ensures the value in src will be in dest
// after execution, but might do so in strange ways. Do not count on
// being able to smash dest to a different register in the future, e.g.
static inline void
emitMovRegRegAsm(X64Assembler& a, PhysReg src, PhysReg dest) {
  if (src != dest) {
    a.  mov_reg64_reg64(src, dest);
  }
}

static inline void
emitSaveFPReg(X64Assembler& a, PhysReg fpReg = rVmFp, PhysReg scratchReg = rScratch) {
  PhysReg rEC = scratchReg;
  emitTLSLoad<ExecutionContext>(a, g_context, rEC);
  static COff fpOff = offsetof(ExecutionContext, m_fp);
  a.    add_imm32_reg64(fpOff, rEC);
  a.    store_reg64_disp_reg64 (fpReg, 0, rEC);
}

void
TranslatorX64::emitMovRegReg(PhysReg src, PhysReg dest) {
  emitMovRegRegAsm(a, src, dest);
}

static void
emitImmReg(X64Assembler& a, uint64_t imm, PhysReg dest) {
  if (imm == 0) {
    a.  xor_reg32_reg32(dest, dest);
    return;
  }
  if (LIKELY(int32(imm) > 0 && x64::deltaFits(imm, x64::sz::dword))) {
    // This will zero out the high-order bits.
    a.  mov_imm32_reg32(imm, dest);
    return;
  }
  a.    mov_imm64_reg(imm, dest);
}

// NB: leaves count field uninitialized. Use only for stack values.
static void
emitTypedValueStore(X64Assembler& a, DataType type, PhysReg val,
                    int disp, PhysReg dest) {
  a.    store_imm32_disp_reg  (type, disp + offsetof(Cell, m_type), dest);
  a.    store_reg64_disp_reg64(val,  disp + offsetof(Cell, m_data), dest);
}

// Assumes caller has already zeroed out zeroedReg
static void
emitStoreUninitNull(X64Assembler& a,
                    PhysReg zeroedReg,
                    int disp,
                    PhysReg dest) {
  a.    store_reg64_disp_reg64(zeroedReg, disp + offsetof(Cell, _count), dest);
}

// vstackOffset --
// emitVStackLoad --
// emitVStackStore --
//
//   Load from the "virtual stack" at a normalized instruction boundary.
//   Since rVmSp points to the stack at entry to the current BB, we need to
//   adjust stack references relative to it.
//
//   For all parameters, offsets and sizes are in bytes. Sizes are expected
//   to be a hardware size: 1, 2, 4, or 8 bytes.
static int
vstackOffset(const NormalizedInstruction& ni, COff off) {
  return off - cellsToBytes(ni.stackOff);
}

static void
emitVStackStoreImm(X64Assembler &a, const NormalizedInstruction &ni,
                   uint64_t imm, int off, int size = sz::qword,
                   RegAlloc *regAlloc = NULL) {
  int hwOff = vstackOffset(ni, off);
  if (size == sz::qword) {
    PhysReg immReg = regAlloc ? regAlloc->getImmReg(imm) : RegAlloc::InvalidReg;
    if (immReg == RegAlloc::InvalidReg) {
      emitImmReg(a, imm, rScratch);
      immReg = rScratch;
    }
    a.   store_reg64_disp_reg64(immReg, hwOff, rVmSp);
  } else if (size == sz::dword) {
    a.   store_imm32_disp_reg(imm, hwOff, rVmSp);
  } else {
    not_implemented();
  }
}

static void
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

/*
 * emitPushAR --
 *
 *   Push an activation record. Will return to the next instruction emitted by
 *   the invoker. Called on behalf of FPushFuncD and FPushFunc. If func is
 *   unknown, we will leave it to the caller to fill in m_func.
 */
void
TranslatorX64::emitPushAR(const NormalizedInstruction& i, const Func* func,
                          const int bytesPopped) {
  /*
   *     rspOffset      offset from base of new ActRec
   *     0              32
   *     -16            16
   *     -32            0
   */
  if (func && phpBreakpointEnabled(func->m_name->data())) {
    translator_debug_break(a);
  }
  ASSERT(sizeof(Cell) < sizeof(ActRec));
  // We are about to push an ActRec onto the stack. The stack grows down,
  // so the offset of the beginning of the ActRec from the top of stack
  // is -sizeof(ActRec).
  int numArgs = i.imm[0].u_IVA;
  int startOfActRec = bytesPopped - sizeof(ActRec);
  size_t funcOff    = offsetof(ActRec, m_func) + startOfActRec;
  size_t thisOff    = offsetof(ActRec, m_this) + startOfActRec;
  size_t nargsOff   = offsetof(ActRec, m_numArgs) + startOfActRec;
  size_t varenvOff  = offsetof(ActRec, m_varEnv) + startOfActRec;
  size_t savedRbpOff = offsetof(ActRec, m_savedRbp) + startOfActRec;

  ASSERT(sizeof(((ActRec*)NULL)->m_numArgs) == sizeof(int32_t));
  /*
   * rVmSp might not be up-to-date here, so we use emitVStackStore and
   * emitVStackStoreImm which know how to compute the where the top of
   * stack currently is.
   */
  if (func) {
    // If we statically know the function, it must be a non-method
    // invocation, so $this is null. Otherwise, let our caller sort
    // out setting $this.
    emitVStackStoreImm(a, i, (uintptr_t)func, funcOff);
    emitVStackStoreImm(a, i, 0,               thisOff, sz::qword, &m_regMap);
  }
  emitVStackStoreImm(a, i,   numArgs,         nargsOff, sz::dword);
  emitVStackStoreImm(a, i,   (uintptr_t)NULL, varenvOff, sz::qword, &m_regMap);
  emitVStackStore(a, i,      rVmFp,           savedRbpOff, sz::qword);
}

/*
 * emitCallReentrantPrologue --
 * emitCallSaveRegs --
 * emitCallPassLoc --
 * emitCall --
 *
 *   Helpers for callout prologue and epilogue: save dirty registers,
 *   and helpers for passing arguments.
 *
 *   Prefer EMIT_CALL* when possible.
 */
static inline bool
regIsInCallingSequence(PhysReg r) {
  switch(r) {
    case rax:
    case rdi:
    case rsi:
    case rcx:
    case rdx:
    case r8:
    case r9:
      return true;
    default:
      return false;
  }
}

static set<PhysReg>
callerSaved() {
  set<PhysReg> s;
  PhysReg callerSaved[] = {
    rax,         /* rbx, rbp callee-saved */
    rcx, rdx, rsi, rdi,
    r8, r9, r11, /* r12 - r15 callee-saved */
  };
  for (unsigned i = 0; i < sizeof(callerSaved) / sizeof(callerSaved[0]);
       i++) {
    s.insert(callerSaved[i]);
  }
  return s;
}

class WithPhysRegSaver {
protected:
  X64Assembler& a;
  set<PhysReg> s;
public:
  WithPhysRegSaver(X64Assembler& a_, set<PhysReg> s_) : a(a_), s(s_) {
    for (std::set<PhysReg>::const_iterator it = s.begin();
         it != s.end(); ++it) {
      a.   pushr(*it);
    }
  }
  ~WithPhysRegSaver() {
    for (std::set<PhysReg>::const_reverse_iterator it = s.rbegin();
         it != s.rend(); ++it) {
      a.   popr(*it);
    }
  }
};

void
TranslatorX64::emitCallSaveRegs() {
  m_regMap.cleanRegs(callerSaved());
}

class ArgManager {
public:
  ArgManager(TranslatorX64 &tx64) : m_tx64(tx64) { }

  void addImm(uint64_t imm) {
    TRACE(6, "ArgManager: push arg %zd imm:%lu\n",
          m_args.size(), imm);
    m_args.push_back(ArgContent(ArgContent::ArgImm, RegAlloc::InvalidReg, imm));
  }

  void addLoc(const Location &loc) {
    TRACE(6, "ArgManager: push arg %zd loc:(%s, %d)\n",
          m_args.size(), loc.spaceName(), loc.offset);
    m_args.push_back(ArgContent(ArgContent::ArgLoc, loc));
  }

  void addDeref(const Location &loc) {
    TRACE(6, "ArgManager: push arg %zd deref:(%s, %d)\n",
          m_args.size(), loc.spaceName(), loc.offset);
    m_args.push_back(ArgContent(ArgContent::ArgDeref, loc));
  }

  void addReg(PhysReg reg) {
    TRACE(6, "ArgManager: push arg %zd reg:r%d\n",
          m_args.size(), reg);
    m_args.push_back(ArgContent(ArgContent::ArgReg, reg, 0));
  }

  void addRegPlus(PhysReg reg, uint64_t off) {
    TRACE(6, "ArgManager: push arg %zd regplus:r%d+%lu\n",
          m_args.size(), reg, off);
    m_args.push_back(ArgContent(ArgContent::ArgRegPlus, reg, off));
  }

  void addLocAddr(const Location &loc) {
    TRACE(6, "ArgManager: push arg %zd addr:(%s, %d)\n",
          m_args.size(), loc.spaceName(), loc.offset);
    m_args.push_back(ArgContent(ArgContent::ArgLocAddr, loc));
  }

  void emitArguments() {
    size_t n = m_args.size();
    ASSERT((int)n <= kNumRegisterArgs);
    std::map<PhysReg, size_t> used;
    PhysReg InvalidReg = RegAlloc::InvalidReg;
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
      m_kind(kind), m_reg(RegAlloc::InvalidReg), m_loc(&loc), m_imm(0) { }
  };

  TranslatorX64 &m_tx64;
  std::vector<ArgContent> m_args;

  ArgManager(); // Don't build without reference to translator

  void computeUsed(std::map<PhysReg, size_t> &used,
                   std::vector<PhysReg> &actual);
  void shuffleRegisters(std::map<PhysReg, size_t> &used,
                     std::vector<PhysReg> &actual);
  void emitValues(std::vector<PhysReg> &actual);
};

void ArgManager::computeUsed(std::map<PhysReg, size_t> &used,
                             std::vector<PhysReg> &actual) {
  size_t n = m_args.size();
  for (size_t i = 0; i < n; i++) {
    PhysReg reg = RegAlloc::InvalidReg;
    if (m_args[i].m_kind == ArgContent::ArgReg ||
        m_args[i].m_kind == ArgContent::ArgRegPlus) {
      reg = m_args[i].m_reg;
    } else if (m_args[i].m_kind == ArgContent::ArgLoc ||
               m_args[i].m_kind == ArgContent::ArgDeref) {
      reg = m_tx64.getReg(*m_args[i].m_loc);
    } else {
      continue;
    }
    TRACE(6, "ArgManager: arg %zd incoming reg r%d\n", i, reg);
    used[reg] = i;
    actual[i] = reg;
  }
}

void ArgManager::shuffleRegisters(std::map<PhysReg, size_t> &used,
                                  std::vector<PhysReg> &actual) {
  size_t n = m_args.size();
  for (size_t i = 0; i < n; i++) {
    if (actual[i] == RegAlloc::InvalidReg)
      continue;

    if (!mapContains(used, argNumToRegName[i])) {
      // There's no conflict, so just copy
      TRACE(6, "ArgManager: arg %zd reg available, copying from r%d to r%d\n",
            i, actual[i], argNumToRegName[i]);
      // Do copy and data structure update here, because this way
      // we can reuse the register in actual[i] later without problems.
      m_tx64.emitMovRegReg(actual[i], argNumToRegName[i]);
      used.erase(actual[i]);
      actual[i] = argNumToRegName[i];
    } else {
      size_t j = used[argNumToRegName[i]];
      if (actual[j] != actual[i]) {
        // The register is used by some other value, so we must swap the two
        // registers.
        ASSERT(j > i);
        ASSERT(actual[j] != RegAlloc::InvalidReg);
        PhysReg ri = actual[i],
                rj = actual[j];
        TRACE(6, "ArgManager: arg %zd register used by arg %zd, "
                 "swapping r%d with r%d\n", i, j, ri, rj);

        // Clean the registers first
        std::set<PhysReg> regs;
        regs.insert(ri);
        regs.insert(rj);
        m_tx64.m_regMap.cleanRegs(regs);

        // Emit the actual swap
        m_tx64.m_regMap.swapRegisters(ri, rj);
        m_tx64.a.  xchg_reg64_reg64(ri, rj);

        // Update the data structure for later steps
        for (size_t k = 0; k < n; k++) {
          if (actual[k] == ri) {
            actual[k] = rj;
          } else if (actual[k] == rj) {
            actual[k] = ri;
          }
        }
        used[ri] = j;
        used[rj] = i;
      }
    }
  }
}

void ArgManager::emitValues(std::vector<PhysReg> &actual) {
  for (size_t i = 0; i < m_args.size(); i++) {
    switch(m_args[i].m_kind) {
    case ArgContent::ArgLoc:
    case ArgContent::ArgDeref:
    case ArgContent::ArgReg:
      TRACE(6, "ArgManager: copying arg %zd from r%d to r%d\n",
            i, actual[i], argNumToRegName[i]);
      m_tx64.emitMovRegReg(actual[i], argNumToRegName[i]);
      // Emit dereference if needed
      if (m_args[i].m_kind == ArgContent::ArgDeref) {
        emitDeref(m_tx64.a, argNumToRegName[i], argNumToRegName[i]);
      }
      break;

    // For any of these cases, the register should already be available.
    // If it was used previously by an input value, shuffleRegisters
    // should have moved it to the proper register from argNumToRegName.
    case ArgContent::ArgImm:
      emitImmReg(m_tx64.a, m_args[i].m_imm, argNumToRegName[i]);
      break;

    case ArgContent::ArgRegPlus:
      ASSERT(!regIsInCallingSequence(m_args[i].m_reg));
      m_tx64.a.  lea_reg64_disp_reg64(m_args[i].m_reg, m_args[i].m_imm,
                                     argNumToRegName[i]);
      break;

    case ArgContent::ArgLocAddr:
      {
        PhysReg base;
        int disp;
        locToRegDisp(*m_args[i].m_loc, &base, &disp);
        m_tx64.a.  lea_reg64_disp_reg64(base, disp, argNumToRegName[i]);
      }
      break;

    default:
      // Should never happen
      ASSERT(false);
    }
  }
}

void
TranslatorX64::emitCall(TCA dest) {
  a.    call(dest);
  // All caller-saved regs are now suspect.
  m_regMap.smashPhysRegs(callerSaved());
}

// Some macros to make writing calls palatable. You have to "type" the
// arguments
#define EMIT_CALL_PROLOGUE() do { \
  SpaceRecorder sr("_HCallInclusive", a);  \
  ArgManager _am(*this);          \
  emitCallSaveRegs();             \

#define EMIT_CALL_EPILOGUE(dest) \
  _am.emitArguments();           \
  { \
    SpaceRecorder sr("_HCallExclusive", a); \
    emitCall((TCA)(dest));         \
  } \
} while(0)

// The argument macros have to be statements.
#define IMM(i) _am.addImm(i)

#define V(loc) _am.addLoc(loc)

#define DEREF(loc) _am.addDeref(loc)

#define R(r) _am.addReg(r)

#define RPLUS(r,off) _am.addRegPlus(r, off)

#define A(loc) _am.addLocAddr(loc)

#define EMIT_CALL0(dest) \
  EMIT_CALL_PROLOGUE() \
  EMIT_CALL_EPILOGUE(dest)

#define EMIT_CALL1(dest,A) \
  EMIT_CALL_PROLOGUE() \
  A; \
  EMIT_CALL_EPILOGUE(dest)

#define EMIT_CALL2(dest,A1,A2) \
  EMIT_CALL_PROLOGUE() \
  A1; A2; \
  EMIT_CALL_EPILOGUE(dest)

#define EMIT_CALL3(dest,A1,A2,A3) \
  EMIT_CALL_PROLOGUE() \
  A1; A2; A3; \
  EMIT_CALL_EPILOGUE(dest)

#define EMIT_CALL4(dest,A1,A2,A3,A4) \
  EMIT_CALL_PROLOGUE() \
  A1; A2; A3; A4; \
  EMIT_CALL_EPILOGUE(dest)

#define EMIT_CALL5(dest,A1,A2,A3,A4,A5) \
  EMIT_CALL_PROLOGUE() \
  A1; A2; A3; A4; A5; \
  EMIT_CALL_EPILOGUE(dest)

void
TranslatorX64::emitIncRef(PhysReg base, DataType dtype) {
  if (!IS_REFCOUNTED_TYPE(dtype) && dtype != KindOfInvalid) {
    return;
  }
  SpaceRecorder sr("_IncRef", a);
  ASSERT(sizeof(((Cell*)NULL)->_count == sizeof(int32_t)));
  { // if !static then
    IfCountNotStatic ins(a, base, dtype);
    /*
     * The optimization guide cautions against using inc; while it is
     * compact, it only writes the low-order 8 bits of eflags, causing a
     * partial dependency for any downstream flags-dependent code.
     */
    a.    add_imm32_disp_reg32(1, offsetof(Cell, _count), base);
  } // endif
}

void
TranslatorX64::emitIncRefGeneric(PhysReg base, int disp) {
  { // if RC
    IfRefCounted irc(a, base, disp);
    ScratchReg tmpReg(m_regMap);
    a.    load_reg64_disp_reg64(base, disp + offsetof(Cell, m_data),
                                *tmpReg);
    { // if !static
      IfCountNotStatic ins(a, *tmpReg);
      a.  add_imm32_disp_reg32(1, offsetof(Cell, _count), *tmpReg);
    } // endif
  } // endif
}

static void emitGetGContext(X64Assembler& a, PhysReg dest) {
  emitTLSLoad<ExecutionContext>(a, g_context, dest);
}

// RAII register savers. We use block scope for debugging currently,
// but also might need it for restore some day.
//
// WithVMRegsSaved --
//   Inline. Saves regs in-place in the TC. Can be used if the inputs
//   are in strange registers, e.g., in the calling sequence.
//
// WithVMRegsSavedStub --
//   Out-of-line. Saves regs using a call to a shared save stub that
//   is passed in.
class WithVMRegsSaved {
 public:
  /**
   * Stores the hardware rVmSp and rVmFp to their corresponding fields in
   * ExecutionContext. If savePC is true, this will also save the contents of %rdi
   * to ExecutionContext's m_pc field. The emitted code will clobber rScratch.
   *
   * The excessive parameterization is required to support inline saving
   * for the case of calling a builtin, which is pretty aggressively
   * optimized.
   */
  static TCA
  emitSaveVMRegisters(X64Assembler& a,
                      bool saveFP = false, bool savePC = false,
                      PhysReg scratchReg = rScratch,
                      PhysReg fpReg = rVmFp,
                      PhysReg pcReg = rdi,
                      PhysReg funcFpReg = rVmFp) {
    TCA start = a.code.frontier;
    PhysReg rEC = scratchReg;
    emitGetGContext(a, rEC);

    static COff spOff = offsetof(ExecutionContext, m_stack) +
      Stack::topOfStackOffset();
    static COff fpOff = offsetof(ExecutionContext, m_fp) - spOff;
    static COff pcOff = offsetof(ExecutionContext, m_pc) - spOff;
    static COff isValidOff = offsetof(ExecutionContext, m_isValid) - spOff;

    ASSERT(spOff != 0);
    // Istruction selection note: this is an lea, but add is more
    // compact and we can afford the flags bash.
    a.    add_imm32_reg64(spOff, rEC);
    a.    store_reg64_disp_reg64 (rVmSp, 0, rEC);
    if (savePC) {
      // We're going to temporarily abuse rVmSp to hold the current unit.
      PhysReg rBC = rVmSp;
      a.  pushr(rBC);
      // m_fp -> m_func -> m_unit -> m_bc + pcReg
      a.  load_reg64_disp_reg64(funcFpReg, offsetof(ActRec, m_func), rBC);
      a.  load_reg64_disp_reg64(rBC, offsetof(Func, m_unit), rBC);
      // No need to check for a null unit; that can only happen for
      // builtins, and if we're here, the current function wasn't a
      // builtin.
      a.  load_reg64_disp_reg64(rBC, Unit::bcOff(), rBC);
      a.  add_reg64_reg64(rBC, pcReg);
      a.  store_reg64_disp_reg64(pcReg, pcOff, rEC);
      a.  popr(rBC);
    }
    if (saveFP) {
      a.    store_reg64_disp_reg64 (fpReg, fpOff, rEC);
    }
    if (debug) {
      a.  store_imm32_disp_reg(true, isValidOff, rEC);
    }
    return start;
  }

  static void
  emitVolatilizeVMRegisters(X64Assembler& a) {
    if (debug) {
      static const COff isValidOff = offsetof(ExecutionContext, m_isValid);
      emitGetGContext(a, rScratch);
      a.  store_imm32_disp_reg(false, isValidOff, rScratch);
    }
  }

 protected:
  X64Assembler& a;

  // Default ctor that does nothing, for child classes that want their
  // own prologues.
  WithVMRegsSaved(X64Assembler& a_) : a(a_) { }
public:
  WithVMRegsSaved(X64Assembler& a_,
                  bool saveFP, bool savePC,
                  PhysReg scratchReg = rScratch,
                  PhysReg fpReg = rVmFp,
                  PhysReg pcReg = rdi,
                  PhysReg funcFpReg = rVmFp) : a(a_) {
    emitSaveVMRegisters(a, saveFP, savePC, scratchReg, fpReg, pcReg, funcFpReg);
  }
  ~WithVMRegsSaved() {
    emitVolatilizeVMRegisters(a);
  }
};

class WithVMRegsSavedStub : protected WithVMRegsSaved {
  void emit(Offset offset, TCA stub) {
    a.   pushr(rdi);
    emitImmReg(a, offset, rdi);
    a.   call(stub);
    a.   popr(rdi);
  }
 public:
  WithVMRegsSavedStub(X64Assembler& a_, Offset offset, TCA stub)
      : WithVMRegsSaved(a_) {
    emit(offset, stub);
  }
  WithVMRegsSavedStub(X64Assembler& a_, const NormalizedInstruction& i,
                      TCA stub)
      : WithVMRegsSaved(a_) {
    emit(i.offset(), stub);
  }
  // Use WithVMRegsSaved's destructor.
};

/**
 * emitDecRef --
 *
 *   Decrement a value's refcount and call the release helper if
 *   appropriate. emitDecRef requires that the caller knows the
 *   type at translation time.
 */
void TranslatorX64::emitDecRef(PhysReg rDatum, DataType type) {
  ASSERT(type != KindOfInvalid);
  if (!IS_REFCOUNTED_TYPE(type)) {
    return;
  }

  SpaceRecorder sr("_DecRef", a);
  { // if !static
    IfCountNotStatic ins(a, rDatum, type);
    a.    sub_imm32_disp_reg32(1, offsetof(Cell, _count), rDatum);

    { // if zero
      JccBlock<CC_NZ> ifZero(a);
      ASSERT(type >= 0 && type < MaxNumDataTypes);
      callUnaryStub(m_dtorStubs[type], rDatum);
    } // endif
  } // endif
}

/**
 * emitDecRefGeneric --
 *
 *   Decrement a value's refcount and call the release helper if
 *   appropriate. emitDecRefGeneric should only be used when the type
 *   is not known at translation time.
 *
 *   emitDecRefGeneric operates on the memory location given by
 *   srcReg+disp, so the caller is responsible for ensuring that the
 *   memory location is up to date and not enregistered.
 */
void TranslatorX64::emitDecRefGeneric(PhysReg srcReg, int disp) {
  SpaceRecorder sr("_DecRefGeneric", a);
  ScratchReg tmpReg(m_regMap);

  { // if RC
    IfRefCounted irc(a, srcReg, disp);
    a.    load_reg64_disp_reg64(srcReg, disp + offsetof(Cell, m_data), *tmpReg);
    { // if !static
      IfCountNotStatic ins(a, *tmpReg, KindOfInvalid);
      a.  sub_imm32_disp_reg32(1, offsetof(Cell, _count), *tmpReg);
      { // if zero
        JccBlock<CC_NZ> ifZero(a);
        callUnaryStub(m_dtorGenericStub, srcReg, disp);
      } // endif
    } // endif
  } // endif
}

/*
 * Translation call targets. It is a lot easier, and a bit more
 * portable, to use C linkage from assembly.
 */
TCA TranslatorX64::retranslate(SrcKey sk, bool align) {
  if (!m_writeLease.acquire()) return NULL;
  SKTRACE(1, sk, "retranslate\n");
  return translate(&sk, align);
}

/*
 * Satisfy an alignment constraint. If we're in a reachable section
 * of code, bridge the gap with nops. Otherwise, int3's.
 */
void
TranslatorX64::moveToAlign(X64Assembler &aa,
                           const size_t align /* =kJmpTargetAlign */,
                           bool unreachable /* =true */) {
  using namespace HPHP::Util;
  SpaceRecorder sr("_Align", aa);
  ASSERT(isPowerOfTwo(align));
  size_t leftInBlock = align - ((align - 1) & uintptr_t(aa.code.frontier));
  if (leftInBlock == align) return;
  if (unreachable) {
    aa.emitInt3s(leftInBlock);
    return;
  }
  aa.emitNop(leftInBlock);
}

/*
 * Req machinery. We sometimes emit code that is unable to proceed
 * without translator assistance; e.g., a basic block whose successor is
 * unknown. We leave one of these request arg blobs in m_data, and point
 * to it at callout-time.
 */

// REQ_BIND_CALL
struct ReqBindCall {
  TCA m_toSmash;
  Func* m_func;
  int m_nArgs;
  SrcKey m_sourceInstr;
} m_bindCall;

/*
 * Find or create a translation for sk. Returns TCA of "best" current
 * translation. May return NULL if it is currently impossible to create
 * a translation.
 */
TCA
TranslatorX64::getTranslation(const SrcKey *sk, bool align) {
  SKTRACE(2, *sk, "getTranslation: curUnit %s md5 %llx:%llx offset %d\n",
          curUnit()->m_filepath->data(),
          sk->m_md5.q[0],
          sk->m_md5.q[1],
          sk->offset());
  SKTRACE(2, *sk, "   unit md5s: %llx:%llx\n",
          curUnit()->m_md5.q[0],
          curUnit()->m_md5.q[1]);

  SrcDB::iterator entry = srcDB.find(*sk);
  if (entry != srcDB.end()) {
    SKTRACE(2, *sk, "getTranslation: found %p\n",
            entry->second.getTopTranslation());
    return entry->second.getTopTranslation();
  }

  /*
   * Try to become the writer. We delay this until we *know* we will have
   * a need to create new translations, instead of just trying to win the
   * lottery at the dawn of time. Hopefully lots of requests won't require
   * any new translation.
   */
  if (!m_writeLease.acquire()) {
    return NULL;
  }

  // We put retranslate requests at the end of our slab to more frequently
  //   allow conditional jump fall-throughs

  TCA start = emitServiceReq(REQ_RETRANSLATE, 1, sk->offset());
  SKTRACE(1, *sk, "inserting anchor translation for (%p,%d) at %p\n",
          curUnit(), sk->offset(), start);

  srcDB[*sk].newTranslation(astubs, start);
  transDB[start] = TransRec(*sk);
  ASSERT(getTxRec(start).isAnchor);

  return retranslate(*sk, align);
}

TCA
TranslatorX64::translate(const SrcKey *sk, bool align) {
  ASSERT(vmfp() >= vmsp());
  ASSERT(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  ASSERT(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);
  Tracelet tlet = analyze(sk);

  if (align) {
    moveToAlign(a);
  }

  TCA start = a.code.frontier;
  translateTracelet(tlet);
  SKTRACE(1, *sk, "translate moved head from %p to %p\n",
          srcDB[*sk].getTopTranslation(), start);
  ONTRACE(10, {
    ofstream f("cfg.dot", ios_base::trunc);
    drawCFG(f);
    f.close();
  });
  return start;
}

/*
 * Returns true if a's current frontier can have an nBytes-long
 * instruction written without any risk of cache-tearing.
 */
bool
TranslatorX64::isSmashable(X64Assembler& a, int nBytes) {
  ASSERT(nBytes <= int(kX64CacheLineSize));
  static const uint64 kCacheMask = ~(uint64(kX64CacheLineSize) - 1);
  uintptr_t iFrontier = uintptr_t(a.code.frontier);
  uintptr_t lastByte = iFrontier + nBytes - 1;
  return (iFrontier & kCacheMask) == (lastByte & kCacheMask);
}

void
TranslatorX64::prepareForSmash(int nBytes) {
  if (UNLIKELY(!isSmashable(a, nBytes))) {
    moveToAlign(a, kX64CacheLineSize, false);
  }
  ASSERT(isSmashable(a, nBytes));
}

void
TranslatorX64::smash(X64Assembler &a, TCA src, TCA dest) {
  TRACE(2, "smash: %p -> %p\n", src, dest);
  /*
   * !
   *
   * We are about to smash reachable code in the translation cache. If
   * the code were shared among multiple threads, this would not be safe.
   *
   * To make it so, you would need to do two things:
   *
   *    1. Do the write transactionally, using cmpxchg8b.
   *
   *    2. Ensure that the smashed region contains only a single
   *       instruction in the orignal instruction stream; otherwise, there
   *       is a chance that another thread could execute the nonsense
   *       instruction that is part way through the new bytes.
   *
   * Let's continue to pay careful attention to points like this; if we
   * can keep them satisfying these two contraints, we will save ourselves
   * the pain of implementing "stop-the-world" primitive for code
   * modifications.
   */
  CodeCursor cg(a, src);
  ASSERT(isSmashable(a, kJmpLen));
  if (dest > src && dest - src <= 7) {
    a.emitNop(dest - src);
  } else {
    a.    jmp(dest); // XXX: Modifying reachable code.
  }
}

void
TranslatorX64::emitStackCheck(int funcDepth, Offset pc) {
  uint64_t stackMask = cellsToBytes(Stack::kDefaultStackMax) - 1;
  a.    mov_imm64_reg(uintptr_t(pc), rdi);
  a.    mov_reg64_reg64(rVmSp, rScratch); // We're going to destroy this copy
  a.    and_imm64_reg64(stackMask, rScratch);
  a.    sub_imm64_reg64(funcDepth, rScratch);

  if (m_stackOverflowHelper == NULL) {
    m_stackOverflowHelper = a.code.frontier;
  }
  a.    jl(m_stackOverflowHelper); // Unlikely branch to failure.
  // Success.
}

void
TranslatorX64::emitStackCheckDynamic(int numArgs, Offset pc) {
  using namespace HPHP::Util;
  uint64_t stackMask = cellsToBytes(Stack::kDefaultStackMax) - 1;
  ASSERT(isPowerOfTwo(sizeof(Cell)));
  uint32_t logSizeofCell = log2(sizeof(Cell));

  a.    mov_imm64_reg(uintptr_t(pc), rdi);

  // m_fp -> m_func -> m_maxStackCells
  a.    load_reg64_disp_reg64(rVmSp,
                   offsetof(ActRec, m_func) + cellsToBytes(numArgs), rax);
  a.    load_reg64_disp_reg32(rax, offsetof(Func, m_maxStackCells), rax);
  // m_maxStackCells -> m_maxStackCells * sizeof(Cell)
  a.    shl_reg64(logSizeofCell, rax);

  a.    mov_reg64_reg64(rVmSp, rScratch); // We're going to destroy this copy
  a.    and_imm64_reg64(stackMask, rScratch);
  a.    sub_reg64_reg64(rax, rScratch);

  if (m_stackOverflowHelper == NULL) {
    m_stackOverflowHelper = a.code.frontier;
  }
  a.    jl(m_stackOverflowHelper); // Unlikely branch to failure.
  // Success.
}

// Loads g_context->m_dynTracer into rScratch
void
TranslatorX64::emitLoadDynTracer() {
  emitGetGContext(a, rScratch);
  static COff dtOff = offsetof(ExecutionContext, m_dynTracer);
  a.load_reg64_disp_reg64(rScratch, dtOff, rScratch);
}

void
TranslatorX64::emitBuiltinCall(const Func* func,
                               int nArgs,
                               Offset afterCall) {
  ASSERT(func->m_info);
  Func::BuiltinFunction runtimeFunc = func->m_builtinFuncPtr;
  BuiltinClassFunction runtimeClassFunc = func->m_builtinClassFuncPtr;

  if (false) { // typecheck
      ActRec* ar = NULL;
      runtimeClassFunc(ar);
      runtimeFunc(ar);
  }
  ASSERT(!func->m_preClass || runtimeClassFunc);
  ASSERT(func->m_preClass || runtimeFunc);
  TRACE(2, "builtin preClass %p cFunc %p func %p\n",
        func->m_preClass, runtimeClassFunc, runtimeFunc);

  /* builtins take a single ActRec parameter */
  a.   lea_reg64_disp_reg64(rVmSp, cellsToBytes(nArgs), argNumToRegName[0]);

  {
    emitImmReg(a, uintptr_t(afterCall), rsi);
    WithVMRegsSaved svm(a, false, true,
                        rax, // scratch
                        argNumToRegName[0], // live copy of vmfp
                        argNumToRegName[1]  // pc to save
                        ); // builtins can reenter the VM.
    /*
     * Below, we'll want vmSp to point to the return value too. rVmSp
     * is callee-saved, so we can do this here.
     */
    a.   lea_reg64_disp_reg64(argNumToRegName[0], offsetof(ActRec, m_r), rVmSp);

    if (func->m_preClass) {
      a.   call((TCA)runtimeClassFunc);
    } else {
      a.   call((TCA)runtimeFunc);
    }
  }

  emitLoadDynTracer();
  a.test_reg64_reg64(rScratch, rScratch);
  {
    UnlikelyIfBlock<CC_NZ> ifTracer(a, astubs);
    // rVmSp contains &ar->m_r right now, so reconstruct a pointer to
    // ar with that knowledge.
    astubs.lea_reg64_disp_reg64(rVmSp, -(int)offsetof(ActRec, m_r),
                                argNumToRegName[0]);
    astubs.call((TCA)&DynTracer::FunctionExit);
    m_regMap.smashPhysRegs(callerSaved());
  }
}

void
TranslatorX64::trimExtraArgs(ActRec* ar, int numArgs) {
  ASSERT(!ar->hasInvName());
  ASSERT(numArgs > ar->m_func->m_numParams);
  if (ar->m_varEnv == NULL) {
    ar->m_varEnv = new VarEnv();
    ar->m_varEnv->lazyAttach(ar);
  }
  // Stash the excess args in the VarEnv attached to the ActRec. They'll be
  // decref'ed, if needed, when the VarEnv gets destructed.
  const Func* f = ar->m_func;
  int numParams = f->m_numParams;
  TRACE(1, "trimExtraArgs: %d args, function %s takes only %d, ar %p\n",
        numArgs, f->m_name->data(), numParams, ar);
  ar->m_varEnv->setExtraArgs(
    (TypedValue*)(uintptr_t(ar) - numArgs * sizeof(TypedValue)),
    numArgs - numParams);
}

void
TranslatorX64::setArgInActRec(ActRec* ar, int argNum, uint64_t datum,
                              DataType t) {
  TypedValue* tv =
    (TypedValue*)(uintptr_t(ar) - (argNum+1) * sizeof(TypedValue));
  tv->m_data.num = datum;
  tv->_count = 0;
  tv->m_type = t;
}

void
TranslatorX64::shuffleArgsForMagicCall(ActRec* ar) {
  const Func* f UNUSED = ar->m_func;
  ASSERT(f && f->m_magic == Func::kMagic);
  ASSERT(f->m_name->isame(s___call.get())
         || f->m_name->isame(s___callStatic.get()));
  ASSERT(f->m_numParams == 2);
  TRACE(1, "shuffleArgsForMagicCall: ar %p\n", ar);
  ASSERT(ar->hasInvName());
  StringData* invName = ar->getInvName();
  ASSERT(invName);
  ar->setVarEnv(NULL);
  int nargs = ar->m_numArgs;
  // We need to make an array containing all the arguments passed by the
  // caller and put it where the second argument is
  HphpArray* argArray = NEW(HphpArray)(nargs);
  argArray->incRefCount();
  for (int i = 0; i < nargs; ++i) {
    TypedValue* tv =
      (TypedValue*)(uintptr_t(ar) - (i+1) * sizeof(TypedValue));
    argArray->nvAppend(tv, false);
    tvRefcountedDecRef(tv);
  }
  // Put invName in the slot for first argument
  setArgInActRec(ar, 0, uint64_t(invName), BitwiseKindOfString);
  // Put argArray in the slot for second argument
  setArgInActRec(ar, 1, uint64_t(argArray), KindOfArray);
  // Fix up ActRec's m_numArgs
  ar->m_numArgs = 2;
}

/*
 * Call/return hand-shaking is a bit funny initially. At translation time,
 * we don't necessarily know what function we're calling. For instance,
 *
 *   f(g());
 *
 * Will lead to a set of basic blocks like:
 *
 * b1: pushfuncd "f"
 *     pushfuncd "g"
 *     fcallc
 * b2: fcallc
 *
 * The fcallc labelled "b2" above is not statically bindable in our
 * execution model.
 *
 * We decouple the call work into a per-callsite portion, responsible
 * for recording the return address, and a per-callee X numArgs portion,
 * responsible for fixing up arguments and dispatching to remaining
 * code. We call the per-callee portion a "prologue."
 *
 * funcPrologue --
 *
 *   Given a callee and a number of args, match up to the callee's
 *   argument expectations and dispatch.
 */
TCA
TranslatorX64::funcPrologue(const Func* func, int nArgs, int flags) {
  FuncPrologueMap::const_iterator i =
    m_funcPrologues.find(FuncPrologueKey(func, nArgs, flags));
  if (i != m_funcPrologues.end()) {
    return i->second;
  }

  if (!m_writeLease.acquire()) {
    return NULL;
  }
  TCA start = a.code.frontier;
  SpaceRecorder sr("_FuncPrologue", a);
  m_funcPrologues.insert(
    pair<FuncPrologueKey, TCA>(FuncPrologueKey(func, nArgs, flags),
                               start));
  // Only user-defined functions have prologues.
  ASSERT(!func->m_info);

  // NB: We have the whole register file to play with, since we know we're
  // between BB's. So, we hardcode some registers here rather than using
  // the scratch allocator.
  TRACE(2, "funcPrologue: user function: %s\n", func->m_name->data());

  // Put the future value of FP into a callee-saved register, so we don't need
  // to bother saving it across helper calls.
  PhysReg rStashedAR = r15;
  a.    lea_reg64_disp_reg64(rVmSp, cellsToBytes(nArgs), rStashedAR);

  if (flags & FuncPrologueMagicCall) {
    ASSERT(func->m_numParams == 2);
    // Special __call prologue
    a.  mov_reg64_reg64(rStashedAR, argNumToRegName[0]);
    a.  call(TCA(TranslatorX64::shuffleArgsForMagicCall));
    // Fix up nArgs and hardware stack pointer
    nArgs = 2;
    a.  lea_reg64_disp_reg64(rStashedAR, -cellsToBytes(nArgs), rVmSp);
  }

  if (flags & FuncPrologueIntercepted) {
    // Check for intercepted functions.
    if (false) {  // typecheck
      ActRec* ar;
      FuncDict::InterceptData* d = intercept_data(ar);
      bool c UNUSED = run_intercept_handler(ar, d);
    }
    // Both callee-saved
    PhysReg rSavedRip = r13;
    a.  mov_reg64_reg64(rStashedAR, argNumToRegName[0]);
    // NB: intercept_data cannot reenter! No need to save VM regs yet.
    a.  call(TCA(intercept_data));
    a.  test_reg64_reg64(rax, rax);
    {
      UnlikelyIfBlock<CC_NZ> ifIntercepted(a, astubs);
      // save-regs stub is within its rights to clobber rax
      astubs.    pushr(rax);
      WithVMRegsSavedStub saveRegs(astubs, func->m_base, m_saveVMRegsStub);
      // Fish out the saved rip. We may need to jump there, and the helper will
      // have wiped out the ActRec.
      astubs.    load_reg64_disp_reg64(rStashedAR, offsetof(ActRec, m_savedRip),
                                       rSavedRip);
      astubs.    popr(argNumToRegName[1]);  // InterceptData*, pushed from rax
      astubs.    mov_reg64_reg64(rStashedAR, argNumToRegName[0]);
      astubs.    call(TCA(run_intercept_handler));
      astubs.    test_reg64_reg64(rax, rax);
      {
        // If the helper returned false, don't execute this function. The helper
        // will have cleaned up the interceptee's arguments and AR, and pushed
        // the handler's return value; we now need to get out.
        //
        // We don't need to touch rVmFp; it's still pointing to the caller of
        // the interceptee. We need to adjust rVmSp. Then we need to jump to the
        // saved rip from the interceptee's ActRec.
        JccBlock<CC_NZ> ifDontEnterFunction(astubs);
        astubs.  lea_reg64_disp_reg64(rStashedAR, offsetof(ActRec, m_r), rVmSp);
        astubs.  jmp_reg(rSavedRip);
      }
    }
  }

  int numParams = func->m_numParams;
  Offset dvInitializer = InvalidAbsoluteOffset;

  if (nArgs > numParams) {
    // Too many args; a weird case, so just callout. Stash ar
    // somewhere callee-saved.
    a.  mov_reg64_reg64(rStashedAR, argNumToRegName[0]);
    if (false) { // typecheck
      TranslatorX64::trimExtraArgs((ActRec*)NULL, 12);
    }
    // ar argument already in rdi
    emitImmReg(a, nArgs, argNumToRegName[1]);
    a.  call(TCA(TranslatorX64::trimExtraArgs));
    // We'll fix rVmSp below.
  } else if (nArgs < numParams) {
    // Figure out which, if any, default value initializer to go to
    for (int i = nArgs; i < numParams; ++i) {
      const Func::ParamInfo& pi = func->m_params[i];
      if (pi.hasDefaultValue()) {
        dvInitializer = pi.m_funcletOff;
        break;
      }
    }
    TRACE(1, "Only have %d of %d args; getting dvFunclet\n",
          nArgs, numParams);
    emitImmReg(a, 0, rdx);
    emitImmReg(a, nArgs, rax);
    // do { *(--rVmSp) = NULL; nArgs++; } while (nArgs < numParams);
    // This should be an unusual case, so optimize for code density
    // rather than execution speed; i.e., don't unroll the loop.
    TCA loopTop = a.code.frontier;
    a.  sub_imm32_reg64(sizeof(Cell), rVmSp);
    a.  add_imm32_reg32(1, rax);
    // XXX "missing argument" warnings need to go here
    emitStoreUninitNull(a, rdx, 0, rVmSp);
    a.  cmp_imm32_reg32(numParams, rax);
    a.  jcc8(CC_L, loopTop);
  }

  // Copy the old rbp into the savedRbp pointer.
  a.    store_reg64_disp_reg64(rbp, 0, rStashedAR);

  // Args are kosher. Frame linkage: set fp = ar.
  a.   mov_reg64_reg64(rStashedAR, rVmFp);
  emitSaveFPReg(a);

  // We're in the callee frame; initialize locals. Unroll the loop all the
  // way if there are a modest number of locals to update; otherwise, do
  // it in a compact loop.
  int numUninitLocals = func->m_numLocals - numParams;
  ASSERT(numUninitLocals >= 0);
  if (numUninitLocals > 0) {
    SpaceRecorder sr("_InitializeLocals", a);

    // If there are too many locals, then emitting a loop to initialize locals
    // is more compact, rather than emitting a slew of movs inline.
    if (numUninitLocals > kLocalsToInitializeInline) {
      PhysReg loopReg = rcx;

      // rVmFp + rcx points to the count/type fields of the TypedValue we're
      // about to write to.
      int loopStart = -func->m_numLocals * sizeof(TypedValue)
        + offsetof(TypedValue, _count);
      int loopEnd = -numParams * sizeof(TypedValue)
        + offsetof(TypedValue, _count);

      emitImmReg(a, loopStart, loopReg);
      emitImmReg(a, 0, rdx);

      TCA topOfLoop = a.code.frontier;
      // do {
      //   rVmFp[rcx].m_type = KindOfUninit;
      // } while(++rcx != loopEnd);

      //  mov %rdx, 0x0(%rVmFp, %rcx, 1)
      a.  emitRM(instr_mov, rVmFp, loopReg, 1, 0, rdx);
      a.  add_imm32_reg64(sizeof(Cell), loopReg);
      a.  cmp_imm32_reg64(loopEnd, loopReg);
      a.  jcc8(CC_NE, topOfLoop);
    } else {
      PhysReg base;
      int disp, k;
      emitImmReg(a, 0, rdx);
      for (k = numParams; k < func->m_numLocals; ++k) {
        locToRegDisp(Location(Location::Local, k), &base, &disp);
        emitStoreUninitNull(a, rdx, disp, base);
      }
    }
  }

  // Move rVmSp to the right place: just past all locals
  int frameCells = func->numSlotsInFrame();
  a.   lea_reg64_disp_reg64(rVmFp, -cellsToBytes(frameCells), rVmSp);
  const Opcode* destPC = func->m_unit->entry() + func->m_base;
  if (dvInitializer != InvalidAbsoluteOffset) {
    // dispatch to funclet.
    destPC = func->m_unit->entry() + dvInitializer;
  }

  SrcKey funcBody(func->m_unit, destPC);
  emitBindJmp(funcBody);

  recordGdbTranslation(funcBody, func->m_unit, start, a.code.frontier - start,
                       false, true);
  return start;
}

void
TranslatorX64::emitBindCall(const NormalizedInstruction &ni,
                            Offset atCall, Offset afterCall) {
  int numArgs = ni.imm[0].u_IVA;
  if (ni.funcd) {
    if (ni.funcd->m_info) {
      emitBuiltinCall(ni.funcd, numArgs, afterCall);
      return;
    }
    emitStackCheck(cellsToBytes(ni.funcd->maxStackCells()), atCall);
    // The Func in the ActRec we're activating is statically known; we don't
    // need any further runtime lookup. Make space to burn in a TCA.
    ReqBindCall* req = m_globalData.alloc<ReqBindCall>();
    prepareForSmash(kJmpLen);
    TCA start = a.code.frontier;
    a.jmp(emitServiceReq(REQ_BIND_CALL, 1, req));
    TRACE(1, "will bind static call: tca %p, this %p, funcd %p\n",
          start, this, ni.funcd);
    req->m_toSmash = start;
    req->m_func = (Func*)ni.funcd;
    req->m_nArgs = numArgs;
    req->m_sourceInstr = ni.source;
    return;
  }

  emitStackCheckDynamic(numArgs, atCall);
  // Dynamic dispatch. CallCache is not used to deal with mutable Func->TCA
  // mappings, because those mappings are not mutable. Rather, it's used to deal
  // with the fact that the Func on the stack when entering this instruction can
  // vary across runs of this code.
  using namespace TargetCache;
  // Allocate a cache for this callsite.
  CacheHandle ch = CallCache::alloc();
  WithVMRegsSaved sav(a, false, false);
  // Call it.
  if (false) { // typeczech
    ActRec* ar = NULL;
    TCA UNUSED retval = CallCache::lookup(ch, ar);
  }
  SKTRACE(1, ni.source, "ch %d\n", ch);
  a.    mov_imm32_reg32(ch, argNumToRegName[0]);
  a.    lea_reg64_disp_reg64(rVmSp, cellsToBytes(numArgs), argNumToRegName[1]);
  emitImmReg(a, afterCall, argNumToRegName[2]);
  a.    call(TCA(CallCache::lookup));

  // CallCache::lookup returns NULL if it's already done the work for us
  // (e.g., it was a builtin), and returns a TCA to continue executing in
  // otherwise. If it returned NULL, fix up rVmSp to pop its args and
  // actrec.
  a.    test_reg64_reg64(rax, rax);
  { // if (rax) {
    IfElseBlock<CC_Z> ifRax(a);
    a.  jmp_reg(rax);
    ifRax.Else(); // } else {
    a.  lea_reg64_disp_reg64(rVmSp, cellsToBytes(numArgs - 1)  + sizeof(ActRec),
                             rVmSp);
    // }
  }
}

static const PhysReg serviceReqArgRegs[] = {
  /* rdi: contains req */ rsi, rdx, rcx, r8, r9
};

// for documentation see bindJmpccFirst below
void
TranslatorX64::emitCondJmp(const SrcKey &skTrue, const SrcKey &skFalse) {
  // should be true for SrcKeys generated via OpJmpZ/OpJmpNZ
  ASSERT(skTrue.m_md5 == skFalse.m_md5);

  // reserve space for a smashable jnz/jmp pair; both initially point
  // to our stub
  prepareForSmash(kJmpLen + kJmpccLen);
  TCA old = a.code.frontier;

  moveToAlign(astubs);
  TCA stub = astubs.code.frontier;

  // begin code for the stub

  // We need to be careful here, as we are passing an extra paramter to
  //   REQ_BIND_JMPCC_FIRST. However we can't pass this parameter via
  //   emitServiceReq because that only supports constants/immediates, but
  //   in our case we need to compute the last argument via setcc.
  astubs.setnz(serviceReqArgRegs[3]);
  emitServiceReq(false /* align */, REQ_BIND_JMPCC_FIRST, 3,
                 old, skTrue.offset(), skFalse.offset());

  a.jnz(stub); // MUST use 4-byte immediate form
  a.jmp(stub); // MUST use 4-byte immediate form
}

static void skToName(const SrcKey& sk, char* name) {
  sprintf(name, "sk_%08lx:%05d",
          long(sk.m_md5.hash()), sk.offset());
}

static void skToClusterName(const SrcKey& sk, char* name) {
  sprintf(name, "skCluster_%08lx:%05d",
          long(sk.m_md5.hash()), sk.offset());
}


static void translToName(const TCA tca, char* name) {
  sprintf(name, "tc_%p", tca);
}

void TranslatorX64::drawCFG(std::ofstream& out) const {
  const char* indent = "    ";
  static int genCount;
  int numSrcKeys = 0;
  int numTranslations = 0;
  out << "digraph srcdb" << genCount++ <<" {\n";
  out << indent << "size = \"8,11\";\n";
  out << indent << "ratio = fill;\n";
  for (SrcDB::const_iterator entry = srcDB.begin();
       entry != srcDB.end(); ++entry) {
    const SrcKey& sk = entry->first;
    // 1 subgraph per srcKey.
    char name[64];
    skToClusterName(sk, name);
    numSrcKeys++;
    out << indent << "subgraph " << name << "{\n";
    char* indent = "        ";
    skToName(sk, name);
    out << indent << name << "[shape=box];\n";
    const vector<TCA>& transls = entry->second.translations;
    for (vector<TCA>::const_iterator t = transls.begin(); t != transls.end();
         ++t) {
      char transname[64];
      translToName(*t, transname);
      numTranslations++;
      out << indent << transname << "[fontsize=11.0];\n";
      out << indent << name << " -> " << transname << ";\n";
    }
    // And, all translations on the same line
    out << indent << "{ rank = same; ";
    out << name << " ";
    for (vector<TCA>::const_iterator t = transls.begin(); t != transls.end();
         ++t) {
      char transname[64];
      translToName(*t, transname);
      out << transname << " ";
    }
    out << indent << "}\n"; // subgraph
    out << indent << "}\n";
  }

  // OK! Those were all the nodes. Now edges. While edges are physically
  // from translation to translation, they're virtually from srcKey to
  // srcKey, and that is how the db represents them.
  for (SrcDB::const_iterator entry = srcDB.begin(); entry != srcDB.end();
       ++entry) {
    char destName[64];
    skToName(entry->first, destName);
    const vector<IncomingBranch>& ibs = entry->second.incomingBranches;
    out << indent << "// incoming branches to " << destName << "\n";
    for (vector<IncomingBranch>::const_iterator ib = ibs.begin();
         ib != ibs.end(); ++ib) {
      // Find the start of the translation that contains this branch
      const char *branchTypeToColorStr[] = {
        "black", // JMP
        "green", // JZ
        "red",   // JNZ
      };
      TransDB::const_iterator lowerTCA = transDB.lower_bound(ib->src);
      ASSERT(lowerTCA != transDB.end());
      char srcName[64];
      skToName(lowerTCA->second.src, srcName);
      out << indent << srcName << " -> " << destName << "[ color = " <<
        branchTypeToColorStr[ib->type] << "];\n";
    }
  }
  out << indent << "// " << numSrcKeys << " srckeys, " << numTranslations <<
    " tracelets\n";
  out << "}\n\n";
}

/*
 * bindJmp --
 *
 *   Runtime service handler that patches a jmp to the translation of
 *   u:dest from toSmash.
 */
TCA
TranslatorX64::bindJmp(TCA toSmash, SrcKey destSk) {
  TCA tDest = getTranslation(&destSk, false);
  if (!tDest) return NULL;
  SrcRec &destRec = getSrcRec(destSk);
  destRec.chainFrom(a, IncomingBranch::JMP, toSmash);
  return tDest;
}

/*
 * When we end a tracelet with a conditional jump, we (emitCondJmp) reserves
 * enough space for:
 *
 * jnz foo1
 * jmp foo1
 *
 * just in case. We have foo1 be a (hopefully far away) stub which will
 * eventually call bindJmpccFirst. Ideally we'll next transform this into:
 *
 * jnz foo2
 * // raw code for the first taken branch
 * // NOT aligned, as most likely the only entrance to this basic block is
 * //   the fall-through.
 *
 * where we at runtime choose between jz and jnz so that the first case that we
 * see is the fall-through. Foo2 is a stub which will eventually call
 * bindJmpccSecond. If the code frontier moved such that we can't build a
 * fall-through, we will instead emit:
 *
 * jnz foo2
 * jmp code
 *
 * // ... anything ...
 *
 * code:
 * // raw code for the first taken branch
 *
 * This way when we first execute the second branch we always smash a
 * __conditional__ jump, so that we can have foo2 eventually call
 * bindJmpccSecond in both cases.
 *
 * The least significant byte of toTake indicates which branch should be
 * taken.
 */

TCA
TranslatorX64::bindJmpccFirst(TCA toSmash,
                              Offset offTrue, Offset offFalse,
                              int64_t toTake) {
  const Unit* u = curUnit();
  if (!m_writeLease.acquire()) return NULL;
  SrcKey dest(u, (toTake & 1) ? offTrue : offFalse);

  TCA stub =
    emitServiceReq(TranslatorX64::REQ_BIND_JMPCC_SECOND, 3,
                   toSmash, (toTake & 1) ? offFalse : offTrue,
                   toTake & 1);
  // can we just directly fall through?
  // a jmp + jz takes 5 + 6 = 11 bytes
  bool fallThru = toSmash + kJmpccLen + kJmpLen == a.code.frontier &&
      srcDB.find(dest) == srcDB.end();

  TCA tDest;
  ASSERT(a.code.isValidAddress(toSmash));

  tDest = getTranslation(&dest, !fallThru /* align */);
  /*
   * Careful! We're modifying reachable code. As things stand, the code
   * looks like:
   *
   *     toSmash:    jz    <jmpccFirstStub>
   *     toSmash+6:  jmp   <jmpccFirstStub>
   *     toSmash+11: <probably the new translation == tdest>
   *
   * We're going to leave it as:
   *
   *     toSmash:    j[n]z <jmpccSecondStub>
   *     toSmash+6:  nop5
   *     toSmash+11: newHotness
   */
  CodeCursor cg(a, toSmash);
  // OpJmpNZ jumps if true
  if (toTake & 1) {
    a.jz(stub);
  } else {
    a.jnz(stub);
  }
  srcDB[dest].chainFrom(a, IncomingBranch::JMP, a.code.frontier);
  return tDest;
}

// smashes a jz/jnz to point to a new destination
TCA
TranslatorX64::bindJmpccSecond(TCA toSmash, const Offset off, bool isJz) {
  const Unit* u = curUnit();
  SrcKey dest(u, off);
  TCA branch = getTranslation(&dest, true);
  if (branch) {
    srcDB[dest].chainFrom(a, isJz ? IncomingBranch::JZ : IncomingBranch::JNZ,
      toSmash);
  }
  return branch;
}

/*
 * emitBindJmp --
 *
 *   Emit code to lazily branch to the srckey in next. Assumes current
 *   basic block is closed (outputs synced, etc.).
 */
void
TranslatorX64::emitBindJmp(const SrcKey& dest) {
  prepareForSmash(kJmpLen);
  TCA toSmash = a.code.frontier;
  a.   jmp(emitServiceReq(REQ_BIND_JMP, 2,
                          toSmash, dest.offset()));
}

void
TranslatorX64::checkType(const Location& l,
                         const RuntimeType& rtt,
                         SrcRec &fail) {
  // Homes don't require guards
  if (rtt.isHome()) return;

  PhysReg base;
  int disp = 0;
  SpaceRecorder sr("_CheckType", a);

  TRACE(1, Trace::prettyNode("Precond", DynLocation(l, rtt)) + "\n");

  locToRegDisp(l, &base, &disp);
  TRACE(2, "TypeCheck: %d(%%r%d)\n", disp, base);
  // Negative offsets from RSP are not yet allocated; they had
  // better not be inputs to the tracelet.
  ASSERT(l.space != Location::Stack || disp >= 0);
  if (rtt.isValue() && IS_STRING_TYPE(rtt.typeCheckValue())) {
    // Treat KindOfString and KindOfStaticString identically; they
    // are bitwise identical. This is a port of our IS_STRING_TYPE
    // macro to assembly, and will have to change in sync with it.
    ASSERT(IS_STRING_TYPE(7) && IS_STRING_TYPE(6));
    a.   load_reg64_disp_reg32(base, disp + rtt.typeCheckOffset(), rax);
    a.   and_imm32_reg32(~1, rax);
    a.   cmp_imm32_reg32(6, rax);
  } else {
    a.   cmp_imm32_disp_reg32(rtt.typeCheckValue(),
                              disp + rtt.typeCheckOffset(),
                              base);
  }
  emitFallbackJmp(fail);
  if (rtt.isValue() && rtt.outerType() == KindOfVariant) {
    // It's possible for the inner type to be unknown because we didn't use the
    // location until after an operation that invalidates inner types. Thus,
    // we only emit a guard for the inner type if it is known.
    if (rtt.innerType() != KindOfInvalid) {
      // If we are dealing with a var, we need to do a typecheck for the
      // inner type
      emitDispDeref(a, base, disp, rax);
      a.   cmp_imm32_disp_reg32(rtt.innerType(), offsetof(Cell, m_type), rax);
      emitFallbackJmp(fail);
    }
  }
}

void
TranslatorX64::emitFallbackJmp(SrcRec& dest) {
  prepareForSmash(kJmpccLen);
  dest.emitFallbackJump(a, IncomingBranch::JNZ);
}

static inline uint64_t
packBitVec(const vector<bool>& bits, unsigned i) {
  uint64_t retval = 0;
  ASSERT(i % 64 == 0);
  ASSERT(i < bits.size());
  while (i < bits.size()) {
    retval |= bits[i] << (i % 64);
    if ((++i % 64) == 0) {
      break;
    }
  }
  return retval;
}

void
TranslatorX64::checkRefs(const Tracelet& t, SrcRec& fail) {
  if (t.m_refDeps.m_mask.size() == 0) {
    return;
  }

  // We're still between BB's, so ignore the register allocator.
  PhysReg rNumParams = rdi;
  PhysReg rMask = rdx;
  PhysReg rBits = rax;
  PhysReg rExpectedBits = rcx;
  PhysReg rBitsValue = rsi;

  // Be careful! The actual Func might have fewer refs than the number
  // of args we're passing. To forestall this, we're going to have to
  // keep checking i against the number of params. We consider invocations
  // with too many arguments to have passed their checks.
  vector<JccBlock<CC_L> > skips;
  int32_t funcOff = cellsToBytes(t.m_arState.m_entryArDelta) +
    offsetof(ActRec, m_func);
  a.    load_reg64_disp_reg64(rVmSp, funcOff, rBits); // rBits <- Func*
  a.    load_reg64_disp_reg32(rBits, offsetof(Func, m_numParams),
                              rNumParams);
  a.    load_reg64_disp_reg64(rBits, offsetof(Func, m_refBitVec),
                              rBits);  // rBits <- m_refBitVec

  for (unsigned i = 0; i < t.m_refDeps.m_mask.size(); i += 64) {
    ASSERT(i < t.m_refDeps.m_vals.size());
    uint64_t mask = packBitVec(t.m_refDeps.m_mask, i);
    if (mask == 0) {
      continue;
    }
    uint64_t value = packBitVec(t.m_refDeps.m_vals, i);
    // Before trying to load this block off the bit vector, make sure
    // it actually exists.
    a.  cmp_imm32_reg32(i + 1, rNumParams);
    skips.push_back(JccBlock<CC_L>(a));
    // Load the appropriate qword off of the top actRec's func*.
    SKTRACE(2, t.m_sk, "reffiness mask %lx value %lx, ar @%d\n",
            mask, value, t.m_arState.m_entryArDelta);
    emitImmReg(a, mask,  rMask);
    emitImmReg(a, value, rExpectedBits);
    a.  load_reg64_disp_reg64(rBits, sizeof(uint64) * (i / 64),
                              rBitsValue);  // rBitsValue <- rBits[i / 64]
    a.  and_reg64_reg64(rMask, rBitsValue); // rBitsValue &= rMask
    a.  cmp_reg64_reg64(rBitsValue, rExpectedBits);
    emitFallbackJmp(fail);
  }
  // resolved: contents of the skips vector all jump here.
}

void TranslatorX64::emitRefTest(PhysReg rBitVec,
                                int argNum, TCA* outToSmash,
                                bool shouldBeRef) {
  // We're going to do this with a 32-bit immediate.
  static const int kBitsPerDword = 4;
  uint32_t disp = sizeof(uint32_t) * (argNum / kBitsPerDword);
  uint32_t bit = argNum % kBitsPerDword;
  uint32_t bitMask = 1u << bit;
  a.  test_imm32_disp_reg32(bitMask, disp, rBitVec);
  *outToSmash = a.code.frontier;
  if (shouldBeRef) {
    a.jnz(a.code.frontier);
  } else {
    a.jz(a.code.frontier);
  }
}


/*
 * emitRetFromInterpretedFrame --
 *
 *   When the interpreter pushes a call frame, there is necessarily no
 *   machine RIP available to return to. This helper fishes out the
 *   destination from the frame and redirects execution to it via enterTC.
 */
TCA
TranslatorX64::emitRetFromInterpretedFrame() {
  int32_t arBase = sizeof(ActRec) - sizeof(Cell);
  moveToAlign(astubs);
  TCA stub = astubs.code.frontier;
  // Marshall our own args by hand here.
  astubs.   lea_reg64_disp_reg64(rVmSp, -arBase, serviceReqArgRegs[0]);
  astubs.   mov_reg64_reg64(rVmFp, serviceReqArgRegs[1]);
  (void) emitServiceReq(false, REQ_POST_INTERP_RET, 0);
  return stub;
}

/*
 * enterTCHelper
 *
 * This helper routine is written in x64 assembly to take care of the details
 * when transferring control between jitted code and the translator.
 *   rdi:  Cell* vm_sp
 *   rsi:  Cell* vm_fp
 *   rdx:  unsigned char* start
 *   rcx:  uint64_t* requestNumAddr
 *   r8:   uintptr_t* argsAddr
 *   r9:   ActRec* firstARAddr
 */
asm (
  ".byte 0\n"
  ".align 16\n"
  "__enterTCHelper:\n"
  // Prologue
  // Define CFI rules for determining frame pointer and unwinding RBP
  ".cfi_startproc\n"
  "push %rbp\n"
  ".cfi_def_cfa_offset 16\n"
  ".cfi_offset rbp, -16\n"
  "mov %rsp, %rbp\n"
  ".cfi_def_cfa_register rbp\n"
  // Save all callee-saved registers
  "push %rbx\n"
  "push %r12\n"
  "push %r13\n"
  "push %r14\n"
  "push %r15\n"
  // Save requestNumAddr and argsAddr
  "push %r8\n"
  "push %rcx\n"
  // Call into jitted code
  "mov %rbp, (%r9)\n"
  "mov %rdi, %rbx\n"
  "mov %rsi, %rbp\n"
  "call *%rdx\n"
  // Restore requestNumAddr into r14 and argsAddr into r15
  "pop %r14\n"
  "pop %r15\n"
  // Copy the values passed from jitted code into *requestNumAddr and
  // *argsAddr
  "mov %rdi, 0x0(%r14);\n"
  "mov %rsi, 0x0(%r15);\n"
  "mov %rdx, 0x8(%r15);\n"
  "mov %rcx, 0x10(%r15);\n"
  "mov %r8,  0x18(%r15);\n"
  "mov %r9,  0x20(%r15);\n"
  // Restore callee-saved registers
  "pop %r15\n"
  "pop %r14\n"
  "pop %r13\n"
  "pop %r12\n"
  "pop %rbx\n"
  // Epilogue
  "pop %rbp\n"
  ".cfi_def_cfa rsp, 8\n"
  "ret\n"
  ".cfi_endproc\n"
);

void enterTCHelper(Cell* vm_sp, Cell* vm_fp,
                   unsigned char* start,
                   uint64_t* requestNumAddr,
                   uintptr_t* argsAddr,
                   ActRec* firstARAddr) asm ("__enterTCHelper");

struct DepthGuard {
  static __thread int m_depth;
  DepthGuard()  { m_depth++; TRACE(2, "DepthGuard: %d {\n", m_depth); }
  ~DepthGuard() { TRACE(2, "DepthGuard: %d }\n", m_depth); m_depth--; }
};
__thread int DepthGuard::m_depth;
void
TranslatorX64::enterTC(SrcKey sk) {
  ASSERT(rVmSp == rbx);
  ASSERT(rVmFp == rbp);
  TCA start = getTranslation(&sk, true);

  DepthGuard d;
  for (;;) {
    uint64_t  requestNum;
    uintptr_t args[5];
    ASSERT(vmfp() >= vmsp() - 1);
    ASSERT(sizeof(Cell) == 16);
    ASSERT(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
    ASSERT(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);

    TRACE(4, "enterTC: %p fp%p(%s) sp%p enter {\n", start,
          vmfp(), ((ActRec*)vmfp())->m_func->m_name->data(), vmsp());
    m_writeLease.gremlinUnlock();
    // Keep dispatching until we end up somewhere the translator
    // recognizes, or we luck out and the leaseholder exits.
    while (!start) {
      TRACE(4, "enterTC forwarding BB to interpreter\n");
      g_context->m_pc = curUnit()->at(sk.offset());
      g_context->dispatchBB();
      if (UNLIKELY(g_context->isHalted() || g_context->m_pc == NULL)) {
        return;
      }
      sk = SrcKey(curUnit(), g_context->m_pc);
      start = getTranslation(&sk, true);
    }
    ASSERT(start);
    g_context->m_isValid = 0;
    enterTCHelper(vmsp(), vmfp(), start, &requestNum,
                  args, vmFirstAR());
    // Debugging code: cede the write lease half the time.
    if (debug && (RuntimeOption::EvalJitStressLease)) {
      if (d.m_depth == 1 && (rand() % 2) == 0) {
        m_writeLease.gremlinLock();
      }
    }

    TRACE(4, "enterTC: %p fp%p sp%p } return\n", start,
             vmfp(), vmsp());
    if (g_context->isHalted()) {
      return;
    }
    TRACE(3, "enterTC: request(%ld) args: %lx %lx %lx %lx %lx\n", requestNum,
             args[0], args[1], args[2], args[3], args[4]);
    ASSERT(vmfp() >= vmsp() - 1 || requestNum == REQ_EXIT);

    if (debug) {
      // Ensure that each case either returns, or drives start to a valid
      // value.
      start = TCA(0xbee5face);
    }

    // The contract is that each case will either exit, by returning, or
    // set sk to the place where execution should resume, and optionally
    // set start to the hardware translation of the resumption point.
    //
    // start and sk might be subtly different; i.e., there are cases where
    // start != NULL && start != getTranslation(sk). For instance,
    // REQ_BIND_CALL has not finished executing the OpCall when it gets
    // here, and has even done some work on its behalf. sk == OpFCall,
    // while start == the point in the TC that's "half-way through" the
    // Call instruction. If we punt to the interpreter, the interpreter
    // will redo some of the work that the translator has already done.
    switch(requestNum) {
      case REQ_EXIT: {
        // fp is not valid anymore
        vmfp() = NULL;
        return;
      }

      case REQ_BIND_CALL: {
        ReqBindCall* req = (ReqBindCall*)args[0];
        TCA toSmash = req->m_toSmash;
        ASSERT(a.code.isValidAddress(toSmash));
        Func *func = req->m_func;
        int prologueFlags =
          (g_context->m_funcDict.getInterceptData(func).get() != NULL
           ? FuncPrologueIntercepted : FuncPrologueNormal);
        TRACE(4, "enterTC: bindCall %s\n", func->m_name->data());
        int nArgs = req->m_nArgs;
        TCA dest = tx64->funcPrologue(func, nArgs, prologueFlags);
        if (dest) {
          smash(tx64->a, toSmash, dest);
          // sk: stale, but doesn't matter since we have a valid dest TCA.
        } else {
          // We need translator help; we're not at the callee yet, so
          // roll back. The prelude has done some work already,
          sk = req->m_sourceInstr;
        }
        start = dest;
      } break;

      case REQ_BIND_JMP: {
        TCA toSmash = (TCA)args[0];
        Offset off = args[1];
        ASSERT(a.code.isValidAddress(toSmash));
        sk = SrcKey(curUnit(), off);
        ASSERT(a.code.isValidAddress(toSmash));
        start = bindJmp(toSmash, sk);
      } break;

      case REQ_BIND_JMPCC_FIRST: {
        TCA toSmash = (TCA)args[0];
        Offset offTrue = (Offset)args[1];
        Offset offFalse = (Offset)args[2];
        int64_t toTake = int64(args[3]);
        ASSERT(a.code.isValidAddress(toSmash));
        start = bindJmpccFirst(toSmash, offTrue, offFalse, toTake);
        // SrcKey: we basically need to emulate the fail
        sk = SrcKey(curUnit(), (toTake & 1) ? offTrue : offFalse);
      } break;

      case REQ_BIND_JMPCC_SECOND: {
        TCA toSmash = (TCA)args[0];
        Offset off = (Offset)args[1];
        bool isJz = (bool)args[2];
        ASSERT(a.code.isValidAddress(toSmash));
        start = bindJmpccSecond(toSmash, off, isJz);
        sk = SrcKey(curUnit(), off);
      } break;

      case REQ_RETRANSLATE: {
        sk = SrcKey(curUnit(), (Offset)args[0]);
        start = retranslate(sk, true);
        SKTRACE(2, sk, "retranslated @%p\n", start);
      } break;

      case REQ_INTERPRET: {
        Offset off = args[0];
        int numInstrs = args[1];
        g_context->m_pc = curUnit()->at(off);
        /*
         * We know the compilation unit has not changed; basic blocks do
         * not span files. I claim even exceptions do not violate this
         * axiom.
         */
        ASSERT(numInstrs > 0);
        ONTRACE(3, SrcKey(curUnit(), off).trace("interp: enter\n"));
        g_context->dispatchN(numInstrs);
        if (UNLIKELY(g_context->m_pc == NULL)) {
          return;
        }
        SrcKey newSk(curUnit(), g_context->m_pc);
        SKTRACE(3, newSk, "interp: exit\n");
        sk = newSk;
        start = getTranslation(&newSk, true);
      } break;

      case REQ_POST_INTERP_RET: {
        ActRec* ar = (ActRec*)args[0];
        ActRec* caller = (ActRec*)args[1];
        ASSERT((Cell*) ar < vmsp());
        ASSERT((Cell*) caller > vmsp());
        Unit* destUnit = caller->m_func->m_unit;
        PC destPC = destUnit->at(caller->m_func->m_base + ar->m_soff);
        SrcKey dest(destUnit, destPC);
        sk = dest;
        start = getTranslation(&dest, true);
        SKTRACE(3, sk, "PostInterpRet: to tca %p\n", start);
      } break;

      case REQ_RESUME: {
        SrcKey dest(curUnit(), vmpc());
        sk = dest;
        start = getTranslation(&dest, true);
      } break;

      case REQ_STACK_OVERFLOW: {
        raise_error("Stack overflow");
        return;
      }
    }
  }
  NOT_REACHED();
}

void TranslatorX64::resume(SrcKey sk) {
  class LeaseDropper {
    Lease &m_l;
   public:
    LeaseDropper(Lease& l) : m_l(l) { }
    ~LeaseDropper() {
      m_l.drop();
    }
  };
  // Be resilient to exceptions, etc.
  LeaseDropper ld(m_writeLease);
  enterTC(sk);
}

/*
 * emitServiceReq --
 *
 *   Call a translator service co-routine. The code emitted here is
 *   reenters the enterTC loop, invoking the requested service. Control
 *   will be returned non-locally to the next logical instruction in
 *   the TC.
 *
 *   Return value is a destination; we emit the bulky service
 *   request code into astubs.
 */

TCA
TranslatorX64::emitServiceReqVA(bool align, ServiceRequest req, int numArgs,
                                va_list args) {
  if (align) {
    moveToAlign(astubs);
  }
  TCA retval = astubs.code.frontier;
  WithVMRegsSaved wsv(astubs, false, false);
  /*
   * Move args into appropriate regs.
   */
  for (int i = 0; i < numArgs; i++) {
    uint64_t argVal = va_arg(args, uint64_t);
    emitImmReg(astubs, argVal, serviceReqArgRegs[i]);
  }
  emitImmReg(astubs, req, rdi);
  /*
   * Weird hand-shaking with enterTC: reverse-call a service routine.
   */
  astubs.    ret();
  translator_not_reached(astubs);
  return retval;
}

TCA
TranslatorX64::emitServiceReq(ServiceRequest req, int numArgs, ...) {
  va_list args;
  va_start(args, numArgs);
  TCA retval = emitServiceReqVA(true, req, numArgs, args);
  va_end(args);
  return retval;
}

TCA
TranslatorX64::emitServiceReq(bool align, ServiceRequest req, int numArgs,
                              ...) {
  va_list args;
  va_start(args, numArgs);
  TCA retval = emitServiceReqVA(align, req, numArgs, args);
  va_end(args);
  return retval;
}

void TranslatorX64::X64SpillFill::spillHome(RegAlloc& regMap,
                                            const Location& homeLoc,
                                            const Location& dest) {
  ASSERT(homeLoc.isValid());
  PhysReg base;
  int disp;
  ScratchReg scr(regMap);
  locToRegDisp(homeLoc, &base, &disp);
  a.   lea_reg64_disp_reg64(base, disp, *scr);
  spill(dest, KindOfHome, *scr, true);
  TRACE(1, "spillHome: saving Home @%d(%%r%d) to loc (%s, %d)\n",
        disp, base,
        dest.spaceName(), dest.offset);
}

void
TranslatorX64::X64SpillFill::spill(const Location& loc, DataType type,
                                   PhysReg reg, bool writeType) {
  PhysReg base;
  int disp;
  locToRegDisp(loc, &base, &disp);
  ASSERT_NOT_IMPLEMENTED(writeType);
  ASSERT(offsetof(Cell, _count) + 4 == offsetof(Cell, m_type));
  // Zero out the count at the same type as writing the type.
  const uint64 typeAndZero = uint64(type) << 32;
  SpaceRecorder sr("_Spill", a);

  PhysReg immReg = regAlloc.getImmReg(typeAndZero);
  if (immReg == RegAlloc::InvalidReg) {
    a.  store_imm_disp_reg64(typeAndZero, disp + offsetof(Cell, _count), base);
  } else {
    a.  store_reg64_disp_reg64(immReg, disp + offsetof(Cell, _count), base);
  }

  a.    store_reg64_disp_reg64(reg, disp, base);
  TRACE(2, "%s: (%s, %d) -> v: %d(r%d) type%d\n",
        __func__,
        loc.spaceName(), loc.offset, disp, base, type);
}

void
TranslatorX64::X64SpillFill::fill(const Location& loc, PhysReg reg) {
  SpaceRecorder sr("_Fill", a);
  PhysReg base;
  int disp;
  locToRegDisp(loc, &base, &disp);
  TRACE(2, "fill: (%s, %d) -> reg %d\n",
        loc.spaceName(), loc.offset, reg);
  a.    load_reg64_disp_reg64(base, disp, reg);
}

void
TranslatorX64::X64SpillFill::loadImm(int64 immVal, PhysReg reg) {
  TRACE(2, "loadImm: 0x%llx -> reg %d\n", immVal, reg);
  emitImmReg(a, immVal, reg);
}

// #define POISON
void
TranslatorX64::X64SpillFill::poison(PhysReg dest) {
#ifdef POISON
  emitImmReg(a, 0xbadf00d105e5babe, dest);
#endif
}

// syncOutputs --
//
//   Put live values to sleep once all of a tracelet's computation is
//   complete.
void
TranslatorX64::syncOutputs(const Tracelet& t) {
  SpaceRecorder sr("_SyncOuts", a);
  m_regMap.scrubStackEntries(t.m_stackChange);
  m_regMap.cleanAll();

  if (t.m_stackChange != 0) {
    TRACE(1, "syncOutputs: rVmSp + %d\n", t.m_stackChange);
    // t.stackChange is in negative Cells, not bytes.
    a.    add_imm64_reg64(-cellsToBytes(t.m_stackChange), rVmSp);
  }

}

/*
 * getBinaryStackInputs --
 *
 *   Helper for a common pattern of instruction, where two items are popped
 *   and one is pushed. The second item on the stack at the beginning of
 *   the instruction is both a source and destination.
 */
static void
getBinaryStackInputs(const RegAlloc& regmap, const NormalizedInstruction& i,
                     PhysReg& rsrc, PhysReg& rsrcdest) {
  ASSERT(i.inputs.size()   == 2);
  ASSERT(i.outStack && !i.outLocal);
  rsrcdest = regmap.getReg(i.outStack->location);
  rsrc     = regmap.getReg(i.inputs[0]->location);
  ASSERT(regmap.getReg(i.inputs[1]->location) == rsrcdest);
}

// emitBox --
//   Leave a boxed version of input in RAX. Destroys the register
//   mapping.
void
TranslatorX64::emitBox(DataType t, PhysReg rSrc) {
  if (false) { // typecheck
    TypedValue* retval = tvBoxHelper(KindOfArray, 0xdeadbeef01ul);
    (void)retval;
  }
  // tvBoxHelper will set the refcount of the inner cell to 1
  // for us. Because the inner cell now holds a reference to the
  // original value, we don't need to perform a decRef.
  EMIT_CALL2(tvBoxHelper, IMM(t), R(rSrc));
}

// emitUnboxTopOfStack --
//   Unbox the known-to-be Variant on top of stack in place.
void
TranslatorX64::emitUnboxTopOfStack(const NormalizedInstruction& i) {
  const vector<DynLocation*>& inputs  = i.inputs;

  ASSERT(inputs.size() == 1);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(inputs[0]->isStack());
  ASSERT(i.outStack && i.outStack->location == inputs[0]->location);
  DataType outType = inputs[0]->rtt.innerType();
  ASSERT(outType != KindOfInvalid);
  ASSERT(outType == i.outStack->outerType());
  PhysReg rSrc = getReg(inputs[0]->location);
  // Detach the register rSrc from the input location. We must
  // do this dance because the input and output location are the
  // same and we want to have separate registers for the input
  // and output.
  m_regMap.invalidate(inputs[0]->location);
  m_regMap.bind(rSrc, Location(), KindOfInvalid, RegInfo::SCRATCH);
  // This call to allocOutputRegs will allocate a new register
  // for the output location
  m_regMap.allocOutputRegs(i);
  PhysReg rDest = getReg(i.outStack->location);
  emitDeref(a, rSrc, rDest);
  emitIncRef(rDest, outType);
  // decRef the var on the evaluation stack
  emitDecRef(rSrc, KindOfVariant);
  m_regMap.freeScratchReg(rSrc);
}

// setOpOpToOpcodeOp --
//   The SetOp opcode space has nothing to do with the bytecode opcode
//   space. Reasonable people like it that way, so translate them here.
Opcode
setOpOpToOpcodeOp(SetOpOp soo) {
  switch(soo) {
#define SETOP_OP(_soo, _bc) case _soo: return _bc;
    SETOP_OPS
#undef SETOP_OP
    default: ASSERT(false);
  }
  return -1;
}

void
TranslatorX64::binaryIntegerArith(const NormalizedInstruction& i,
                                  Opcode op,
                                  PhysReg srcReg,
                                  PhysReg srcDestReg) {
  switch (op) {
#define CASEIMM(OpBc, x64op)                                       \
    case OpBc: {                                                   \
      if (i.constImmPos != NormalizedInstruction::kNoImm) {        \
        a.   x64op ## _imm64_reg64(i.constImm.u_I64A, srcDestReg); \
      } else {                                                     \
        a.   x64op ## _reg64_reg64(srcReg, srcDestReg);            \
      } } break;
#define CASE(OpBc, x64op)                                          \
    case OpBc: {                                                   \
        a.   x64op ## _reg64_reg64(srcReg, srcDestReg);            \
    } break;
    CASEIMM(OpAdd,    add)
    CASEIMM(OpSub,    sub)
    CASEIMM(OpBitAnd, and)
    CASEIMM(OpBitOr,  or)
    CASEIMM(OpBitXor, xor)
    CASE(OpMul,       imul)
#undef CASE
#undef CASEIMM

    default: {
      not_reached();
    };
  }
}

void
TranslatorX64::binaryArithCell(const NormalizedInstruction &i,
                               Opcode op, const DynLocation& in1,
                               const DynLocation& inout) {
  ASSERT(in1.rtt.isInt());
  ASSERT(inout.rtt.isInt());
  ASSERT(in1.outerType() != KindOfVariant);
  ASSERT(in1.isStack());
  ASSERT(inout.outerType() != KindOfVariant);
  ASSERT(inout.isStack());
  m_regMap.allocOutputRegs(i);
  PhysReg     srcReg = m_regMap.getReg(in1.location);
  PhysReg srcDestReg = m_regMap.getReg(inout.location);
  binaryIntegerArith(i, op, srcReg, srcDestReg);
}

void
TranslatorX64::binaryArithHome(const NormalizedInstruction &i,
                               Opcode op,
                               const DynLocation& in1,
                               const DynLocation& in2,
                               const DynLocation& out) {
  // The caller must guarantee that these conditions hold
  ASSERT(in1.rtt.isInt());
  ASSERT(in2.rtt.isInt());
  ASSERT(in1.outerType() != KindOfVariant);
  ASSERT(in1.isStack());
  ASSERT(in2.isLocal());
  ASSERT(out.isStack());

  PhysReg srcReg = m_regMap.getReg(in1.location);
  PhysReg outReg = m_regMap.getReg(out.location);
  PhysReg localReg = m_regMap.getReg(in2.location);
  if (in2.outerType() != KindOfVariant) {
    // The local is not a var, so we can operate directly on the
    // local's register. We will need to update outReg after the
    // operation.
    binaryIntegerArith(i, op, srcReg, localReg);
    // We operated directly on the local's register, so we need to update
    // outReg
    emitMovRegReg(localReg, outReg);
  } else {
    // The local is a var, so we have to read its value into outReg
    // on operate on that. We will need to write the result back
    // to the local after the operation.
    emitDeref(a, localReg, outReg);
    binaryIntegerArith(i, op, srcReg, outReg);
    // We operated on outReg, so we need to write the result back to the
    // local
    a.    store_reg64_disp_reg64(outReg, 0, localReg);
  }
}

static inline bool
opcodeChangesFrame(Opcode op) {
  return op == OpRetC || op == OpRetV || op == OpFCall ||
         op == OpThrow || op == OpExit || op == OpFatal;
}

#define O(opcode, imm, pusph, pop, flags) \
/**
 * The interpOne methods set the 'm_pc', 'm_fp', 'm_ar', and 'm_stack.m_top'
 * fields in ExecutionContext, call into the interpreter, and then return a
 * pointer to the current ExecutionContext.
 */ \
ExecutionContext* \
interpOne ## opcode (Offset off, \
                     ActRec *fp, TypedValue *sp) { \
  Fault fault; \
  ExecutionContext* ec = g_context.getNoCheck(); \
  try { \
    ec->m_fp = fp; \
    const Opcode* ip = curUnit()->at(off); \
    SKTRACE(3, SrcKey(curUnit(), ip), "%40s %p %p\n", \
            "interpOne" #opcode " before (fp,sp)", \
            fp, sp); \
    ASSERT(*ip == Op ## opcode); \
    vmsp() = sp; \
    ec->m_pc = ip; \
    ec->m_isValid = true; \
    ec->op##opcode(); \
    return ec; \
  } catch (Exception &e) { \
    if (!g_context->m_propagateException) { \
      fault.m_faultType = Fault::KindOfCPPException; \
      fault.m_cppException = e.clone(); \
    } \
  } catch (...) { \
    if (!g_context->m_propagateException) { \
      fault.m_faultType = Fault::KindOfCPPException; \
      fault.m_cppException = new Exception("unknown exception"); \
    } \
  } \
  if (!g_context->m_propagateException) {\
    g_context->m_faults.push_back(fault); \
  } \
  g_context->hhvmThrow(); \
  return ec; \
}
OPCODES
#undef O

void* interpOneEntryPoints[] = {
#define O(opcode, imm, pusph, pop, flags) \
  (void*)(interpOne ## opcode),
OPCODES
#undef O
};

void
TranslatorX64::emitInterpOne(const Tracelet& t,
                             const NormalizedInstruction &ni) {
  // Write any dirty values to memory
  m_regMap.cleanAll();
  // Call into the appropriate interpOne method. Note that this call will
  // preserve the callee-saved registers including rVmFp and rVmSp.
  if (false) { /* typecheck */
    Offset off = 1;
    ActRec fp;
    TypedValue sp;
    ExecutionContext* ec;
    ec = interpOnePopC(off, &fp, &sp);
    printf("%p\n", ec); // use ec
  }
  void* func = interpOneEntryPoints[ni.op()];
  TRACE(3, "ip %p of unit %p -> interpOne @%p\n", ni.pc(), ni.unit(), func);
  EMIT_CALL3(func,
             IMM((uintptr_t)ni.offset()),
             R(rVmFp),
             RPLUS(rVmSp, -cellsToBytes(ni.stackOff)));
  // The interpreter may have written to memory, so we need to invalidate
  // all locations
  m_regMap.reset();
  // The interpOne method returned a pointer to the current
  // ExecutionContext in rax, so we can read the 'm_*' fields
  // by adding the appropriate offset to rax and dereferencing.

  // If this instruction ends the tracelet, we have some extra work to do.
  if (ni.breaksBB) {
    // Read the 'm_fp' and 'm_stack.m_top' fields into the rVmFp and rVmSp
    // registers. Note that we have to do this _after_ spilling the homes
    // because the logic that spills the homes uses rVmSp.
    a.  load_reg64_disp_reg64(rax, offsetof(ExecutionContext, m_fp), rVmFp);
    a.  load_reg64_disp_reg64(rax, offsetof(ExecutionContext, m_stack) +
                              Stack::topOfStackOffset(), rVmSp);
    if (opcodeBreaksBB(ni.op())) {
      // If this instruction unpredictably changes m_pc, we emit a service
      // request to figure out where to go next.
      TCA stubDest = emitServiceReq(REQ_RESUME, 0);
      a.    jmp(stubDest);
    } else {
      // Otherwise, emit a service request to jump to the next instruction.
      SrcKey dest = ni.source;
      dest.advance(curUnit());
      emitBindJmp(dest);
    }
  } else {
    // All instructions that change the frame must break the basic block.
    // For instructions that do not change the frame or end the basic block,
    // we do not need to update rVmFp and rVmSp.
    ASSERT(!opcodeChangesFrame(ni.op()));
  }
}

static bool
isSupportedBinaryArithOp(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  return i.inputs[0]->isInt() &&
         i.inputs[1]->isInt();
}

void
TranslatorX64::analyzeBinaryArithOp(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedBinaryArithOp(i);
}

void
TranslatorX64::translateBinaryArithOp(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  const Opcode op = i.op();
  ASSERT(op == OpSub || op == OpMul || op == OpBitAnd ||
         op == OpBitOr || op == OpBitXor);
  ASSERT(isSupportedBinaryArithOp(i));
  ASSERT(i.inputs.size() == 2);

  binaryArithCell(i, op, *i.inputs[0], *i.outStack);
}

static inline bool sameDataTypes(DataType t1, DataType t2) {
  return TypeConstraint::equivDataTypes(t1, t2);
}

static bool
isSupportedSameOp_SameTypes(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  const RuntimeType& left = i.inputs[0]->rtt;
  const RuntimeType& right = i.inputs[1]->rtt;
  DataType leftType = left.outerType();
  DataType rightType = right.outerType();
  return sameDataTypes(leftType, rightType) &&
         (left.isNull() || leftType == KindOfBoolean ||
          left.isInt() || left.isString());
}

static bool
isSupportedSameOp_DifferentTypes(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  DataType leftType = i.inputs[0]->outerType();
  DataType rightType = i.inputs[1]->outerType();
  return !sameDataTypes(leftType, rightType);
}

void
TranslatorX64::analyzeSameOp(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedSameOp_SameTypes(i) ||
                  isSupportedSameOp_DifferentTypes(i);
}

void
TranslatorX64::translateSameOp(const Tracelet& t,
                               const NormalizedInstruction& i) {
  const Opcode op = i.op();
  ASSERT(op == OpSame || op == OpNSame);
  const vector<DynLocation*>& inputs  = i.inputs;
  bool instrNeg = (op == OpNSame);
  ASSERT(inputs.size() == 2);
  ASSERT(i.outStack && !i.outLocal);
  DataType leftType = i.inputs[0]->outerType();
  DataType rightType = i.inputs[1]->outerType();
  ASSERT(leftType != KindOfVariant);
  ASSERT(rightType != KindOfVariant);

  if (isSupportedSameOp_DifferentTypes(i)) {
    // Some easy cases: when the valueTypes do not match,
    // NSame -> true and Same -> false.
    SKTRACE(1, i.source, "different types %d %d\n",
            leftType, rightType);
    // decRef the left hand side and right hand side as appropriate
    PhysReg src, srcdest;
    getBinaryStackInputs(m_regMap, i, src, srcdest);
    emitDecRef(src, leftType);
    emitDecRef(srcdest, rightType);
    m_regMap.allocOutputRegs(i);
    emitImmReg(a, instrNeg, getReg(i.outStack->location));
    return; // Done
  }

  ASSERT(isSupportedSameOp_SameTypes(i));

  m_regMap.allocOutputRegs(i);

  if (IS_NULL_TYPE(leftType)) {
    // null === null is always true
    SKTRACE(2, i.source, "straightening null/null comparison\n");
    emitImmReg(a, !instrNeg, getReg(i.outStack->location));
    return; // Done
  }
  if (IS_STRING_TYPE(leftType)) {
    EMIT_CALL2(same_str_str,
               V(inputs[0]->location),
               V(inputs[1]->location));
    if (instrNeg) {
      a.  xor_imm32_reg32(1, rax);
    }
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
    return; // Done
  }
  PhysReg src, srcdest;
  getBinaryStackInputs(m_regMap, i, src, srcdest);
  ASSERT(getReg(i.outStack->location) == srcdest);
  a.   cmp_reg64_reg64(src, srcdest);
  if (op == OpSame) {
    a. setz           (srcdest);
  } else {
    a. setnz          (srcdest);
  }
  a.   mov_reg8_reg64_unsigned(srcdest, srcdest);
}

static bool
trivialEquivType(const RuntimeType& rtt) {
  DataType t = rtt.valueType();
  return t == KindOfUninit || t == KindOfNull || t == KindOfBoolean ||
    rtt.isInt() || rtt.isString();
}

static bool
isSupportedEqOp(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  return trivialEquivType(i.inputs[0]->rtt) &&
         trivialEquivType(i.inputs[1]->rtt);
}

static void
emitConvertToBool(X64Assembler &a, PhysReg src, PhysReg dest, bool instrNeg) {
  a.    test_reg64_reg64(src, src);
  if (instrNeg) {
    a.  setz(dest);
  } else {
    a.  setnz(dest);
  }
  a.    mov_reg8_reg64_unsigned(dest, dest);
}

void
TranslatorX64::analyzeEqOp(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedEqOp(i);
}

void
TranslatorX64::translateEqOp(const Tracelet& t,
                             const NormalizedInstruction& i) {
  const Opcode op = i.op();
  ASSERT(op == OpEq || op == OpNeq);
  ASSERT(isSupportedEqOp(i));
  const vector<DynLocation*>& inputs  = i.inputs;
  bool instrNeg = (op == OpNeq);
  ASSERT(inputs.size() == 2);
  ASSERT(i.outStack && !i.outLocal);
  DataType leftType = i.inputs[0]->outerType();
  DataType rightType = i.inputs[1]->outerType();
  ASSERT(leftType != KindOfVariant);
  ASSERT(rightType != KindOfVariant);

  m_regMap.allocOutputRegs(i);

  if (IS_STRING_TYPE(leftType) || IS_STRING_TYPE(rightType)) {
    void* fptr = NULL;
    bool leftIsString = false;
    bool eqNullStr = false;
    switch (leftType) {
      STRINGCASE(): {
        leftIsString = true;
        switch (rightType) {
          STRINGCASE(): fptr = (void*)eq_str_str; break;
          INTCASE(): fptr = (void*)eq_int_str; break;
          case KindOfBoolean: fptr = (void*)eq_bool_str; break;
          NULLCASE(): fptr = (void*)eq_null_str; eqNullStr = true; break;
          default: ASSERT(false); break;
        }
      } break;
      INTCASE(): fptr = (void*)eq_int_str; break;
      case KindOfBoolean: fptr = (void*)eq_bool_str; break;
      NULLCASE(): fptr = (void*)eq_null_str; eqNullStr = true; break;
      default: ASSERT(false); break;
    }
    if (eqNullStr) {
      ASSERT(fptr == (void*)eq_null_str);
      EMIT_CALL1(fptr,
                 V(inputs[leftIsString ? 0 : 1]->location));
    } else {
      ASSERT(fptr != NULL);
      EMIT_CALL2(fptr,
                 V(inputs[leftIsString ? 1 : 0]->location),
                 V(inputs[leftIsString ? 0 : 1]->location));
    }
    if (instrNeg) {
      a.  xor_imm32_reg32(1, rax);
    }
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
    return;
  }

  PhysReg src, srcdest;
  getBinaryStackInputs(m_regMap, i, src, srcdest);
  ASSERT(getReg(i.outStack->location) == srcdest);
  if (IS_NULL_TYPE(leftType) && IS_NULL_TYPE(rightType)) {
    // null == null is always true
    SKTRACE(2, i.source, "straightening null/null comparison\n");
    emitImmReg(a, !instrNeg, getReg(i.outStack->location));
    return; // Done
  }
  if (IS_NULL_TYPE(leftType) || IS_NULL_TYPE(rightType)) {
    if (IS_NULL_TYPE(leftType)) {
      a.   test_reg64_reg64(srcdest, srcdest);
    } else {
      a.   test_reg64_reg64(src, src);
    }
  } else if (leftType  == KindOfBoolean ||
             rightType == KindOfBoolean) {
    ScratchReg tmp(m_regMap);
    PhysReg src2 = src;
    if (leftType != KindOfBoolean) {
      emitConvertToBool(a, src, *tmp, false);
      src2 = *tmp;
    }
    if (rightType != KindOfBoolean)
      emitConvertToBool(a, srcdest, srcdest, false);
    a.     cmp_reg64_reg64(src2, srcdest);
  } else {
    a.     cmp_reg64_reg64(src, srcdest);
  }
  if (instrNeg) {
    a.     setnz          (srcdest);
  } else {
    a.     setz           (srcdest);
  }
  a.       mov_reg8_reg64_unsigned(srcdest, srcdest);
}

static bool
isSupportedLtGtOp(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  const RuntimeType& left = i.inputs[0]->rtt;
  DataType leftType = left.outerType();
  DataType rightType = i.inputs[1]->outerType();
  return sameDataTypes(leftType, rightType) &&
         (left.isNull() || leftType == KindOfBoolean || left.isInt());
}

void
TranslatorX64::analyzeLtGtOp(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedLtGtOp(i);
}

void
TranslatorX64::translateLtGtOp(const Tracelet& t,
                             const NormalizedInstruction& i) {
  const Opcode op = i.op();
  ASSERT(op == OpLt || op == OpLte || op == OpGt || op == OpGte);
  ASSERT(isSupportedLtGtOp(i));
  ASSERT(i.inputs.size() == 2);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.inputs[0]->outerType() != KindOfVariant);
  ASSERT(i.inputs[1]->outerType() != KindOfVariant);

  bool fEquals = (op == OpLte || op == OpGte);
  bool fLessThan = (op == OpLt || op == OpLte);

  m_regMap.allocOutputRegs(i);
  if (IS_NULL_TYPE(i.inputs[0]->outerType())) {
    // null < null is always false, null <= null is always true
    SKTRACE(2, i.source, "straightening null/null comparison\n");
    emitImmReg(a, (op == OpLte || op == OpGte),
               getReg(i.outStack->location));
    return; // Done
  }
  PhysReg src, srcdest;
  getBinaryStackInputs(m_regMap, i, src, srcdest);
  ASSERT(getReg(i.outStack->location) == srcdest);
  if (fLessThan) {
    a.     cmp_reg64_reg64(src, srcdest);
  } else {
    a.     cmp_reg64_reg64(srcdest, src);
  }
  if (fEquals) {
    a.     setle          (srcdest);
  } else {
    a.     setl           (srcdest);
  }
  a.       mov_reg8_reg64_unsigned(srcdest, srcdest);
}

static bool
isSupportedUnaryBooleanOp(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  RuntimeType& rtt = i.inputs[0]->rtt;
  DataType inType = rtt.valueType();
  return (inType == KindOfUninit || inType == KindOfNull ||
          inType == KindOfBoolean || rtt.isInt() ||
          rtt.isString() || inType == KindOfArray);
}

void
TranslatorX64::analyzeUnaryBooleanOp(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedUnaryBooleanOp(i);
}

void
TranslatorX64::translateUnaryBooleanOp(const Tracelet& t,
                                       const NormalizedInstruction& i) {
  const Opcode op = i.op();
  ASSERT(op == OpCastBool || op == OpEmptyH);
  const vector<DynLocation*>& inputs  = i.inputs;
  ASSERT(inputs.size() == 1);
  ASSERT(i.outStack && !i.outLocal);
  bool instrNeg = (op == OpEmptyH);
  DataType inType = inputs[0]->valueType();
  const Location& inLoc = inputs[0]->location;
  bool boxedForm = (inputs[0]->outerType() == KindOfVariant);

  ASSERT(isSupportedUnaryBooleanOp(i));

  switch (inType) {
    NULLCASE(): {
      m_regMap.allocOutputRegs(i);
      PhysReg outReg = getReg(i.outStack->location);
      emitImmReg(a, instrNeg, outReg);
    } break;
    case KindOfBoolean: {
      if (op == OpCastBool) {
        // Casting bool to bool is a nop.  CastBool's input must be
        // a cell on the stack as per the bytecode specification.
        ASSERT(inputs[0]->isStack());
        ASSERT(inputs[0]->outerType() != KindOfVariant);
        ASSERT(inputs[0]->location.space == Location::Stack);
        break;
      }
      m_regMap.allocOutputRegs(i);
      PhysReg reg = getReg(inLoc);
      PhysReg outReg = getReg(i.outStack->location);
      if (boxedForm) {
        emitDeref(a, reg, outReg);
      } else {
        emitMovRegReg(reg, outReg);
      }
      if (instrNeg) {
        a.  xor_imm32_reg32(1, outReg);
      }
    } break;
    INTCASE(): {
      m_regMap.allocOutputRegs(i);
      PhysReg reg = getReg(inLoc);
      PhysReg outReg = getReg(i.outStack->location);
      ScratchReg scratch(m_regMap);
      if (boxedForm) {
        emitDeref(a, reg, *scratch);
        emitConvertToBool(a, *scratch, outReg, instrNeg);
      } else {
        emitConvertToBool(a, reg, outReg, instrNeg);
      }
    } break;
    STRINGCASE():
    case KindOfArray: {
      bool doDecRef = (inputs[0]->isStack());
      void* fptr = IS_STRING_TYPE(inType) ?
          (doDecRef ? (void*)str_to_bool : (void*)str0_to_bool) :
          (doDecRef ? (void*)arr_to_bool : (void*)arr0_to_bool);
      if (boxedForm) {
        EMIT_CALL1(fptr, DEREF(inLoc));
      } else {
        EMIT_CALL1(fptr, V(inLoc));
      }
      if (instrNeg) {
        a.    xor_imm32_reg32(1, rax);
      }
      m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                    RegInfo::DIRTY);
    } break;
    default: {
      ASSERT(false);
    } break;
  }
}

void
TranslatorX64::analyzeBranchOp(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

// Helper for decoding dests of branch-like instructions.
void branchDests(const NormalizedInstruction& i,
                 SrcKey* outNotTaken, SrcKey* outTaken,
                 int immIdx = 0) {
  *outNotTaken = i.source;
  outNotTaken->advance(curUnit());

  int dest = i.imm[immIdx].u_BA;
  *outTaken = SrcKey(curUnit(), i.offset() + dest);
}
void
TranslatorX64::translateBranchOp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  const Opcode op = i.op();
  ASSERT(op == OpJmpZ || op == OpJmpNZ);
  ASSERT(i.inputs.size()  == 1);
  ASSERT(!i.outStack && !i.outLocal);
  m_regMap.allocOutputRegs(i);
  const DynLocation& in = *i.inputs[0];
  const RuntimeType& rtt = in.rtt;
  const Location& inLoc = in.location;
  DataType inputType = rtt.outerType();
  PhysReg src = getReg(inLoc);
  /*
   * Careful. We're operating with intimate knowledge of the
   * constraints of the register allocator from here out.
   */
  if (rtt.isString() || inputType == KindOfArray) {
    // str_to_bool and arr_to_bool will decRef for us
    void* fptr = IS_STRING_TYPE(inputType) ? (void*)str_to_bool :
                                               (void*)arr_to_bool;
    EMIT_CALL1(fptr, V(inLoc));
    src = rax;
  } else if (inputType != KindOfUninit &&
             inputType != KindOfNull &&
             inputType != KindOfBoolean &&
             !rtt.isInt()) {
    // input might be in-flight
    {
      std::set<PhysReg> regs;
      regs.insert(m_regMap.getReg(inLoc));
      m_regMap.cleanRegs(regs);
    }
    // Cast to a bool.
    if (false) {
      TypedValue *tv = NULL;
      int64 ret = tv_to_bool(tv);
      if (ret) {
        printf("zoot");
      }
    }
    TRACE(2, Trace::prettyNode("tv_to_bool", inLoc) + string("\n"));
    // tv_to_bool will decRef for us if appropriate
    EMIT_CALL1(tv_to_bool, A(inLoc));
    /*
     * It's ok to hold onto rax; it has to be free, thanks to
     * EMIT_CALL*, and it won't get recycled before we test it below,
     * because we're careful not to allocate any registers between here
     * and there.
     */
    src = rax;
  }

  syncOutputs(t);

  // not_taken
  SrcKey notTaken, taken;
  branchDests(i, &notTaken, &taken);

  // Since null always evaluates to false, we can emit an
  // unconditional jump. OpJmpNZ will never take the branch
  // while OpJmpZ will always take the branch.
  if (IS_NULL_TYPE(inputType)) {
    TRACE(1, "branch on Null -> always Z\n");
    emitBindJmp(op == OpJmpNZ ? notTaken : taken);
    return;
  }
  a.    test_reg64_reg64(src, src);
  // OpJmpNZ jumps if true, OpJmpZ jumps if false
  if (op == OpJmpNZ) {
    emitCondJmp(taken, notTaken);
  } else {
    emitCondJmp(notTaken, taken);
  }
}

void
TranslatorX64::analyzeCGetHOp(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateCGetHOp(const Tracelet& t,
                                const NormalizedInstruction& i) {
  const UNUSED Opcode op = i.op();
  ASSERT(op == OpCGetH || op == OpCGetH2);
  const vector<DynLocation*>& inputs  = i.inputs;
  ASSERT(inputs.size() == 1);
  ASSERT(i.outStack);
  ASSERT(inputs[0]->isLocal());
  PhysReg localReg = getReg(inputs[0]->location);
  DataType outType = inputs[0]->valueType();
  ASSERT(outType != KindOfInvalid);
  ASSERT(outType == i.outStack->outerType());
  m_regMap.allocOutputRegs(i);
  PhysReg dest = getReg(i.outStack->location);
  // We know the output type, so operate on the registers
  if (inputs[0]->outerType() != KindOfVariant) {
    emitMovRegReg(localReg, dest);
  } else {
    emitDeref(a, localReg, dest);
  }
  emitIncRef(dest, outType);
}

void
TranslatorX64::analyzeVGetH(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateVGetH(const Tracelet& t,
                              const NormalizedInstruction& i) {
  const UNUSED Opcode op = i.op();
  ASSERT(op == OpVGetH);
  const vector<DynLocation*>& inputs  = i.inputs;
  ASSERT(inputs.size() == 1);
  ASSERT(i.outStack);
  ASSERT(inputs[0]->isLocal());
  ASSERT(inputs[0]->rtt.valueType() != KindOfInvalid);
  ASSERT(i.outStack->rtt.outerType() == KindOfVariant);

  PhysReg localReg = getReg(inputs[0]->location);
  PhysReg dest;
  if (inputs[0]->rtt.outerType() != KindOfVariant) {
    emitBox(inputs[0]->rtt.outerType(), localReg);
    m_regMap.bind(rax, inputs[0]->location, KindOfVariant,
                  RegInfo::DIRTY);
    m_regMap.allocOutputRegs(i);
    dest = getReg(i.outStack->location);
    emitMovRegReg(rax, dest);
  } else {
    m_regMap.allocOutputRegs(i);
    dest = getReg(i.outStack->location);
    emitMovRegReg(localReg, dest);
  }
  emitIncRef(dest, KindOfVariant);
}

void
TranslatorX64::analyzeAssignToLocalOp(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateAssignToLocalOp(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  const Opcode op = i.op();
  ASSERT(op == OpSetH || op == OpBindH);
  const vector<DynLocation*>& inputs  = i.inputs;
  ASSERT(inputs.size() >= 2);
  ASSERT(i.outLocal);
  const int valIdx  = 0;
  const int homeIdx = 1;

  ASSERT(inputs[homeIdx]->isLocal());
  ASSERT(inputs[valIdx]->isStack());
  ASSERT((op == OpBindH) ==
         (inputs[valIdx]->outerType() == KindOfVariant));
  m_regMap.allocOutputRegs(i);
  PhysReg val = getReg(inputs[valIdx]->location);
  PhysReg localReg = getReg(inputs[homeIdx]->location);
  ASSERT(localReg != val);

  // Increment the refcount of val, since we're about to
  // write val to the local
  if (i.outStack) {
    emitIncRef(val, inputs[valIdx]->outerType());
  }
  DataType localType = inputs[homeIdx]->outerType();
  if (op == OpSetH && localType == KindOfVariant) {
    DataType innerType = inputs[homeIdx]->valueType();
    // decRef the old value if appropriate because we are about to
    // overwrite it
    if (IS_REFCOUNTED_TYPE(innerType)) {
      ScratchReg scratch(m_regMap);
      emitDeref(a, localReg, *scratch);
      emitDecRef(*scratch, innerType);
    }
    emitTypedValueStore(a, inputs[valIdx]->outerType(), val, 0,
                        localReg);
  } else {
    // decRef the old value if appropriate because we are about to
    // overwrite it
    emitDecRef(localReg, localType);
    // XXX We can do better here - we could simply manipulate the register
    // allocator's mappings so that the Location that used to be assoicated
    // with 'home' is set to be associated with 'val'
    {
      SpaceRecorder("_RegMove", a);
      emitMovRegReg(val, localReg);
    }
  }
  if (i.outStack) {
    PhysReg stackOutReg = getReg(i.outStack->location);
    // XXX We can do better here - we could simply manipulate the register
    // allocator's mappings so that the Location that used to be assoicated
    // with 'home' is set to be associated with 'val'
    SpaceRecorder("_RegMove", a);
    emitMovRegReg(val, stackOutReg);
  }
}

void
TranslatorX64::analyzePopC(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translatePopC(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  DataType type = i.inputs[0]->outerType();
  PhysReg srcReg = getReg(i.inputs[0]->location);
  emitDecRef(srcReg, type);
}

void
TranslatorX64::analyzePopV(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translatePopV(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  ASSERT(!i.outStack && !i.outLocal);
  ASSERT(i.inputs[0]->rtt.isVariant());
  emitDecRef(getReg(i.inputs[0]->location), KindOfVariant);
}

void
TranslatorX64::analyzePopR(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translatePopR(const Tracelet& t,
                             const NormalizedInstruction& i) {
  // Type knowledge lets us treat PopR and PopC identically.
  translatePopC(t, i);
}

void
TranslatorX64::analyzeUnboxR(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateUnboxR(const Tracelet& t,
                               const NormalizedInstruction& i) {
  // If the value on the top of a stack is a var, unbox it and
  // leave it on the top of the stack.
  if (i.inputs[0]->isVariant()) {
    emitUnboxTopOfStack(i);
  }
}

void
TranslatorX64::analyzeNull(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateNull(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 0);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.outStack->outerType() == KindOfNull);

  // We have to mark the output register as dirty to ensure that
  // the type gets spilled at the tend of the tracelet
  m_regMap.allocOutputRegs(i);

  /* nop */
}

void
TranslatorX64::analyzeTrue(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateTrue(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 0);
  ASSERT(i.outStack && !i.outLocal);
  m_regMap.allocOutputRegs(i);
  PhysReg rdest = getReg(i.outStack->location);
  emitImmReg(a, 1, rdest);
}

void
TranslatorX64::analyzeFalse(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateFalse(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 0);
  ASSERT(i.outStack && !i.outLocal);
  m_regMap.allocOutputRegs(i);
  PhysReg dest = getReg(i.outStack->location);
  emitImmReg(a, false, dest);
}

void
TranslatorX64::analyzeInt(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateInt(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  ASSERT(i.inputs.size()  == 0);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.outStack->isInt());
  m_regMap.allocOutputRegs(i);
  PhysReg dest = getReg(i.outStack->location);
  uint64_t srcImm = i.imm[0].u_I64A;
  emitImmReg(a, srcImm, dest);
}

void
TranslatorX64::analyzeString(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateString(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  ASSERT(i.inputs.size()  == 0);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(Translator::typeIsString(i.outStack->outerType()));
  m_regMap.allocOutputRegs(i);
  PhysReg dest = getReg(i.outStack->location);
  uint64_t srcImm = (uintptr_t)curUnit()->lookupLitstrId(i.imm[0].u_SA);
  // XXX: can simplify the lookup here by just fishing it out of the
  // output's valueString().
  // We are guaranteed that the string is static, so we do not need to
  // increment the refcount
  ASSERT(((StringData*)srcImm)->isStatic());
  SKTRACE(2, i.source, "Litstr %d -> %p \"%s\"\n",
      i.imm[0].u_SA, (StringData*)srcImm,
      Util::escapeStringForCPP(((StringData*)srcImm)->data()).c_str());
  emitImmReg(a, srcImm, dest);
}

void
TranslatorX64::analyzeArray(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateArray(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 0);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.outStack->outerType() == KindOfArray);
  m_regMap.allocOutputRegs(i);
  ArrayData* ad = curUnit()->lookupArrayId(i.imm[0].u_AA);
  PhysReg r = getReg(i.outStack->location);
  emitImmReg(a, uint64(ad), r);
  // We are guaranteed that the array is static, so we do not need to
  // increment the refcount
  ASSERT(ad->isStatic());
}

void
TranslatorX64::analyzeNewArray(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateNewArray(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 0);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.outStack->outerType() == KindOfArray);
  m_regMap.allocOutputRegs(i);
  PhysReg r = getReg(i.outStack->location);
  emitImmReg(a, uint64(StaticEmptyHphpArray::Get()), r);
  // We are guaranteed that the new array is static, so we do not need to
  // increment the refcount
  ASSERT(StaticEmptyHphpArray::Get()->isStatic());
}

static bool
isSupportedInstrAddElemC(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 3);
  return i.inputs[2]->outerType() == KindOfArray &&
    (i.inputs[1]->isInt() || i.inputs[1]->isString());
}

void
TranslatorX64::analyzeAddElemC(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrAddElemC(i);
}

void
TranslatorX64::translateAddElemC(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.inputs.size() >= 3);
  const DynLocation& arr = *i.inputs[2];
  const DynLocation& key = *i.inputs[1];
  const DynLocation& val = *i.inputs[0];
  ASSERT(!arr.isVariant()); // not handling variants.
  ASSERT(!key.isVariant());
  ASSERT(!val.isVariant());

  const Location& arrLoc = arr.location;
  const Location& keyLoc = key.location;
  const Location& valLoc = val.location;

  ASSERT(arrLoc.isStack());
  ASSERT(keyLoc.isStack());
  ASSERT(arrLoc.isStack());

  // We only support translation under these conditions
  ASSERT(isSupportedInstrAddElemC(i));

  // If either the key or the rhs is not Int64, we will need to pass the
  // rhs by address, so we need to sync it back to memory
  if (!key.rtt.isInt() || !val.rtt.isInt()) {
    std::set<PhysReg> regs;
    regs.insert(m_regMap.getReg(valLoc));
    m_regMap.cleanRegs(regs);
  }

  WithVMRegsSavedStub sav(a, i, m_saveVMRegsStub);

  // The array_setm helpers will decRef any old value that is overwritten if
  // appropriate. If copy-on-write occurs, it will also incRef the new array
  // and decRef the old array for us. Finally, some of the array_setm helpers
  // will decRef the key if it is a string (for cases where the key is not
  // a home), while others do not (for cases where the key is a home).
  void* fptr;
  if (key.rtt.isInt() && val.rtt.isInt()) {
    if (false) { // type-check
      TypedValue* cell = NULL;
      ArrayData* arr = NULL;
      ArrayData* ret = array_setm_ik1_iv(cell, arr, 12, 3);
      printf("%p", ret); // use ret
    }
    // If the rhs is Int64, we can use a specialized helper
    EMIT_CALL4(array_setm_ik1_iv,
               IMM(0),
               V(arrLoc),
               V(keyLoc),
               V(valLoc));
  } else if (key.rtt.isInt() || key.rtt.isString()) {
    if (false) { // type-check
      TypedValue* cell = NULL;
      TypedValue* rhs = NULL;
      StringData* strkey = NULL;
      ArrayData* arr = NULL;
      ArrayData* ret;
      ret = array_setm_ik1_v0(cell, arr, 12, rhs);
      printf("%p", ret); // use ret
      ret = array_setm_sk1_v0(cell, arr, strkey, rhs);
      printf("%p", ret); // use ret
      ret = array_setm_s0k1_v0(cell, arr, strkey, rhs);
      printf("%p", ret); // use ret
    }
    // Otherwise, we pass the rhs by address
    fptr = key.rtt.isString() ? (void*)array_setm_sk1_v0 :
      (void*)array_setm_ik1_v0;
    EMIT_CALL4(fptr,
               IMM(0),
               V(arrLoc),
               V(keyLoc),
               A(valLoc));
  } else {
    ASSERT(false);
  }
  // The array value may have changed, so we need to invalidate any
  // register we have associated with arrLoc
  m_regMap.invalidate(arrLoc);
  // The array_setm helper returns the up-to-date array pointer in rax.
  // Therefore, we can bind rax to arrLoc and mark it as dirty.
  m_regMap.bind(rax, arrLoc, KindOfArray, RegInfo::DIRTY);
}

static bool
isSupportedInstrAddNewElemC(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  return (i.inputs[1]->outerType() == KindOfArray);
}

void
TranslatorX64::analyzeAddNewElemC(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrAddNewElemC(i);
}

void
TranslatorX64::translateAddNewElemC(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.inputs[0]->outerType() != KindOfVariant);
  ASSERT(i.inputs[1]->outerType() != KindOfVariant);
  ASSERT(i.inputs[0]->isStack());
  ASSERT(i.inputs[1]->isStack());

  // We only support translation under these conditions
  ASSERT(isSupportedInstrAddNewElemC(i));

  Location arrLoc = i.inputs[1]->location;
  Location valLoc = i.inputs[0]->location;

  // We pass the rhs by address, so we need to sync it back to memory
  std::set<PhysReg> regs;
  regs.insert(m_regMap.getReg(valLoc));
  m_regMap.cleanRegs(regs);

  WithVMRegsSavedStub sav(a, i, m_saveVMRegsStub);

  // The array_setm helpers will decRef any old value that is overwritten if
  // appropriate. If copy-on-write occurs, it will also incRef the new array
  // and decRef the old array for us. Finally, some of the array_setm helpers
  // will decRef the key if it is a string (for cases where the key is not
  // a home), while others do not (for cases where the key is a home).
  if (false) { // type-check
    TypedValue* cell = NULL;
    TypedValue* rhs = NULL;
    ArrayData* arr = NULL;
    ArrayData* ret;
    ret = array_setm_wk1_v0(cell, arr, rhs);
    printf("%p", ret); // use ret
  }
  EMIT_CALL3(array_setm_wk1_v0,
             IMM(0),
             V(arrLoc),
             A(valLoc));
  // The array value may have changed, so we need to invalidate any
  // register we have associated with arrLoc
  m_regMap.invalidate(arrLoc);
  // The array_setm helper returns the up-to-date array pointer in rax.
  // Therefore, we can bind rax to arrLoc and mark it as dirty.
  m_regMap.bind(rax, arrLoc, KindOfArray, RegInfo::DIRTY);
}

void
TranslatorX64::analyzeCns(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateCns(const Tracelet& t,
                            const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 0);
  ASSERT(i.outStack && !i.outLocal);

  // OK to burn "name" into TC: it was merged into the static string
  // table, so as long as this code is reachable, so shoud the string
  // be.
  StringData* name = curUnit()->lookupLitstrId(i.imm[0].u_SA);
  TypedValue* tv = g_context->getCns(name, true, false);
  if (tv) {
    m_regMap.allocOutputRegs(i);
    // Built-in. Its type and value are known at compile-time.
    PhysReg r = getReg(i.outStack->location);
    a.   mov_imm64_reg(tv->m_data.num, r);
    emitIncRef(r, tv->m_type);
    return;
  }

  using namespace TargetCache;
  CacheHandle ch = allocConstant(name);
  // Load the constant out of the thread-private tl_targetCaches.
  ScratchReg cns(m_regMap), tmp(m_regMap);
  emitTLSLoad(a, &tl_targetCaches.base, *cns);
  a.    cmp_imm32_disp_reg32(0, ch + offsetof(TypedValue, m_type), *cns);
  TCA astubsRet;
  a.   load_reg64_disp_reg64(*cns, ch + 0, *tmp);
  int stackDest = 0 - int(sizeof(Cell)); // popped - pushed
  {
    // It's tempting to dedup these, but not obvious we really can;
    // at least stackDest and tmp are specific to the translation
    // context.
    UnlikelyIfBlock<CC_Z> ifb(a, astubs);
    // Need to save the VM regs for line number identification
    WithVMRegsSavedStub sav(astubs, i, m_saveVMRegsStub);
    astubs.  pushr(rdi); // {
    astubs.  mov_imm64_reg(uintptr_t(name), *tmp); // keep a copy
    astubs.  mov_reg64_reg64(*tmp, rdi);
    astubs.  call(m_raiseUndefCnsStub);
    astubs.  popr(rdi); // }
    // Drop the string in the output location.
    uint64_t typeAndZero = uint64_t(BitwiseKindOfString) << 32;
    BOOST_STATIC_ASSERT(offsetof(Cell, _count) == 8 &&
                        offsetof(Cell, m_type) == 12);
    emitVStackStoreImm(astubs, i, typeAndZero, stackDest + 8);
    emitVStackStore(astubs, i, *tmp, stackDest + 0);
    astubsRet = astubs.code.frontier;
    astubs.  jmp(astubsRet);
  }
  // Bitwise copy to output area.
  // Cleanup: this is very similar to CGetMProp.
  emitVStackStore(a, i, *tmp, stackDest + 0);
  a.   load_reg64_disp_reg64(*cns, ch + 8, *tmp);
  emitVStackStore(a, i, *tmp, stackDest + 8);
  a.patchJmp(astubsRet, a.code.frontier);
  m_regMap.invalidate(i.outStack->location);
}

void
TranslatorX64::analyzeConcat(Tracelet& t, NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  const RuntimeType& r = i.inputs[0]->rtt;
  const RuntimeType& l = i.inputs[1]->rtt;
  // The concat translation isn't reentrant; objects that override
  // __toString() can cause reentry.
  i.txSupported = r.valueType() != KindOfObject &&
    l.valueType() != KindOfObject;
}

void
TranslatorX64::translateConcat(const Tracelet& t,
                               const NormalizedInstruction& i) {
  m_regMap.allocOutputRegs(i);
  ASSERT(i.inputs.size() == 2);
  const DynLocation& r = *i.inputs[0];
  const DynLocation& l = *i.inputs[1];
  // We have specialized helpers for concatenating two strings, a
  // string and an int, and an int an a string.
  void* fptr = NULL;
  if (l.rtt.isString() && r.rtt.isString()) {
    fptr = (void*)concat_ss;
  } else if (l.rtt.isString() && r.rtt.isInt()) {
    fptr = (void*)concat_si;
  } else if (l.rtt.isInt() && r.rtt.isString()) {
    fptr = (void*)concat_is;
  }
  if (fptr) {
    // If we have a specialized helper, use it
    if (false) { // type check
      StringData* v1 = NULL;
      StringData* v2 = NULL;
      StringData* retval = concat_ss(v1, v2);
      printf("%p", retval); // use retval
    }
    // The concat helper will decRef the inputs and incRef the output
    // for us if appropriate
    EMIT_CALL2(fptr,
               V(l.location),
               V(r.location));
    ASSERT(i.outStack->rtt.isString());
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
  } else {
    // Otherwise, use the generic concat helper
    if (false) { // type check
      uint64_t v1, v2;
      DataType t1, t2;
      StringData *retval = concat(t1, v1, t2, v2);
      printf("%p", retval); // use retval
    }
    // concat will decRef the two inputs and incRef the output
    // for us if appropriate
    EMIT_CALL4(concat,
               IMM(l.valueType()), V(l.location),
               IMM(r.valueType()), V(r.location));
    ASSERT(i.outStack->isString());
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
  }
}

static bool
isSupportedInstrAdd_Int(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  return i.inputs[0]->isInt() &&
         i.inputs[1]->isInt();
}

static bool
isSupportedInstrAdd_Array(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  return (i.inputs[0]->valueType() == KindOfArray &&
          i.inputs[1]->valueType() == KindOfArray);
}

void
TranslatorX64::analyzeAdd(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrAdd_Int(i) || isSupportedInstrAdd_Array(i);
}

void
TranslatorX64::translateAdd(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);

  if (isSupportedInstrAdd_Array(i)) {
    // Handle adding two arrays
    ASSERT(i.outStack->outerType() == KindOfArray);
    m_regMap.allocOutputRegs(i);
    if (false) { // type check
      ArrayData* v = NULL;
      v = array_add(v, v);
    }
    WithVMRegsSavedStub sav(a, i, m_saveVMRegsStub);
    // The array_add helper will decRef the inputs and incRef the output
    // for us if appropriate
    EMIT_CALL2(array_add,
               V(i.inputs[1]->location),
               V(i.inputs[0]->location));
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
    return;
  }

  ASSERT(isSupportedInstrAdd_Int(i));
  binaryArithCell(i, OpAdd, *i.inputs[0], *i.outStack);
}

static bool
isSupportedInstrXor(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  return (i.inputs[0]->outerType() == KindOfBoolean ||
          i.inputs[0]->isInt()) &&
         (i.inputs[1]->outerType() == KindOfBoolean ||
          i.inputs[1]->isInt());
}

void
TranslatorX64::analyzeXor(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrXor(i);
}

static inline void
emitIntToCCBool(X64Assembler &a, PhysReg srcdest, PhysReg scratch,
                int CC) {
  /*
   *    test    %srcdest, %srcdest
   *    set<CC> %scratchL
   *    movzbq  %scratchL, %srcdest
   */
  a.   test_reg64_reg64(srcdest, srcdest);
  a.   setcc           (CC, scratch);
  a.   mov_reg8_reg64_unsigned(scratch, srcdest);
}

static inline void
emitIntToBool(X64Assembler &a, PhysReg srcdest, PhysReg scratch) {
  emitIntToCCBool(a, srcdest, scratch, CC_NZ);
}

static inline void
emitIntToNegBool(X64Assembler &a, PhysReg srcdest, PhysReg scratch) {
  emitIntToCCBool(a, srcdest, scratch, CC_Z);
}

void
TranslatorX64::translateXor(const Tracelet& t,
                            const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrXor(i));
  PhysReg src, srcdest;
  getBinaryStackInputs(m_regMap, i, src, srcdest);
  m_regMap.allocOutputRegs(i);
  ScratchReg scr(m_regMap);
  if (i.inputs[0]->isInt()) {
    emitIntToBool(a, src, *scr);
  }
  if (i.inputs[1]->isInt()) {
    emitIntToBool(a, srcdest, *scr);
  }
  a.    xor_reg64_reg64(src, srcdest);
}

static bool
isSupportedInstrNot(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  return i.inputs[0]->isInt() ||
         i.inputs[0]->outerType() == KindOfBoolean;
}

void
TranslatorX64::analyzeNot(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrNot(i);
}

void
TranslatorX64::translateNot(const Tracelet& t,
                            const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrNot(i));
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(!i.inputs[0]->isVariant());
  m_regMap.allocOutputRegs(i);
  PhysReg srcdest = m_regMap.getReg(i.outStack->location);
  ScratchReg scr(m_regMap);
  emitIntToNegBool(a, srcdest, *scr);
}

static bool
isSupportedInstrBitNot(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  return i.inputs[0]->isInt();
}

void
TranslatorX64::analyzeBitNot(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrBitNot(i);
}

void
TranslatorX64::translateBitNot(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrBitNot(i));
  ASSERT(i.outStack && !i.outLocal);
  m_regMap.allocOutputRegs(i);
  PhysReg srcdest = m_regMap.getReg(i.outStack->location);
  a.   not_reg64(srcdest);
}

static bool
isSupportedInstrCastInt(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  return i.inputs[0]->isInt();
}

void
TranslatorX64::analyzeCastInt(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrCastInt(i);
}

void
TranslatorX64::translateCastInt(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrCastInt(i));
  ASSERT(i.inputs.size() == 1);
  ASSERT(i.outStack && !i.outLocal);

  /* nop */
}

static bool
isSupportedInstrPrint(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  const RuntimeType& rtt = i.inputs[0]->rtt;
  DataType type = rtt.outerType();
  return
    type == KindOfUninit ||
    type == KindOfNull ||
    type == KindOfBoolean ||
    rtt.isInt() ||
    rtt.isString();
}

void
TranslatorX64::analyzePrint(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrPrint(i);
}

void
TranslatorX64::translatePrint(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrPrint(i));
  const vector<DynLocation*>& inputs  = i.inputs;
  ASSERT(inputs.size()   == 1);
  ASSERT(!i.outLocal);
  ASSERT(!i.outStack || i.outStack->isInt());
  Location  loc = inputs[0]->location;
  DataType type = inputs[0]->outerType();
  switch(type) {
    STRINGCASE():       EMIT_CALL1(print_string,  V(loc)); break;
    INTCASE():          EMIT_CALL1(print_int,     V(loc)); break;
    case KindOfBoolean: EMIT_CALL1(print_boolean, V(loc)); break;
    NULLCASE():         /* do nothing */                   break;
    default: {
      // Translation is only supported for Null, Boolean, Int, and String
      ASSERT(false);
      break;
    }
  }
  m_regMap.allocOutputRegs(i);
  if (i.outStack) {
    PhysReg outReg = getReg(i.outStack->location);
    emitImmReg(a, 1, outReg);
  }
}

void
TranslatorX64::analyzeJmp(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateJmp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  m_regMap.allocOutputRegs(i);
  syncOutputs(t);
  SrcKey sk(curUnit(), i.offset() + i.imm[0].u_BA);
  emitBindJmp(sk);
}

void
TranslatorX64::analyzeRetC(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

static void
wrapped_frame_free_locals(ActRec* fp, Offset o) {
  vmpc() = fp->m_func->m_unit->at(o);
  frame_free_locals(fp);
}

// translateRetC --
//
//   Return to caller with the current activation record replaced with the
//   top-of-stack return value. Call with outputs sync'ed, so the code
//   we're emmitting runs "in between" basic blocks.
void
TranslatorX64::translateRetC(const Tracelet& t,
                             const NormalizedInstruction& i) {

  /**
   * This method chooses one of two ways to generate machine code for RetC
   * depending on whether we are generating a specialized return (where we
   * free the locals inline when possible) or a generic return (where we call
   * a helper function to free locals).
   *
   * For the specialized return, we emit the following flow:
   *
   *   Check if varenv is NULL
   *   If it's not NULL, branch to label 2
   *   Free each local variable
   * 1:
   *   Teleport the return value to appropriate memory location
   *   Restore the old values for rVmFp and rVmSp, and
   *   unconditionally transfer control back to the caller
   * 2:
   *   Call the frame_free_locals helper
   *   Jump to label 1
   *
   * For a generic return, we emit the following flow:
   *
   *   Call the frame_free_locals helper
   *   Teleport the return value to appropriate memory location
   *   Restore the old values for rVmFp and rVmSp, and
   *   unconditionally transfer control back to the caller
   */

  syncOutputs(t);

  bool freeLocalsInline =
      (curFunc()->m_numLocals <= Translator::kFewLocals);

  TCA varEnvCheck = NULL;

  if (freeLocalsInline) {
    // Emit specialized code inline to clean up the locals
    ASSERT(curFunc()->m_numLocals == (int)i.inputs.size());
    ScratchReg scratch(m_regMap);
    a. load_reg64_disp_reg64(rVmFp, offsetof(ActRec, m_varEnv), *scratch);
    a. test_reg64_reg64(*scratch, *scratch);
    varEnvCheck = a.code.frontier;
    a. jnz(0); // 2f
    for (unsigned int k = 0; k < i.inputs.size(); ++k) {
      // RetC's inputs should all be locals
      ASSERT(i.inputs[k]->location.space == Location::Local);
      DataType t = i.inputs[k]->outerType();
      if (IS_REFCOUNTED_TYPE(t)) {
        PhysReg reg = m_regMap.allocReg(i.inputs[k]->location, t,
                                        RegInfo::CLEAN);
        emitDecRef(reg, t);
      }
    }
    if (((ActRec*)vmfp())->hasThis()) {
      // This is a method called on an object; clean up $this.
      a. load_reg64_disp_reg64(rVmFp, offsetof(ActRec, m_this), rdi);
      emitDecRef(rdi, KindOfObject);
    }

    emitLoadDynTracer();
    a.test_reg64_reg64(rScratch, rScratch);
    {
      UnlikelyIfBlock<CC_NZ> ifTracer(a, astubs);
      astubs.mov_reg64_reg64(rVmFp, argNumToRegName[0]);
      astubs.call((TCA)&DynTracer::FunctionExit);
      m_regMap.smashPhysRegs(callerSaved());
    }
  } else {
    // If we are doing the generic return flow, we emit a call to
    // frame_free_locals here
    ASSERT(i.inputs.size() == 0);
    if (false) {
      ActRec* ar = NULL;
      frame_free_locals(ar);
    }
    emitFrameRelease(a, i);
  }

  // 1:
  TCA localsFreed = a.code.frontier;

  const PhysReg rRetAddr = rax;
  a.   load_reg64_disp_reg64(rVmFp, offsetof(ActRec, m_savedRip), rRetAddr);
  a.   load_reg64_disp_reg64(rVmFp, offsetof(ActRec, m_savedRbp), rVmFp);
  emitSaveFPReg(a);

  /*
   * Having gotten everything we care about out of the current frame
   * pointer, smash the return address type and value over it. We don't
   * care about reference counts: as long as this runs to completion, we're
   * refcount-neutral.
   */
  const Func *callee = ((ActRec *)vmfp())->m_func;
  int nLocalCells =
    callee == NULL ? 0 : // This happens for returns from pseudo-main.
    callee->numSlotsInFrame();
  int retvalSrcBase  = cellsToBytes(0); // 0(rVmSp) is retval

  /*
   * The (1 + nLocalCells) skips 1 slot for the return value.
   */
  TRACE(2, "nLocalCells: %d\n", nLocalCells);
  int retvalDestDisp = cellsToBytes(1 + nLocalCells) + offsetof(ActRec, m_r);

  /*
   * Splat the return value on top of the activation record. We've already
   * read all the fields from it we'll need.
   */
  ASSERT(sizeof(Cell) == 16);
  a.   load_reg64_disp_reg64 (rVmSp,    retvalSrcBase,      rScratch);
  a.   store_reg64_disp_reg64(rScratch, retvalDestDisp,     rVmSp);
  a.   load_reg64_disp_reg64 (rVmSp,    retvalSrcBase + 8,  rScratch);
  a.   store_reg64_disp_reg64(rScratch, retvalDestDisp + 8, rVmSp);
  /*
   * Now update the principle hardware registers.
   *
   * Stack pointer has to skip over all the locals as well as the
   * activation record.
   */
  a.   add_imm64_reg64(sizeof(ActRec) + cellsToBytes(nLocalCells), rVmSp);
  a.   jmp_reg        (rRetAddr);
  translator_not_reached(a);

  if (freeLocalsInline) {
    // If we are doing the specialized return flow, we emit the call to
    // frame_free_locals down here instead to make the case where varenv
    // is NULL faster.
    ASSERT(varEnvCheck);
    // 2:
    a.patchJcc(varEnvCheck, a.code.frontier);
    emitFrameRelease(a, i);
    a. jmp(localsFreed); // 1b
  }
}

// Warning: smashes rsi and rdi. Usually ok between functions.
void
TranslatorX64::emitFrameRelease(X64Assembler& a,
                                const NormalizedInstruction& i) {
  if (false) { // typecheck
    Offset o = 022707;
    wrapped_frame_free_locals(curFrame(), o);
  }

  a.    mov_reg64_reg64(rVmFp, argNumToRegName[0]);
  emitImmReg(a, i.offset(), argNumToRegName[1]);
  a.    call((TCA)wrapped_frame_free_locals);
}

void
TranslatorX64::analyzeLoc(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateLoc(const Tracelet& t,
                            const NormalizedInstruction& i) {
  ASSERT(i.outStack && !i.outLocal);
  m_regMap.invalidate(i.outStack->location);
  m_spf.spillHome(m_regMap, Location(Location::Local, i.imm[0].u_IVA),
                  i.outStack->location);
}

void
TranslatorX64::analyzeCls(Tracelet& t, NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.outStack->valueType() == KindOfClass);
  const RuntimeType& rtt = i.inputs[0]->rtt;
  ASSERT(!rtt.isVariant());
  // TODO: support object Cls's as well...
  i.txSupported = rtt.isString() || rtt.valueType() == KindOfObject;
}

// emitStringToClass --
// emitObjToClass --
// emitClsAndPals --
//   Helpers for ClsH and Cls.
void
TranslatorX64::emitStringToClass(const NormalizedInstruction& i) {
  const Location& in = i.inputs[0]->location;
  const Location& out = i.outStack->location;
  using namespace TargetCache;
  CacheHandle ch = ClassCache::alloc();
  if (false) {
    StringData *name = NULL;
    const UNUSED Class* cls = ClassCache::lookup(ch, name);
  }
  TRACE(1, "ClassCache @ %d\n", int(ch));
  if (i.inputs[0]->rtt.isVariant()) {
      EMIT_CALL2(ClassCache::lookup,
                 IMM(ch),
                 DEREF(in));
  } else {
      EMIT_CALL2(ClassCache::lookup,
                 IMM(ch),
                 V(in));
  }
  m_regMap.bind(rax, out, KindOfClass, RegInfo::DIRTY);
}

void
TranslatorX64::emitObjToClass(const NormalizedInstruction& i) {
  m_regMap.allocOutputRegs(i);
  const Location& in = i.inputs[0]->location;
  const Location& out = i.outStack->location;
  PhysReg src = getReg(in);
  ScratchReg tmp(m_regMap);
  if (i.inputs[0]->rtt.isVariant()) {
    emitDeref(a, src, *tmp);
    src = *tmp;
  }
  ASSERT(i.outStack->valueType() == KindOfClass);
  a.   load_reg64_disp_reg64(src, ObjectData::getVMClassOffset(), getReg(out));
}

void
TranslatorX64::emitClsAndPals(const NormalizedInstruction& ni) {
  void (TranslatorX64::*emitter)(const NormalizedInstruction& ni) =
    ni.inputs[0]->isString() ?
    &HPHP::VM::Transl::TranslatorX64::emitStringToClass :
    &HPHP::VM::Transl::TranslatorX64::emitObjToClass;
  (this->*emitter)(ni);
}

void TranslatorX64::translateCls(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  emitClsAndPals(i);
}

void TranslatorX64::analyzeClsH(Tracelet& t, NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.outStack->valueType() == KindOfClass);
  const RuntimeType& rtt = i.inputs[0]->rtt;
  ASSERT(!rtt.isHome());
  i.txSupported = rtt.isString() || rtt.valueType() == KindOfObject;
}

void TranslatorX64::translateClsH(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  emitClsAndPals(i);
}

// Helper function for static property access.  This function emits code
// which leaves a pointer to the static property for clsInput::$propInput in
// register scr. We destroy scr early on, yet do not consume inputs until
// later, so scr must not alias an input register.  This also handles
// the decref for the case where prop is not a static string.
void TranslatorX64::emitStaticPropInlineLookup(const NormalizedInstruction& i,
                                               const DynLocation& clsInput,
                                               const DynLocation& propInput,
                                               PhysReg scr) {
  const Class* cls = clsInput.rtt.valueClass();
  const StringData* propName = propInput.rtt.valueString();
  using namespace TargetCache;
  CacheHandle ch;
  PreClass* ctx UNUSED = arGetContextPreClass(curFrame());

  ASSERT(cls && propName);
  // Use the uniquely known cls / prop to generate a single cache per prop
  const StringData* clsName = cls->m_preClass->m_name;
  StringData sd(clsName->data(), CopyString);
  sd.append(":", 1);
  sd.append(propName->data(), propName->size());
  ch = SPropCache::alloc(&sd);
  SKTRACE(1, i.source, "SPropInlineLookup %s %d\n", sd.data(), int(ch));

  // For the simple case of statically known class and prop name, we inline
  // the target cache lookup, and outline the miss case.
  // Load the TV pointer out of the thread-private tl_targetCaches.
  ASSERT(offsetof(SPropCache, m_tv) == 0);
  emitTLSLoad(a, &tl_targetCaches.base, scr);
  a.   load_reg64_disp_reg64(scr, ch, scr);
  a.   test_reg64_reg64(scr, scr);

  // Outline the calls - and N.B. - this requires some care.
  // We can't use the EMIT_CALL machinery as it may modify regmap.
  // Instead, we forcibly push and pop all caller saved regs.
  {
    UnlikelyIfBlock<CC_Z> shucks(a, astubs);
    set<PhysReg> s = callerSaved();
    s.erase(scr);
    WithPhysRegSaver rs(astubs, s);

    // If the class's static properties have not yet been initialized, we may
    // re-enter.  We can't throw any exceptions, as static properties can
    // only be initialized from constants, thus we can be a bit lazy about
    // syncing state.  We may also fatal, but not if bound.
    emitImmReg(astubs, (int64_t)i.offset(), scr);
    WithVMRegsSaved sav(astubs, false, true, rScratch, rVmFp, scr);
    PhysReg loc = getReg(clsInput.location);
    ASSERT (loc != scr);
    emitMovRegRegAsm(astubs, loc, rsi);
    emitImmReg(astubs, ch, rdi);
    emitImmReg(astubs, uint64_t(propName), rdx);

    // Precondition for this lookup - we don't need to pass the preClass,
    // as we only translate in class lookups.
    ASSERT(cls->m_preClass.get() == ctx);
    if (false) { // typecheck
      StringData *data = NULL;
      SPropCache::lookup(ch, cls, data);
    }
    astubs.   call((TCA)SPropCache::lookup);
    emitMovRegRegAsm(astubs, rax, scr);

    // We're consuming the name as input, but it is static, no decref needed
    ASSERT(propInput.rtt.valueString()->isStatic());
    // astubs.  jmp(a.code.frontier); -- implicit
  }
}

void TranslatorX64::analyzeCGetS(Tracelet& t, NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 2);
  ASSERT(i.inputs[1]->valueType() == KindOfClass);
  ASSERT(i.outStack);
  const Class* cls = i.inputs[1]->rtt.valueClass();
  const StringData* propName = i.inputs[0]->rtt.valueString();
  i.txSupported = cls && propName &&
                  (arGetContextPreClass(curFrame()) == cls->m_preClass.get());
}

void TranslatorX64::translateCGetS(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  ScratchReg sprop(m_regMap);
  emitStaticPropInlineLookup(i, *i.inputs[1], *i.inputs[0], *sprop);
  { // if (field->m_type == KindOfVar) {
    IfVariant ifVar(a, *sprop);
    emitDeref(a, *sprop, *sprop);
    // }
  }
  emitIncRefGeneric(*sprop, 0);
  // Finally copy the thing to the stack
  ScratchReg tmp1(m_regMap), tmp2(m_regMap);
  a.   load_reg64_disp_reg64(*sprop, 0, *tmp1);
  a.   load_reg64_disp_reg64(*sprop, 8, *tmp2);
  int stackDest = 2 * sizeof(Cell) - sizeof(Cell); // popped - pushed
  emitVStackStore(a, i, *tmp1, stackDest + 0);
  emitVStackStore(a, i, *tmp2, stackDest + 8);
  // Clean this range of the stack.
  m_regMap.markAsClean(i.outStack->location);
}

void TranslatorX64::analyzeSetS(Tracelet& t, NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 3);
  ASSERT(i.inputs[2]->valueType() == KindOfClass);
  ASSERT(i.outStack);
  const Class* cls = i.inputs[2]->rtt.valueClass();
  const StringData* propName = i.inputs[1]->rtt.valueString();
  i.txSupported = cls && propName &&
                  (arGetContextPreClass(curFrame()) == cls->m_preClass.get());
}

void TranslatorX64::translateSetS(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  m_regMap.allocOutputRegs(i);
  ScratchReg sprop(m_regMap);
  const RuntimeType& rhsType = i.inputs[0]->rtt;
  PhysReg rhsReg = getReg(i.inputs[0]->location);
  PhysReg outReg = getReg(i.outStack->location);
  emitStaticPropInlineLookup(i, *i.inputs[2], *i.inputs[1], *sprop);
  emitIncRef(rhsReg, rhsType.outerType());
  {
    // If the property is a Var, we need to dereference the Var so that
    // sprop holds the address of the inner cell.
    IfVariant ifVar(a, *sprop);
    emitDeref(a, *sprop, *sprop);
  }
  // decRef the lhs.
  emitDecRefGeneric(*sprop);
  // Drop rhs on the lhs.
  int typeDisp = offsetof(TypedValue, m_type);
  int dataDisp = offsetof(TypedValue, m_data);
  a.    store_reg64_disp_reg64(rhsReg, dataDisp, *sprop);
  a.    store_imm32_disp_reg(rhsType.outerType(), typeDisp, *sprop);
  // The input stays on the stack, so we don't need to consume it
  ASSERT(i.inputs[2]->location == i.outStack->location);
  emitMovRegReg(rhsReg, outReg);
}

/*
 * Some helpers for property accesses.
 */
static int
normInstrToPropOff(const NormalizedInstruction& i,
                   int propInput,
                   int objInput) {
  ASSERT(i.inputs.size() >= 2 && propInput != objInput);
  const Class *c = i.inputs[objInput]->rtt.valueClass();
  const StringData *sd = i.inputs[propInput]->rtt.valueString();
  int propIdx = c->lookupDeclProp(sd);
  if (propIdx < 0) {
    return propIdx;
  }
  return sizeof(ObjectData) + c->m_builtinPropSize
         + propIdx * sizeof(TypedValue);
}
static bool
isNormalPropertyAccess(const NormalizedInstruction& i,
                       int propInput,
                       int objInput) {
  return
    i.inputs[propInput]->isString() &&
    i.inputs[propInput]->rtt.valueString() != NULL && // we know the field
    i.inputs[objInput]->outerType() == KindOfObject &&
    i.inputs[objInput]->rtt.valueClass() != NULL &&  // ..and the class
    normInstrToPropOff(i, propInput, objInput) >= 0; // to a declared member
}

// $this->declaredProperty is a common pattern
static bool
isSupportedCGetM_CP(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  const ImmVector* iv = i.immVecPtr;
  SKTRACE(2, i.source, "CGetM prop candidate: MP-ness %d, in[0] %s in[1] %s\n",
          iv->get(1) == MP,
          i.inputs[0]->rtt.pretty().c_str(),
          i.inputs[1]->rtt.pretty().c_str());
  return
    // Supported prop instruction
    iv->len == 2 && iv->get(0) == LC &&
      iv->get(1) == MP &&
      isNormalPropertyAccess(i, 0, 1);
}

static bool
isSupportedCGetM_HE(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  DynLocation* arrInput = i.inputs[i.inputs.size() - 1];
  ImmVector* iv = i.immVecPtr;
  ASSERT(iv);
  // Return true iff this is a CGetM <H E> instruction where
  // the base is an array and the key is an int or string
  return iv->len == 2 && iv->get(0) == LH && iv->get(1) == ME &&
    arrInput->valueType() == KindOfArray && i.inputs.size() == 2 &&
    (i.inputs[0]->isInt() || i.inputs[0]->isString());
}

static bool
isSupportedCGetM_HEE(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  DynLocation* arrInput = i.inputs[i.inputs.size() - 1];
  ImmVector* iv = i.immVecPtr;
  ASSERT(iv);
  // Return true iff this is a CGetM <H E E> instruction where
  // the base is an array, the first key is a string and the second
  // key is an int
  return iv->len == 3 && iv->get(0) == LH && iv->get(1) == ME &&
    iv->get(2) == ME && arrInput->valueType() == KindOfArray &&
    i.inputs.size() == 3 &&
    i.inputs[0]->isString() &&
    i.inputs[1]->isInt();
}

static bool
isSupportedCGetM_RE(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  DynLocation* arrInput = i.inputs[i.inputs.size() - 1];
  ImmVector* iv = i.immVecPtr;
  ASSERT(iv);
  return (iv->len == 2 &&
          iv->get(0) == LR &&
          iv->get(1) == ME &&
          arrInput->valueType() == KindOfArray && i.inputs.size() == 2 &&
          (i.inputs[0]->isInt() ||
           i.inputs[0]->isString()));
}

static bool
isSupportedCGetM(const NormalizedInstruction& i) {
  return  isSupportedCGetM_HE(i) || isSupportedCGetM_RE(i) ||
    isSupportedCGetM_HEE(i) ||
    isSupportedCGetM_CP(i);
}

void
TranslatorX64::analyzeCGetM(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedCGetM(i);
}

void
TranslatorX64::translateCGetMProp(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  DynLocation* baseInput = i.inputs[i.inputs.size() - 1];
  // If the class field was known at translation time, we can translate
  // this as a straight load.
  int fieldDisp = normInstrToPropOff(i, 0, 1);
  PhysReg base = getReg(baseInput->location);
  ScratchReg fieldAddr(m_regMap);
  ScratchReg tmpLo(m_regMap);
  ScratchReg tmpHi(m_regMap);
  a.    lea_reg64_disp_reg64(base, fieldDisp, *fieldAddr);
  { // if (field->m_type == KindOfVar) {
    IfVariant ifVar(a, *fieldAddr);
    emitDeref(a, *fieldAddr, *fieldAddr);
    // }
  }
  // Hoist the loads above the incRef.
  a.   load_reg64_disp_reg64(*fieldAddr,  0, *tmpLo);
  a.   load_reg64_disp_reg64(*fieldAddr,  8, *tmpHi);
  // We may be creating a reference to the field.
  emitIncRefGeneric(*fieldAddr, 0);
  // Copy to output location manually.
  int stackDest = 2 * sizeof(Cell) - sizeof(Cell); // popped - pushed
  emitVStackStore(a, i, *tmpLo, stackDest + 0);
  emitVStackStore(a, i, *tmpHi, stackDest + 8);
  // Release the base
  emitDecRef(base, KindOfObject);
  // Clean the stack.
  m_regMap.invalidate(i.outStack->location);
}

void
TranslatorX64::translateCGetM(const Tracelet& t,
                              const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  ASSERT(i.outStack);
  DynLocation* baseInput = i.inputs[i.inputs.size() - 1];
  Location outLoc = i.outStack->location;
  ASSERT(baseInput->isLocal() ||
         isSupportedCGetM_CP(i) ||
         isSupportedCGetM_RE(i));
  TRACE(2, "emitCGetM(): %d inputs\n", int(i.inputs.size()));
  for (unsigned j = 0; j < i.inputs.size(); j++) {
    TRACE(3, "emitCGetM input %d :: t%d\n", j, i.inputs[j]->outerType());
  }

  if (isSupportedCGetM_HE(i) || isSupportedCGetM_RE(i)) {
    DynLocation* keyIn = i.inputs[0];
    Location homeOfArray = baseInput->location;
    Location keyLoc = keyIn->location;
    Location outLoc = i.outStack->location;
    bool decRefBase = isSupportedCGetM_RE(i);
    if (false) { // type-check
      void *a;
      TypedValue tv;
      array_getm_i(a, 1, &tv);
      StringData *sd = NULL;
      array_getm_s(a, sd, &tv);
      array_getm_s0(a, sd, &tv);
    }
    void* fptr;
    // Let the array helpers handle refcounting logic: keys and array down,
    // return value up.
    if (keyIn->isInt()) {
      fptr = (void*)array_getm_i;
    } else {
      ASSERT(keyIn->isString());
      bool decRefKey = keyIn->isStack();
      bool doIntCheck = (i.constImmPos == NormalizedInstruction::kNoImm);
      fptr = (decRefKey ?
              (doIntCheck ? (void*)array_getm_s : (void*)array_getm_s_fast) :
              (doIntCheck ? (void*)array_getm_s0 : (void*)array_getm_s0_fast));
    }
    SKTRACE(1, i.source, "emitCGetM: committed to unary load\n");
    // The array helpers can reenter; need to sync state. The superior solution
    // would be to build a database that allows us to reconstruct the VM regs'
    // state given the address of this array_getm_* callsite, and use that
    // database at the point in C++ where these pieces of state are needed.
    WithVMRegsSavedStub sav(a, i, m_saveVMRegsStub);
    if (baseInput->outerType() == KindOfVariant) {
      EMIT_CALL3(fptr, DEREF(homeOfArray), V(keyLoc), A(outLoc));
    } else {
      EMIT_CALL3(fptr, V(homeOfArray), V(keyLoc), A(outLoc));
    }
    m_regMap.invalidate(outLoc);
    if (decRefBase) {
      // For convenience of decRefs, the callees return the ArrayData*.
      emitDecRef(rax, KindOfArray);
    }
    return;
  }

  if (isSupportedCGetM_CP(i)) {
    translateCGetMProp(t, i);
    return;
  }

  ASSERT(isSupportedCGetM_HEE(i));
  Location strKey = i.inputs[0]->location;
  Location intKey = i.inputs[1]->location;
  Location homeOfArray = baseInput->location;
  TRACE(1, "nvGet2 outLoc: (%s, %d)\n", outLoc.spaceName(),outLoc.offset);
  if (false) { // typeCheck
    void *a;
    TypedValue tv;
    StringData *sd = NULL;
    array_getm_is(a, 666, sd, &tv);
    array_getm_is0(a, 666, sd, &tv);
  }
  void* fptr;
  // array_getm_is incRefs the return value if appropriate and
  // it decRefs the string key for us. We don't need to decRef
  // the array because it was passed via a home
  bool decRefStrKey = i.inputs[0]->isStack();
  fptr = decRefStrKey ? ((void*)array_getm_is) : ((void*)array_getm_is0);
  WithVMRegsSavedStub sav(a, i, m_saveVMRegsStub);
  if (baseInput->outerType() == KindOfVariant) {
    EMIT_CALL4(fptr, DEREF(homeOfArray), V(intKey), V(strKey), A(outLoc));
  } else {
    EMIT_CALL4(fptr, V(homeOfArray), V(intKey), V(strKey), A(outLoc));
  }
  m_regMap.invalidate(outLoc);
}

static bool
isSupportedInstrVGetG(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 1);
  return (i.inputs[0]->rtt.isString());
}

void
TranslatorX64::cleanLocalReg(const NormalizedInstruction& i,
                             const StringData* maybeName,
                             bool onlyPseudoMain/* = false*/) {
  Func *f = curUnit()->getFunc(i.offset());
  if (onlyPseudoMain && !f->isPseudoMain())
    return; // This is only needed for locals in pseudo-main

  if (maybeName != NULL) {
    Id localId = -1;
    if (mapGet(f->m_name2pind, maybeName, &localId)) {
      // Get the register for the local and clean it
      Location localLoc = Location(Location::Local, localId);
      if (m_regMap.hasReg(localLoc)) {
        PhysReg localReg = getReg(localLoc);
        SKTRACE(2, i.source, "cleaning local %s in r%d\n",
                maybeName->data(), localReg);
        std::set<PhysReg> regs;
        regs.insert(localReg);
        m_regMap.cleanRegs(regs);
      }
    }
    // Since numbered locals are only added to m_name2pind by
    // the emitter and only those locals get assigned to registers,
    // any variable that's not in m_name2pind cannot be in a dirty
    // register.
  } else {
    m_regMap.cleanLocals();
  }
}

void
TranslatorX64::analyzeVGetG(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrVGetG(i);
}

void
TranslatorX64::translateVGetG(const Tracelet& t,
                              const NormalizedInstruction& i) {
  using namespace TargetCache;
  const StringData* maybeName = i.inputs[0]->rtt.valueString();
  CacheHandle ch = BoxedGlobalCache::alloc(maybeName);

  if (false) { // typecheck
    StringData *key = NULL;
    TypedValue UNUSED *glob = BoxedGlobalCache::lookup(ch, key);
  }
  SKTRACE(1, i.source, "ch %d\n", ch);
  cleanLocalReg(i, maybeName, true);
  EMIT_CALL2(BoxedGlobalCache::lookup,
             IMM(ch),
             V(i.inputs[0]->location));
  m_regMap.bind(rax, i.outStack->location, KindOfVariant, RegInfo::DIRTY);
}

void
TranslatorX64::analyzeVGetM(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = false; // TODO: implement VGetM meaningfully.
}

void
TranslatorX64::translateVGetM(const Tracelet& t,
                              const NormalizedInstruction& i) {
  NOT_REACHED();
}

void
TranslatorX64::analyzeCGetG(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrVGetG(i); // Share this code.
}

void
TranslatorX64::translateCGetG(const Tracelet& t,
                              const NormalizedInstruction& i) {
  using namespace TargetCache;
  const StringData *maybeName = i.inputs[0]->rtt.valueString();
  CacheHandle ch = GlobalCache::alloc(maybeName);
  ASSERT(i.inputs.size() >= 1);
  if (false) {
    StringData *key = NULL;
    TypedValue* glob = GlobalCache::lookup(ch, key);
    printf("%p %p\n", key, glob);
  }
  SKTRACE(1, i.source, "ch %d\n", ch);
  cleanLocalReg(i, maybeName, true);
  EMIT_CALL2(GlobalCache::lookup,
             IMM(ch),
             V(i.inputs[0]->location));
  // rax now points to the in-memory location of the object of unknown
  // type. lookup() has already counted the reference we're about to create
  // to it. We know we have rsi and rdi to play with, since we just blew
  // them away in the EMIT_CALL2 above, so let's use rsi/rdi as scratch
  // regs. We could go through m_regMap, but then we'd have to make
  // sure it didn't use rax.
  ASSERT(i.outStack && !i.outLocal);
  a.   load_reg64_disp_reg64(rax, 0, rdi);
  a.   load_reg64_disp_reg64(rax, 8, rsi);
  emitVStackStore(a, i, rdi, 0);
  emitVStackStore(a, i, rsi, 8);
  // Clean this range of the stack.
  m_regMap.invalidate(i.outStack->location);
}

void
TranslatorX64::analyzeFPassH(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateFPassH(const Tracelet& t,
                               const NormalizedInstruction& i) {
  const vector<DynLocation*>& inputs  = i.inputs;
  ASSERT(inputs.size() == 1 || inputs.size() == 2);
  ASSERT(i.outStack);
  ASSERT(inputs[0]->isLocal());
  bool outputIsRef = i.preppedByRef;
  PhysReg localReg = getReg(inputs[0]->location);
  DataType outType = i.outStack->outerType();
  TRACE(1, "FPassH: Outer type: %d\n", outType);
  ASSERT(outType != KindOfInvalid);
  PhysReg dest;
  TRACE(1, "FPassH: byRef? %d\n", outputIsRef);
  // We know the output type, so operate on the registers
  if (outputIsRef && inputs[0]->outerType() != KindOfVariant) {
    // Handle the case where we need to box. This has two, separate
    // outputs: one on the stack, which we rename as RAX, and another
    // in locals, mirrored in localReg.  We handle setting up the
    // outputs manually, don't need to call allocOutputRegs().
    // emitBox sets the refcount of the new inner cell to 1; we do
    // not need to decRef the value because now the inner cell
    // references it.
    emitBox(inputs[0]->outerType(), localReg);
    m_regMap.bind(rax, i.outStack->location, KindOfVariant,
                  RegInfo::DIRTY);
    dest = rax;
    localReg = m_regMap.allocReg(inputs[0]->location,
                                 KindOfVariant, RegInfo::DIRTY);
    SKTRACE(1, i.source, "realloced localReg to r%d\n", localReg);
    ASSERT(localReg != dest);
    emitMovRegReg(dest, localReg);
  } else {
    // For cases where we don't need to box, allocate the output
    // registers normally
    m_regMap.allocOutputRegs(i);
    dest = getReg(i.outStack->location);
    if (!outputIsRef && inputs[0]->outerType() == KindOfVariant) {
      // Handle the case where we need to unbox
      emitDeref(a, localReg, dest);
    } else {
      // No boxing or unboxing required
      emitMovRegReg(localReg, dest);
    }
  }
  emitIncRef(dest, outType);
}

static bool
isSupportedInstrIssetH(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  const DynLocation& in = *i.inputs[0];
  DataType inType = in.valueType();
  return in.isString() ||
         in.isInt() ||
         inType == KindOfUninit ||
         inType == KindOfNull ||
         inType == KindOfBoolean ||
         inType == KindOfArray;
}

void
TranslatorX64::analyzeIssetH(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrIssetH(i);
}

void
TranslatorX64::translateIssetH(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  const vector<DynLocation*>& inputs  = i.inputs;
  ASSERT(inputs.size() == 1);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(isSupportedInstrIssetH(i));

  m_regMap.allocOutputRegs(i);
  PhysReg outReg = getReg(i.outStack->location);
  emitImmReg(a, !IS_NULL_TYPE(inputs[0]->valueType()), outReg);
}

static bool
isSupportedInstrIssetM(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  const RuntimeType &arrayRtt = i.inputs[1]->rtt;
  const RuntimeType &keyRtt = i.inputs[0]->rtt;
  ImmVector* iv = i.immVecPtr;
  ASSERT(iv);
  return (iv->len == 2 && iv->get(0) == LH && iv->get(1) == ME) &&
         (i.inputs.size() == 2) && (keyRtt.isString()) &&
         (arrayRtt.valueType() == KindOfArray);
}

void
TranslatorX64::analyzeIssetM(Tracelet& t, NormalizedInstruction& i) { i.txSupported = isSupportedInstrIssetM(i);
}

void
TranslatorX64::translateIssetM(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  dumpInstrInputs(1, i);
  const RuntimeType &arrayRtt = i.inputs[1]->rtt;
  ASSERT(isSupportedInstrIssetM(i));
  ASSERT(i.inputs[1]->isLocal());

  bool variantArray = arrayRtt.outerType() == KindOfVariant;
  SKTRACE(1, i.source, "array(v:%d)[string]\n", variantArray);
  const DynLocation& arr = *i.inputs[1];
  const DynLocation& key = *i.inputs[0];
  Location arrLoc = arr.location;
  Location keyLoc = key.location;
  bool decRefKey = keyLoc.isStack();
  PhysReg arrReg = getReg(arrLoc);

  ScratchReg scratch(m_regMap);
  if (variantArray) {
    emitDeref(a, arrReg, *scratch);
    arrReg = *scratch;
    SKTRACE(1, i.source, "loaded variant\n");
  }

  bool doIntCheck = (i.constImmPos == NormalizedInstruction::kNoImm);
  uint64 (*helper)(const void* arr, StringData* sd) =
    (decRefKey ?
     (doIntCheck ? array_issetm_s : array_issetm_s_fast) :
     (doIntCheck ? array_issetm_s0 : array_issetm_s0_fast));
  // The array helpers can reenter; need to sync state.
  WithVMRegsSavedStub sav(a, i, m_saveVMRegsStub);
  EMIT_CALL2(helper, R(arrReg), V(keyLoc));

  // We didn't bother allocating the single output reg above;
  // it lives in rax now.
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.outStack->outerType() == KindOfBoolean);
  m_regMap.bind(rax, i.outStack->location, KindOfBoolean,
                RegInfo::DIRTY);
}

/*
 * SetM. We care about two cases in particular: objects and arrays.
 */
static inline bool
isSupportedSetMArray(const NormalizedInstruction& i) {
  const ImmVector* iv = i.immVecPtr;
  return
    iv->len == 2 && iv->get(0) == LH && iv->get(1) == ME &&
    i.inputs.size() == 3 && // special-case 1 subscript
    (i.inputs[1]->isInt()|| // key
     i.inputs[1]->isString()) &&
    i.inputs[2]->valueType() == KindOfArray; // array
}

static bool
isSupportedSetMProp(const NormalizedInstruction& i) {
  if (i.inputs.size() <= 2) return false;
  const ImmVector* iv = i.immVecPtr;
  SKTRACE(2, i.source, "setM prop candidate: MP-ness %d, rtt %s\n",
          iv->get(1) == MP,
          i.inputs[2]->rtt.pretty().c_str());
  return
    // Supported prop instruction
    iv->len == 2 && iv->get(0) == LC && iv->get(1) == MP &&
    isNormalPropertyAccess(i, 1, 2);
}

void
TranslatorX64::translateSetMProp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  SKTRACE(2, i.source, "constImmPos: %d\n", i.constImmPos);
  // RHS == inputs[0] will be invariant; don't refcount if we don't have
  // to.
  ASSERT(i.inputs[2]->location == i.outStack->location);
  m_regMap.allocOutputRegs(i);
  ScratchReg rField(m_regMap);
  const RuntimeType& rhsType = i.inputs[0]->rtt;
  PhysReg rhsReg = getReg(i.inputs[0]->location);
  PhysReg instanceReg = getReg(i.inputs[2]->location);
  PhysReg outReg = getReg(i.outStack->location);
  emitIncRef(rhsReg, rhsType.outerType());

  int fieldDisp = normInstrToPropOff(i, 1, 2);
  int typeDisp = offsetof(TypedValue, m_type);
  int dataDisp = offsetof(TypedValue, m_data);

  ASSERT(fieldDisp >= 0);

  // Load the address of the field into rField.
  a.    lea_reg64_disp_reg64(instanceReg, fieldDisp, *rField);
  {
    // If the field is a Var, we need to dereference the Var so that
    // rField holds the address of the inner cell.
    IfVariant ifVar(a, *rField);
    emitDeref(a, *rField, *rField);
  }
  // decRef the lhs.
  emitDecRefGeneric(*rField);
  // Drop rhs on the lhs.
  a.    store_reg64_disp_reg64(rhsReg, dataDisp, *rField);
  a.    store_imm32_disp_reg(rhsType.outerType(), typeDisp, *rField);
  emitDecRef(instanceReg, KindOfObject);
  emitMovRegReg(rhsReg, outReg);
}

static inline bool
isSupportedSetM(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  return isSupportedSetMArray(i) || isSupportedSetMProp(i);
}

void
TranslatorX64::analyzeSetM(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedSetM(i);
  SKTRACE(2, i.source, "support: %d\n", i.txSupported);
}

void
TranslatorX64::translateSetM(const Tracelet& t,
                             const NormalizedInstruction& i) {
  ASSERT(isSupportedSetM(i));
  if (isSupportedSetMProp(i)) {
    translateSetMProp(t, i);
    return;
  }
  ASSERT(i.inputs.size() >= 2);
  ASSERT(i.inputs[0]->outerType() != KindOfVariant);
  ASSERT(i.inputs[0]->isStack());
  ASSERT(i.inputs[2]->isLocal());
  ASSERT(i.outStack);

  const DynLocation& arr = *i.inputs[2];
  const DynLocation& key = *i.inputs[1];
  const DynLocation& val = *i.inputs[0];

  Location arrLoc = arr.location;
  Location keyLoc = key.location;
  Location valLoc = val.location;

  // Instead of calling allocOutputRegs(), this instruction manually
  // sets up its outputs using bind().
  Location valOutLoc = i.outStack->location;
  // valLoc is about to become dead and we're stealing away its register,
  // so mark it as clean to avoid violating the assertion in bind().
  m_regMap.markAsClean(valLoc);
  // We are about to bind a new register to valOutLoc, so mark it as clean
  // to prevent bind() from writing an old dirty value back to memory.
  m_regMap.markAsClean(valOutLoc);
  PhysReg valReg = m_regMap.getReg(valLoc);
  m_regMap.bind(valReg, valOutLoc, val.outerType(), RegInfo::DIRTY);

  // If either the key or the rhs is not Int64, we will need to pass the
  // rhs by address, so we need to sync it back to memory
  if (!key.isInt() || !val.isInt()) {
    std::set<PhysReg> regs;
    regs.insert(m_regMap.getReg(valOutLoc));
    m_regMap.cleanRegs(regs);
  }

  WithVMRegsSavedStub sav(a, i, m_saveVMRegsStub);
  // The array_setm helpers will decRef any old value that is overwritten if
  // appropriate. If copy-on-write occurs, it will also incRef the new array
  // and decRef the old array for us. Finally, some of the array_setm helpers
  // will decRef the key if it is a string (for cases where the key is not
  // a home), while others do not (for cases where the key is a home).
  bool useBoxedForm = arr.isVariant();
  void* fptr;
  if (false) { // helper type-checks
    TypedValue* cell = NULL;
    ArrayData* arr = NULL;
    TypedValue* rhs = NULL;
    StringData* strKey = NULL;
    UNUSED ArrayData* ret = array_setm_ik1_iv(cell, arr, 12, 3);
    ret = array_setm_ik1_v(cell, arr, 12, rhs);
    ret = array_setm_sk1_v(cell, arr, strKey, rhs);
    ret = array_setm_s0k1_v(cell, arr, strKey, rhs);
  }
  if (key.isInt() && val.isInt()) {
    // If the rhs is Int64, we can use a specialized helper
    if (useBoxedForm) {
      EMIT_CALL4(array_setm_ik1_iv,
                 V(arrLoc),
                 DEREF(arrLoc),
                 V(keyLoc),
                 V(valOutLoc));
    } else {
      EMIT_CALL4(array_setm_ik1_iv,
                 IMM(0),
                 V(arrLoc),
                 V(keyLoc),
                 V(valOutLoc));
    }
  } else {
    // Otherwise, we pass the rhs by address
    bool decRefKey = key.rtt.isString() && i.inputs[1]->isStack();
    fptr = key.rtt.isString() ?
      (decRefKey ? (void*)array_setm_sk1_v :
                   (void*)array_setm_s0k1_v) :
              (void*)array_setm_ik1_v;
    if (useBoxedForm) {
      EMIT_CALL4(fptr,
                 V(arrLoc),
                 DEREF(arrLoc),
                 V(keyLoc),
                 A(valOutLoc));
    } else {
      EMIT_CALL4(fptr,
                 IMM(0),
                 V(arrLoc),
                 V(keyLoc),
                 A(valOutLoc));
    }
  }
  // If we did not used boxed form, we need to tell the register allocator
  // to associate rax with arrLoc
  if (!useBoxedForm) {
    // The array_setm helper returns the up-to-date array pointer in rax.
    // Therefore, we can bind rax to arrLoc and mark it as dirty.
    m_regMap.markAsClean(arrLoc);
    m_regMap.bind(rax, arrLoc, KindOfArray, RegInfo::DIRTY);
  }
}

static bool
isSupportedInstrSetOpH(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  const SetOpOp subOp = (SetOpOp) *(i.pc() + 1);
  Opcode arithOp = setOpOpToOpcodeOp(subOp);
  return i.inputs[0]->isInt() &&
         i.inputs[1]->isInt() &&
         (arithOp == OpAdd || arithOp == OpSub || arithOp == OpMul ||
          arithOp == OpBitAnd || arithOp == OpBitOr || arithOp == OpBitXor);
}

void
TranslatorX64::analyzeSetOpH(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrSetOpH(i);
}

void
TranslatorX64::translateSetOpH(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrSetOpH(i));
  const vector<DynLocation*>& inputs  = i.inputs;
  ASSERT(inputs.size() >= 2);
  ASSERT(i.outStack && i.outLocal);
  const int valIdx  = 0;
  const int homeIdx = 1;
  ASSERT(inputs[homeIdx]->isLocal());
  ASSERT(inputs[valIdx]->isStack());
  ASSERT(inputs[valIdx]->outerType() != KindOfVariant);

  const SetOpOp subOp = (SetOpOp) *(i.pc() + 1);
  Opcode arithOp = setOpOpToOpcodeOp(subOp);
  m_regMap.allocOutputRegs(i);
  binaryArithHome(i, arithOp, *inputs[valIdx], *inputs[homeIdx],
                  *i.outStack);
}

static bool
isSupportedInstrIncDecH(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1 || i.inputs.size() == 2);
  return i.inputs[0]->isInt();
}

void
TranslatorX64::analyzeIncDecH(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrIncDecH(i);
}

void
TranslatorX64::translateIncDecH(const Tracelet& t,
                                const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrIncDecH(i));
  const vector<DynLocation*>& inputs  = i.inputs;
  ASSERT(inputs.size() == 1 || inputs.size() == 2);
  ASSERT(i.outLocal);
  ASSERT(inputs[0]->isLocal());
  const IncDecOp oplet = (IncDecOp) *(i.pc() + 1);
  ASSERT(oplet == PreInc || oplet == PostInc || oplet == PreDec ||
         oplet == PostDec);
  ASSERT(!i.outStack ||
         i.outStack->valueType() == inputs[0]->valueType());
  bool post = (oplet == PostInc || oplet == PostDec);
  bool pre  = !post;
  bool inc  = (oplet == PostInc || oplet == PreInc);

  m_regMap.allocOutputRegs(i);
  PhysReg localVal = getReg(inputs[0]->location);
  if (i.outStack && post) { // $a++, $a--
    PhysReg output   = getReg(i.outStack->location);
    emitMovRegReg(localVal, output);
  }
  if (inc) {
    a.  inc_reg64(localVal);
  } else {
    a.  dec_reg64(localVal);
  }
  if (i.outStack && pre) { // --$a, ++$a
    PhysReg output   = getReg(i.outStack->location);
    emitMovRegReg(localVal, output);
  }
}

void
TranslatorX64::analyzeUnsetH(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateUnsetH(const Tracelet& t,
                               const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  ASSERT(!i.outStack && i.outLocal);
  const int homeIdx = 0;
  const DynLocation& homeDl = *i.inputs[homeIdx];
  ASSERT(homeDl.isLocal());

  // We have to mark the output register as dirty to ensure that
  // the type gets spilled at the tend of the tracelet
  m_regMap.allocOutputRegs(i);

  DataType ht = homeDl.outerType();
  // decRef the value that currently lives in the local if appropriate.
  if (IS_REFCOUNTED_TYPE(ht)) {
    emitDecRef(getReg(homeDl.location), ht);
  }
}

static bool
isSupportedInstrUnsetM(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 2);
  const RuntimeType &arrayRtt = i.inputs[1]->rtt;
  const RuntimeType &keyRtt = i.inputs[0]->rtt;
  ImmVector* iv = i.immVecPtr;
  ASSERT(iv);
  return iv->len == 2 && iv->get(0) == LH && iv->get(1) == ME &&
         i.inputs.size() == 2 &&
         keyRtt.isString() &&
         arrayRtt.valueType() == KindOfArray &&
         !arrayRtt.isVariant();
}

void
TranslatorX64::analyzeUnsetM(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrUnsetM(i);
}

void
TranslatorX64::translateUnsetM(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrUnsetM(i));
  dumpInstrInputs(1, i);
  bool decRefKey = i.inputs[0]->isStack();
  ASSERT(i.inputs[1]->isLocal());
  ASSERT(!i.outStack && !i.outLocal);

  Location arrLoc = i.inputs[1]->location;
  Location keyLoc = i.inputs[0]->location;

  WithVMRegsSavedStub sav(a, i, m_saveVMRegsStub);

  if (decRefKey) {
    if (false) { // type check
      ArrayData *arr = NULL;
      StringData *sd = NULL;
      arr = array_unsetm_s(arr, sd);
    }
    // array_unsetm_s will decRef the key for us, if the key is present in the
    // array is will also decRef the unset value for us if appropriate. If
    // copy-on-write is triggered, it will also incRef the new array and decRef
    // the old array for us.
    EMIT_CALL2(array_unsetm_s, V(arrLoc), V(keyLoc));
  } else {
    if (false) { // type check
      ArrayData *arr = NULL;
      StringData *sd = NULL;
      arr = array_unsetm_s0(arr, sd);
    }
    // If the key is present in the array, array_unsetm_s0 will decRef the
    // unset value for us if appropriate. If copy-on-write is triggered, it
    // will also incRef the new array and decRef the old array for us.
    EMIT_CALL2(array_unsetm_s0, V(arrLoc), V(keyLoc));
  }
  m_regMap.invalidate(arrLoc);
  m_regMap.bind(rax, arrLoc, KindOfArray, RegInfo::DIRTY);
}

void
TranslatorX64::analyzeFPushFunc(Tracelet& t, NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 1);
  // The input might be an object implementing __invoke()
  i.txSupported = i.inputs[0]->isString();
}

void
TranslatorX64::translateFPushFunc(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  using namespace TargetCache;
  CacheHandle ch = FuncCache::alloc();
  ASSERT(i.inputs.size() == 1);
  Location& inLoc = i.inputs[0]->location;

  m_regMap.allocOutputRegs(i);
  m_regMap.scrubStackRange(i.stackOff - 1,
                           i.stackOff - 1 + kNumActRecCells);
  // Popped one cell, pushed an actrec
  int startOfActRec = int(sizeof(Cell)) - int(sizeof(ActRec));
  size_t funcOff = offsetof(ActRec, m_func) + startOfActRec;
  size_t thisOff = offsetof(ActRec, m_this) + startOfActRec;
  emitVStackStoreImm(a, i, 0, thisOff, sz::qword, &m_regMap);
  emitPushAR(i, NULL, sizeof(Cell) /* bytesPopped */);
  if (false) { // typecheck
    StringData sd("foo");
    const UNUSED Func* f = FuncCache::lookup(ch, &sd);
  }
  SKTRACE(1, i.source, "ch %d\n", ch);
  EMIT_CALL2(FuncCache::lookup, IMM(ch), V(inLoc));
  emitVStackStore(a, i, rax, funcOff, sz::qword);
}

void
TranslatorX64::analyzeFPushObjMethodD(Tracelet& t,
                                      NormalizedInstruction &i) {
  DynLocation* objLoc = i.inputs[0];
  i.txSupported = objLoc->valueType() == KindOfObject;
}

void
TranslatorX64::translateFPushObjMethodD(const Tracelet &t,
                                        const NormalizedInstruction& i) {
  using namespace TargetCache;
  CacheHandle ch = MethodCache::alloc();
  ASSERT(i.inputs.size() == 1);
  Location& objLoc = i.inputs[0]->location;
  ASSERT(i.inputs[0]->valueType() == KindOfObject);

  m_regMap.allocOutputRegs(i);
  m_regMap.scrubStackRange(i.stackOff - 1,
                           i.stackOff - 1 + kNumActRecCells);
  int id = i.imm[1].u_IVA;
  const StringData* name = curUnit()->lookupLitstrId(id);
  // Popped one cell, pushed an actrec
  int startOfActRec = int(sizeof(Cell)) - int(sizeof(ActRec));
  size_t thisOff = offsetof(ActRec, m_this) + startOfActRec;
  // Save $this before we lose it in the helper call.
  emitVStackStore(a, i, getReg(objLoc), thisOff, sz::qword);
  // Save fp in case looking up a private method via call context, as well as
  // pc in case a fatal occurs.
  ScratchReg scratch(m_regMap);
  emitImmReg(a, i.offset(), *scratch);
  WithVMRegsSaved wsv(a, false, true, rScratch, rScratch, *scratch);
  emitPushAR(i, NULL, sizeof(Cell) /*bytesPopped*/);
  if (false) { // typecheck
    ActRec* ar = NULL;
    MethodCache::lookup(ch, ar, name);
  }
  int arOff = vstackOffset(i, startOfActRec);
  SKTRACE(1, i.source, "ch %d\n", ch);
  EMIT_CALL3(MethodCache::lookup, IMM(ch),
             RPLUS(rVmSp, arOff), IMM(uint64_t(name)));
}

void
TranslatorX64::analyzeThis(Tracelet &t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateThis(const Tracelet &t,
                             const NormalizedInstruction &i) {
  ASSERT(i.outStack && !i.outLocal);
  m_regMap.allocOutputRegs(i);
  PhysReg out = getReg(i.outStack->location);
  a.   load_reg64_disp_reg64(rVmFp, offsetof(ActRec, m_this), out);

  // XXX: To implement: Properly fatal on null $this. This just exits.
  TCA exitSR = emitServiceReq(REQ_EXIT, 0);
  // TODO: We can do better here. If we know that we are in a method
  // then there must be a this or a class, so we can omit the first
  // check here.
  a.   test_reg64_reg64(out, out);
  a.   jz(exitSR);
  a.   test_imm32_reg64(1, out);
  a.   jnz(exitSR);

  emitIncRef(out, KindOfObject);
}

void
TranslatorX64::analyzeFPushFuncD(Tracelet& t, NormalizedInstruction& i) {
  const StringData* name = curUnit()->lookupLitstrId(i.imm[1].u_SA);
  const Func* func = g_context->lookupFunc(name);
  i.txSupported = (func != NULL);
}

void
TranslatorX64::translateFPushFuncD(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 0);
  ASSERT(!i.outStack && !i.outLocal);
  const StringData* name = curUnit()->lookupLitstrId(i.imm[1].u_SA);
  const Func* func       = g_context->lookupFunc(name);

  // Translation is only supported if function lookup succeeds
  ASSERT(func);
  if (Trace::enabled && !func) {
    TRACE(1, "Attempt to invoke undefined function %s\n", name->data());
  }

  m_regMap.allocOutputRegs(i);
  // Inform the register allocator that we just annihilated a range of
  // possibly-dirty stack entries.
  m_regMap.scrubStackRange(i.stackOff,
                           i.stackOff + kNumActRecCells);

  if (func->isNameBindingImmutable(curUnit())) {
    // We're guaranteed that no matter when we did the lookupFunc(name) above,
    // the resultant Func is always the same. It's safe to burn the Func into
    // the translation.
    emitPushAR(i, func);
  } else {
    // That Func could be different come runtime. Use a FuncCache.
    using namespace TargetCache;
    CacheHandle ch = FixedFuncCache::alloc(name);
    size_t funcOff = offsetof(ActRec, m_func) - sizeof(ActRec);
    size_t thisOff = offsetof(ActRec, m_this) - sizeof(ActRec);
    size_t funcCacheOff = ch + offsetof(FixedFuncCache, m_func);

    emitVStackStoreImm(a, i, 0, thisOff, sz::qword, &m_regMap);
    emitPushAR(i, NULL);
    if (false) { // typecheck
      StringData sd("foo");
      const UNUSED Func* f = FixedFuncCache::lookup(ch, &sd);
    }
    SKTRACE(1, i.source, "ch %d\n", ch);

    // We *might* make a function call below, so we have to
    // unconditionally save the caller save registers here. One nice
    // side effect of this is that we can use rax as a free scratch
    // register.
    emitCallSaveRegs();
    emitTLSLoad(a, &tl_targetCaches.base, rax);
    a.load_reg64_disp_reg64(rax, funcCacheOff, rax);
    a.test_reg64_reg64(rax, rax);
    {
      JccBlock<CC_NZ> cond(a);
      EMIT_CALL2(FixedFuncCache::lookup, IMM(ch),
                 IMM(uintptr_t(name)));
    }
    emitVStackStore(a, i, rax, funcOff, sz::qword);
  }
}

static bool isSupportedFPassCOp(const NormalizedInstruction& i) {
  const Opcode op = i.op();
  ASSERT(op == OpFPassC || op == OpFPassCW || op == OpFPassCE);
  if (op == OpFPassC) {
    return true;
  }
  return !i.preppedByRef;
}

void
TranslatorX64::analyzeFPassCOp(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedFPassCOp(i);
}

void
TranslatorX64::translateFPassCOp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  ASSERT(isSupportedFPassCOp(i));
  ASSERT(i.inputs.size() == 0);
  ASSERT(!i.outStack && !i.outLocal);
}

void
TranslatorX64::analyzeFPassM(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported =
    // TODO: once we have VGetM, implement the byRef case in terms of it.
    !i.preppedByRef &&
    // Implement this in terms of CGetM.
    isSupportedCGetM(i);
  TRACE(2, "FPassM: isSupported %d preppedByRef %d\n",
        i.txSupported, i.preppedByRef);
}

void
TranslatorX64::translateFPassM(const Tracelet& t,
                               const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() >= 1);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT_NOT_IMPLEMENTED(!i.preppedByRef);
  translateCGetM(t, i);
}


void
TranslatorX64::analyzeFPassR(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateFPassR(const Tracelet& t,
                               const NormalizedInstruction& i) {
  /*
   * Like FPassC, FPassR is able to cheat on boxing if the current
   * parameter is pass by reference but we have a cell: the box would refer
   * to exactly one datum (the value currently on the stack).
   *
   * However, if the callee wants a cell and we have a variant we must
   * unbox; otherwise we might accidentally make callee changes to its
   * parameter globally visible.
   */
  ASSERT(i.inputs.size() == 1);
  const RuntimeType& inRtt = i.inputs[0]->rtt;
  if (inRtt.isVariant() && !i.preppedByRef) {
    emitUnboxTopOfStack(i);
  }
}

void
TranslatorX64::analyzeFCall(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = true;
}

void
TranslatorX64::translateFCall(const Tracelet& t,
                              const NormalizedInstruction& i) {
  // Save outputs and adjust the stack pointer, to make the chaining to
  // some manual manipulation.
  m_regMap.allocOutputRegs(i);
  syncOutputs(t);
  int numArgs = i.imm[0].u_IVA;

  emitLoadDynTracer();
  a.test_reg64_reg64(rScratch, rScratch);
  {
    UnlikelyIfBlock<CC_NZ> ifTracer(a, astubs);
    astubs.lea_reg64_disp_reg64(rVmSp, cellsToBytes(numArgs),
                                argNumToRegName[0]);
    astubs.call((TCA)&DynTracer::FunctionEnter);
    m_regMap.smashPhysRegs(callerSaved());
  }

  const Opcode* atCall = i.pc();
  const Opcode* after = i.pc();
  advance(&after);

  // Caller-specific fields: return addresses and the frame pointer
  // offset.
  ASSERT(sizeof(Cell) == 1 << 4);
  const Func* srcFunc = curFunc();
  // Record the hardware return address. This will be patched up below; 2
  // is a magic number dependent on assembler implementation.
  uint64 *retIP = (uint64*)(a.code.frontier + 2);
  emitImmReg(a, kUninitializedRIP, rax);
  ASSERT(*retIP == kUninitializedRIP);
  a.    store_reg64_disp_reg64 (rax,
                                cellsToBytes(numArgs) + offsetof(ActRec, m_savedRip),
                                rVmSp);

  // The kooky offset here a) gets us to the current ActRec (i.imm[0] for
  // FCall is the number of args), and b) accesses m_soff.
  int32 callOffsetInUnit = srcFunc->m_unit->offsetOf(after - srcFunc->m_base);
  a.    store_imm32_disp_reg(callOffsetInUnit,
                             cellsToBytes(numArgs) + offsetof(ActRec, m_soff),
                             rVmSp);

  emitBindCall(i, curUnit()->offsetOf(atCall),
                  curUnit()->offsetOf(after)); // ...
  *retIP = uint64(a.code.frontier);

  SrcKey fallThru(curUnit(), after);
  emitBindJmp(fallThru);
}

static bool
isSupportedInstrIterInit(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  return (i.inputs[0]->valueType() == KindOfArray);
}

void
TranslatorX64::analyzeIterInit(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrIterInit(i);
}

void
TranslatorX64::translateIterInit(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrIterInit(i));
  ASSERT(i.inputs.size() == 1);
  ASSERT(!i.outStack && !i.outLocal);
  DynLocation* arrIn = i.inputs[0];
  ASSERT(arrIn->outerType() != KindOfVariant);

  PhysReg src = getReg(arrIn->location);
  SKTRACE(1, i.source, "IterInit: committed to translation\n");
  syncOutputs(t); // Ends BB
  SrcKey notTaken, taken;
  branchDests(i, &notTaken, &taken, 1 /* immIdx */);
  Location iterLoc(Location::Iter, i.imm[0].u_IVA);
  PhysReg base;
  int offset;
  locToRegDisp(iterLoc, &base, &offset);
  TRACE(2, "IterInit: iter %d -> base r%d, offset %d\n", iterLoc.offset,
      base, offset);
  if (false) {
    Iter *dest = NULL;
    HphpArray *arr = NULL;
    new_iter(dest, arr);
  }
  // We've already synced the outputs, so we need to manually
  // set up the parameters and do the call
  emitMovRegReg(src, argNumToRegName[1]);
  a.    lea_reg64_disp_reg64(base, offset, argNumToRegName[0]);
  // If a new iterator is created, new_iter will not adjust the refcount of
  // the array. If a new iterator is not created, new_iter will decRef the
  // array for us.
  a.    call((TCA)new_iter);
  // new_iter returns 0 if an iterator was not created, otherwise it
  // returns 1
  a.    test_reg64_reg64(rax, rax);
  TCA toPatch = a.code.frontier;
  a.    jz(0); // 1f
  emitBindJmp(notTaken);
  // 1:
  a.patchJcc(toPatch, a.code.frontier);
  emitBindJmp(taken);
  translator_not_reached(a);
}

static bool
isSupportedInstrIterValueC(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  return (i.inputs[0]->rtt.iterType() == Iter::TypeArray);
}

void
TranslatorX64::analyzeIterValueC(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrIterValueC(i);
}

void
TranslatorX64::translateIterValueC(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrIterValueC(i));
  ASSERT(i.inputs.size() == 1);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.inputs[0]->rtt.isIter());

  Location outLoc = i.outStack->location;
  if (false) { // type check
    Iter* it = NULL;
    TypedValue* out = NULL;
    iter_value_cell(it, out);
  }
  EMIT_CALL2(iter_value_cell, A(i.inputs[0]->location), A(outLoc));
  m_regMap.invalidate(outLoc);
}

static bool
isSupportedInstrIterKey(const NormalizedInstruction& i) {
  return (i.inputs[0]->rtt.iterType() == Iter::TypeArray);
}

void
TranslatorX64::analyzeIterKey(Tracelet& t, NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  i.txSupported = isSupportedInstrIterKey(i);
}

void
TranslatorX64::translateIterKey(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrIterKey(i));
  ASSERT(i.inputs.size() == 1);
  ASSERT(i.outStack && !i.outLocal);
  ASSERT(i.inputs[0]->rtt.isIter());

  Location outLoc = i.outStack->location;
  if (false) { // type check
    Iter* it = NULL;
    TypedValue* out = NULL;
    iter_key_cell(it, out);
  }
  EMIT_CALL2(iter_key_cell, A(i.inputs[0]->location), A(outLoc));
  m_regMap.invalidate(outLoc);
}

static bool
isSupportedInstrIterNext(const NormalizedInstruction& i) {
  ASSERT(i.inputs.size() == 1);
  return (i.inputs[0]->rtt.iterType() == Iter::TypeArray);
}

void
TranslatorX64::analyzeIterNext(Tracelet& t, NormalizedInstruction& i) {
  i.txSupported = isSupportedInstrIterNext(i);
}

void
TranslatorX64::translateIterNext(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  ASSERT(isSupportedInstrIterNext(i));
  ASSERT(i.inputs.size() == 1);
  ASSERT(!i.outStack && !i.outLocal);
  ASSERT(i.inputs[0]->rtt.isIter());

  if (false) { // type check
    Iter* it = NULL;
    int64 ret = iter_next_array(it);
    if (ret) printf("\n");
  }
  m_regMap.cleanAll(); // input might be in-flight
  // If the iterator reaches the end, iter_next_array will handle
  // freeing the iterator and it will decRef the array
  EMIT_CALL1(iter_next_array, A(i.inputs[0]->location));
  // RAX is now a scratch register with no progam meaning...
  m_regMap.bind(rax, Location(), KindOfInvalid, RegInfo::SCRATCH);

  // syncOutputs before we handle the branch.
  syncOutputs(t);
  SrcKey notTaken, taken;
  branchDests(i, &notTaken, &taken, 1 /* destImmIdx*/);

  a.   test_reg64_reg64(rax, rax);
  TCA toPatch = a.code.frontier;
  a.   jnz(0); // 1f
  // Fall through; iter_next_array returned 0, so we have reached
  // the end of the array
  emitBindJmp(notTaken);
  // 1:
  a.   patchJcc(toPatch, a.code.frontier);
  emitBindJmp(taken);
  translator_not_reached(a);
}

// PSEUDOINSTR_DISPATCH is a switch() fragment that routes opcodes to their
// shared handlers, as per the PSEUDOINSTRS macro.
#define PSEUDOINSTR_DISPATCH(prefix) \
  case OpBitAnd: \
  case OpBitOr: \
  case OpBitXor: \
  case OpSub: \
  case OpMul: \
    prefix ## BinaryArithOp(t, i); \
    break; \
  case OpSame: \
  case OpNSame:  \
    prefix ## SameOp(t, i); \
    break; \
  case OpEq: \
  case OpNeq:  \
    prefix ## EqOp(t, i); \
    break; \
  case OpLt: \
  case OpLte:  \
  case OpGt: \
  case OpGte:  \
    prefix ## LtGtOp(t, i); \
    break; \
  case OpEmptyH: \
  case OpCastBool:  \
    prefix ## UnaryBooleanOp(t, i); \
    break; \
  case OpJmpZ: \
  case OpJmpNZ:  \
    prefix ## BranchOp(t, i); \
    break; \
  case OpCGetH: \
  case OpCGetH2: \
    prefix ## CGetHOp(t, i); \
    break; \
  case OpSetH: \
  case OpBindH: \
    prefix ## AssignToLocalOp(t, i); \
    break; \
  case OpFPassC: \
  case OpFPassCW: \
  case OpFPassCE: \
    prefix ## FPassCOp(t, i); \
    break;

void
TranslatorX64::analyzeInstr(Tracelet& t,
                            NormalizedInstruction& i) {
  const Opcode op = i.op();
  switch (op) {
#define CASE(iNm) \
  case Op ## iNm: { \
    analyze ## iNm(t, i); \
  } break;

  INSTRS
  PSEUDOINSTR_DISPATCH(analyze)

#undef CASE
    default: {
      i.txSupported = false;
    }
  }
}

void
TranslatorX64::translateInstr(const Tracelet& t,
                              const NormalizedInstruction& i) {
  /**
   * translateInstr() translates an individual instruction in a tracelet,
   * either by directly emitting machine code for that instruction or by
   * emitting a call to the interpreter.
   *
   * If the instruction ends the current tracelet, we must emit machine code
   * to transfer control to some target that will continue to make forward
   * progress. This target may be the beginning of another tracelet, or it may
   * be a translator service request. Before transferring control, a tracelet
   * must ensure the following invariants hold:
   *   1) The machine registers rVmFp and rVmSp are in sync with vmfp()
   *      and vmsp().
   *   2) All "dirty" values are synced in memory. This includes the
   *      evaluation stack, locals, globals, statics, and any other program
   *      accessible locations. This also means that all homes must be spilled
   *      to the evaluation stack and all refcounts must be up to date.
   */
  ASSERT(!i.outStack || i.outStack->isStack());
  ASSERT(!i.outLocal || i.outLocal->isLocal());

  if (!i.txSupported || RuntimeOption::EvalThreadingJit) {
    // If the problem is local to this instruction, just call out to
    // the interpreter. emitInterpOne will perform end-of-tracelet duties
    // if this instruction ends the tracelet.
    SKTRACE(1, i.source, "Interp\n");
    emitInterpOne(t, i);
    return;
  }

  const char *opNames[] = {
#define O(name, imm, push, pop, flags) \
#name,
  OPCODES
#undef O
  };
  SpaceRecorder sr(opNames[i.op()], a);
  SKTRACE(1, i.source, "translate %#lx\n", long(a.code.frontier));
  const Opcode op = i.op();
  // Allocate the input regs upfront for all instructions
  // except RetC, which is special
  if (op != OpRetC) {
    m_regMap.allocInputRegs(i);
  }
  switch (op) {
#define CASE(iNm) \
    case Op ## iNm: \
      translate ## iNm(t, i); \
      break;
    INSTRS
    PSEUDOINSTR_DISPATCH(translate)
#undef CASE
    default: ASSERT(false);
  }
  // Invalidate locations that are no longer live
  for (unsigned k = 0; k < i.deadLocs.size(); ++k) {
    const Location& l = i.deadLocs[k];
    m_regMap.invalidate(l);
  }
  if (i.breaksBB && !opcodeBreaksBB(i.op())) {
    // If this instruction's opcode always ends the tracelet then the
    // instruction case is responsible for performing end-of-tracelet
    // duties. Otherwise, we handle ending the tracelet here.
    syncOutputs(t);
    SrcKey dest = i.source;
    dest.advance(curUnit());
    emitBindJmp(dest);
  }
}

static const PhysReg regs[] = {
  /*
   * Allocate this tracelet's values from this set of registers
   * statically. (Yes, this is fairly bush-league.)
   *
   * Favor the new x64 regs. They exercise the assembler more thoroughly,
   * overlap with calling conventions and special-purpose registers less,
   * and several are callee-saved.
   *
   * Unfortunately, they make us emit bulkier code due to rex prefixes.
   */
  r12, r13, r14, r15, r8, r9, r11,
    /*
     * XXX: rbp == rVmFp, rbx == rVmSp, r10 == rScratch are
     * reserved. rsp belongs to the C++ runtime.
     */
  // The function call regs, in descending likelihood of conflict
  rdx, rcx, rsi, rdi, rax,
};

vector<PhysReg>
TranslatorX64::x64TranslRegs() {
  vector<PhysReg> retval;
  for (unsigned i = 0; i < sizeof(regs) / sizeof(regs[0]); i++) {
    retval.push_back(regs[i]);
  }
  return retval;
}

static void
extendCountersVec(TraceletCountersVec *vec, int64_t newSize) {
  if (newSize <= vec->m_size)
    return;

  Lock lock(vec->m_lock);
  vec->m_elms = (TraceletCounters*)realloc(vec->m_elms,
                                           newSize * sizeof(TraceletCounters));
  memset(&vec->m_elms[vec->m_size], 0,
         (newSize - vec->m_size) * sizeof(TraceletCounters));
  vec->m_size = newSize;
}

/**
 * Emit a counter increment for the given tracelet.
 *
 * This has two modes of operation, given by Trace::counters:
 *   1 = single-thread mode, few threads
 *       only the "lucky" counter thread get an increment
 *       the increment is on the likely path
 *   2 = single-thread mode, many threads
 *       the increment is on the unlikely path
 *   3 = all-threads mode; each thread gets its own separate set of counters
 */
class WithCounters {
  TranslatorX64 &m_tx64;
  bool m_allThreads;

  // These must be callee-saved and preserved from
  // the constructor all the way to the final call to
  // emitIncCounter.
  static const PhysReg rThreadIdx  = r12;
  static const PhysReg rCounterPtr = r13;

public:
  WithCounters(TranslatorX64 &tx64, TCA start)
    : m_tx64(tx64) {
    if (!moduleEnabled(Trace::counters, 1))
      return;

    TraceletCountersVec *vec = NULL;
    {
      TranslatorX64::TraceletCountersVecMap::accessor itt;
      tx64.m_threadCounters.insert(itt, start);
      if (!itt->second.get())
        itt->second.reset(new TraceletCountersVec());
      vec = itt->second.get();
      // The accessor lock can be released now, since the vector has
      // its own lock on the list of elements
    }

    // Pre-allocate the vector, so that the check is quicker at run-time
    m_allThreads = moduleEnabled(Trace::counters, 3);
    if (m_allThreads) {
      extendCountersVec(vec, ExecutionContext::s_threadIdxCounter);
    } else {
      extendCountersVec(vec, 1);
    }

    X64Assembler &a(m_tx64.a);
    PhysReg rEC = r8, rVec = r14; // rVec must be callee-saved
    emitGetGContext(a, rEC);
    a.      load_reg64_disp_reg64(rEC, offsetof(ExecutionContext,
                                                m_currentThreadIdx),
                                  rThreadIdx);
    if (m_allThreads) {
      emitImmReg(a, (intptr_t)vec, rVec);
      a.    cmp_reg64_disp_reg64(rThreadIdx, offsetof(TraceletCountersVec,
                                                      m_size), rVec);
      {
        JccBlock<CC_G> ifLessEq(a);
        // if rVec->m_size <= rThreadIdx, then...
        void (*extendCountersVecPtr)(TraceletCountersVec*, int64_t)
              = extendCountersVec;
        // rEC isn't needed any more, so don't save it
        a.  mov_reg64_reg64(rVec, rdi);
        a.  lea_reg64_disp_reg64(rThreadIdx, 1, rsi);
        a.  call((TCA)extendCountersVecPtr);
      }

      // Compute the pointer to the element
      CT_ASSERT(sizeof(TraceletCounters) == 16);
      a.    load_reg64_disp_reg64(rVec, offsetof(TraceletCountersVec,
                                                 m_elms), rCounterPtr);
      a.    shl_imm32_reg64(4, rThreadIdx);
      a.    add_reg64_reg64(rThreadIdx, rCounterPtr);
    } else {
      ASSERT(m_tx64.m_counterThreadIdx != -1);
      emitImmReg(a, (intptr_t)vec->m_elms, rCounterPtr);
    }
  }

  void emitIncCounter(int cntOfs) {
    if (!moduleEnabled(Trace::counters, 1))
      return;

    if (m_allThreads) {
      m_tx64.a.    inc_mem64(rCounterPtr, cntOfs);
    } else {
      m_tx64.a.    cmp_imm64_reg64(m_tx64.m_counterThreadIdx, rThreadIdx);
      if (moduleEnabled(Trace::counters, 2)) {
        // This codepath is unlikely, so put it in astubs
        // For tests with low number of threads (all of verify_perf_jit)
        // this is actually much slower than putting it in a. and using CC_NE
        // However, the expected case here is that there are a lot of threads
        UnlikelyIfBlock<CC_E> ifEqual(m_tx64.a, m_tx64.astubs);
        m_tx64.astubs. inc_mem64(rCounterPtr, cntOfs);
      } else {
        JccBlock<CC_NE> ifEqual(m_tx64.a);
        m_tx64.a.      inc_mem64(rCounterPtr, cntOfs);
      }
    }
  }
};

void
TranslatorX64::translateTracelet(const Tracelet& t) {
  const SrcKey &sk = t.m_sk;

  SKTRACE(1, sk, "translateTracelet\n");
  ASSERT(srcDB.find(sk) != srcDB.end());
  TCA        start = a.code.frontier;
  TCA    stubStart = astubs.code.frontier;
  SrcRec &fallback = srcDB[sk];

  try {
    if (t.m_analysisFailed) {
      punt();
    }
    WithCounters ctrs(*this, start);
    // Increment enter counter for this tracelet
    ctrs.emitIncCounter(offsetof(TraceletCounters, m_numEntered));

    // Check input prereqs.
    for (DepMap::const_iterator dep = t.m_dependencies.begin();
         dep != t.m_dependencies.end();
         ++dep) {
      checkType(dep->first, dep->second->rtt, fallback);
    }

    // Check any param reffiness prereqs.
    checkRefs(t, fallback);

    if (srcDB[t.m_sk].translations.size() == SrcRec::kMaxTranslations) {
#ifdef DEBUG
      if (Trace::enabled) {
        SrcDB::iterator entry = srcDB.find(t.m_sk);
        ASSERT(entry != srcDB.end());
        const vector<TCA>& tns = srcDB[t.m_sk].translations;
        TRACE(1, "Too many (%ld) translations: %s, BC offset %d\n",
              tns.size(), curUnit()->m_filepath->data(),
              t.m_sk.offset());
        SKTRACE(2, t.m_sk, "{\n", tns.size());
        TCA topTrans = entry->second.getTopTranslation();
        for (size_t i = 0; i < tns.size(); ++i) {
          const TransRec& rec = transDB[tns[i]];
          SKTRACE(2, t.m_sk, "%d %p\n", i, tns[i]);
          if (tns[i] == topTrans) {
            SKTRACE(2, t.m_sk, "%d: *Top*\n", i);
          }
          if (rec.isAnchor) {
            SKTRACE(2, t.m_sk, "%d: Anchor\n", i);
          } else {
            SKTRACE(2, t.m_sk, "%d: guards {\n", i);
            for (unsigned j = 0; j < rec.dependencies.size(); ++j) {
              TRACE(2, rec.dependencies[j]);
            }
            SKTRACE(2, t.m_sk, "%d } guards\n", i);
          }
        }
        SKTRACE(2, t.m_sk, "} /* Too many translations */\n");
      }
#endif
      punt();
    }
    // We got this far. Inputs all match.
#ifdef DEBUG
    TRACE(3, "----------------------------------------------\n");
    TRACE(3, "  Translating from file %s at %p:\n",
          curUnit()->m_filepath->data(),
          a.code.frontier);
    TRACE(3, "  preconds:\n");
    for (DepMap::const_iterator i = t.m_dependencies.begin();
         i != t.m_dependencies.end(); ++i) {
      TRACE(3, "%-5s\n", i->second->pretty().c_str());
    }
    TRACE(3, "  postconds:\n");
    for (ChangeMap::const_iterator i = t.m_changes.begin();
         i != t.m_changes.end(); ++i) {
      TRACE(3, "%-5s\n", i->second->pretty().c_str());
    }
    for (NormalizedInstruction* ni = t.m_instrStream.first; ni;
         ni = ni->next) {
      string s = instrToString(curUnit()->at(ni->offset()), curUnit());
      TRACE(3, "  %6d: %s\n", ni->offset(), s.c_str());
    }
    TRACE(3, "----------------------------------------------\n");
    // prettyStack() expects to use vmpc(). Leave it in the state we
    // found it since this code is debug-only, and we don't want behavior
    // to vary across the optimized/debug builds.
    PC oldPC = vmpc();
    vmpc() = curUnit()->at(sk.offset());
    TRACE(3, g_context->prettyStack(string(" tx64 ")));
    vmpc() = oldPC;
    TRACE(3, "----------------------------------------------\n");
#endif
    // Increment execute counter for this tracelet
    ctrs.emitIncCounter(offsetof(TraceletCounters, m_numExecuted));

    // Translate each instruction in the tracelet
    for (NormalizedInstruction* ni = t.m_instrStream.first; ni; ni = ni->next) {
      translateInstr(t, *ni);
    }
  } catch (TranslationFailedExc& tfe) {
    // The whole translation failed; give up on this BB. Since it is not
    // linked into srcDB yet, it is guaranteed not to be reachable.
    m_regMap.reset();
    // Permanent reset; nothing is reachable yet.
    a.code.frontier = start;
    astubs.code.frontier = stubStart;
    TRACE(1, "emitting %d-instr interp request for failed translation @%s:%d\n",
          int(t.m_numOpcodes), tfe.m_file, tfe.m_line);
    a.    jmp(
      emitServiceReq(REQ_INTERPRET, 2, t.m_sk.offset(), t.m_numOpcodes));
    // Fall through.
  }
  // SrcRec::newTranslation() makes this code reachable.
  TRACE(1, "newTranslation: %p\n", start);
  srcDB[t.m_sk].newTranslation(a, start);
  transDB[start] = TransRec(t.m_sk, t);
  if (astubs.code.frontier != stubStart) {
    transDB[stubStart] = TransRec(t.m_sk, t);
  }
  recordGdbTranslation(sk, curUnit(), start, a.code.frontier - start, false, false);
  m_regMap.reset();
  TRACE(1, "tx64: %zd-byte tracelet\n", a.code.frontier - start);
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease("tx64: %zd bytes in cache\n",
                        a.code.frontier - a.code.base);
  }
}

TranslatorX64::TranslatorX64()
: Translator(),
  m_spf(a, m_regMap),
  m_regMap(x64TranslRegs(), &m_spf),
  m_funcPrologues()
{
  TRACE(1, "TranslatorX64@%p startup\n", this);
}

TranslatorX64*
TranslatorX64::Get() {
  if (!tx64) {
    tx64 = new TranslatorX64();
  }
  return tx64;
}

TCA
TranslatorX64::emitUnaryStub(void* fptr, bool savePC) {
  // These dtor stubs are meant to be called with the call
  // instruction, unlike most translator code.
  moveToAlign(astubs);
  TCA start = astubs.code.frontier;
  // Preserve most caller-saved regs. The calling code has already
  // preserved rdi; we push eight more regs (the 7 below plus rbp in
  // AsmFunction), occupying 9 qwords in total, and the incoming
  // call instruction made it 10. This is an even number of pushes,
  // so we preserve the SSE-friendliness of our execution environment.
  if (savePC) astubs.    mov_reg64_reg64(rbp, rScratch);
  astubs.    pushr(rbp); // {
  astubs.    mov_reg64_reg64(rsp, rbp);
  {
    set<PhysReg> s = callerSaved();
    s.erase(rdi);
    WithPhysRegSaver rs(astubs, s);
    // XXX terrible hack.
    //
    // We need a valid PC in case we re-enter the VM. Unfortunately,
    // this stub is shared by lots of TC entry points; we don't know
    // which PC called us. We could start passing more than one arg,
    // but it's unnecessary.
    //
    // For normal control flow, re-entry will not need a valid PC here;
    // when the stub is done, control will pass back to the TC.
    //
    // The "exception" is exceptions. However, they fatal in destructors,
    // and exceptions are the only reason to unwind through a destructor,
    // and destructors are the only unary stubs that re-enter! So this
    // actually, probably works. It does look really broken, though.
    emitImmReg(astubs, 0, rsi); // The 0'th byte of whatever
                                // unit we're currently running.
    if (savePC) WithVMRegsSaved::emitSaveVMRegisters(astubs,
                                                     false, true,
                                                     rax, rScratch,
                                                     rsi, rScratch);
    astubs.    call(TCA(fptr));
    if (savePC) WithVMRegsSaved::emitVolatilizeVMRegisters(astubs);
  }
  astubs.    popr(rbp);  // }
  astubs.    ret();
  return start;
}

void
TranslatorX64::callUnaryStub(TCA stub, PhysReg arg, int disp/*=0*/) {
  // Call the generic dtor stub. They all take one arg.
  a.    pushr(rdi);
  if (disp == 0) {
    emitMovRegReg(arg, rdi);
  } else {
    a.    lea_reg64_disp_reg64(arg, disp, rdi);
  }
  ASSERT(astubs.code.isValidAddress(stub));
  a.    call(stub);
  a.    popr(rdi);
}

void
TranslatorX64::requestInit() {
  TRACE(1, "in requestInit(%ld)\n", g_context->m_currentThreadIdx);
  requestResetHighLevelTranslator();
  TargetCache::requestInit();

  if (m_counterThreadIdx == -1) {
    // Don't care about race conditions here, as long as
    // -1 gets replaced by some unique valid threadId
    // that never gets replaced later by another one
    m_counterThreadIdx = g_context->m_currentThreadIdx;
    TRACE_MOD(Trace::counters, 1, "counter thread: %ld\n",
              m_counterThreadIdx);
  }
}

void
TranslatorX64::requestExit() {
  int64_t threadIdx = g_context->m_currentThreadIdx;
  if (!moduleEnabled(Trace::counters, 3)) {
    // 1000 is just a placeholder; there is only one element in m_elms
    // so the only valid value is 0
    threadIdx = (threadIdx == m_counterThreadIdx) ? 0 : 1000;
  }
  for (TraceletCountersVecMap::const_iterator it = m_threadCounters.begin();
       it != m_threadCounters.end(); ++it) {
    if (threadIdx >= it->second->m_size)
      continue;

    TraceletCounters &tc  = it->second->m_elms[threadIdx];
    TraceletCountersMap::accessor itg;
    m_globalCounters.insert(itg, it->first);
    itg->second.m_numEntered  += tc.m_numEntered;
    itg->second.m_numExecuted += tc.m_numExecuted;
    tc.m_numEntered = tc.m_numExecuted = 0;

    TRACE_MOD(Trace::counters, 1,
              "tracelet@%p executed/total %lu/%lu times\n",
              it->first, itg->second.m_numExecuted, itg->second.m_numEntered);
  }
  TRACE(1, "done requestExit(%ld)\n", g_context->m_currentThreadIdx);
}

void raiseUndefCns(const StringData* nm) {
  raise_notice("Undefined constant: %s", nm->data());
}

void TranslatorX64::processInit() {
  static const int aSize = 64 << 20;
  static const int astubsSize = 64 << 20;
  // We want to ensure that the block for "a" and the block for "astubs" are
  // nearby so that we can short jump between them. Thus we allocate one slab
  // and divide it between "a" and "astubs."
  uint8_t *base = allocSlab(aSize + astubsSize);
  a.init(base, aSize);
  astubs.init(base + aSize, astubsSize);

  m_globalData.size = aSize; // SWAG at data size
  m_globalData.init();

  m_counterThreadIdx = -1;

  // Emit a call to exit with whatever value the program leaves on
  // the return stack.
  m_callToExit = a.code.frontier;
  a.jmp(emitServiceReq(REQ_EXIT, 0));

  // On a backtrace, gdb tries to locate the calling frame at address
  // returnRIP-1. However, for the first VM frame, there is no code at
  // returnRIP-1, since the AR was set up manually. For this frame,
  // record the tracelet address as starting from callToExit-1, so gdb
  // does not barf
  if (!RuntimeOption::EvalJitNoGdb) {
    m_debugInfo.recordTracelet(m_callToExit-1, a.code.frontier,
      NULL, NULL, true, false);
  }

  // Emit some special helpers that are shared across translations.
  m_retHelper = a.code.frontier;
  a.jmp(emitRetFromInterpretedFrame());
  if (!RuntimeOption::EvalJitNoGdb) {
    m_debugInfo.recordTracelet(m_retHelper, a.code.frontier,
      NULL, NULL, true, false);
  }

  m_stackOverflowHelper = a.code.frontier;
  WithVMRegsSaved wsv(a, true, true);
  a.jmp(emitServiceReq(REQ_STACK_OVERFLOW, 0));

  // The decRef helper for when we bring the count down to zero. Callee needs to
  // bring the value into rdi. These can be burned in for all time, and for all
  // translations.
  if (false) { // type-check
    StringData* str = NULL;
    ArrayData* arr = NULL;
    ObjectData* obj = NULL;
    Variant* var = NULL;
    tv_release_str(str);
    tv_release_arr(arr);
    tv_release_obj(obj);
    tv_release_var(var);
  }
  typedef void* vp;
  m_dtorStubs[BitwiseKindOfString]  = emitUnaryStub(vp(tv_release_str));
  m_dtorStubs[KindOfStaticString]  = emitUnaryStub(vp(tv_release_str));
  // Aggregates: all need to sync FP.
  m_dtorStubs[KindOfArray]   = emitUnaryStub(vp(tv_release_arr), true);
  m_dtorStubs[KindOfObject]  = emitUnaryStub(vp(tv_release_obj), true);
  m_dtorStubs[KindOfVariant] = emitUnaryStub(vp(tv_release_var), true);
  m_dtorGenericStub = emitUnaryStub(vp(tv_release_generic), true);
  m_raiseUndefCnsStub = emitUnaryStub(vp(raiseUndefCns));

  {
    m_saveVMRegsStub = astubs.code.frontier;
    WithVMRegsSaved::emitSaveVMRegisters(astubs, false, true);
    astubs.ret();
  }
}

void TranslatorX64::recordGdbTranslation(const SrcKey& sk,
                                         const Unit* srcUnit,
                                         TCA start,
                                         int numTCBytes, bool exit,
                                         bool inPrologue) {
  if (!RuntimeOption::EvalJitNoGdb) {
    ASSERT(m_writeLease.amOwner());
    m_debugInfo.recordTracelet(start, start + numTCBytes, srcUnit,
                               srcUnit->at(sk.offset()),
                               exit, inPrologue);
  }
}

void TranslatorX64::defineCns(StringData* name) {
  TargetCache::fillConstant(name);
}

SrcRec* TranslatorX64::TCAToSrcRec(TCA tca) {
  TranslatorX64* that = static_cast<TranslatorX64*>(g_context->__getTransl());
  const TransRec &tr = that->getTxRec(tca);
  SrcRec& out = that->getSrcRec(tr.src);
  return &out;
}

} } }
