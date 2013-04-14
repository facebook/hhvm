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
#define __STDC_FORMAT_MACROS
#include <cinttypes>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <strstream>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <string>
#include <queue>
#include <unwind.h>

#ifdef __FreeBSD__
# include <ucontext.h>
typedef __sighandler_t *sighandler_t;
# define RIP_REGISTER(v) (v).mc_rip
#else
#  if defined(__x86_64__)
#    define RIP_REGISTER(v) (v).gregs[REG_RIP]
#  elif defined(__AARCH64EL__)
#    define RIP_REGISTER(v) (v).pc
#  endif
#endif

#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <boost/utility/typed_in_place_factory.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/scoped_ptr.hpp>

#include "folly/Format.h"

#include "util/asm-x64.h"
#include "util/bitops.h"
#include "util/debug.h"
#include "util/disasm.h"
#include "util/maphuge.h"
#include "util/rank.h"
#include "util/ringbuffer.h"
#include "util/timer.h"
#include "util/trace.h"
#include "util/meta.h"
#include "util/util.h"
#include "util/repo_schema.h"

#include "runtime/vm/bytecode.h"
#include "runtime/vm/php_debug.h"
#include "runtime/vm/runtime.h"
#include "runtime/base/complex_types.h"
#include "runtime/base/execution_context.h"
#include "runtime/base/strings.h"
#include "runtime/base/zend/zend_string.h"
#include "runtime/base/runtime_option.h"
#include "runtime/base/server/source_root_info.h"
#include "runtime/ext/ext_closure.h"
#include "runtime/ext/ext_continuation.h"
#include "runtime/ext/ext_function.h"
#include "runtime/vm/debug/debug.h"
#include "runtime/vm/translator/targetcache.h"
#include "runtime/vm/translator/translator-deps.h"
#include "runtime/vm/translator/translator-inline.h"
#include "runtime/vm/translator/translator-x64.h"
#include "runtime/vm/translator/srcdb.h"
#include "runtime/vm/translator/x64-util.h"
#include "runtime/vm/translator/unwind-x64.h"
#include "runtime/vm/stats.h"
#include "runtime/vm/pendq.h"
#include "runtime/vm/treadmill.h"
#include "runtime/vm/repo.h"
#include "runtime/vm/type_profile.h"
#include "runtime/vm/member_operations.h"
#include "runtime/vm/translator/abi-x64.h"
#include "runtime/eval/runtime/file_repository.h"
#include "runtime/vm/translator/hopt/hhbctranslator.h"

#include "runtime/vm/translator/translator-x64-internal.h"

namespace HPHP {
namespace VM {
namespace Transl {

using namespace reg;
using namespace Util;
using namespace Trace;
using std::max;

#define TRANS_PERF_COUNTERS \
  TPC(translate) \
  TPC(retranslate) \
  TPC(interp_bb) \
  TPC(interp_instr) \
  TPC(interp_one) \
  TPC(max_trans) \
  TPC(enter_tc) \
  TPC(service_req)

static const char* const kInstrCountTx64Name = "instr_tx64";
static const char* const kInstrCountIRName = "instr_hhir";

#define TPC(n) "trans_" #n,
static const char* const kPerfCounterNames[] = {
  TRANS_PERF_COUNTERS
  kInstrCountTx64Name,
  kInstrCountIRName,
};
#undef TPC

#define TPC(n) tpc_ ## n,
enum TransPerfCounter {
  TRANS_PERF_COUNTERS
  tpc_num_counters
};
#undef TPC
static __thread int64_t s_perfCounters[tpc_num_counters];
#define INC_TPC(n) ++s_perfCounters[tpc_ ## n];

#define NULLCASE() \
  case KindOfUninit: case KindOfNull

#define STRINGCASE() \
  case BitwiseKindOfString: case KindOfStaticString

// nextTx64: Global shared state. The tx64 that should be used for
// new requests going forward.
TranslatorX64* volatile nextTx64;
// tx64: Thread-local state. The tx64 we're using for the current request.
__thread TranslatorX64* tx64;

// Register dirtiness: thread-private.
__thread VMRegState tl_regState = REGSTATE_CLEAN;

__thread JmpHitMap* tl_unlikelyHits = nullptr;
__thread JmpHitMap* tl_jccHits = nullptr;

static StaticString s___call(LITSTR_INIT("__call"));
static StaticString s___callStatic(LITSTR_INIT("__callStatic"));

// Initialize at most this many locals inline in function body prologue; more
// than this, and emitting a loop is more compact. To be precise, the actual
// crossover point in terms of code size is 6; 9 was determined by experiment to
// be the optimal point in certain benchmarks. #microoptimization
static const int kLocalsToInitializeInline = 9;

// An intentionally funny-looking-in-core-dumps constant for uninitialized
// instruction pointers.
static const uint64_t kUninitializedRIP = 0xba5eba11acc01ade;

static int
localOffset(int loc) {
  PhysReg base;
  int offset;
  locToRegDisp(Location(Location::Local, loc), &base, &offset);
  assert(base == rVmFp);
  return offset;
}

// Return the SrcKey for the operation that should follow the supplied
// NormalizedInstruction.  (This might not be the next SrcKey in the
// unit if we merged some instructions or otherwise modified them
// during analysis.)
SrcKey nextSrcKey(const Tracelet& t, const NormalizedInstruction& i) {
  return i.next ? i.next->source : t.m_nextSk;
}

// stubBlock --
//   Used to emit a bunch of outlined code that is unconditionally jumped to.
template <typename L>
void stubBlock(X64Assembler& hot, X64Assembler& cold, const L& body) {
  hot.  jmp(cold.code.frontier);
  guardDiamond(cold, body);
  cold. jmp(hot.code.frontier);
}

// IfElseBlock: like CondBlock, but with an else clause.
//    a.   test_reg_reg(rax, rax);
//    {  IfElseBlock<CC_Z> ifRax(a);
//       // Code executed for rax != 0
//       ifRax.Else();
//       // Code executed for rax == 0
//    }
//
template <ConditionCode Jcc>
class IfElseBlock : boost::noncopyable {
  X64Assembler& m_a;
  TCA m_jcc8;
  TCA m_jmp8;
  bool useElseJmp;
 public:
  explicit IfElseBlock(X64Assembler& a, bool elseJmp = true) :
    m_a(a), m_jcc8(a.code.frontier), m_jmp8(nullptr), useElseJmp(elseJmp) {
    tx64->m_regMap.freeze();
    m_a.jcc8(Jcc, m_a.code.frontier);  // 1f
  }
  void Else() {
    if (useElseJmp) {
      assert(m_jmp8 == nullptr);
      m_jmp8 = m_a.code.frontier;
      m_a.jmp8(m_jmp8); // 2f
    }
    // 1:
    m_a.patchJcc8(m_jcc8, m_a.code.frontier);
  }
  ~IfElseBlock() {
    if (useElseJmp) {
      assert(m_jmp8 != nullptr);
      // 2:
      m_a.patchJmp8(m_jmp8, m_a.code.frontier);
    }
    tx64->m_regMap.defrost();
  }
};

static bool
typeCanBeStatic(DataType t) {
  return t != KindOfObject && t != KindOfRef;
}

// IfCountNotStatic --
//   Emits if (%reg->_count != RefCountStaticValue) { ... }.
//   May short-circuit this check if the type is known to be
//   static already.
struct IfCountNotStatic {
  typedef CondBlock<FAST_REFCOUNT_OFFSET,
                    RefCountStaticValue,
                    CC_Z,
                    field_type(RefData, _count)> NonStaticCondBlock;
  NonStaticCondBlock *m_cb; // might be null
  IfCountNotStatic(X64Assembler& a,
                   PhysReg reg,
                   DataType t = KindOfInvalid) {
    // Objects and variants cannot be static
    if (typeCanBeStatic(t)) {
      m_cb = new NonStaticCondBlock(a, reg);
    } else {
      m_cb = nullptr;
    }
  }

  ~IfCountNotStatic() {
    delete m_cb;
  }
};

inline static bool
classIsPersistent(const Class* cls) {
  return RuntimeOption::RepoAuthoritative &&
    cls &&
    TargetCache::isPersistentHandle(cls->m_cachedOffset);
}

bool
classIsUnique(const Class* cls) {
  return RuntimeOption::RepoAuthoritative &&
    cls &&
    (cls->attrs() & AttrUnique);
}

bool
classIsUniqueOrCtxParent(const Class* cls) {
  if (!cls) return false;
  if (classIsUnique(cls)) return true;
  Class* ctx = arGetContextClass(curFrame());
  if (!ctx) return false;
  return ctx->classof(cls);
}

bool
classIsUniqueNormalClass(const Class* cls) {
  return classIsUnique(cls) &&
    !(cls->attrs() & (AttrInterface | AttrTrait));
}

// Segfault handler: figure out if it's an intentional segfault
// (timeout exception) and if so, act appropriately. Otherwise, pass
// the signal on.
void TranslatorX64::SEGVHandler(int signum, siginfo_t *info, void *ctx) {
  TranslatorX64 *self = Get();
  void *surprisePage =
    ThreadInfo::s_threadInfo->m_reqInjectionData.surprisePage;
  if (info->si_addr == surprisePage) {
    ucontext_t *ucontext = (ucontext_t*)ctx;
    TCA rip = (TCA)RIP_REGISTER(ucontext->uc_mcontext);
    SignalStubMap::const_accessor a;
    if (!self->m_segvStubs.find(a, rip)) {
      NOT_REACHED();
    }
    TCA astubsCall = a->second;

    // When this handler returns, "call" the astubs code for this
    // surprise check.
    RIP_REGISTER(ucontext->uc_mcontext) = (uintptr_t)astubsCall;

    // We've processed this event; reset the page in case execution
    // continues normally.
    g_vmContext->m_stack.unprotect();
  } else {
    sighandler_t handler = (sighandler_t)self->m_segvChain;
    if (handler == SIG_DFL || handler == SIG_IGN) {
      signal(signum, handler);
      raise(signum);
    } else {
      self->m_segvChain(signum, info, ctx);
    }
  }
}

/*
 * Copy a heap cell from memory to the stack.
 *
 * Use emitCopyToStack when you can safely change the state of the
 * register map.  When using emitCopyToStackRegSafe, you'll need to
 * invalidate the stack location manually at an appropriate time.
 */

void
TranslatorX64::emitCopyToStackRegSafe(X64Assembler& a,
                                      const NormalizedInstruction& ni,
                                      PhysReg src,
                                      int off,
                                      PhysReg tmpReg) {
  assert(off % sizeof(Cell) == 0);
  emitCopyTo(a, src, 0, rVmSp, vstackOffset(ni, off), tmpReg);
}

void
TranslatorX64::emitCopyToStack(X64Assembler& a,
                               const NormalizedInstruction& ni,
                               PhysReg src,
                               int off) {
  ScratchReg scratch(m_regMap);
  {
    FreezeRegs freeze(m_regMap);
    emitCopyToStackRegSafe(a, ni, src, off, r(scratch));
  }
  // Forget whatever we thought we knew about the stack.
  m_regMap.invalidate(ni.outStack->location);
}

/*
 * Emit code that does the same thing as tvSet().
 *
 * The `oldType' and `oldData' registers are used for temporary
 * storage and unconditionally destroyed.
 * `toPtr' will be destroyed iff the cell we're storing to is
 * KindOfRef.
 * The variant check will not be performed if toOffset is nonzero, so
 * only pass a nonzero offset if you know the destination is not
 * KindOfRef.
 * `from' will not be modified.
 */
void TranslatorX64::emitTvSetRegSafe(const NormalizedInstruction& i,
                                     PhysReg from,
                                     DataType fromType,
                                     PhysReg toPtr,
                                     int toOffset,
                                     PhysReg oldType,
                                     PhysReg oldData,
                                     bool incRefFrom) {
  assert(!i.isNative());
  assert(!i.isSimple());
  assert(fromType != KindOfRef);

  if (toOffset == 0) {
    emitDerefIfVariant(a, toPtr);
  }

  emitLoadTVType(a, toPtr[toOffset + TVOFF(m_type)], r32(oldType));
  a.  loadq  (toPtr[toOffset + TVOFF(m_data)], oldData);
  emitStoreTypedValue(a, fromType, from, toOffset, toPtr);
  if (incRefFrom) {
    emitIncRef(from, fromType);
  }
  emitDecRefGenericReg(oldData, oldType);
}

void TranslatorX64::emitTvSet(const NormalizedInstruction& i,
                              PhysReg from,
                              DataType fromType,
                              PhysReg toPtr,
                              int toOffset,
                              bool incRefFrom) {
  ScratchReg oldType(m_regMap);
  ScratchReg oldData(m_regMap);
  emitTvSetRegSafe(i, from, fromType, toPtr, toOffset,
                   r(oldType), r(oldData), incRefFrom);
}

// Logical register move: ensures the value in src will be in dest
// after execution, but might do so in strange ways. Do not count on
// being able to smash dest to a different register in the future, e.g.
void
emitMovRegReg(X64Assembler& a, PhysReg src, PhysReg dest) {
  SpaceRecorder("_RegMove", a);
  if (src != dest) {
    a.  movq (src, dest);
  }
}

void
emitLea(X64Assembler& a, PhysReg base, int disp, PhysReg dest) {
  if (!disp) {
    emitMovRegReg(a, base, dest);
    return;
  }
  a.   lea  (base[disp], dest);
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
                          const int bytesPopped /* = 0 */,
                          bool isCtor /* = false */,
                          bool clearThis /* = true */,
                          uintptr_t varEnvInvName /* = 0 */) {
  if (func && phpBreakpointEnabled(func->name()->data())) {
    translator_debug_break(a);
  }
  assert(sizeof(Cell) < sizeof(ActRec));
  // We are about to push an ActRec onto the stack. The stack grows down,
  // so the offset of the beginning of the ActRec from the top of stack
  // is -sizeof(ActRec).
  int numArgs = i.imm[0].u_IVA;
  int startOfActRec = bytesPopped - sizeof(ActRec);
  size_t funcOff     = startOfActRec + AROFF(m_func);
  size_t thisOff     = startOfActRec + AROFF(m_this);
  size_t nargsOff    = startOfActRec + AROFF(m_numArgsAndCtorFlag);
  size_t varenvOff   = startOfActRec + AROFF(m_varEnv);
  size_t savedRbpOff = startOfActRec + AROFF(m_savedRbp);

  BOOST_STATIC_ASSERT((
    sizeof(((ActRec*)nullptr)->m_numArgsAndCtorFlag) == sizeof(int32_t)
  ));
  /*
   * rVmSp might not be up-to-date here, so we use emitVStackStore and
   * emitVStackStoreImm which know how to compute the where the top of
   * stack currently is.
   */
  if (func) {
    emitVStackStoreImm(a, i, (uintptr_t)func, funcOff);
    if (clearThis) {
      emitVStackStoreImm(a, i, 0,               thisOff, sz::qword, &m_regMap);
    }
  }
  emitVStackStoreImm(a, i,   ActRec::encodeNumArgs(numArgs, isCtor),
                                              nargsOff, sz::dword);
  emitVStackStoreImm(a, i,   varEnvInvName,   varenvOff, sz::qword, &m_regMap);
  emitVStackStore(a, i,      rVmFp,           savedRbpOff, sz::qword);
}

void
TranslatorX64::emitCallSaveRegs() {
  assert(!m_regMap.frozen());
  m_regMap.cleanRegs(kCallerSaved);
}

static void UNUSED tc_debug_print(const char* message,
                           uintptr_t r1,
                           uintptr_t r2,
                           uintptr_t r3,
                           ActRec* fp) {
  TRACE(1, "*********************** %s: %p %p %p  (for : %s)\n",
           message, (void*)r1, (void*)r2, (void*)r3,
           fp->m_func ? fp->m_func->fullName()->data() : "[?]");
}

// Utility for debugging translations that will print a message,
// followed by the value of up to three registers.
void TranslatorX64::emitDebugPrint(Asm& a,
                                   const char* message,
                                   PhysReg r1,
                                   PhysReg r2,
                                   PhysReg r3) {
  boost::optional<PhysRegSaver> aSaver;
  boost::optional<PhysRegSaverStub> astubsSaver;

  if (&a == &this->a) {
    aSaver = boost::in_place<PhysRegSaver>(boost::ref(a), kAllX64Regs);
  } else {
    astubsSaver = boost::in_place<PhysRegSaverStub>(boost::ref(a),
      kAllX64Regs);
  }

  a.  mov_imm64_reg  (uintptr_t(message), argNumToRegName[0]);
  a.  mov_reg64_reg64(r1, argNumToRegName[1]);
  a.  mov_reg64_reg64(r2, argNumToRegName[2]);
  a.  mov_reg64_reg64(r3, argNumToRegName[3]);
  a.  mov_reg64_reg64(rVmFp, argNumToRegName[4]);
  a.  call((TCA)tc_debug_print);
}

void ArgManager::cleanLocs() {
  for (size_t i = 0; i < m_args.size(); ++i) {
    // We only need to clean locations we are passing the address of.
    // (ArgLoc passes the value in the register mapped for a given
    // location, not the address of the location itself, so it doesn't
    // need cleaning here.)
    if (m_args[i].m_kind != ArgContent::ArgLocAddr) continue;
    m_tx64.m_regMap.cleanLoc(*m_args[i].m_loc);
  }
}

void ArgManager::computeUsed(std::map<PhysReg, size_t> &used,
                             std::vector<PhysReg> &actual) {
  size_t n = m_args.size();
  for (size_t i = 0; i < n; i++) {
    PhysReg reg = InvalidReg;
    if (m_args[i].m_kind == ArgContent::ArgReg ||
        m_args[i].m_kind == ArgContent::ArgRegPlus) {
      reg = m_args[i].m_reg;
    } else if (m_args[i].m_kind == ArgContent::ArgLoc ||
               m_args[i].m_kind == ArgContent::ArgLocRef ||
               m_args[i].m_kind == ArgContent::ArgDeref) {
      reg = m_tx64.getReg(*m_args[i].m_loc);
    } else {
      continue;
    }
    TRACE(6, "ArgManager: arg %zd incoming reg r%d\n", i, int(reg));
    used[reg] = i;
    actual[i] = reg;
  }
}

void
TranslatorX64::emitRB(X64Assembler& a,
                      RingBufferType t,
                      SrcKey sk, RegSet toSave) {
  if (!Trace::moduleEnabledRelease(Trace::tx64, 3)) {
    return;
  }
  PhysRegSaver rs(a, toSave | kSpecialCrossTraceRegs);
  int arg = 0;
  emitImmReg(a, t, argNumToRegName[arg++]);
  emitImmReg(a, sk.getFuncId(), argNumToRegName[arg++]);
  emitImmReg(a, sk.m_offset, argNumToRegName[arg++]);
  a.    call((TCA)ringbufferEntry);
}

void
TranslatorX64::emitRB(X64Assembler& a,
                      RingBufferType t,
                      const char* msg,
                      RegSet toSave) {
  if (!Trace::moduleEnabledRelease(Trace::tx64, 3)) {
    return;
  }
  PhysRegSaver save(a, toSave | kSpecialCrossTraceRegs);
  int arg = 0;
  emitImmReg(a, (uintptr_t)msg, argNumToRegName[arg++]);
  emitImmReg(a, strlen(msg), argNumToRegName[arg++]);
  emitImmReg(a, t, argNumToRegName[arg++]);
  a.    call((TCA)ringbufferMsg);
}

/*
 * allocate the input registers for i, trying to
 * match inputs to call arguments.
 * if args[j] == ArgDontAllocate, the input i.inputs[j] is skipped
 * if args[j] == ArgAnyReg, it will be allocated as normal
 * otherwise, args[j] should be a positional call argument,
 * and allocInputsForCall will attempt to allocate it to
 * argNumToRegName[args[j]].
 */
void
TranslatorX64::allocInputsForCall(const NormalizedInstruction& i,
                                  const int* args) {
  RegSet blackList;
  int arg;
  /*
   * If any of the inputs is already in an argument
   * register, blacklist it. ArgManager already takes
   * care of shuffling registers efficiently
   */
  for (arg = i.inputs.size(); arg--; ) {
    if (args[arg] != ArgDontAllocate &&
        m_regMap.hasReg(i.inputs[arg]->location)) {
      blackList |= RegSet(getReg(i.inputs[arg]->location));
    }
  }
  bool hasAnyReg = false;
  for (arg = i.inputs.size(); arg--; ) {
    if (args[arg] != ArgAnyReg) {
      if (args[arg] != ArgDontAllocate &&
        !m_regMap.hasReg(i.inputs[arg]->location)) {
        PhysReg target = argNumToRegName[args[arg]];
        if (!blackList.contains(target)) {
          m_regMap.cleanRegs(RegSet(target));
          m_regMap.smashRegs(RegSet(target));
        } else {
          target = InvalidReg;
        }
        m_regMap.allocInputReg(i, arg, target);
      }
    } else {
      hasAnyReg = true;
    }
  }
  if (hasAnyReg) {
    for (arg = i.inputs.size(); arg--; ) {
      if (args[arg] == ArgAnyReg) {
        m_regMap.allocInputReg(i, arg);
      }
    }
  }
}

void ArgManager::shuffleRegisters(std::map<PhysReg, size_t> &used,
                                  std::vector<PhysReg> &actual) {
  size_t n = m_args.size();
  for (size_t i = 0; i < n; i++) {
    if (actual[i] == InvalidReg)
      continue;

    if (!mapContains(used, argNumToRegName[i])) {
      // There's no conflict, so just copy
      TRACE(6, "ArgManager: arg %zd reg available, copying from r%d to r%d\n",
            i, int(actual[i]), int(argNumToRegName[i]));
      // Do copy and data structure update here, because this way
      // we can reuse the register in actual[i] later without problems.
      emitMovRegReg(m_a, actual[i], argNumToRegName[i]);
      used.erase(actual[i]);
      actual[i] = argNumToRegName[i];
    } else {
      size_t j = used[argNumToRegName[i]];
      if (actual[j] != actual[i]) {
        // The register is used by some other value, so we must swap the two
        // registers.
        assert(j > i);
        assert(actual[j] != InvalidReg);
        PhysReg ri = actual[i],
                rj = actual[j];
        TRACE(6, "ArgManager: arg %zd register used by arg %zd, "
                 "swapping r%d with r%d\n", i, j, int(ri), int(rj));

        // Clean the registers first
        RegSet regs = RegSet(ri) | RegSet(rj);
        m_tx64.m_regMap.cleanRegs(regs);

        // Emit the actual swap
        m_tx64.m_regMap.swapRegisters(ri, rj);
        m_a.  xchgq(ri, rj);

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
    auto kind = m_args[i].m_kind;
    auto argReg = argNumToRegName[i];
    switch (kind) {
    case ArgContent::ArgLoc:
    case ArgContent::ArgLocRef:
    case ArgContent::ArgDeref:
    case ArgContent::ArgReg:
      TRACE(6, "ArgManager: copying arg %zd from r%d to r%d\n",
            i, int(actual[i]), int(argReg));
      emitMovRegReg(m_a, actual[i], argReg);
      // Emit dereference if needed
      if (kind == ArgContent::ArgDeref) {
        emitDerefRef(m_a, argReg, argReg);
      } else if (kind == ArgContent::ArgLocRef && RefData::tvOffset()) {
        // argReg holds a RefData*; adjust it to be TypedValue* to the value.
        m_a.addq(RefData::tvOffset(), argReg);
      }
      break;

    // For any of these cases, the register should already be available.
    // If it was used previously by an input value, shuffleRegisters
    // should have moved it to the proper register from argNumToRegName.
    case ArgContent::ArgImm:
      emitImmReg(m_a, m_args[i].m_imm, argReg);
      break;

    case ArgContent::ArgRegPlus:
      if (m_args[i].m_imm) {
        m_a.  add_imm32_reg64(m_args[i].m_imm, argReg);
      }
      break;

    case ArgContent::ArgLocAddr:
      {
        PhysReg base;
        int disp;
        locToRegDisp(*m_args[i].m_loc, &base, &disp);
        emitLea(m_a, base, disp, argReg);
      }
      break;

    default:
      // Should never happen
      assert(false);
    }
  }
}

void
TranslatorX64::emitCall(X64Assembler& a, TCA dest, bool killRegs) {
  if (a.jmpDeltaFits(dest) && !Stats::enabled()) {
    a.    call(dest);
  } else {
    a.    call(getNativeTrampoline(dest));
  }
  if (killRegs) {
    // All caller-saved regs are now suspect.
    m_regMap.smashRegs(kCallerSaved);
  }
}

void
TranslatorX64::emitCall(X64Assembler& a, Call call, bool killRegs) {
  if (call.isDirect()) {
    return emitCall(a, (TCA)call.getAddress(), killRegs);
  }
  // Virtual call.
  // Load method's address from proper offset off of object in rdi,
  // using rax as scratch.
  a.loadq(*rdi, rax);
  a.call(rax[call.getOffset()]);
  if (killRegs) {
    m_regMap.smashRegs(kCallerSaved);
  }
}

void
TranslatorX64::recordSyncPoint(X64Assembler& a, Offset pcOff, Offset spOff) {
  m_pendingFixups.push_back(PendingFixup(a.code.frontier,
                                         Fixup(pcOff, spOff)));
}

void
TranslatorX64::recordIndirectFixup(CTCA addr, int dwordsPushed) {
  m_fixupMap.recordIndirectFixup(
    a.code.frontier, IndirectFixup((2 + dwordsPushed) * 8));
}

void
TranslatorX64::recordCall(Asm& a, const NormalizedInstruction& i) {
  recordCallImpl<false>(a, i);
}

void
TranslatorX64::recordCall(const NormalizedInstruction& i) {
  recordCall(a, i);
}

template <bool reentrant>
void
TranslatorX64::recordCallImpl(X64Assembler& a,
                              const NormalizedInstruction& i,
                              bool advance /* = false */,
                              int adjust /* = 0 */) {
  SrcKey sk = i.source;
  Offset stackOff = i.stackOff + (vmfp() - vmsp());
  if (UNLIKELY(curFunc()->isGenerator())) {
    // FP is pointing somewhere outside the main stack. SP and FP have no
    // meaningful relationship to each other at this point, and the offset
    // between them can be different on different executions of this piece of
    // code. We reconstruct where FP *would* be, relative to SP, if it were on
    // the main stack, by using what we know about the generator's caller (which
    // is always on the main stack, and always calls the generator with an empty
    // eval stack). We reverse this calculation in FixupMap::regsFromActRec.
    Cell* genStackBase = Stack::generatorStackBase(curFrame());
    stackOff = i.stackOff + (genStackBase - vmsp());
  }
  if (advance) {
    sk.advance(curUnit());
    stackOff += getStackDelta(i);
  }
  stackOff += adjust;
  assert(i.checkedInputs ||
         (reentrant && !i.isSimple()) ||
         (!reentrant && !i.isNative()));
  Offset pcOff = sk.offset() - curFunc()->base();
  SKTRACE(2, sk, "record%sCall pcOff %d\n",
             reentrant ? "Reentrant" : "", int(pcOff));
  recordSyncPoint(a, pcOff, stackOff);
  SKTRACE(2, sk, "record%sCall stackOff %d\n",
             reentrant ? "Reentrant" : "", int(stackOff));

  /*
   * Right now we assume call sites that need to record sync points
   * may also throw exceptions.  We record information about dirty
   * callee-saved registers so we can spill their contents during
   * unwinding.  See unwind-x64.cpp.
   */
  if (!m_pendingUnwindRegInfo.empty()) {
    if (Trace::moduleLevel(Trace::tunwind) >= 2) {
      sk.trace("recordCallImpl has dirty callee-saved regs\n");
      TRACE_MOD(Trace::tunwind, 2,
                   "CTCA: %p saving dirty callee regs:\n",
                   a.code.frontier);
      for (int i = 0; i < UnwindRegInfo::kMaxCalleeSaved; ++i) {
        if (m_pendingUnwindRegInfo.m_regs[i].dirty) {
          TRACE_MOD(Trace::tunwind, 2, "  %s\n",
                    m_pendingUnwindRegInfo.m_regs[i].pretty().c_str());
        }
      }
    }
    m_unwindRegMap.insert(a.code.frontier, m_pendingUnwindRegInfo);
    m_pendingUnwindRegInfo.clear();
  }
}

void TranslatorX64::prepareCallSaveRegs() {
  emitCallSaveRegs(); // Clean caller-saved regs.
  m_pendingUnwindRegInfo.clear();

  RegSet rset = kCalleeSaved;
  PhysReg reg;
  while (rset.findFirst(reg)) {
    rset.remove(reg);
    if (!m_regMap.regIsDirty(reg)) continue;
    const RegInfo* ri = m_regMap.getInfo(reg);
    assert(ri->m_cont.m_kind == RegContent::Loc);

    // If the register is dirty, we'll record this so that we can
    // restore it during stack unwinding if an exception is thrown.
    m_pendingUnwindRegInfo.add(reg, ri->m_type, ri->m_cont.m_loc);
  }
}

void
TranslatorX64::emitIncRef(PhysReg base, DataType dtype) {
  emitIncRef(a, base, dtype);
}

void
TranslatorX64::emitIncRef(X64Assembler &a, PhysReg base, DataType dtype) {
  if (!IS_REFCOUNTED_TYPE(dtype) && dtype != KindOfInvalid) {
    return;
  }
  assert(m_regMap.getInfo(base));
  SpaceRecorder sr("_IncRef", a);
  assert(sizeof(Countable) == sizeof(int32_t));
  { // if !static then
    IfCountNotStatic ins(a, base, dtype);
    /*
     * The optimization guide cautions against using inc; while it is
     * compact, it only writes the low-order 8 bits of eflags, causing a
     * partial dependency for any downstream flags-dependent code.
     */
    a.    incl(base[FAST_REFCOUNT_OFFSET]);
  } // endif
}

void
TranslatorX64::emitIncRefGenericRegSafe(PhysReg base,
                                        int disp,
                                        PhysReg tmpReg) {
  assert(m_regMap.getInfo(base));
  { // if RC
    IfRefCounted irc(a, base, disp);
    a.    load_reg64_disp_reg64(base, disp + TVOFF(m_data),
                                tmpReg);
    { // if !static
      IfCountNotStatic ins(a, tmpReg);
      a.  incl(tmpReg[FAST_REFCOUNT_OFFSET]);
    } // endif
  } // endif
}

void TranslatorX64::emitIncRefGeneric(PhysReg base, int disp) {
  ScratchReg tmpReg(m_regMap);
  emitIncRefGenericRegSafe(base, disp, r(tmpReg));
}

static void emitGetGContext(X64Assembler& a, PhysReg dest) {
  emitTLSLoad<ExecutionContext>(a, g_context, dest);
}

// emitEagerVMRegSave --
//   Inline. Saves regs in-place in the TC. This is an unusual need;
//   you probably want to lazily save these regs via recordCall and
//   its ilk.
//
//   SaveFP uses rVmFp, as usual. SavePC requires the caller to have
//   placed the PC offset of the instruction about to be executed in
//   rdi.
enum RegSaveFlags {
  SaveFP = 1,
  SavePC = 2
};

static TCA
emitEagerVMRegSave(X64Assembler& a,
                   int flags /* :: RegSaveFlags */) {
  TCA start = a.code.frontier;
  bool saveFP = bool(flags & SaveFP);
  bool savePC = bool(flags & SavePC);
  assert((flags & ~(SavePC | SaveFP)) == 0);

  Reg64 pcReg = rdi;
  PhysReg rEC = rScratch;
  assert(!kSpecialCrossTraceRegs.contains(rdi));

  emitGetGContext(a, rEC);

  static COff spOff = offsetof(VMExecutionContext, m_stack) +
    Stack::topOfStackOffset();
  static COff fpOff = offsetof(VMExecutionContext, m_fp) - spOff;
  static COff pcOff = offsetof(VMExecutionContext, m_pc) - spOff;

  assert(spOff != 0);
  // Instruction selection note: this is an lea, but add is more
  // compact and we can afford the flags bash.
  a.    addq   (spOff, r64(rEC));
  a.    storeq (rVmSp, *rEC);
  if (savePC) {
    // We're going to temporarily abuse rVmSp to hold the current unit.
    Reg64 rBC = rVmSp;
    a.  push   (rBC);
    // m_fp -> m_func -> m_unit -> m_bc + pcReg
    a.  loadq  (rVmFp[AROFF(m_func)], rBC);
    a.  loadq  (rBC[Func::unitOff()], rBC);
    a.  loadq  (rBC[Unit::bcOff()], rBC);
    a.  addq   (rBC, pcReg);
    a.  storeq (pcReg, rEC[pcOff]);
    a.  pop    (rBC);
  }
  if (saveFP) {
    a.  storeq (rVmFp, rEC[fpOff]);
  }
  return start;
}

Call TranslatorX64::getDtorCall(DataType type) {
  switch (type) {
  case BitwiseKindOfString:
    return Call(getMethodPtr(&StringData::release));
  case KindOfArray:
    return Call(getMethodPtr(&ArrayData::release));
  case KindOfObject:
    return Call(getMethodPtr(&ObjectData::release));
  case KindOfRef:
    return Call(getMethodPtr(&RefData::release));
  default:
    assert(false);
    NOT_REACHED();
  }
}

/**
 * emitDecRef --
 *
 *   Decrement a value's refcount and call the release helper if
 *   appropriate. emitDecRef requires that the caller knows the
 *   type at translation time.
 */
void TranslatorX64::emitDecRef(Asm& a,
                               const NormalizedInstruction& i,
                               PhysReg rDatum,
                               DataType type) {
  assert(type != KindOfInvalid);
  if (!IS_REFCOUNTED_TYPE(type)) {
    return;
  }

  SpaceRecorder sr("_DecRef", a);
  { // if !static
    IfCountNotStatic ins(a, rDatum, type);
    a.    decl(rDatum[FAST_REFCOUNT_OFFSET]);

    assert(type >= 0 && type < MaxNumDataTypes);
    if (&a == &this->astubs) {
      JccBlock<CC_NZ> ifZero(a);
      callUnaryStub(a, i, m_dtorStubs[typeToDestrIndex(type)], rDatum);
      return;
    }

    UnlikelyIfBlock ifZero(CC_Z, this->a, astubs);

    auto getPushSet = [&] {
      RegSet ret;
      auto regs = kCallerSaved;
      PhysReg reg;
      while (regs.findFirst(reg)) {
        regs.remove(reg);
        auto* info = m_regMap.getInfo(reg);
        if (info->m_state != RegInfo::FREE) {
          if (info->m_cont.m_kind == RegContent::Int ||
              info->m_cont.m_loc.isLiteral()) {
            // RegAlloc::reconcile can rematerialize these, no need to
            // push.  But get it out of the reg map so reconcile
            // notices.
            m_regMap.smashReg(reg);
          } else {
            ret.add(reg);
          }
        }
      }
      return ret;
    };

    const RegSet savedSet = getPushSet();
    const RegAlloc saved = m_regMap;
    {
      PhysRegSaver saver(astubs, savedSet);

      // Try to make it more likely we'll get a scratch from the set
      // we just pushed by informing the register allocator about it.
      m_regMap.scrubRegs(savedSet -
        m_regMap.getRegsLike(RegInfo::SCRATCH).add(rDatum));
      m_regMap.smashRegs(savedSet - RegSet(rDatum));

      assert(rDatum != rsp && rDatum != rbx);
      emitMovRegReg(astubs, rDatum, argNumToRegName[0]);
      emitCall(astubs, getDtorCall(type));
      if (typeReentersOnRelease(type)) {
        recordReentrantStubCall(*m_curNI);
      } else {
        recordStubCall(*m_curNI);
      }
    }
    m_regMap = saved;
  } // endif
}

void TranslatorX64::emitDecRef(const NormalizedInstruction& i,
                               PhysReg rDatum,
                               DataType type) {
  emitDecRef(a, i, rDatum, type);
}

void TranslatorX64::emitDecRefInput(Asm& a,
                                    const NormalizedInstruction& i,
                                    int input) {
  DynLocation* value = i.inputs[input];
  if (IS_REFCOUNTED_TYPE(value->outerType())) {
    m_regMap.allocInputReg(i, input);
    PhysReg rValue = getReg(value->location);
    emitDecRef(a, i, rValue, value->outerType());
  }
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
void TranslatorX64::emitDecRefGeneric(const NormalizedInstruction& i,
                                      PhysReg srcReg, int disp /* = 0 */) {
  SpaceRecorder sr("_DecRefGeneric", a);
  /*
   * The full, inlined generic dec ref looks like:
   *
   *    TypedValue* d = srcReg + disp;
   *    if (IS_REFCOUNTED_TYPE(d->m_type) && // a)
   *        d->_count != kStaticCount     && // b)
   *        d->_count-- == 0)             && // c)
   *            GenericDestroy(d);           // d)
   *
   * We originally inlined *all* of a-d, and have experimented with sharing
   * them all, too. At this writing (05-12-2012), inlining a) and outlining
   * b-d seems to strike the right balance between compactness and not
   * doing too much work in the common case where it is not refcounted.
   */
  {
    IfRefCounted irc(a, srcReg, disp);
    callUnaryReentrantStub(a, i, m_dtorGenericStub, srcReg, disp);
  }
}

// Same as emitDecRefGeneric, except for when we have the type in a
// register as well.  Same inlining/outlining choices as
// emitDecRefGeneric above.
void TranslatorX64::emitDecRefGenericReg(PhysReg rData, PhysReg rType) {
  SpaceRecorder sr("_DecRefGeneric", a);

  assert(rData != rScratch && rType != rScratch);

  auto body = [&](X64Assembler& a){
    // Calling convention: m_data in rdi, m_type in r10 (rScratch).
    // (See emitGenericDecRefHelpers.)
    assert(!kAllRegs.contains(rScratch));
    a.    movl   (r32(rType), r32(rScratch));
    callUnaryReentrantStub(a, *m_curNI, m_dtorGenericStubRegs, rData);
  };

  Op op = m_curNI->op();
  emitCmpTVType(a, KindOfRefCountThreshold, rType);
  if (op == OpSetM || op == OpContSend || op == OpSetG) {
    // Semi-likely cases
    semiLikelyIfBlock(CC_A, a, std::bind(body, std::ref(a)));
  } else if (op == OpContNext) {
    // Unlikely cases
    UnlikelyIfBlock counted(CC_A, a, astubs);
    body(astubs);
  } else {
    JccBlock<CC_BE> ifRefCounted(a);
    body(a);
  }
}

/*
 * callDestructor/jumpDestructor --
 *
 * Emit a call or jump to the appropriate destructor for a dynamically
 * typed value.
 *
 * No registers are saved; most translated code should be using
 * emitDecRefGeneric{Reg,} instead of this.
 *
 *   Inputs:
 *
 *     - typeReg is destroyed and may not be argNumToRegName[0].
 *     - argNumToRegName[0] should contain the m_data for this value.
 *     - scratch is destoyed.
 */

static IndexedMemoryRef lookupDestructor(X64Assembler& a,
                                         PhysReg typeReg,
                                         PhysReg scratch) {
  assert(typeReg != r32(argNumToRegName[0]));
  assert(scratch != argNumToRegName[0]);

  static_assert((BitwiseKindOfString >> kShiftDataTypeToDestrIndex == 0) &&
                (KindOfArray         >> kShiftDataTypeToDestrIndex == 1) &&
                (KindOfObject        >> kShiftDataTypeToDestrIndex == 2) &&
                (KindOfRef           >> kShiftDataTypeToDestrIndex == 3),
                "lookup of destructors depends on KindOf* values");

  a.    shrl   (kShiftDataTypeToDestrIndex, r32(typeReg));
  a.    movq   (&g_destructors, scratch);
  return scratch[typeReg*8];
}

static void callDestructor(X64Assembler& a,
                           PhysReg typeReg,
                           PhysReg scratch) {
  a.    call   (lookupDestructor(a, typeReg, scratch));
}

static void jumpDestructor(X64Assembler& a,
                           PhysReg typeReg,
                           PhysReg scratch) {
  a.    jmp    (lookupDestructor(a, typeReg, scratch));
}

void TranslatorX64::emitGenericDecRefHelpers() {
  FreezeRegs brr(m_regMap);
  Label release;

  // m_dtorGenericStub just takes a pointer to the TypedValue in rdi.
  moveToAlign(a, kNonFallthroughAlign);
  m_irPopRHelper = a.code.frontier;
  // popR: Move top-of-stack pointer to rdi
  emitMovRegReg(a, rVmSp, rdi);
  // fall through
  m_dtorGenericStub = a.code.frontier;
  emitLoadTVType(a, rdi[TVOFF(m_type)], r32(rScratch));
  a.    loadq  (rdi[TVOFF(m_data)], rdi);
  // Fall through to the regs stub.

  /*
   * Custom calling convention: m_type goes in rScratch, m_data in
   * rdi.  We don't ever store program locations in rScratch, so the
   * caller didn't need to spill anything.  The assembler sometimes
   * uses rScratch, but we know the stub won't need to and it makes it
   * possible to share the code for both decref helpers.
   */
  m_dtorGenericStubRegs = a.code.frontier;
  a.    cmpl   (RefCountStaticValue, rdi[FAST_REFCOUNT_OFFSET]);
  jccBlock<CC_Z>(a, [&] {
    a.  decl   (rdi[FAST_REFCOUNT_OFFSET]);
    release.jcc8(a, CC_Z);
  });
  a.    ret    ();

asm_label(a, release);
  {
    PhysRegSaver prs(a, kCallerSaved - RegSet(rdi));
    callDestructor(a, rScratch, rax);
    recordIndirectFixup(a.code.frontier, prs.rspAdjustment());
  }
  a.    ret    ();

  TRACE(1, "HOTSTUB: generic dtor start: %lx\n",
        uintptr_t(m_irPopRHelper));
  TRACE(1, "HOTSTUB: genericDtorStub: %lx\n", uintptr_t(m_dtorGenericStub));
  TRACE(1, "HOTSTUB: genericDtorStubRegs: %lx\n",
        uintptr_t(m_dtorGenericStubRegs));
  TRACE(1, "HOTSTUB: total dtor generic stubs %zu bytes\n",
        size_t(a.code.frontier - m_dtorGenericStub));
}

/*
 * Translation call targets. It is a lot easier, and a bit more
 * portable, to use C linkage from assembly.
 */
TCA TranslatorX64::retranslate(SrcKey sk, bool align, bool allowIR) {
  if (isDebuggerAttachedProcess() && isSrcKeyInBL(curUnit(), sk)) {
    // We are about to translate something known to be blacklisted by
    // debugger, exit early
    SKTRACE(1, sk, "retranslate abort due to debugger\n");
    return nullptr;
  }
  LeaseHolder writer(s_writeLease);
  if (!writer) return nullptr;
  SKTRACE(1, sk, "retranslate\n");
  return translate(sk, align, allowIR);
}

// Only use comes from HHIR's cgExitTrace() case TraceExitType::SlowNoProgress
TCA TranslatorX64::retranslateAndPatchNoIR(SrcKey sk,
                                           bool   align,
                                           TCA    toSmash) {
  if (isDebuggerAttachedProcess() && isSrcKeyInBL(curUnit(), sk)) {
    // We are about to translate something known to be blacklisted by
    // debugger, exit early
    SKTRACE(1, sk, "retranslateAndPatchNoIR abort due to debugger\n");
    return nullptr;
  }
  LeaseHolder writer(s_writeLease);
  if (!writer) return nullptr;
  SKTRACE(1, sk, "retranslateAndPatchNoIR\n");
  SrcRec* srcRec = getSrcRec(sk);
  if (srcRec->translations().size() == SrcRec::kMaxTranslations + 1) {
    // we've gone over the translation limit and already have an anchor
    // translation that will interpret, so just return NULL and force
    // interpretation of this BB.
    return nullptr;
  }
  TCA start = translate(sk, align, false);
  if (start != nullptr) {
    smashJmp(getAsmFor(toSmash), toSmash, start);
  }
  return start;
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
  assert(isPowerOfTwo(align));
  size_t leftInBlock = align - ((align - 1) & uintptr_t(aa.code.frontier));
  if (leftInBlock == align) return;
  if (unreachable) {
    if (leftInBlock > 2) {
      aa.ud2();
      leftInBlock -= 2;
    }
    if (leftInBlock > 0) {
      aa.emitInt3s(leftInBlock);
    }
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
  SrcKey m_sourceInstr;
  TCA m_toSmash;
  int m_nArgs;
  bool m_isImmutable; // call was to known func.
} m_bindCall;

// ID to name mapping for tracing.
static inline const char*
reqName(int req) {
  static const char* reqNames[] = {
#define REQ(nm) #nm,
    SERVICE_REQUESTS
#undef REQ
  };
  return reqNames[req];
}

/*
 * Find or create a translation for sk. Returns TCA of "best" current
 * translation. May return NULL if it is currently impossible to create
 * a translation.
 */
TCA
TranslatorX64::getTranslation(SrcKey sk, bool align,
                              bool forceNoHHIR /* = false */) {
  curFunc()->validate();
  SKTRACE(2, sk, "getTranslation: curUnit %s funcId %" PRIx64 " offset %d\n",
          curUnit()->filepath()->data(),
          sk.getFuncId(),
          sk.offset());
  SKTRACE(2, sk, "   funcId: %" PRIx64 "\n",
          curFunc()->getFuncId());

  if (curFrame()->hasVarEnv() && curFrame()->getVarEnv()->isGlobalScope()) {
    SKTRACE(2, sk, "punting on pseudoMain\n");
    return nullptr;
  }
  if (const SrcRec* sr = m_srcDB.find(sk)) {
    TCA tca = sr->getTopTranslation();
    if (tca) {
      SKTRACE(2, sk, "getTranslation: found %p\n", tca);
      return tca;
    }
  }
  return createTranslation(sk, align, forceNoHHIR);
}

int
TranslatorX64::numTranslations(SrcKey sk) const {
  if (const SrcRec* sr = m_srcDB.find(sk)) {
    return sr->translations().size();
  }
  return 0;
}

TCA
TranslatorX64::createTranslation(SrcKey sk, bool align,
                                 bool forceNoHHIR /* = false */) {
  /*
   * Try to become the writer. We delay this until we *know* we will have
   * a need to create new translations, instead of just trying to win the
   * lottery at the dawn of time. Hopefully lots of requests won't require
   * any new translation.
   */
  auto retransl = [&] {
    return retranslate(sk, align, !forceNoHHIR);
  };
  LeaseHolder writer(s_writeLease);
  if (!writer) return nullptr;
  if (SrcRec* sr = m_srcDB.find(sk)) {
    TCA tca = sr->getTopTranslation();
    if (tca) {
      // Handle extremely unlikely race; someone may have just already
      // added the first instance of this SrcRec while we did a
      // non-blocking wait on the write lease.
      return tca;
    } else {
      // Since we are holding the write lease, we know that sk is properly
      // initialized, except that it has no translations (due to
      // replaceOldTranslations)
      return retransl();
    }
  }

  // We put retranslate requests at the end of our slab to more frequently
  //   allow conditional jump fall-throughs
  AHotSelector ahs(this, curFunc()->attrs() & AttrHot);

  TCA astart = a.code.frontier;
  TCA stubstart = astubs.code.frontier;
  TCA req = emitServiceReq(SRFlags::SRNone, REQ_RETRANSLATE,
                           1, uint64_t(sk.offset()));
  SKTRACE(1, sk, "inserting anchor translation for (%p,%d) at %p\n",
          curUnit(), sk.offset(), req);
  SrcRec* sr = m_srcDB.insert(sk);
  sr->setFuncInfo(curFunc());
  sr->setAnchorTranslation(req);

  size_t asize = a.code.frontier - astart;
  size_t stubsize = astubs.code.frontier - stubstart;
  assert(asize == 0);
  if (stubsize) {
    addTranslation(TransRec(sk, curUnit()->md5(), TransAnchor,
                            astart, asize, stubstart, stubsize));
    assert(!isTransDBEnabled() || getTransRec(stubstart)->kind == TransAnchor);
  }

  return retransl();
}

TCA
TranslatorX64::lookupTranslation(SrcKey sk) const {
  if (SrcRec* sr = m_srcDB.find(sk)) {
    return sr->getTopTranslation();
  }
  return nullptr;
}

TCA
TranslatorX64::translate(SrcKey sk, bool align, bool allowIR) {
  bool useHHIR = allowIR && RuntimeOption::EvalJitUseIR;
  INC_TPC(translate);
  assert(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  assert(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);

  if (useHHIR) {
    if (m_numHHIRTrans == RuntimeOption::EvalMaxHHIRTrans) {
      useHHIR = m_useHHIR = false;
      RuntimeOption::EvalJitUseIR = false;
    } else {
      m_useHHIR = true;
    }
  } else {
    assert(m_useHHIR == false);
  }

  AHotSelector ahs(this, curFunc()->attrs() & AttrHot);

  if (align) {
    moveToAlign(a, kNonFallthroughAlign);
  }

  TCA start = a.code.frontier;
  m_lastHHIRPunt.clear();
  translateTracelet(sk, m_useHHIR || RuntimeOption::EvalHHIRDisableTx64);

  SKTRACE(1, sk, "translate moved head from %p to %p\n",
          getTopTranslation(sk), start);
  if (Trace::moduleEnabledRelease(tcdump, 1)) {
    static __thread int n;
    if (++n % 10000 == 0) {
      std::ofstream f("cfg.dot", std::ios_base::trunc);
      drawCFG(f);
      f.close();
    }
  }
  return start;
}

/*
 * Returns true if the given current frontier can have an nBytes-long
 * instruction written without any risk of cache-tearing.
 */
bool
TranslatorX64::isSmashable(Address frontier, int nBytes, int offset /* = 0 */) {
  assert(nBytes <= int(kX64CacheLineSize));
  uintptr_t iFrontier = uintptr_t(frontier) + offset;
  uintptr_t lastByte = uintptr_t(frontier) + nBytes - 1;
  return (iFrontier & ~kX64CacheLineMask) == (lastByte & ~kX64CacheLineMask);
}

/*
 * Call before emitting a test-jcc sequence. Inserts a nop gap such that after
 * writing a testBytes-long instruction, the frontier will be smashable.
 */
void
TranslatorX64::prepareForTestAndSmash(int testBytes, TestAndSmashFlags flags) {
  if (flags == kAlignJcc) {
    prepareForSmash(testBytes + kJmpccLen, testBytes);
    assert(isSmashable(a.code.frontier + testBytes, kJmpccLen));
  } else if (flags == kAlignJccImmediate) {
    prepareForSmash(testBytes + kJmpccLen,
                    testBytes + kJmpccLen - kJmpImmBytes);
    assert(isSmashable(a.code.frontier + testBytes, kJmpccLen,
                       kJmpccLen - kJmpImmBytes));
  } else if (flags == kAlignJccAndJmp) {
    // Ensure that the entire jcc, and the entire jmp are smashable
    // (but we dont need them both to be in the same cache line)
    prepareForSmash(testBytes + kJmpccLen, testBytes);
    prepareForSmash(testBytes + kJmpccLen + kJmpLen, testBytes + kJmpccLen);
    assert(isSmashable(a.code.frontier + testBytes, kJmpccLen));
    assert(isSmashable(a.code.frontier + testBytes + kJmpccLen, kJmpLen));
  } else {
    not_reached();
  }
}

void
TranslatorX64::prepareForSmash(X64Assembler& a, int nBytes,
                               int offset /* = 0 */) {
  if (!isSmashable(a.code.frontier, nBytes, offset)) {
    int gapSize = (~(uintptr_t(a.code.frontier) + offset) &
                   kX64CacheLineMask) + 1;
    a.emitNop(gapSize);
    assert(isSmashable(a.code.frontier, nBytes, offset));
  }
}

void
TranslatorX64::prepareForSmash(int nBytes, int offset /* = 0 */) {
  prepareForSmash(a, nBytes, offset);
}

void
TranslatorX64::smash(X64Assembler &a, TCA src, TCA dest, bool isCall) {
  assert(canWrite());
  TRACE(2, "smash: %p -> %p\n", src, dest);
  /*
   * !
   *
   * We are about to smash reachable code in the translation cache. A
   * hardware thread might be executing the very instruction we're
   * modifying. This is safe because:
   *
   *    1. We align smashable instructions so that they reside on a single
   *       cache line;
   *
   *    2. We modify the instruction with a single processor store; and
   *
   *    3. The smashed region contains only a single instruction in the
   *       orignal instruction stream (see jmp() -> emitJ32() -> bytes() in
   *       the assembler.
   */
  CodeCursor cg(a, src);
  assert(isSmashable(a.code.frontier, kJmpLen));
  if (dest > src && dest - src <= 7) {
    assert(!isCall);
    a.    emitNop(dest - src);
  } else if (!isCall) {
    a.    jmp(dest);
  } else {
    a.    call(dest);
  }
}

void TranslatorX64::protectCode() {
  mprotect(tx64->ahot.code.base,
           tx64->astubs.code.base - tx64->ahot.code.base +
           tx64->astubs.code.size, PROT_READ | PROT_EXEC);

}

void TranslatorX64::unprotectCode() {
  mprotect(tx64->ahot.code.base,
           tx64->astubs.code.base - tx64->ahot.code.base +
           tx64->astubs.code.size,
           PROT_READ | PROT_WRITE | PROT_EXEC);
}

void
TranslatorX64::emitStackCheck(int funcDepth, Offset pc) {
  uint64_t stackMask = cellsToBytes(RuntimeOption::EvalVMStackElms) - 1;
  a.    mov_reg64_reg64(rVmSp, rScratch); // copy to destroy
  a.    and_imm64_reg64(stackMask, rScratch);
  a.    sub_imm64_reg64(funcDepth + Stack::sSurprisePageSize, rScratch);
  assert(m_stackOverflowHelper);
  a.    jl(m_stackOverflowHelper); // Unlikely branch to failure.
  // Success.
}

// Tests the surprise flags for the current thread. Should be used
// before a jnz to surprise handling code.
void
TranslatorX64::emitTestSurpriseFlags(Asm& a) {
  static_assert(RequestInjectionData::LastFlag < (1 << 8),
                "Translator assumes RequestInjectionFlags fit in one byte");
  a.    testb((int8_t)0xff, rVmTl[TargetCache::kConditionFlagsOff]);
}

void
TranslatorX64::emitCheckSurpriseFlagsEnter(bool inTracelet, Fixup fixup) {
  emitTestSurpriseFlags(a);
  {
    UnlikelyIfBlock ifTracer(CC_NZ, a, astubs);
    if (false) { // typecheck
      const ActRec* ar = nullptr;
      EventHook::FunctionEnter(ar, 0);
    }
    astubs.mov_reg64_reg64(rVmFp, argNumToRegName[0]);
    static_assert(EventHook::NormalFunc == 0,
                  "EventHook::NormalFunc should be the first enum element");
    astubs.xor_reg32_reg32(argNumToRegName[1], argNumToRegName[1]);
    emitCall(astubs, (TCA)&EventHook::FunctionEnter);
    if (inTracelet) {
      recordSyncPoint(astubs, fixup.m_pcOffset, fixup.m_spOffset);
    } else {
      // If we're being called while generating a func prologue, we
      // have to record the fixup directly in the fixup map instead of
      // going through m_pendingFixups like normal.
      m_fixupMap.recordFixup(astubs.code.frontier, fixup);
    }
  }
}

void
TranslatorX64::setArgInActRec(ActRec* ar, int argNum, uint64_t datum,
                              DataType t) {
  TypedValue* tv =
    (TypedValue*)(uintptr_t(ar) - (argNum+1) * sizeof(TypedValue));
  tv->m_data.num = datum;
  tv->m_type = t;
}

int
TranslatorX64::shuffleArgsForMagicCall(ActRec* ar) {
  if (!ar->hasInvName()) {
    return 0;
  }
  const Func* f UNUSED = ar->m_func;
  f->validate();
  assert(f->name()->isame(s___call.get())
         || f->name()->isame(s___callStatic.get()));
  assert(f->numParams() == 2);
  TRACE(1, "shuffleArgsForMagicCall: ar %p\n", ar);
  assert(ar->hasInvName());
  StringData* invName = ar->getInvName();
  assert(invName);
  ar->setVarEnv(nullptr);
  int nargs = ar->numArgs();
  // We need to make an array containing all the arguments passed by the
  // caller and put it where the second argument is
  HphpArray* argArray = NEW(HphpArray)(nargs);
  argArray->incRefCount();
  for (int i = 0; i < nargs; ++i) {
    TypedValue* tv =
      (TypedValue*)(uintptr_t(ar) - (i+1) * sizeof(TypedValue));
    argArray->nvAppend(tv);
    tvRefcountedDecRef(tv);
  }
  // Put invName in the slot for first argument
  setArgInActRec(ar, 0, uint64_t(invName), BitwiseKindOfString);
  // Put argArray in the slot for second argument
  setArgInActRec(ar, 1, uint64_t(argArray), KindOfArray);
  // Fix up ActRec's numArgs
  ar->initNumArgs(2);
  return 1;
}

/*
 * The standard VMRegAnchor treatment won't work for some cases called
 * during function preludes.
 *
 * The fp sync machinery is fundamentally based on the notion that
 * instruction pointers in the TC are uniquely associated with source
 * HHBC instructions, and that source HHBC instructions are in turn
 * uniquely associated with SP->FP deltas.
 *
 * run_intercept_helper/trimExtraArgs is called from the prologue of
 * the callee. The prologue is 1) still in the caller frame for now,
 * and 2) shared across multiple call sites. 1 means that we have the
 * fp from the caller's frame, and 2 means that this fp is not enough
 * to figure out sp.
 *
 * However, the prologue passes us the callee actRec, whose predecessor
 * has to be the caller. So we can sync sp and fp by ourselves here.
 * Geronimo!
 */
static void sync_regstate_to_caller(ActRec* preLive) {
  assert(tl_regState == REGSTATE_DIRTY);
  vmfp() = (TypedValue*)preLive->m_savedRbp;
  vmsp() = (TypedValue*)preLive - preLive->numArgs();
  if (ActRec* fp = g_vmContext->m_fp) {
    if (fp->m_func && fp->m_func->unit()) {
      vmpc() = fp->m_func->unit()->at(fp->m_func->base() + preLive->m_soff);
    }
  }
  tl_regState = REGSTATE_CLEAN;
}

static uint64_t run_intercept_helper(ActRec* ar, Variant* ihandler) {
  sync_regstate_to_caller(ar);
  bool ret = run_intercept_handler<true>(ar, ihandler);
  /*
   * Restore tl_regState manually in the no-exception case only.  (The
   * VM regs are clean here---we only need to set them dirty if we are
   * stopping to execute in the TC again, which we won't be doing if
   * an exception is propagating.)
   */
  tl_regState = REGSTATE_DIRTY;
  return ret;
}

void
TranslatorX64::trimExtraArgs(ActRec* ar) {
  assert(!ar->hasInvName());

  sync_regstate_to_caller(ar);
  const Func* f = ar->m_func;
  int numParams = f->numParams();
  int numArgs = ar->numArgs();
  assert(numArgs > numParams);
  int numExtra = numArgs - numParams;

  TRACE(1, "trimExtraArgs: %d args, function %s takes only %d, ar %p\n",
        numArgs, f->name()->data(), numParams, ar);

  if (f->attrs() & AttrMayUseVV) {
    assert(!ar->hasExtraArgs());
    ar->setExtraArgs(ExtraArgs::allocateCopy(
      (TypedValue*)(uintptr_t(ar) - numArgs * sizeof(TypedValue)),
      numArgs - numParams));
  } else {
    // Function is not marked as "MayUseVV", so discard the extra arguments
    TypedValue* tv = (TypedValue*)(uintptr_t(ar) - numArgs*sizeof(TypedValue));
    for (int i = 0; i < numExtra; ++i) {
      tvRefcountedDecRef(tv);
      ++tv;
    }
    ar->setNumArgs(numParams);
  }

  // Only go back to dirty in a non-exception case.  (Same reason as
  // above.)
  tl_regState = REGSTATE_DIRTY;
}

TCA
TranslatorX64::getInterceptHelper() {
  if (false) {  // typecheck
    Variant *h = get_intercept_handler(CStrRef((StringData*)nullptr),
                                       (char*)nullptr);
    bool c UNUSED = run_intercept_helper((ActRec*)nullptr, h);
  }
  if (!m_interceptHelper) {
    m_interceptHelper = TCA(astubs.code.frontier);
    astubs.    loadq  (rStashedAR[AROFF(m_func)], rax);
    astubs.    lea    (rax[Func::fullNameOff()], argNumToRegName[0]);
    astubs.    lea    (rax[Func::maybeInterceptedOff()], argNumToRegName[1]);

    astubs.    sub_imm32_reg64(8, rsp); // Stack parity {
    astubs.    call(TCA(get_intercept_handler));
    astubs.    add_imm32_reg64(8, rsp);  // }
    astubs.    test_reg64_reg64(rax, rax);
    {
      JccBlock<CC_NZ> ifNotIntercepted(astubs);
      astubs.  ret();
    }

    // we might re-enter, so align the stack
    astubs.    sub_imm32_reg64(8, rsp);
    // Copy the old rbp into the savedRbp pointer.
    astubs.    store_reg64_disp_reg64(rbp, 0, rStashedAR);

    PhysReg rSavedRip = r13; // XXX ideally don't hardcode r13 ... but
                             // we need callee-saved and don't have
                             // any scratch ones.

    // Fish out the saved rip. We may need to jump there, and the helper will
    // have wiped out the ActRec.
    astubs.    load_reg64_disp_reg64(rStashedAR, AROFF(m_savedRip),
                                     rSavedRip);
    astubs.    mov_reg64_reg64(rStashedAR, argNumToRegName[0]);
    astubs.    mov_reg64_reg64(rax, argNumToRegName[1]);
    astubs.    call(TCA(run_intercept_helper));

    // Normally we'd like to recordReentrantCall here, but the vmreg sync'ing
    // for run_intercept_handler is a special little snowflake. See
    // run_intercept_handler for details.
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
      astubs.  add_imm32_reg64(16, rsp);
      astubs.  lea_reg64_disp_reg64(rStashedAR, AROFF(m_r), rVmSp);
      astubs.  jmp(rSavedRip);
    }
    astubs.    add_imm32_reg64(8, rsp);
    astubs.    ret();
  }
  return m_interceptHelper;
}

TCA
TranslatorX64::getCallArrayProlog(Func* func) {
  TCA tca = func->getFuncBody();
  if (tca != (TCA)funcBodyHelperThunk) return tca;

  int numParams = func->numParams();
  std::vector<std::pair<int,Offset> > dvs;
  for (int i = 0; i < numParams; ++i) {
    const Func::ParamInfo& pi = func->params()[i];
    if (pi.hasDefaultValue()) {
      dvs.push_back(std::make_pair(i, pi.funcletOff()));
    }
  }
  if (dvs.size()) {
    LeaseHolder writer(s_writeLease);
    if (!writer) return nullptr;
    tca = func->getFuncBody();
    if (tca != (TCA)funcBodyHelperThunk) return tca;
    tca = a.code.frontier;
    if (dvs.size() == 1) {
      a.   cmp_imm32_disp_reg32(dvs[0].first,
                                AROFF(m_numArgsAndCtorFlag), rVmFp);
      emitBindJcc(a, CC_LE, SrcKey(func, dvs[0].second));
      emitBindJmp(a, SrcKey(func, func->base()));
    } else {
      a.   load_reg64_disp_reg32(rVmFp, AROFF(m_numArgsAndCtorFlag), rax);
      for (unsigned i = 0; i < dvs.size(); i++) {
        a.   cmp_imm32_reg32(dvs[i].first, rax);
        emitBindJcc(a, CC_LE, SrcKey(func, dvs[i].second));
      }
      emitBindJmp(a, SrcKey(func, func->base()));
    }
  } else {
    SrcKey sk(func, func->base());
    tca = tx64->getTranslation(sk, false);
  }

  return tca;
}

TCA
TranslatorX64::emitPrologueRedispatch(X64Assembler& a) {
  TCA retval;
  moveToAlign(a);
  retval = a.code.frontier;
  TRACE(1, "HOTSTUB: emitPrologueRedispatch: %lx\n", uintptr_t(a.code.frontier));

  // We're in the wrong func prologue.

  assert(kScratchCrossTraceRegs.contains(rax));
  assert(kScratchCrossTraceRegs.contains(rdx));
  assert(kScratchCrossTraceRegs.contains(rcx));

  //    Get the called func in rax
  a.    load_reg64_disp_reg64(rStashedAR, AROFF(m_func), rax);
  //    Get the number of passed parameters in rdx
  a.    load_reg64_disp_reg32(rStashedAR, AROFF(m_numArgsAndCtorFlag), rdx);
  a.    and_imm32_reg32(0x7fffffff, rdx);
  //    Get the number of declared parameters in rcx
  a.    load_reg64_disp_reg32(rax, Func::numParamsOff(), rcx);

  // If we didn't pass too many args, directly dereference
  // func->m_prologues.
  a.    cmp_reg32_reg32(rdx, rcx);
  TCA bToFixedProloguesCheck = a.code.frontier;
  a.    jcc8(CC_L, bToFixedProloguesCheck);

  //   cmp $kNumFixedPrologues, %rdx
  //   jl numParamsCheck
  TCA actualDispatch = a.code.frontier;

  // rcx: prologueIdx
  // rax = func->prologues[numParams]
  // jmp rax
  a.    loadq  (rax[rdx*8 + Func::prologueTableOff()], rax);
  a.    jmp    (rax);
  a.    ud2    ();

  // Hmm, more parameters passed than the function expected. Did we pass
  // kNumFixedPrologues or more? If not, %rdx is still a perfectly
  // legitimate index into the func prologue table.
  // numParamsCheck:
  //    cmp $kNumFixedPrologues, %rcx
  //    jl  dispatch
  a.patchJcc8(bToFixedProloguesCheck, a.code.frontier); // numParamsCheck:
  a.    cmp_imm32_reg32(kNumFixedPrologues, rdx);
  a.    jcc8(CC_L, actualDispatch);

  // Too many gosh-darned parameters passed. Go to numExpected + 1, which
  // is always a "too many params" entry point.
  //
  //    mov %rdx, %rcx
  //    add $1, %rcx
  //    jmp dispatch
  a.    load_reg64_disp_index_reg64(rax,
                                    // %rcx + 1
                                    Func::prologueTableOff() + sizeof(TCA),
                                    rcx,
                                    rax);
  a.    jmp(rax);
  a.    ud2();
  return retval;
}

// The funcGuard gets skipped and patched by other code, so we have some
// magic offsets.
static const int kFuncMovImm = 6; // Offset to the immediate for 8 byte Func*
static const int kFuncCmpImm = 4; // Offset to the immediate for 4 byte Func*
static const int kFuncGuardLen = 23;
static const int kFuncGuardShortLen = 14;

template<typename T>
static T*
funcPrologToGuardImm(TCA prolog) {
  assert(sizeof(T) == 4 || sizeof(T) == 8);
  T* retval = (T*)(prolog - (sizeof(T) == 8 ?
                             kFuncGuardLen - kFuncMovImm :
                             kFuncGuardShortLen - kFuncCmpImm));
  // We padded these so the immediate would fit inside a cache line
  assert(((uintptr_t(retval) ^ (uintptr_t(retval + 1) - 1)) &
          ~(TranslatorX64::kX64CacheLineSize - 1)) == 0);

  return retval;
}

static inline bool
funcPrologHasGuard(TCA prolog, const Func* func) {
  intptr_t iptr = uintptr_t(func);
  if (deltaFits(iptr, sz::dword)) {
    return *funcPrologToGuardImm<int32_t>(prolog) == iptr;
  }
  return *funcPrologToGuardImm<int64_t>(prolog) == iptr;
}

static void
disableFuncGuard(TCA prolog, Func* func) {
  assert(funcPrologHasGuard(prolog, func));
  if (deltaFits((intptr_t)func, sz::dword)) {
    *funcPrologToGuardImm<int32_t>(prolog) = 0;
  } else {
    *funcPrologToGuardImm<int64_t>(prolog) = 0;
  }
  assert(!funcPrologHasGuard(prolog, func));
}

static TCA
funcPrologToGuard(TCA prolog, const Func* func) {
  if (!prolog || prolog == (TCA)fcallHelperThunk) return prolog;
  return prolog -
    (deltaFits(uintptr_t(func), sz::dword) ?
     kFuncGuardShortLen :
     kFuncGuardLen);
}

TCA
TranslatorX64::emitFuncGuard(X64Assembler& a, const Func* func) {
  assert(kScratchCrossTraceRegs.contains(rax));
  assert(kScratchCrossTraceRegs.contains(rdx));

  const int kAlign = kX64CacheLineSize;
  const int kAlignMask = kAlign - 1;
  int loBits = uintptr_t(a.code.frontier) & kAlignMask;
  int delta, size;

  // Ensure the immediate is safely smashable
  // the immediate must not cross a qword boundary,
  if (!deltaFits((intptr_t)func, sz::dword)) {
    size = 8;
    delta = loBits + kFuncMovImm;
  } else {
    size = 4;
    delta = loBits + kFuncCmpImm;
  }

  delta = (delta + size - 1) & kAlignMask;
  if (delta < size - 1) {
    a.emitNop(size - 1 - delta);
  }

  TCA aStart DEBUG_ONLY = a.code.frontier;
  if (!deltaFits((intptr_t)func, sz::dword)) {
    a.    load_reg64_disp_reg64(rStashedAR, AROFF(m_func), rax);
    /*
      Although func doesnt fit in a signed 32-bit immediate, it may still
      fit in an unsigned one. Rather than deal with yet another case
      (which only happens when we disable jemalloc) just force it to
      be an 8-byte immediate, and patch it up afterwards.
    */
    a.    mov_imm64_reg(0xdeadbeeffeedface, rdx);
    assert(((uint64_t*)a.code.frontier)[-1] == 0xdeadbeeffeedface);
    ((uint64_t*)a.code.frontier)[-1] = uintptr_t(func);
    a.    cmp_reg64_reg64(rax, rdx);
  } else {
    a.    cmp_imm32_disp_reg32(uint64_t(func), AROFF(m_func), rStashedAR);
  }

  assert(m_funcPrologueRedispatch);

  a.    jnz(m_funcPrologueRedispatch);
  assert(funcPrologToGuard(a.code.frontier, func) == aStart);
  assert(funcPrologHasGuard(a.code.frontier, func));
  return a.code.frontier;
}

/*
 * funcPrologue --
 *
 * Given a callee and a number of args, match up to the callee's
 * argument expectations and dispatch.
 *
 * Call/return hand-shaking is a bit funny initially. At translation time,
 * we don't necessarily know what function we're calling. For instance,
 *
 *   f(g());
 *
 * Will lead to a set of basic blocks like:
 *
 * b1: pushfuncd "f"
 *     pushfuncd "g"
 *     fcall
 * b2: fcall
 *
 * The fcallc labelled "b2" above is not statically bindable in our
 * execution model.
 *
 * We decouple the call work into a per-callsite portion, responsible
 * for recording the return address, and a per-(callee, numArgs) portion,
 * responsible for fixing up arguments and dispatching to remaining
 * code. We call the per-callee portion a "prologue."
 *
 * Also, we are called from two distinct environments. From REQ_BIND_CALL,
 * we're running "between" basic blocks, with all VM registers sync'ed.
 * However, we're also called in the middle of basic blocks, when dropping
 * entries into func->m_prologues. So don't go around using the
 * translation-time values of vmfp()/vmsp(), since they have an
 * unpredictable relationship to the source.
 */
bool
TranslatorX64::checkCachedPrologue(const Func* func, int paramIdx,
                                   TCA& prologue) const {
  prologue = (TCA)func->getPrologue(paramIdx);
  if (prologue != (TCA)fcallHelperThunk && !s_replaceInFlight) {
    TRACE(1, "cached prologue %s(%d) -> cached %p\n",
          func->fullName()->data(), paramIdx, prologue);
    assert(isValidCodeAddress(prologue));
    return true;
  }
  return false;
}

// pops the return address pushed by fcall and stores it into the actrec
void
TranslatorX64::emitPopRetIntoActRec(Asm& a) {
  a.    pop  (rStashedAR[AROFF(m_savedRip)]);
}

static void interp_set_regs(ActRec* ar, Cell* sp, Offset pcOff) {
  assert(tl_regState == REGSTATE_DIRTY);
  tl_regState = REGSTATE_CLEAN;
  vmfp() = (Cell*)ar;
  vmsp() = sp;
  vmpc() = curUnit()->at(pcOff);
}

TCA
TranslatorX64::funcPrologue(Func* func, int nPassed, ActRec* ar) {
  func->validate();
  TRACE(1, "funcPrologue %s(%d)\n", func->fullName()->data(), nPassed);
  int numParams = func->numParams();
  int paramIndex = nPassed <= numParams ? nPassed : numParams + 1;

  bool funcIsMagic = func->isMagic();

  // Do a quick test before grabbing the write lease
  TCA prologue;
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;
  if (func->isClonedClosure()) {
    assert(ar);
    const Func::ParamInfoVec& paramInfo = func->params();
    Offset entry = func->base();
    for (int i = nPassed; i < numParams; ++i) {
      const Func::ParamInfo& pi = paramInfo[i];
      if (pi.hasDefaultValue()) {
        entry = pi.funcletOff();
        break;
      }
    }
    interp_set_regs(ar, (Cell*)ar - func->numSlotsInFrame(), entry);
    SrcKey funcBody(func, entry);
    TCA tca = getTranslation(funcBody, false);
    tl_regState = REGSTATE_DIRTY;
    if (tca) {
      // racy, but ok...
      func->setPrologue(paramIndex, tca);
    }
    return tca;
  }

  // If the translator is getting replaced out from under us, refuse to
  // provide a prologue; we don't know whether this request is running on the
  // old or new context.
  LeaseHolder writer(s_writeLease);
  if (!writer || s_replaceInFlight) return nullptr;
  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  AHotSelector ahs(this, func->attrs() & AttrHot);

  SpaceRecorder sr("_FuncPrologue", a);
  // If we're close to a cache line boundary, just burn some space to
  // try to keep the func and its body on fewer total lines.
  if (((uintptr_t)a.code.frontier & kX64CacheLineMask) >= 32) {
    moveToAlign(a, kX64CacheLineSize);
  }
  // Careful: this isn't necessarily the real entry point. For funcIsMagic
  // prologues, this is just a possible prologue.
  TCA aStart    = a.code.frontier;
  TCA start     = aStart;
  TCA stubStart = astubs.code.frontier;

  // Guard: we're in the right callee. This happens in magicStart for
  // magic callees.
  if (!funcIsMagic) {
    start = aStart = emitFuncGuard(a, func);
  }

  emitRB(a, RBTypeFuncPrologueTry, func->fullName()->data());

  // NB: We have most of the register file to play with, since we know
  // we're between BB's. So, we hardcode some registers here rather
  // than using the scratch allocator.
  TRACE(2, "funcPrologue: user function: %s\n", func->name()->data());

  // Add a counter for the translation if requested
  if (RuntimeOption::EvalJitTransCounters) {
    emitTransCounterInc(a);
  }

  if (!funcIsMagic) {
    emitPopRetIntoActRec(a);
    // entry point for magic methods comes later
    emitRB(a, RBTypeFuncEntry, func->fullName()->data());
    // Guard: we have stack enough stack space to complete this function.
    emitStackCheck(cellsToBytes(func->maxStackCells()), func->base());
  }

  SrcKey skFuncBody = emitPrologue(func, nPassed);

  if (funcIsMagic) {
    // entry points for magic methods is here
    TCA magicStart = emitFuncGuard(a, func);
    emitPopRetIntoActRec(a);
    emitRB(a, RBTypeFuncEntry, func->fullName()->data());
    // Guard: we have stack enough stack space to complete this function.
    emitStackCheck(cellsToBytes(func->maxStackCells()), func->base());
    assert(numParams == 2);
    // Special __call prologue
    a.  mov_reg64_reg64(rStashedAR, argNumToRegName[0]);
    emitCall(a, TCA(TranslatorX64::shuffleArgsForMagicCall));
    // if shuffleArgs returns 0, that means this was not a magic call
    // and we should proceed to a prologue specialized for nPassed;
    // otherwise, proceed to a prologue specialized for nPassed==numParams (2).
    if (nPassed == 2) {
      a.jmp(start);
    } else {
      a.test_reg64_reg64(rax, rax);
      // z ==> not a magic call, go to prologue for nPassed
      if (deltaFits(start - (a.code.frontier + kJcc8Len), sz::byte)) {
        a.jcc8(CC_Z, start);
      } else {
        a.jcc(CC_Z, start);
      }
      // this was a magic call
      // nPassed == 2
      // Fix up hardware stack pointer
      nPassed = 2;
      emitLea(a, rStashedAR, -cellsToBytes(nPassed), rVmSp);
      // Optimization TODO: Reuse the prologue for args == 2
      emitPrologue(func, nPassed);
    }
    start = magicStart;
  }
  assert(funcPrologHasGuard(start, func));
  TRACE(2, "funcPrologue tx64 %p %s(%d) setting prologue %p\n",
        this, func->fullName()->data(), nPassed, start);
  assert(isValidCodeAddress(start));
  func->setPrologue(paramIndex, start);

  addTranslation(TransRec(skFuncBody, func->unit()->md5(),
                          TransProlog, aStart, a.code.frontier - aStart,
                          stubStart, astubs.code.frontier - stubStart));

  recordGdbTranslation(skFuncBody, func,
                       a, aStart,
                       false, true);
  recordBCInstr(OpFuncPrologue, a, start);

  return start;
}

TCA
TranslatorX64::emitInterceptPrologue(Func* func) {
  TCA start = a.code.frontier;
  emitImmReg(a, int64_t(func), rax);
  a.    cmpb (0, rax[Func::maybeInterceptedOff()]);
  semiLikelyIfBlock(CC_NE, a, [&]{
    // Prologues are not really sites for function entry yet; we can get
    // here via an optimistic bindCall. Check that the func is as expected.
    a.  cmpq (rax, rStashedAR[AROFF(m_func)]);
    {
      JccBlock<CC_NZ> skip(a);
      a.call(getInterceptHelper());
    }
  });
  return start;
}

void
TranslatorX64::interceptPrologues(Func* func) {
  if (!RuntimeOption::EvalJitEnableRenameFunction &&
      !(func->attrs() & AttrDynamicInvoke)) {
    return;
  }
  if (func->maybeIntercepted() == -1) {
    return;
  }
  func->maybeIntercepted() = -1;
  assert(s_writeLease.amOwner());
  int maxNumPrologues = func->numPrologues();
  for (int i = 0; i < maxNumPrologues; i++) {
    TCA prologue = func->getPrologue(i);
    if (prologue == (unsigned char*)fcallHelperThunk)
      continue;
    assert(funcPrologHasGuard(prologue, func));
    // There might already be calls hard-coded to this via FCall.
    // blow away immediate comparison, so that we always use the Func*'s
    // prologue table. We use 0 (== NULL on our architecture) as the bit
    // pattern for an impossible Func.
    //
    // Note that we're modifying reachable code.
    disableFuncGuard(prologue, func);

    // There's a prologue already generated; redirect it to first
    // call the intercept helper. First, reset it (leaking the old
    // prologue), so funcPrologue will re-emit it.
    func->setPrologue(i, (TCA)fcallHelperThunk);
    TCA addr = funcPrologue(func, i);
    assert(funcPrologHasGuard(addr, func));
    assert(addr);
    func->setPrologue(i, addr);
    TRACE(1, "interceptPrologues %s prologue[%d]=%p\n",
          func->fullName()->data(), i, (void*)addr);
  }
}

static void raiseMissingArgument(const char* name, int expected, int got) {
  if (expected == 1) {
    raise_warning(Strings::MISSING_ARGUMENT, name, got);
  } else {
    raise_warning(Strings::MISSING_ARGUMENTS, name, expected, got);
  }
}

SrcKey
TranslatorX64::emitPrologue(Func* func, int nPassed) {
  int numParams = func->numParams();
  const Func::ParamInfoVec& paramInfo = func->params();
  assert(IMPLIES(func->maybeIntercepted() == -1,
                 m_interceptsEnabled));
  if (m_interceptsEnabled &&
      !func->isPseudoMain() &&
      !func->isGenerator() &&
      (RuntimeOption::EvalJitEnableRenameFunction ||
       func->attrs() & AttrDynamicInvoke)) {
    emitInterceptPrologue(func);
  }

  Offset dvInitializer = InvalidAbsoluteOffset;

  assert(IMPLIES(func->isGenerator(), nPassed == numParams));
  if (nPassed > numParams) {
    // Too many args; a weird case, so just callout. Stash ar
    // somewhere callee-saved.
    if (false) { // typecheck
      TranslatorX64::trimExtraArgs((ActRec*)nullptr);
    }
    a.  mov_reg64_reg64(rStashedAR, argNumToRegName[0]);
    emitCall(a, TCA(TranslatorX64::trimExtraArgs));
    // We'll fix rVmSp below.
  } else if (nPassed < numParams) {
    // Figure out which, if any, default value initializer to go to
    for (int i = nPassed; i < numParams; ++i) {
      const Func::ParamInfo& pi = paramInfo[i];
      if (pi.hasDefaultValue()) {
        dvInitializer = pi.funcletOff();
        break;
      }
    }
    TRACE(1, "Only have %d of %d args; getting dvFunclet\n",
          nPassed, numParams);
    emitImmReg(a, nPassed, rax);
    // do { *(--rVmSp) = NULL; nPassed++; } while (nPassed < numParams);
    // This should be an unusual case, so optimize for code density
    // rather than execution speed; i.e., don't unroll the loop.
    TCA loopTop = a.code.frontier;
    a.  sub_imm32_reg64(sizeof(Cell), rVmSp);
    a.  incl(eax);
    emitStoreUninitNull(a, 0, rVmSp);
    a.  cmp_imm32_reg32(numParams, rax);
    a.  jcc8(CC_L, loopTop);
  }

  // Entry point for numParams == nPassed is here.
  // Args are kosher. Frame linkage: set fp = ar.
  a.    mov_reg64_reg64(rStashedAR, rVmFp);

  int numLocals = numParams;
  if (func->isClosureBody()) {
    int numUseVars = func->cls()->numDeclProperties();

    emitLea(a, rVmFp, -cellsToBytes(numParams), rVmSp);

    PhysReg rClosure = rcx;
    a.  loadq(rVmFp[AROFF(m_this)], rClosure);

    // Swap in the $this or late bound class
    a.  loadq(rClosure[c_Closure::thisOffset()], rScratch);
    a.  storeq(rScratch, rVmFp[AROFF(m_this)]);

    a.  shrq(1, rScratch);
    if (func->attrs() & AttrStatic) {
      UnlikelyIfBlock ifRealThis(CC_NBE, a, astubs);
      astubs.shlq(1, rScratch);
      emitIncRef(astubs, rScratch, KindOfObject);
    } else {
      JccBlock<CC_BE> ifRealThis(a);
      a.shlq(1, rScratch);
      emitIncRef(rScratch, KindOfObject);
    }

    // Put in the correct context
    a.  loadq(rClosure[c_Closure::funcOffset()], rScratch);
    a.  storeq(rScratch, rVmFp[AROFF(m_func)]);

    // Copy in all the use vars
    int baseUVOffset = sizeof(ObjectData) + func->cls()->builtinPropSize();
    for (int i = 0; i < numUseVars + 1; i++) {
      int spOffset = -cellsToBytes(i+1);

      if (i == 0) {
        // The closure is the first local.
        // We don't incref because it used to be $this
        // and now it is a local, so they cancel out
        emitStoreTypedValue(a, KindOfObject, rClosure, spOffset, rVmSp);
        continue;
      }

      int uvOffset = baseUVOffset + cellsToBytes(i-1);

      emitCopyTo(a, rClosure, uvOffset, rVmSp, spOffset, rScratch);
      emitIncRefGenericRegSafe(rVmSp, spOffset, rScratch);
    }

    numLocals += numUseVars + 1;
  }

  // We're in the callee frame; initialize locals. Unroll the loop all
  // the way if there are a modest number of locals to update;
  // otherwise, do it in a compact loop. If we're in a generator body,
  // named locals will be initialized by UnpackCont so we can leave
  // them alone here.
  int numUninitLocals = func->numLocals() - numLocals;
  assert(numUninitLocals >= 0);
  if (numUninitLocals > 0 && !func->isGenerator()) {
    SpaceRecorder sr("_InitializeLocals", a);

    // If there are too many locals, then emitting a loop to initialize locals
    // is more compact, rather than emitting a slew of movs inline.
    if (numUninitLocals > kLocalsToInitializeInline) {
      PhysReg loopReg = rcx;

      // rVmFp + rcx points to the count/type fields of the TypedValue we're
      // about to write to.
      int loopStart = -func->numLocals() * sizeof(TypedValue) + TVOFF(m_type);
      int loopEnd = -numLocals * sizeof(TypedValue) + TVOFF(m_type);

      emitImmReg(a, loopStart, loopReg);
      emitImmReg(a, KindOfUninit, rdx);

      TCA topOfLoop = a.code.frontier;
      // do {
      //   rVmFp[loopReg].m_type = KindOfUninit;
      // } while(++loopReg != loopEnd);

      emitStoreTVType(a, edx, rVmFp[loopReg]);
      a.  addq   (sizeof(Cell), loopReg);
      a.  cmpq   (loopEnd, loopReg);
      a.  jcc8   (CC_NE, topOfLoop);
    } else {
      PhysReg base;
      int disp, k;
      static_assert(KindOfUninit == 0, "");
      if (numParams < func->numLocals()) {
        a.xorl (eax, eax);
      }
      for (k = numLocals; k < func->numLocals(); ++k) {
        locToRegDisp(Location(Location::Local, k), &base, &disp);
        emitStoreTVType(a, eax, base[disp + TVOFF(m_type)]);
      }
    }
  }

  const Opcode* destPC = func->unit()->entry() + func->base();
  if (dvInitializer != InvalidAbsoluteOffset) {
    // dispatch to funclet.
    destPC = func->unit()->entry() + dvInitializer;
  }
  SrcKey funcBody(func, destPC);

  // Move rVmSp to the right place: just past all locals
  int frameCells = func->numSlotsInFrame();
  if (func->isGenerator()) {
    frameCells = 0;
  } else {
    emitLea(a, rVmFp, -cellsToBytes(frameCells), rVmSp);
  }

  Fixup fixup(funcBody.m_offset - func->base(), frameCells);

  // Emit warnings for any missing arguments
  if (!func->isBuiltin()) {
    for (int i = nPassed; i < numParams; ++i) {
      if (paramInfo[i].funcletOff() == InvalidAbsoluteOffset) {
        emitImmReg(a, (intptr_t)func->name()->data(), argNumToRegName[0]);
        emitImmReg(a, numParams, argNumToRegName[1]);
        emitImmReg(a, i, argNumToRegName[2]);
        emitCall(a, (TCA)raiseMissingArgument);
        m_fixupMap.recordFixup(a.code.frontier, fixup);
      }
    }
  }

  // Check surprise flags in the same place as the interpreter: after
  // setting up the callee's frame but before executing any of its
  // code
  emitCheckSurpriseFlagsEnter(false, fixup);

  if (func->isClosureBody() && func->cls()) {
    int entry = nPassed <= numParams ? nPassed : numParams + 1;
    // Relying on rStashedAR == rVmFp here
    a.    loadq   (rStashedAR[AROFF(m_func)], rax);
    a.    loadq   (rax[Func::prologueTableOff() + sizeof(TCA)*entry], rax);
    a.    jmp     (rax);
  } else {
    emitBindJmp(funcBody);
  }
  return funcBody;
}

static bool
isNativeImplCall(const Func* funcd, int numArgs) {
  return funcd && funcd->isBuiltin() && numArgs == funcd->numParams();
}

int32_t // returns the amount by which rVmSp should be adjusted
TranslatorX64::emitBindCall(SrcKey srcKey, const Func* funcd, int numArgs) {
  // If this is a call to a builtin and we don't need any argument
  // munging, we can skip the prologue system and do it inline.
  if (isNativeImplCall(funcd, numArgs)) {
    StoreImmPatcher patchIP(a, (uint64_t)a.code.frontier, reg::rax,
                            cellsToBytes(numArgs) + AROFF(m_savedRip),
                            rVmSp);
    assert(funcd->numLocals() == funcd->numParams());
    assert(funcd->numIterators() == 0);
    emitLea(a, rVmSp, cellsToBytes(numArgs), rVmFp);
    emitCheckSurpriseFlagsEnter(true, Fixup(0, numArgs));
    // rVmSp is already correctly adjusted, because there's no locals
    // other than the arguments passed.
    auto retval = emitNativeImpl(funcd, false /* don't jump to return */);
    patchIP.patch(uint64_t(a.code.frontier));
    return retval;
  }
  if (debug) {
    a.    storeq (kUninitializedRIP,
                  rVmSp[cellsToBytes(numArgs) + AROFF(m_savedRip)]);
  }
  // Stash callee's rVmFp into rStashedAR for the callee's prologue
  emitLea(a, rVmSp, cellsToBytes(numArgs), rStashedAR);
  emitBindCallHelper(srcKey, funcd, numArgs);
  return 0;
}

void
TranslatorX64::emitBindCallHelper(SrcKey srcKey,
                                  const Func* funcd,
                                  int numArgs) {
  // Whatever prologue we're branching to will check at runtime that we
  // went to the right Func*, correcting if necessary. We treat the first
  // Func we encounter as a decent prediction. Make space to burn in a
  // TCA.
  ReqBindCall* req = m_globalData.alloc<ReqBindCall>();
  prepareForSmash(kCallLen);
  TCA toSmash = a.code.frontier;
  a.    call(astubs.code.frontier);

  astubs.    mov_reg64_reg64(rStashedAR, serviceReqArgRegs[1]);
  emitPopRetIntoActRec(astubs);
  emitServiceReq(SRFlags::SRInline, REQ_BIND_CALL, 1ull, req);

  TRACE(1, "will bind static call: tca %p, this %p, funcd %p, astubs %p\n",
        toSmash, this, funcd, astubs.code.frontier);
  req->m_toSmash = toSmash;
  req->m_nArgs = numArgs;
  req->m_sourceInstr = srcKey;
  req->m_isImmutable = (bool)funcd;

  return;
}

// for documentation see bindJmpccFirst below
void
TranslatorX64::emitCondJmp(SrcKey skTaken, SrcKey skNotTaken,
                           ConditionCode cc) {
  // should be true for SrcKeys generated via OpJmpZ/OpJmpNZ
  assert(skTaken.getFuncId() == skNotTaken.getFuncId());

  // reserve space for a smashable jnz/jmp pair; both initially point
  // to our stub.
  prepareForTestAndSmash(0, kAlignJccAndJmp);
  TCA old = a.code.frontier;
  TCA stub = astubs.code.frontier;

  // begin code for the stub

  // We need to be careful here, as we are passing an extra paramter to
  //   REQ_BIND_JMPCC_FIRST. However we can't pass this parameter via
  //   emitServiceReq because that only supports constants/immediates, so
  //   compute the last argument via setcc.
  astubs.setcc(cc, rbyte(serviceReqArgRegs[4]));
  emitServiceReq(SRFlags::SRInline, REQ_BIND_JMPCC_FIRST, 4ull,
                 old,
                 uint64_t(skTaken.offset()),
                 uint64_t(skNotTaken.offset()),
                 uint64_t(cc));

  a.jcc(cc, stub); // MUST use 4-byte immediate form
  a.jmp(stub); // MUST use 4-byte immediate form
}

static void skToName(SrcKey sk, char* name) {
  sprintf(name, "sk_%08lx_%05d",
          long(sk.getFuncId()), sk.offset());
}

static void skToClusterName(SrcKey sk, char* name) {
  sprintf(name, "skCluster_%08lx_%05d",
          long(sk.getFuncId()), sk.offset());
}

static void translToName(const TCA tca, char* name) {
  sprintf(name, "tc_%p", tca);
}

void TranslatorX64::drawCFG(std::ofstream& out) const {
  if (!isTransDBEnabled()) return;
  const char* indent = "    ";
  static int genCount;
  int numSrcKeys = 0;
  int numTranslations = 0;
  out << "digraph srcdb" << genCount++ <<" {\n";
  out << indent << "size = \"8,11\";\n";
  out << indent << "ratio = fill;\n";
  for (SrcDB::const_iterator entry = m_srcDB.begin();
       entry != m_srcDB.end(); ++entry) {
    const SrcKey sk = SrcKey::fromAtomicInt(entry->first);
    // 1 subgraph per srcKey.
    char name[64];
    skToClusterName(sk, name);
    numSrcKeys++;
    out << indent << "subgraph " << name << "{\n";
    char* indent = "        ";
    skToName(sk, name);
    out << indent << name << "[shape=box];\n";
    const vector<TCA>& transls = entry->second->translations();
    for (vector<TCA>::const_iterator t = transls.begin(); t != transls.end();
         ++t) {
      out << indent << "// Translations: " << transls.size() << "\n";
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
  for (SrcDB::const_iterator entry = m_srcDB.begin(); entry != m_srcDB.end();
       ++entry) {
    char destName[64];
    skToName(SrcKey::fromAtomicInt(entry->first), destName);
    const vector<IncomingBranch>& ibs = entry->second->incomingBranches();
    out << indent << "// incoming branches to " << destName << "\n";
    for (vector<IncomingBranch>::const_iterator ib = ibs.begin();
         ib != ibs.end(); ++ib) {
      // Find the start of the translation that contains this branch
      const char *branchTypeToColorStr[] = {
        "black", // JMP
        "green", // JZ
        "red",   // JNZ
      };
      TransDB::const_iterator lowerTCA = m_transDB.lower_bound(ib->m_src);
      assert(lowerTCA != m_transDB.end());
      char srcName[64];
      const TransRec* transRec = this->getTransRec(lowerTCA->second);
      skToName(transRec->src, srcName);
      out << indent << srcName << " -> " << destName << "[ color = " <<
        branchTypeToColorStr[ib->m_type] << "];\n";
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
TranslatorX64::bindJmp(TCA toSmash, SrcKey destSk,
                       ServiceRequest req, bool& smashed) {
  TCA tDest = getTranslation(destSk, false, req == REQ_BIND_JMP_NO_IR);
  if (!tDest) return nullptr;
  LeaseHolder writer(s_writeLease);
  if (!writer) return tDest;
  smashed = true;
  SrcRec* sr = getSrcRec(destSk);
  if (req == REQ_BIND_ADDR) {
    sr->chainFrom(a, IncomingBranch((TCA*)toSmash));
  } else if (req == REQ_BIND_JCC) {
    sr->chainFrom(getAsmFor(toSmash),
                  IncomingBranch(IncomingBranch::JCC, toSmash));
  } else {
    sr->chainFrom(getAsmFor(toSmash), IncomingBranch(toSmash));
  }
  return tDest;
}

/*
 * When we end a tracelet with a conditional jump, emitCondJmp first emits:
 *
 *   1:         j<CC> stubJmpccFirst
 *              jmp   stubJmpccFirst
 *
 * Our "taken" argument tells us whether the branch at 1: was taken or
 * not; and therefore which of offTaken and offNotTaken to continue executing.
 * If we did take the branch, we now rewrite the code so that the branch is
 * straightened. This predicts that subsequent executions will go the same way
 * as the first execution.
 *
 *              jn<CC> stubJmpccSecond:offNotTaken
 *              nop5   ; fallthru, or jmp if there's already a translation.
 * offTaken:
 *
 * If we did not take the branch, we leave the sense of the condition
 * intact, while patching it up to go to the unexplored code:
 *
 *              j<CC> stubJmpccSecond:offTaken
 *              nop5
 * offNotTaken:
 */
TCA
TranslatorX64::bindJmpccFirst(TCA toSmash,
                              Offset offTaken, Offset offNotTaken,
                              bool taken,
                              ConditionCode cc,
                              bool& smashed) {
  const Func* f = curFunc();
  LeaseHolder writer(s_writeLease);
  if (!writer) return nullptr;
  Offset offWillExplore = taken ? offTaken : offNotTaken;
  Offset offWillDefer = taken ? offNotTaken : offTaken;
  SrcKey dest(f, offWillExplore);
  TRACE(3, "bindJmpccFirst: explored %d, will defer %d; overwriting cc%02x "
        "taken %d\n",
        offWillExplore, offWillDefer, cc, taken);

  // We want the branch to point to whichever side has not been explored
  // yet.
  if (taken) cc = ccNegate(cc);
  TCA stub =
    emitServiceReq(SRFlags::SRNone, REQ_BIND_JMPCC_SECOND, 3,
                   toSmash, uint64_t(offWillDefer), uint64_t(cc));

  Asm &as = getAsmFor(toSmash);
  // Its not clear where chainFrom should go to if as is astubs
  assert(&as != &astubs);

  // can we just directly fall through?
  // a jmp + jz takes 5 + 6 = 11 bytes
  bool fallThru = toSmash + kJmpccLen + kJmpLen == as.code.frontier &&
    !m_srcDB.find(dest);

  TCA tDest;
  tDest = getTranslation(dest, !fallThru /* align */);
  if (!tDest) {
    return 0;
  }
  smashed = true;
  assert(s_writeLease.amOwner());
  /*
   * Roll over the jcc and the jmp/fallthru. E.g., from:
   *
   *     toSmash:    jcc   <jmpccFirstStub>
   *     toSmash+6:  jmp   <jmpccFirstStub>
   *     toSmash+11: <probably the new translation == tdest>
   *
   * to:
   *
   *     toSmash:    j[n]z <jmpccSecondStub>
   *     toSmash+6:  nop5
   *     toSmash+11: newHotness
   */
  CodeCursor cg(as, toSmash);
  as.jcc(cc, stub);
  getSrcRec(dest)->chainFrom(as, IncomingBranch(as.code.frontier));
  TRACE(5, "bindJmpccFirst: overwrote with cc%02x taken %d\n", cc, taken);
  return tDest;
}

// smashes a jcc to point to a new destination
TCA
TranslatorX64::bindJmpccSecond(TCA toSmash, const Offset off,
                               ConditionCode cc, bool& smashed) {
  const Func* f = curFunc();
  SrcKey dest(f, off);
  TCA branch = getTranslation(dest, true);
  LeaseHolder writer(s_writeLease, NO_ACQUIRE);
  if (branch && writer.acquire()) {
    smashed = true;
    SrcRec* destRec = getSrcRec(dest);
    destRec->chainFrom(getAsmFor(toSmash),
                       IncomingBranch(IncomingBranch::JCC, toSmash));
  }
  return branch;
}

static void emitJmpOrJcc(X64Assembler& a, ConditionCode cc, TCA addr) {
  if (cc == CC_None) {
    a.   jmp(addr);
  } else {
    a.   jcc((ConditionCode)cc, addr);
  }
}

/*
 * emitBindJ --
 *
 *   Emit code to lazily branch (optionally on condition cc) to the
 *   srckey in next.
 *   Assumes current basic block is closed (outputs synced, etc.).
 */
void
TranslatorX64::emitBindJ(X64Assembler& _a, ConditionCode cc,
                         SrcKey dest, ServiceRequest req) {
  prepareForSmash(_a, cc == CC_None ? (int)kJmpLen : kJmpccLen);
  TCA toSmash = _a.code.frontier;
  if (&_a == &astubs) {
    emitJmpOrJcc(_a, cc, toSmash);
  }

  TCA sr = emitServiceReq(SRFlags::SRNone, req, 2,
                          toSmash, uint64_t(dest.offset()));

  if (&_a == &astubs) {
    CodeCursor cursor(_a, toSmash);
    emitJmpOrJcc(_a, cc, sr);
  } else {
    emitJmpOrJcc(_a, cc, sr);
  }
}

void
TranslatorX64::emitBindJcc(X64Assembler& _a, ConditionCode cc,
                           SrcKey dest,
                           ServiceRequest req /* = REQ_BIND_JCC */) {
  emitBindJ(_a, cc, dest, req);
}

void
TranslatorX64::emitBindJmp(X64Assembler& _a,
                           SrcKey dest,
                           ServiceRequest req /* = REQ_BIND_JMP */) {
  emitBindJ(_a, CC_None, dest, req);
}

void
TranslatorX64::emitBindJmp(SrcKey dest) {
  emitBindJmp(a, dest);
}

void
TranslatorX64::emitStringCheck(X64Assembler& _a,
                               PhysReg base, int offset) {
  // Treat KindOfString and KindOfStaticString identically; they
  // are bitwise identical. This is a port of our IS_STRING_TYPE
  // macro to assembly, and will have to change in sync with it.
  emitTestTVType(_a, KindOfStringBit, r64(base)[offset]);
}

void
TranslatorX64::emitCheckUncounted(X64Assembler& a,
                                  PhysReg       baseReg,
                                  int           offset,
                                  SrcRec&       fail) {
  emitCmpTVType(a, KindOfStaticString, r64(baseReg)[offset]);
  emitFallbackJmp(a, fail, CC_G);
}

void
TranslatorX64::emitCheckUncountedInit(X64Assembler& a,
                                      PhysReg       baseReg,
                                      int           offset,
                                      SrcRec&       fail) {
  emitTestTVType(a, KindOfUncountedInitBit, r64(baseReg)[offset]);
  emitFallbackJmp(a, fail, CC_Z);
}

void
TranslatorX64::emitTypeCheck(X64Assembler& _a, DataType dt,
                             PhysReg base, int offset,
                             SrcRec* fail /* = NULL */) {
  offset += TVOFF(m_type);
  switch (dt) {
    case KindOfAny:
    case KindOfClass:
      break;
    case KindOfUncounted:
      assert(fail);
      emitCheckUncounted(_a, base, offset, *fail);
      break;
    case KindOfUncountedInit:
      assert(fail);
      emitCheckUncountedInit(_a, base, offset, *fail);
      break;
    case BitwiseKindOfString:
    case KindOfStaticString:
      emitStringCheck(_a, base, offset);
      if (fail) {
        emitFallbackJmp(*fail, CC_Z);
      }
      break;
    default:
      assert(IS_REAL_TYPE(dt));
      emitCmpTVType(_a, dt, r64(base)[offset]);
      if (fail) {
        emitFallbackJmp(*fail);
      }
  }
}

void
TranslatorX64::checkType(X64Assembler& a,
                         const Location& l,
                         const RuntimeType& rtt,
                         SrcRec& fail) {
  // We can get invalid inputs as a side effect of reading invalid
  // items out of BBs we truncate; they don't need guards.
  if (rtt.isVagueValue() || l.isThis()) return;

  if (m_useHHIR) {
    irCheckType(a, l, rtt, fail);
    return;
  }

  PhysReg base;
  int disp = 0;
  SpaceRecorder sr("_CheckType", a);

  TRACE(1, Trace::prettyNode("Precond", DynLocation(l, rtt)) + "\n");

  locToRegDisp(l, &base, &disp);
  TRACE(2, "TypeCheck: %d(%%r%d)\n", int(disp), int(base));
  // Negative offsets from RSP are not yet allocated; they had
  // better not be inputs to the tracelet.
  assert(l.space != Location::Stack || disp >= 0);
  if (Trace::moduleEnabled(Trace::stats, 2)) {
    Stats::emitInc(a, Stats::TraceletGuard_branch);
  }
  assert(!rtt.isIter());
  emitTypeCheck(a, rtt.typeCheckValue(), base, disp, &fail);
}

void
TranslatorX64::emitFallbackJmp(SrcRec& dest, ConditionCode cc /* = CC_NZ */) {
  emitFallbackJmp(a, dest, cc);
}

void
TranslatorX64::emitFallbackJmp(Asm& as, SrcRec& dest,
                               ConditionCode cc /* = CC_NZ */) {
  prepareForSmash(as, kJmpccLen);
  dest.emitFallbackJump(as, as.code.frontier, cc);
}

void
TranslatorX64::emitFallbackUncondJmp(Asm& as, SrcRec& dest) {
  prepareForSmash(as, kJmpLen);
  dest.emitFallbackJump(as, as.code.frontier);
}

void
TranslatorX64::emitFallbackCondJmp(Asm& as, SrcRec& dest, ConditionCode cc) {
  prepareForSmash(as, kJmpccLen);
  dest.emitFallbackJump(as, as.code.frontier, cc);
}

void TranslatorX64::emitReqRetransNoIR(Asm& as, SrcKey& sk) {
  prepareForSmash(as, kJmpLen);
  TCA toSmash = as.code.frontier;
  if (&as == &astubs) {
    as.jmp(toSmash);
  }

  TCA sr = emitServiceReq(REQ_RETRANSLATE_NO_IR, 2,
                          toSmash, sk.offset());

  if (&as == &astubs) {
    CodeCursor cc(as, toSmash);
    as.jmp(sr);
  } else {
    as.jmp(sr);
  }
}

void TranslatorX64::emitRecordPunt(Asm& a, const std::string& name) {
  PhysRegSaver regs(a, kAllX64Regs);
  a.  movq (StringData::GetStaticString("hhir punts"), rdi);
  a.  movq (StringData::GetStaticString(name), rsi);
  a.  movq (1, rdx);
  a.  call ((TCA)Stats::incStatGrouped);
}

uint64_t TranslatorX64::packBitVec(const vector<bool>& bits, unsigned i) {
  uint64_t retval = 0;
  assert(i % 64 == 0);
  assert(i < bits.size());
  while (i < bits.size()) {
    retval |= bits[i] << (i % 64);
    if ((++i % 64) == 0) {
      break;
    }
  }
  return retval;
}

void
TranslatorX64::checkRefs(X64Assembler& a,
                         SrcKey sk,
                         const RefDeps& refDeps,
                         SrcRec& fail) {
  if (refDeps.size() == 0) {
    return;
  }

  /*
   * We're still between BB's, so we're not using the real register
   * allocator.
   */
  RegSet unusedRegs = kScratchCrossTraceRegs;
  DumbScratchReg rNumParams(unusedRegs);
  DumbScratchReg rMask(unusedRegs);
  DumbScratchReg rBits(unusedRegs);
  DumbScratchReg rExpectedBits(unusedRegs);
  DumbScratchReg rBitsValue(unusedRegs);
  DumbScratchReg rFunc(unusedRegs);

  // Set up guards for each pushed ActRec that we've made reffiness
  // assumptions about
  for (RefDeps::ArMap::const_iterator it = refDeps.m_arMap.begin();
       it != refDeps.m_arMap.end(); ++it) {
    // Be careful! The actual Func might have fewer refs than the number
    // of args we're passing. To forestall this, we're going to have to
    // keep checking i against the number of params. We consider invocations
    // with too many arguments to have passed their checks.
    int entryArDelta = it->first;

    if (m_useHHIR) {
      m_hhbcTrans->guardRefs(entryArDelta,
                             it->second.m_mask,
                             it->second.m_vals);
      continue;
    }

    int32_t funcOff = cellsToBytes(entryArDelta) + AROFF(m_func);
    a.    load_reg64_disp_reg64(rVmSp, funcOff, r(rFunc)); // rFunc <- Func*
    a.    load_reg64_disp_reg32(r(rFunc), Func::numParamsOff(),
                                r(rNumParams));
    a.    load_reg64_disp_reg64(r(rFunc), Func::refBitVecOff(),
                                r(rBits));  // rBits <- m_refBitVec

    for (unsigned i = 0; i < it->second.m_mask.size(); i += 64) {
      assert(i < it->second.m_vals.size());
      uint64_t mask = packBitVec(it->second.m_mask, i);
      if (mask == 0) {
        continue;
      }
      uint64_t value = packBitVec(it->second.m_vals, i);

      emitImmReg(a, mask,  r(rMask));
      emitImmReg(a, value, r(rExpectedBits));

      /*
       * Before trying to load this block off the bit vector, make
       * sure it actually exists.  It's ok to index past numArgs
       * within one of these words, because the remaining bits will be
       * set to zero (or one in the case of the variadic by ref
       * builtins).
       */
      if (Trace::moduleEnabled(Trace::stats, 2)) {
        Stats::emitInc(a, Stats::TraceletGuard_branch);
      }
      a.  cmp_imm32_reg32(i + 1, r(rNumParams));
      {
        IfElseBlock<CC_L> ifFewEnoughArgs(a);

        // Load the appropriate qword off of the top actRec's func*.
        SKTRACE(2, sk, "reffiness mask %" PRIx64 " value %" PRIx64 ", ar @%d\n",
                mask, value, entryArDelta);
        a.  load_reg64_disp_reg64(r(rBits), sizeof(uint64_t) * (i / 64),
                                  r(rBitsValue));  // rBitsValue <- rBits[i / 64]
        a.  and_reg64_reg64(r(rMask), r(rBitsValue)); // rBitsValue &= rMask
        a.  cmp_reg64_reg64(r(rBitsValue), r(rExpectedBits));
        emitFallbackJmp(fail);

        ifFewEnoughArgs.Else();

        static_assert(AttrVariadicByRef == (1 << 15),
                      "AttrVariadicByRef assumed to be 1 << 15 in translator");
        uint8_t mask = (1u << (15 % CHAR_BIT));
        int offset = Func::attrsOff() + 15 / CHAR_BIT;
        a.  testb((int8_t)mask, r(rFunc)[offset]);
        {
          IfElseBlock<CC_NZ> ifNotWeirdBuiltin(a);

          // Other than these builtins, we need to have all by value
          // args in this case.
          prepareForTestAndSmash(kTestRegRegLen, kAlignJccImmediate);
          a.  test_reg64_reg64(r(rExpectedBits), r(rExpectedBits));
          emitFallbackJmp(fail);

          ifNotWeirdBuiltin.Else();

          // If it is one of the weird builtins that has reffiness for
          // additional args, we have to make sure our expectation is
          // that these additional args are by ref.
          a.  cmp_imm32_reg64((signed int)(-1ull & mask), r(rExpectedBits));
          emitFallbackJmp(fail);
        }
      }
    }
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
  astubs.   lea  (rVmSp[-arBase], serviceReqArgRegs[0]);
  astubs.   movq (rVmFp, serviceReqArgRegs[1]);
  (void) emitServiceReq(SRFlags(SRInline | SRJmpInsteadOfRet),
                        REQ_POST_INTERP_RET, 0ull);
  return stub;
}

/*
 * Same as above, except has different logic for fetching the AR we are trying
 * to return from, because generators have ARs in different places.
 */
TCA
TranslatorX64::emitRetFromInterpretedGeneratorFrame() {
  // We have to get the Continuation object from the current AR's $this, then
  // find where its embedded AR is.
  moveToAlign(astubs);
  TCA stub = astubs.code.frontier;

  PhysReg rContAR = serviceReqArgRegs[0];
  astubs.    loadq (rVmFp[AROFF(m_this)], rContAR);
  astubs.    loadq (rContAR[CONTOFF(m_arPtr)], rContAR);
  astubs.    movq  (rVmFp, serviceReqArgRegs[1]);
  (void) emitServiceReq(SRFlags(SRInline | SRJmpInsteadOfRet),
                        REQ_POST_INTERP_RET, 0ull);
  return stub;
}

class FreeRequestStubTrigger : public Treadmill::WorkItem {
  TCA m_stub;
 public:
  FreeRequestStubTrigger(TCA stub) : m_stub(stub) {
    TRACE(3, "FreeStubTrigger @ %p, stub %p\n", this, m_stub);
  }
  virtual void operator()() {
    TRACE(3, "FreeStubTrigger: Firing @ %p , stub %p\n", this, m_stub);
    if (TranslatorX64::Get()->freeRequestStub(m_stub) != true) {
    /* If we can't free the stub, enqueue again to retry */
      enqueue(new FreeRequestStubTrigger(m_stub));
    }
  }
};

#ifdef DEBUG

struct DepthGuard {
  static __thread int m_depth;
  DepthGuard()  { m_depth++; TRACE(2, "DepthGuard: %d {\n", m_depth); }
  ~DepthGuard() { TRACE(2, "DepthGuard: %d }\n", m_depth); m_depth--; }

  bool depthOne() const { return m_depth == 1; }
};
__thread int DepthGuard::m_depth;

#else

struct DepthGuard { bool depthOne() const { return false; } };

#endif

/*
 * enterTCHelper does not save callee-saved registers except %rbp. This means
 * when we call it from C++, we have to tell gcc to clobber all the other
 * callee-saved registers.
 */
#if defined(__x86_64__)
#  define CALLEE_SAVED_BARRIER() \
  asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15")
#elif defined(__AARCH64EL__)
#  define CALLEE_SAVED_BARRIER() \
  asm volatile("" : : : "x19", "x20", "x21", "x22", "x23", "x24", "x25", \
               "x26", "x27", "x28")
#else
#  error What are the callee-saved registers on your system?
#endif

/*
 * enterTCHelper is a handwritten assembly function that transfers control in
 * and out of the TC.
 */
static_assert(rVmSp == rbx &&
              rVmFp == rbp &&
              rVmTl == r12 &&
              rStashedAR == r15,
              "__enterTCHelper needs to be modified to use the correct ABI");
static_assert(kReservedRSPScratchSpace == 0x100,
              "enterTCHelper needs to be updated for changes to "
              "kReservedRSPScratchSpace");
static_assert(REQ_BIND_CALL == 0x1,
              "Update assembly test for REQ_BIND_CALL in __enterTCHelper");
extern "C" void enterTCHelper(Cell* vm_sp,
                              Cell* vm_fp,
                              TCA start,
                              TReqInfo* infoPtr,
                              ActRec* firstAR,
                              void* targetCacheBase);


struct TReqInfo {
  uintptr_t requestNum;
  uintptr_t args[5];

  // Some TC registers need to be preserved across service requests.
  uintptr_t saved_rStashedAr;

  // Stub addresses are passed back to allow us to recycle used stubs.
  TCA stubAddr;
};


void
TranslatorX64::enterTC(SrcKey sk, TCA start) {
  using namespace TargetCache;

  if (debug) {
    fflush(stdout);
    fflush(stderr);
  }
  DepthGuard d;
  TReqInfo info;
  info.requestNum = -1;
  info.saved_rStashedAr = 0;
  if (UNLIKELY(!start)) start = getTranslation(sk, true);
  for (;;) {
    assert(sizeof(Cell) == 16);
    assert(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
    assert(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);

    s_writeLease.gremlinUnlock();
    // Keep dispatching until we end up somewhere the translator
    // recognizes, or we luck out and the leaseholder exits.
    while (!start) {
      TRACE(2, "enterTC forwarding BB to interpreter\n");
      g_vmContext->m_pc = curUnit()->at(sk.offset());
      INC_TPC(interp_bb);
      g_vmContext->dispatchBB();
      PC newPc = g_vmContext->getPC();
      if (!newPc) { g_vmContext->m_fp = 0; return; }
      sk = SrcKey(curFunc(), newPc);
      start = getTranslation(sk, true);
    }
    assert(start == (TCA)HPHP::VM::Transl::funcBodyHelperThunk ||
           isValidCodeAddress(start));
    assert(!s_writeLease.amOwner());
    curFunc()->validate();
    INC_TPC(enter_tc);

    TRACE(1, "enterTC: %p fp%p(%s) sp%p enter {\n", start,
          vmfp(), ((ActRec*)vmfp())->m_func->name()->data(), vmsp());
    tl_regState = REGSTATE_DIRTY;

    // We have to force C++ to spill anything that might be in a callee-saved
    // register (aside from rbp). enterTCHelper does not save them.
    CALLEE_SAVED_BARRIER();
    enterTCHelper(vmsp(), vmfp(), start, &info, vmFirstAR(),
                  tl_targetCaches);
    CALLEE_SAVED_BARRIER();
    assert(g_vmContext->m_stack.isValidAddress((uintptr_t)vmsp()));

    tl_regState = REGSTATE_CLEAN; // Careful: pc isn't sync'ed yet.
    TRACE(1, "enterTC: %p fp%p sp%p } return\n", start,
          vmfp(), vmsp());

    if (debug) {
      // Debugging code: cede the write lease half the time.
      if (RuntimeOption::EvalJitStressLease) {
        if (d.depthOne() == 1 && (rand() % 2) == 0) {
          s_writeLease.gremlinLock();
        }
      }
      // Ensure that each case either returns, or drives start to a valid
      // value.
      start = TCA(0xbee5face);
    }

    TRACE(4, "enterTC: request(%s) args: %" PRIx64 " %" PRIx64 " %" PRIx64 " %" PRIx64 " %" PRIx64 "\n",
          reqName(info.requestNum),
          info.args[0], info.args[1], info.args[2], info.args[3],
          info.args[4]);

    if (LIKELY(info.requestNum == REQ_EXIT)) {
      vmfp() = nullptr;
      return;
    }
    if (!handleServiceRequest(info, start, sk)) return;
  }
}

/*
 * The contract is that each case will set sk to the place where
 * execution should resume, and optionally set start to the hardware
 * translation of the resumption point (or otherwise set it to null).
 * Returns false if we need to halt this nesting of the VM.
 *
 * start and sk might be subtly different; i.e., there are cases where
 * start != NULL && start != getTranslation(sk). For instance,
 * REQ_BIND_CALL has not finished executing the OpCall when it gets
 * here, and has even done some work on its behalf. sk == OpFCall,
 * while start == the point in the TC that's "half-way through" the
 * Call instruction. If we punt to the interpreter, the interpreter
 * will redo some of the work that the translator has already done.
 */
bool TranslatorX64::handleServiceRequest(TReqInfo& info,
                                         TCA& start,
                                         SrcKey& sk) {
  const uintptr_t& requestNum = info.requestNum;
  auto* const args = info.args;
  assert(requestNum != REQ_EXIT);
  INC_TPC(service_req);

  bool smashed = false;
  switch (requestNum) {
  case REQ_BIND_CALL: {
    ReqBindCall* req = (ReqBindCall*)args[0];
    ActRec* calleeFrame = (ActRec*)args[1];
    TCA toSmash = req->m_toSmash;
    Func *func = const_cast<Func*>(calleeFrame->m_func);
    int nArgs = req->m_nArgs;
    bool isImmutable = req->m_isImmutable;
    TCA dest = tx64->funcPrologue(func, nArgs);
    TRACE(2, "enterTC: bindCall %s -> %p\n", func->name()->data(), dest);
    if (!isImmutable) {
      // We dont know we're calling the right function, so adjust
      // dest to point to the dynamic check of ar->m_func.
      dest = funcPrologToGuard(dest, func);
    } else {
      TRACE(2, "enterTC: bindCall immutably %s -> %p\n",
            func->fullName()->data(), dest);
    }
    LeaseHolder writer(s_writeLease, NO_ACQUIRE);
    if (dest && writer.acquire()) {
      TRACE(2, "enterTC: bindCall smash %p -> %p\n", toSmash, dest);
      smashCall(tx64->getAsmFor(toSmash), toSmash, dest);
      smashed = true;
      // sk: stale, but doesn't matter since we have a valid dest TCA.
    } else {
      // We need translator help; we're not at the callee yet, so
      // roll back. The prelude has done some work already, but it
      // should be safe to redo.
      TRACE(2, "enterTC: bindCall rollback smash %p -> %p\n",
            toSmash, dest);
      sk = req->m_sourceInstr;
    }
    start = dest;
    if (!start) {
      // EnterTCHelper pushes the return ip onto the stack when the
      // requestNum is REQ_BIND_CALL, but if start is NULL, it will
      // interpret in doFCall, so we clear out the requestNum in this
      // case to prevent enterTCHelper from pushing the return ip
      // onto the stack.
      info.requestNum = ~REQ_BIND_CALL;
    }
  } break;

  case REQ_BIND_SIDE_EXIT:
  case REQ_BIND_JMP:
  case REQ_BIND_JCC:
  case REQ_BIND_JMP_NO_IR:
  case REQ_BIND_ADDR: {
    TCA toSmash = (TCA)args[0];
    Offset off = args[1];
    sk = SrcKey(curFunc(), off);
    if (requestNum == REQ_BIND_SIDE_EXIT) {
      SKTRACE(3, sk, "side exit taken!\n");
    }
    start = bindJmp(toSmash, sk, (ServiceRequest)requestNum, smashed);
  } break;

  case REQ_BIND_JMPCC_FIRST: {
    TCA toSmash = (TCA)args[0];
    Offset offTaken = (Offset)args[1];
    Offset offNotTaken = (Offset)args[2];
    ConditionCode cc = ConditionCode(args[3]);
    bool taken = int64_t(args[4]) & 1;
    start = bindJmpccFirst(toSmash, offTaken, offNotTaken,
                           taken, cc, smashed);
    // SrcKey: we basically need to emulate the fail
    sk = SrcKey(curFunc(), taken ? offTaken : offNotTaken);
  } break;

  case REQ_BIND_JMPCC_SECOND: {
    TCA toSmash = (TCA)args[0];
    Offset off = (Offset)args[1];
    ConditionCode cc = ConditionCode(args[2]);
    start = bindJmpccSecond(toSmash, off, cc, smashed);
    sk = SrcKey(curFunc(), off);
  } break;

  case REQ_BIND_REQUIRE: {
    ReqLitStaticArgs* rlsa = (ReqLitStaticArgs*)args[0];
    sk = SrcKey((Func*)args[1], (Offset)args[2]);
    start = getTranslation(sk, true);
    if (start) {
      LeaseHolder writer(s_writeLease);
      if (writer) {
        smashed = true;
        SrcRec* sr = getSrcRec(sk);
        sr->chainFrom(a, IncomingBranch(&rlsa->m_pseudoMain));
      }
    }
  } break;

  case REQ_RETRANSLATE_NO_IR: {
    TCA toSmash = (TCA)args[0];
    sk = SrcKey(curFunc(), (Offset)args[1]);
    start = retranslateAndPatchNoIR(sk, true, toSmash);
    SKTRACE(1, sk, "retranslated (without IR) @%p\n", start);
  } break;

  case REQ_RETRANSLATE: {
    INC_TPC(retranslate);
    sk = SrcKey(curFunc(), (Offset)args[0]);
    start = retranslate(sk, true, true);
    SKTRACE(2, sk, "retranslated @%p\n", start);
  } break;

  case REQ_INTERPRET: {
    Offset off = args[0];
    int numInstrs = args[1];
    g_vmContext->m_pc = curUnit()->at(off);
    /*
     * We know the compilation unit has not changed; basic blocks do
     * not span files. I claim even exceptions do not violate this
     * axiom.
     */
    assert(numInstrs >= 0);
    ONTRACE(5, SrcKey(curFunc(), off).trace("interp: enter\n"));
    if (numInstrs) {
      s_perfCounters[tpc_interp_instr] += numInstrs;
      g_vmContext->dispatchN(numInstrs);
    } else {
      // numInstrs == 0 means it wants to dispatch until BB ends
      INC_TPC(interp_bb);
      g_vmContext->dispatchBB();
    }
    PC newPc = g_vmContext->getPC();
    if (!newPc) { g_vmContext->m_fp = 0; return false; }
    SrcKey newSk(curFunc(), newPc);
    SKTRACE(5, newSk, "interp: exit\n");
    sk = newSk;
    start = getTranslation(newSk, true);
  } break;

  case REQ_POST_INTERP_RET: {
    // This is only responsible for the control-flow aspect of the Ret:
    // getting to the destination's translation, if any.
    ActRec* ar = (ActRec*)args[0];
    ActRec* caller = (ActRec*)args[1];
    assert((Cell*) caller == vmfp());
    Unit* destUnit = caller->m_func->unit();
    // Set PC so logging code in getTranslation doesn't get confused.
    vmpc() = destUnit->at(caller->m_func->base() + ar->m_soff);
    SrcKey dest(caller->m_func, vmpc());
    sk = dest;
    start = getTranslation(dest, true);
    TRACE(3, "REQ_POST_INTERP_RET: from %s to %s\n",
          ar->m_func->fullName()->data(),
          caller->m_func->fullName()->data());
  } break;

  case REQ_RESUME: {
    SrcKey dest(curFunc(), vmpc());
    sk = dest;
    start = getTranslation(dest, true);
  } break;

  case REQ_STACK_OVERFLOW: {
    /*
     * we need to construct the pc of the fcall from the return
     * address (which will be after the fcall). Because fcall is
     * a variable length instruction, and because we sometimes
     * delete instructions from the instruction stream, we
     * need to use fpi regions to find the fcall.
     */
    const FPIEnt* fe = curFunc()->findPrecedingFPI(
      curUnit()->offsetOf(vmpc()));
    vmpc() = curUnit()->at(fe->m_fcallOff);
    assert(isFCallStar(*vmpc()));
    raise_error("Stack overflow");
    NOT_REACHED();
  }
  }

  if (smashed && info.stubAddr) {
    Treadmill::WorkItem::enqueue(new FreeRequestStubTrigger(info.stubAddr));
  }

  return true;
}

TCA FreeStubList::maybePop() {
  StubNode* ret = m_list;
  if (ret) {
    m_list = ret->m_next;
    ret->m_freed = ~kStubFree;
  }
  return (TCA)ret;
}

void FreeStubList::push(TCA stub) {
  /* A freed stub may be released by Treadmill more than
   * once if multiple threads execute the service request before it is
   * freed. We detect duplicates by marking freed stubs */
  StubNode* n = (StubNode *)stub;
  if (n->m_freed == kStubFree) return;
  n->m_freed = kStubFree;
  n->m_next = m_list;
  m_list = n;
}

bool
TranslatorX64::freeRequestStub(TCA stub) {
  LeaseHolder writer(s_writeLease);
  /* If we can't acquire the write lock, the
   * caller (FreeRequestStubTrigger) retries
   */
  if (!writer) return false;
  assert(astubs.code.isValidAddress(stub));
  m_freeStubs.push(stub);
  return true;
}

TCA
TranslatorX64::getFreeStub(bool inLine) {
  if (inLine) {
    return astubs.code.frontier;
  }
  TCA ret = m_freeStubs.maybePop();
  if (ret) {
    Stats::inc(Stats::Astubs_Reused);
    assert(m_freeStubs.m_list == 0
           || astubs.code.isValidAddress(TCA(m_freeStubs.m_list)));
  } else {
    ret = astubs.code.frontier;
    Stats::inc(Stats::Astubs_New);
  }
  return ret;
}

/*
 * RAII bookmark for temporarily rewinding a.code.frontier.
 */
class ConditionalCodeCursor {
  typedef X64Assembler Asm;
  Asm& m_a;
  TCA m_oldFrontier;
  bool m_changed;
  public:
  ConditionalCodeCursor(Asm& a, TCA newFrontier) :
    m_a(a), m_oldFrontier(a.code.frontier) {
      m_a.code.frontier = newFrontier;
      m_changed = (newFrontier != m_oldFrontier);
      TRACE_MOD(Trace::trans, 1, "RewindTo: %p (from %p)\n",
                m_a.code.frontier, m_oldFrontier);
    }
  ~ConditionalCodeCursor() {
    if (m_changed) {
      m_a.code.frontier = m_oldFrontier;
    }
    TRACE_MOD(Trace::trans, 1, "Restore: %p\n",
              m_a.code.frontier);
  }
};

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
TranslatorX64::emitServiceReqVA(SRFlags flags, ServiceRequest req, int numArgs,
                                va_list args) {
  bool emitInA = bool(flags & SRFlags::SREmitInA);
  bool align   = bool(flags & SRFlags::SRAlign) && !emitInA;
  bool inLine  = bool(flags & SRFlags::SRInline);
  Asm&   as = emitInA ? a : astubs;
  TCA start = emitInA ? a.code.frontier : getFreeStub(inLine);
  ConditionalCodeCursor cg(as, start);
  /* max space for moving to align, saving VM regs plus emitting args */
  static const int kVMRegSpace = 0x14;
  static const int kMovSize = 0xa;
  static const int kNumServiceRegs = sizeof(serviceReqArgRegs)/sizeof(PhysReg);
  static const int kMaxStubSpace = kJmpTargetAlign - 1
                              + kVMRegSpace
                              + kNumServiceRegs * kMovSize;
  if (align) {
    moveToAlign(as);
  }
  TCA retval = as.code.frontier;
  emitEagerVMRegSave(as, SaveFP);
  /*
   * Move args into appropriate regs.
   */
  TRACE(3, "Emit Service Req %s(", reqName(req));
  for (int i = 0; i < numArgs; i++) {
    uint64_t argVal = va_arg(args, uint64_t);
    TRACE(3, "%p,", (void*)argVal);
    emitImmReg(as, argVal, serviceReqArgRegs[i]);
  }
  if (!inLine) {
    /* make sure that the stub has enough space that it can be
     * reused for other service requests, with different number of
     * arguments, alignment, etc.
     */
    as.emitNop(start + kMaxStubSpace - as.code.frontier);
    emitImmReg(as, (uint64_t)start, rScratch);
  } else {
    emitImmReg(as, 0, rScratch);
  }
  TRACE(3, ")\n");
  emitImmReg(as, req, rdi);
  /*
   * Weird hand-shaking with enterTC: reverse-call a service routine.
   *
   * In the case of some special stubs (m_callToExit, m_retHelper), we
   * have already unbalanced the return stack by doing a ret to
   * something other than enterTCHelper.  In that case
   * SRJmpInsteadOfRet indicates to fake the return.
   */
  if (flags & SRFlags::SRJmpInsteadOfRet) {
    as.pop(rax);
    as.jmp(rax);
  } else {
    as.ret();
  }
  recordBCInstr(OpServiceRequest, as, retval);
  translator_not_reached(as);
  return retval;
}

TCA
TranslatorX64::emitServiceReq(ServiceRequest req, int numArgs, ...) {
  va_list args;
  va_start(args, numArgs);
  TCA retval = emitServiceReqVA(SRFlags::SRAlign, req, numArgs, args);
  va_end(args);
  return retval;
}

TCA
TranslatorX64::emitServiceReq(SRFlags flags, ServiceRequest req,
                              int numArgs, ...) {
  va_list args;
  va_start(args, numArgs);
  TCA retval = emitServiceReqVA(flags, req, numArgs, args);
  va_end(args);
  return retval;
}

TCA
TranslatorX64::emitTransCounterInc(X64Assembler& a) {
  TCA start = a.code.frontier;
  if (!isTransDBEnabled()) return start;

  a.    movq (getTransCounterAddr(), rScratch);
  a.    lock ();
  a.    incq (*rScratch);

  return start;
}

void
TranslatorX64::spillTo(DataType type, PhysReg reg, bool writeType,
                       PhysReg base, int disp) {
  X64Assembler& a = *m_spillFillCode;
  // Zero out the count at the same time as writing the type.
  SpaceRecorder sr("_Spill", a);

  Stats::emitInc(a, Stats::Tx64_Spill);
  emitStoreTypedValue(a, type, reg, disp, base, writeType);
}

void
TranslatorX64::spill(const Location& loc, DataType type,
                     PhysReg reg, bool writeType) {
  PhysReg base;
  int disp;
  locToRegDisp(loc, &base, &disp);
  spillTo(type, reg, writeType, base, disp);
  TRACE(2, "%s: (%s, %" PRId64 ") -> v: %d(r%d) type%d\n",
        __func__,
        loc.spaceName(), loc.offset, int(disp + TVOFF(m_data)),
        int(base), type);
}

void
TranslatorX64::fill(const Location& loc, PhysReg reg) {
  SpaceRecorder sr("_Fill", *m_spillFillCode);
  if (loc.isThis()) {
    m_spillFillCode->load_reg64_disp_reg64(rVmFp, AROFF(m_this), reg);
    return;
  }
  PhysReg base;
  int disp;
  locToRegDisp(loc, &base, &disp);
  TRACE(2, "fill: (%s, %" PRId64 ") -> reg %d\n",
        loc.spaceName(), loc.offset, int(reg));
  m_spillFillCode->load_reg64_disp_reg64(base, disp + TVOFF(m_data), reg);
}

void TranslatorX64::fillByMov(PhysReg src, PhysReg dst) {
  SpaceRecorder sr("_FillMov", *m_spillFillCode);
  assert(src != dst);
  m_spillFillCode->mov_reg64_reg64(src, dst);
}

void
TranslatorX64::loadImm(int64_t immVal, PhysReg reg) {
  SpaceRecorder sr("_FillImm", *m_spillFillCode);
  TRACE(2, "loadImm: 0x%" PRIx64 " -> reg %d\n", immVal, int(reg));
  emitImmReg(*m_spillFillCode, immVal, reg);
}

void
TranslatorX64::poison(PhysReg dest) {
  static const bool poison = false;
  if (poison) {
    emitImmReg(*m_spillFillCode, 0xbadf00d105e5babe, dest);
  }
}

/**
 * Spill all dirty registers, mark all registers as 'free' in the
 * register file, and update rVmSp to point to the top of stack at
 * the end of the tracelet.
 */
void
TranslatorX64::syncOutputs(const Tracelet& t) {
  syncOutputs(t.m_stackChange);
}

/**
 * Same as above, except that it sets rVmSp to point to the top of
 * stack at the beginning of the specified instruction.
 */
void
TranslatorX64::syncOutputs(const NormalizedInstruction& i) {
  syncOutputs(i.stackOff);
}

void
TranslatorX64::syncOutputs(int stackOff) {
  SpaceRecorder sr("_SyncOuts", *m_spillFillCode);
  TCA start = m_spillFillCode->code.frontier;
  // Mark all stack locations above the top of stack as dead
  m_regMap.scrubStackEntries(stackOff);
  // Spill all dirty registers
  m_regMap.cleanAll();
  if (stackOff != 0) {
    TRACE(1, "syncOutputs: rVmSp + %d\n", stackOff);
    // t.stackChange is in negative Cells, not bytes.
    m_spillFillCode->add_imm32_reg64(-cellsToBytes(stackOff), rVmSp);
  }
  // All registers have been smashed for realz, yo
  m_regMap.smashRegs(kAllRegs);
  recordBCInstr(OpSyncOutputs, *m_spillFillCode, start);
}

/*
 * getBinaryStackInputs --
 *
 *   Helper for a common pattern of instruction, where two items are popped
 *   and one is pushed. The second item on the stack at the beginning of
 *   the instruction is both a source and destination.
 */
static void
getBinaryStackInputs(RegAlloc& regmap, const NormalizedInstruction& i,
                     PhysReg& rsrc, PhysReg& rsrcdest) {
  assert(i.inputs.size()   == 2);
  assert(i.outStack && !i.outLocal);
  rsrcdest = regmap.getReg(i.outStack->location);
  rsrc     = regmap.getReg(i.inputs[0]->location);
  assert(regmap.getReg(i.inputs[1]->location) == rsrcdest);
}

// emitBox --
//   Leave a boxed version of input in RAX. Destroys the register
//   mapping.
void
TranslatorX64::emitBox(DataType t, PhysReg rSrc) {
  if (false) { // typecheck
    RefData* retval = tvBoxHelper(KindOfArray, 0xdeadbeef01ul);
    (void)retval;
  }
  // tvBoxHelper will set the refcount of the inner cell to 1
  // for us. Because the inner cell now holds a reference to the
  // original value, we don't need to perform a decRef.
  EMIT_CALL(a, tvBoxHelper, IMM(t), R(rSrc));
}

// emitUnboxTopOfStack --
//   Unbox the known-to-be Variant on top of stack in place.
void
TranslatorX64::emitUnboxTopOfStack(const NormalizedInstruction& i) {
  const vector<DynLocation*>& inputs  = i.inputs;

  assert(inputs.size() == 1);
  assert(i.outStack && !i.outLocal);
  assert(inputs[0]->isStack());
  assert(i.outStack && i.outStack->location == inputs[0]->location);
  DataType outType = inputs[0]->rtt.innerType();
  assert(outType != KindOfInvalid);
  assert(outType == i.outStack->outerType());
  PhysReg rSrc = getReg(inputs[0]->location);
  // Detach the register rSrc from the input location. We must
  // do this dance because the input and output location are the
  // same and we want to have separate registers for the input
  // and output.
  m_regMap.invalidate(inputs[0]->location);
  ScratchReg rSrcScratch(m_regMap, rSrc);
  // This call to allocOutputRegs will allocate a new register
  // for the output location
  m_regMap.allocOutputRegs(i);
  PhysReg rDest = getReg(i.outStack->location);
  emitDerefRef(a, rSrc, rDest);
  emitIncRef(rDest, outType);
  // decRef the var on the evaluation stack
  emitDecRef(i, rSrc, KindOfRef);
}

// setOpOpToOpcodeOp --
//   The SetOp opcode space has nothing to do with the bytecode opcode
//   space. Reasonable people like it that way, so translate them here.
static Opcode
setOpOpToOpcodeOp(SetOpOp soo) {
  switch(soo) {
#define SETOP_OP(_soo, _bc) case SetOp##_soo: return _bc;
    SETOP_OPS
#undef SETOP_OP
    default: assert(false);
  }
  return -1;
}

void
TranslatorX64::getInputsIntoXMMRegs(const NormalizedInstruction& ni,
                                    PhysReg lr, PhysReg rr,
                                    RegXMM lxmm,
                                    RegXMM rxmm) {
  const DynLocation& l = *ni.inputs[0];
  const DynLocation& r = *ni.inputs[1];
  // Get the values into their appropriate xmm locations
  auto intoXmm = [&](const DynLocation& l, PhysReg src, RegXMM xmm) {
    if (l.isInt()) {
      // cvtsi2sd doesn't modify the high bits of its target, which can
      // cause false dependencies to prevent register renaming from kicking
      // in. Break the dependency chain by zeroing out the destination reg.
      a.  pxor_xmm_xmm(xmm, xmm);
      a.  cvtsi2sd_reg64_xmm(src, xmm);
    } else {
      a.  mov_reg64_xmm(src, xmm);
    }
  };
  intoXmm(l, lr, lxmm);
  intoXmm(r, rr, rxmm);
}

void
TranslatorX64::binaryMixedArith(const NormalizedInstruction& i,
                           Opcode op,
                           PhysReg srcReg,
                           PhysReg srcDestReg) {
  getInputsIntoXMMRegs(i, srcReg, srcDestReg, xmm1, xmm0);
  switch(op) {
#define CASEIMM(OpBc, x64op)                                       \
    case OpBc:    a.  x64op ##sd_xmm_xmm(xmm1, xmm0); break
    CASEIMM(OpAdd, add);
    CASEIMM(OpSub, sub);
    CASEIMM(OpMul, mul);
#undef CASEIMM
    default: not_reached();
  }
  a.   mov_xmm_reg64(xmm0, srcDestReg);
}

void
TranslatorX64::binaryIntegerArith(const NormalizedInstruction& i,
                                  Opcode op,
                                  PhysReg srcReg,
                                  PhysReg srcDestReg) {
  switch (op) {
#define CASEIMM(OpBc, x64op)                                       \
    case OpBc: {                                                   \
      if (i.hasConstImm) {                                         \
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
  assert(in1.rtt.isInt() || in1.rtt.isDouble());
  assert(inout.rtt.isInt() || inout.rtt.isDouble());
  assert(in1.outerType() != KindOfRef);
  assert(in1.isStack());
  assert(inout.outerType() != KindOfRef);
  assert(inout.isStack());
  m_regMap.allocOutputRegs(i);
  PhysReg     srcReg = m_regMap.getReg(in1.location);
  PhysReg srcDestReg = m_regMap.getReg(inout.location);
  if (in1.rtt.isInt() && inout.rtt.isInt()) {
    binaryIntegerArith(i, op, srcReg, srcDestReg);
  } else {
    binaryMixedArith(i, op, srcReg, srcDestReg);
  }
}

void
TranslatorX64::binaryArithLocal(const NormalizedInstruction &i,
                                Opcode op,
                                const DynLocation& in1,
                                const DynLocation& in2,
                                const DynLocation& out) {
  assert(in1.rtt.isInt() || in1.rtt.isDouble());
  assert(in2.rtt.valueType() == KindOfInt64 ||
         in2.rtt.valueType() == KindOfDouble);
  assert(in1.outerType() != KindOfRef);
  assert(in1.isStack());
  assert(in2.isLocal());
  assert(out.isStack());

  PhysReg srcReg = m_regMap.getReg(in1.location);
  PhysReg outReg = m_regMap.getReg(out.location);
  PhysReg localReg = m_regMap.getReg(in2.location);
  auto emitBody = [&](PhysReg out) {
    if (in1.rtt.isDouble() || in2.rtt.isDouble()) {
      binaryMixedArith(i, op, srcReg, out);
    } else {
      binaryIntegerArith(i, op, srcReg, out);
    }
  };
  if (in2.outerType() != KindOfRef) {
    // The local is not a var, so we can operate directly on the
    // local's register. We will need to update outReg after the
    // operation.
    emitBody(localReg);
    // We operated directly on the local's register, so we need to update
    // outReg
    emitMovRegReg(a, localReg, outReg);
  } else {
    ScratchReg scr(m_regMap);
    // The local is a var, so we have to read its value into outReg
    // on operate on that. We will need to write the result back
    // to the local after the operation.
    emitDerefRef(a, localReg, r(scr));
    emitBody(r(scr));
    // We operated on outReg, so we need to write the result back to the
    // local
    emitMovRegReg(a, r(scr), outReg);
    a.    storeq (r(scr), localReg[RefData::tvOffset() + TVOFF(m_data)]);
  }
}

#define O(opcode, imm, pusph, pop, flags) \
/**
 * The interpOne methods saves m_pc, m_fp, and m_sp ExecutionContext,
 * calls into the interpreter, and then return a pointer to the
 * current ExecutionContext.
 */  \
VMExecutionContext*                                                     \
interpOne##opcode(ActRec* ar, Cell* sp, Offset pcOff) {                 \
  interp_set_regs(ar, sp, pcOff);                                       \
  SKTRACE(5, SrcKey(curFunc(), vmpc()), "%40s %p %p\n",                 \
          "interpOne" #opcode " before (fp,sp)",                        \
          vmfp(), vmsp());                                              \
  assert(*vmpc() == Op ## opcode);                                      \
  VMExecutionContext* ec = g_vmContext;                                 \
  Stats::inc(Stats::Instr_InterpOne ## opcode);                         \
  INC_TPC(interp_one)                                                   \
  /* Correct for over-counting in TC-stats. */                          \
  Stats::inc(Stats::Instr_TC, -1);                                      \
  ec->op##opcode();                                                     \
  /*
   * Only set regstate back to dirty if an exception is not
   * propagating.  If an exception is throwing, regstate for this call
   * is actually still correct, and we don't have information in the
   * fixup map for interpOne calls anyway.
   */ \
  tl_regState = REGSTATE_DIRTY;                                         \
  return ec;                                                            \
}

OPCODES
#undef O

void* interpOneEntryPoints[] = {
#define O(opcode, imm, pusph, pop, flags) \
  (void*)(interpOne ## opcode),
OPCODES
#undef O
};

void TranslatorX64::fixupWork(VMExecutionContext* ec,
                              ActRec* rbp) const {
  assert(RuntimeOption::EvalJit);

  TRACE_SET_MOD(fixup);
  TRACE(1, "fixup(begin):\n");

  auto isVMFrame = [] (ActRec* ar) {
    assert(ar);
    bool ret = uintptr_t(ar) - Util::s_stackLimit >= Util::s_stackSize;
    assert(!ret ||
           (ar >= g_vmContext->m_stack.getStackLowAddress() &&
            ar < g_vmContext->m_stack.getStackHighAddress()) ||
           ar->m_func->isGenerator());
    return ret;
  };

  auto* nextRbp = rbp;
  rbp = 0;
  do {
    auto* prevRbp = rbp;
    rbp = nextRbp;
    assert(rbp && "Missing fixup for native call");
    nextRbp = reinterpret_cast<ActRec*>(rbp->m_savedRbp);
    TRACE(2, "considering frame %p, %p\n", rbp, (void*)rbp->m_savedRip);

    if (isVMFrame(nextRbp)) {
      TRACE(2, "fixup checking vm frame %s\n",
               nextRbp->m_func->name()->data());
      FixupMap::VMRegs regs;
      if (m_fixupMap.getFrameRegs(rbp, prevRbp, &regs)) {
        TRACE(2, "fixup(end): func %s fp %p sp %p pc %p\n",
              regs.m_fp->m_func->name()->data(),
              regs.m_fp, regs.m_sp, regs.m_pc);
        ec->m_fp = const_cast<ActRec*>(regs.m_fp);
        ec->m_pc = regs.m_pc;
        vmsp() = regs.m_sp;
        return;
      }
    }
  } while (rbp && rbp != nextRbp);

  // OK, we've exhausted the entire actRec chain.  We are only
  // invoking ::fixup() from contexts that were known to be called out
  // of the TC, so this cannot happen.
  NOT_REACHED();
}

void TranslatorX64::fixup(VMExecutionContext* ec) const {
  // Start looking for fixup entries at the current (C++) frame.  This
  // will walk the frames upward until we find a TC frame.
  DECLARE_FRAME_POINTER(framePtr);
  fixupWork(ec, framePtr);
}

TCA TranslatorX64::getTranslatedCaller() const {
  DECLARE_FRAME_POINTER(fp);
  ActRec* framePtr = fp;  // can't directly mutate the register-mapped one
  for (; framePtr; framePtr = (ActRec*)framePtr->m_savedRbp) {
    TCA rip = (TCA)framePtr->m_savedRip;
    if (isValidCodeAddress(rip)) {
      return rip;
    }
  }
  return nullptr;
}

void
TranslatorX64::syncWork() {
  assert(tl_regState == REGSTATE_DIRTY);
  fixup(g_vmContext);
  tl_regState = REGSTATE_CLEAN;
  Stats::inc(Stats::TC_Sync);
}

void
TranslatorX64::emitInterpOne(const Tracelet& t,
                             const NormalizedInstruction& ni) {
  // Write any dirty values to memory
  m_regMap.cleanAll();
  // Call into the appropriate interpOne method. Note that this call will
  // preserve the callee-saved registers including rVmFp and rVmSp.
  if (false) { /* typecheck */
    UNUSED VMExecutionContext* ec = interpOnePopC((ActRec*)vmfp(), vmsp(), 0);
  }
  void* func = interpOneEntryPoints[ni.op()];
  TRACE(3, "ip %p of unit %p -> interpOne @%p\n", ni.pc(), ni.unit(), func);
  EMIT_CALL(a, func,
             R(rVmFp),
             RPLUS(rVmSp, -int32_t(cellsToBytes(ni.stackOff))),
             IMM(ni.source.offset()));
  // The interpreter may have written to memory, so we need to invalidate
  // all locations
  m_regMap.reset();
  // The interpOne method returned a pointer to the current
  // ExecutionContext in rax, so we can read the 'm_*' fields
  // by adding the appropriate offset to rax and dereferencing.

  // If this instruction ends the tracelet, we have some extra work to do.
  if (ni.breaksTracelet) {
    // Read the 'm_fp' and 'm_stack.m_top' fields into the rVmFp and
    // rVmSp registers.
    a.  load_reg64_disp_reg64(rax, offsetof(VMExecutionContext, m_fp),
                              rVmFp);
    a.  load_reg64_disp_reg64(rax, offsetof(VMExecutionContext, m_stack) +
                              Stack::topOfStackOffset(), rVmSp);
    if (opcodeChangesPC(ni.op())) {
      // If interpreting this instruction can potentially set PC to point
      // to something other than the next instruction in the bytecode, so
      // we need to emit a service request to figure out where to go next
      TCA stubDest = emitServiceReq(REQ_RESUME, 0ull);
      a.    jmp(stubDest);
    } else {
      // If this instruction always advances PC to the next instruction in
      // the bytecode, then we know what SrcKey to bind to
      emitBindJmp(nextSrcKey(t, ni));
    }
  }
}

// could be static but used in hopt/codegen.cpp
void raiseUndefVariable(StringData* nm) {
  raise_notice(Strings::UNDEFINED_VARIABLE, nm->data());
  // FIXME: do we need to decref the string if an exception is propagating?
  decRefStr(nm);
}

// This intentionally excludes Int/Int, which is handled separately
// from cases involving the FPU.
bool
mathEquivTypes(RuntimeType lt, RuntimeType rt) {
  return (lt.isDouble() && rt.isDouble()) ||
   (lt.isInt() && rt.isDouble()) ||
   (lt.isDouble() && rt.isInt());
}

static TXFlags
planBinaryArithOp(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  if (mathEquivTypes(i.inputs[0]->rtt, i.inputs[1]->rtt)) {
    auto op = i.op();
    return nativePlan(op == OpMul || op == OpAdd || op == OpSub);
  }
  return nativePlan(i.inputs[0]->isInt() && i.inputs[1]->isInt());
}

void
TranslatorX64::analyzeBinaryArithOp(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = planBinaryArithOp(i);
}

void
TranslatorX64::translateBinaryArithOp(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpSub || op == OpMul || op == OpBitAnd ||
         op == OpBitOr || op == OpBitXor);
  assert(planBinaryArithOp(i));
  assert(i.inputs.size() == 2);

  binaryArithCell(i, op, *i.inputs[0], *i.outStack);
}

static inline bool sameDataTypes(DataType t1, DataType t2) {
  return TypeConstraint::equivDataTypes(t1, t2);
}

static TXFlags
planSameOp_SameTypes(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  const RuntimeType& left = i.inputs[0]->rtt;
  const RuntimeType& right = i.inputs[1]->rtt;
  DataType leftType = left.outerType();
  DataType rightType = right.outerType();
  return nativePlan(sameDataTypes(leftType, rightType) &&
                    (left.isNull() || leftType == KindOfBoolean ||
                     left.isInt() || left.isString()));
}

static TXFlags
planSameOp_DifferentTypes(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  DataType leftType = i.inputs[0]->outerType();
  DataType rightType = i.inputs[1]->outerType();
  if (!sameDataTypes(leftType, rightType)) {
    if (IS_REFCOUNTED_TYPE(leftType) || IS_REFCOUNTED_TYPE(rightType)) {
      // For dissimilar datatypes, we might call out to handle a refcount.
      return Supported;
    }
    return Native;
  }
  return Interp;
}

void
TranslatorX64::analyzeSameOp(Tracelet& t, NormalizedInstruction& i) {
  assert(!(planSameOp_SameTypes(i) && planSameOp_DifferentTypes(i)));
  i.m_txFlags = TXFlags(planSameOp_SameTypes(i) | planSameOp_DifferentTypes(i));
  i.manuallyAllocInputs = true;
}

void
TranslatorX64::translateSameOp(const Tracelet& t,
                               const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpSame || op == OpNSame);
  const vector<DynLocation*>& inputs  = i.inputs;
  bool instrNeg = (op == OpNSame);
  assert(inputs.size() == 2);
  assert(i.outStack && !i.outLocal);
  DataType leftType = i.inputs[0]->outerType();
  DataType rightType DEBUG_ONLY = i.inputs[1]->outerType();
  assert(leftType != KindOfRef);
  assert(rightType != KindOfRef);

  if (planSameOp_DifferentTypes(i)) {
    // Some easy cases: when the valueTypes do not match,
    // NSame -> true and Same -> false.
    SKTRACE(1, i.source, "different types %d %d\n",
            leftType, rightType);
    emitDecRefInput(a, i, 0);
    emitDecRefInput(a, i, 1);
    m_regMap.allocOutputRegs(i);
    emitImmReg(a, instrNeg, getReg(i.outStack->location));
    return; // Done
  }

  assert(planSameOp_SameTypes(i));

  if (IS_NULL_TYPE(leftType)) {
    m_regMap.allocOutputRegs(i);
    // null === null is always true
    SKTRACE(2, i.source, "straightening null/null comparison\n");
    emitImmReg(a, !instrNeg, getReg(i.outStack->location));
    return; // Done
  }
  if (IS_STRING_TYPE(leftType)) {
    int args[2];
    args[0] = 0;
    args[1] = 1;
    allocInputsForCall(i, args);
    EMIT_CALL(a, same_str_str,
               V(inputs[0]->location),
               V(inputs[1]->location));
    if (instrNeg) {
      a.  xor_imm32_reg32(1, rax);
    }
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
    return; // Done
  }
  m_regMap.allocInputRegs(i);
  PhysReg src, srcdest;
  getBinaryStackInputs(m_regMap, i, src, srcdest);
  m_regMap.allocOutputRegs(i);
  assert(getReg(i.outStack->location) == srcdest);
  a.    cmp_reg64_reg64(src, srcdest);
  if (op == OpSame) {
    a.  sete(rbyte(srcdest));
  } else {
    a.  setne(rbyte(srcdest));
  }
  a.    movzbl (rbyte(srcdest), r32(srcdest));
}

static bool
trivialEquivType(const RuntimeType& rtt) {
  DataType t = rtt.valueType();
  return t == KindOfUninit || t == KindOfNull || t == KindOfBoolean ||
    rtt.isInt() || rtt.isString();
}

static void
emitConvertToBool(X64Assembler &a, PhysReg src, PhysReg dest, bool instrNeg) {
  a.    test_reg64_reg64(src, src);
  if (instrNeg) {
    a.  setz(rbyte(dest));
  } else {
    a.  setnz(rbyte(dest));
  }
  a.    movzbl (rbyte(dest), r32(dest));
}

void
TranslatorX64::analyzeEqOp(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  RuntimeType &lt = i.inputs[0]->rtt;
  RuntimeType &rt = i.inputs[1]->rtt;
  i.m_txFlags = nativePlan(trivialEquivType(lt) &&
                           trivialEquivType(rt));
  if (!i.m_txFlags) {
    i.m_txFlags = nativePlan(mathEquivTypes(lt, rt));
  }
  if (i.isNative() &&
      IS_NULL_TYPE(lt.outerType()) &&
      IS_NULL_TYPE(rt.outerType())) {
    i.manuallyAllocInputs = true;
  }
}

void
TranslatorX64::fpEq(const NormalizedInstruction& ni,
                    PhysReg lr, PhysReg rr) {
  getInputsIntoXMMRegs(ni, lr, rr, xmm0, xmm1);
  m_regMap.allocOutputRegs(ni);
  a.      ucomisd_xmm_xmm(xmm0, xmm1);
  semiLikelyIfBlock(CC_P, a, [&] {
    // PF means unordered; treat it as !eq. Or 1 into anything at all
    // to clear ZF.
    a.    or_imm32_reg64(1, reg::rScratch);
  });
}

void
TranslatorX64::translateEqOp(const Tracelet& t,
                             const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpEq || op == OpNeq);
  assert(i.isNative());
  const vector<DynLocation*>& inputs  = i.inputs;
  bool instrNeg = (op == OpNeq);
  assert(inputs.size() == 2);
  assert(i.outStack && !i.outLocal);
  DataType leftType = i.inputs[0]->outerType();
  DataType rightType = i.inputs[1]->outerType();
  assert(leftType != KindOfRef);
  assert(rightType != KindOfRef);

  ConditionCode ccBranch = CC_E;
  if (instrNeg) ccBranch = ccNegate(ccBranch);

  // Inputless case.
  if (IS_NULL_TYPE(leftType) && IS_NULL_TYPE(rightType)) {
    assert(i.manuallyAllocInputs);
    // null == null is always true
    bool result = !instrNeg;
    SKTRACE(2, i.source, "straightening null/null comparison\n");
    if (i.changesPC) {
      fuseBranchAfterStaticBool(a, t, i, result);
    } else {
      m_regMap.allocOutputRegs(i);
      emitImmReg(a, result, getReg(i.outStack->location));
    }
    return; // Done
  }

  if (IS_STRING_TYPE(leftType) || IS_STRING_TYPE(rightType)) {
    void* fptr = nullptr;
    bool leftIsString = false;
    bool eqNullStr = false;
    switch (leftType) {
      STRINGCASE(): {
        leftIsString = true;
        switch (rightType) {
          STRINGCASE(): fptr = (void*)eq_str_str; break;
          case KindOfInt64: fptr = (void*)eq_int_str; break;
          case KindOfBoolean: fptr = (void*)eq_bool_str; break;
          NULLCASE(): fptr = (void*)eq_null_str; eqNullStr = true; break;
          default: assert(false); break;
        }
      } break;
      case KindOfInt64: fptr = (void*)eq_int_str; break;
      case KindOfBoolean: fptr = (void*)eq_bool_str; break;
      NULLCASE(): fptr = (void*)eq_null_str; eqNullStr = true; break;
      default: assert(false); break;
    }
    if (eqNullStr) {
      assert(fptr == (void*)eq_null_str);
      EMIT_CALL(a, fptr,
                 V(inputs[leftIsString ? 0 : 1]->location));
    } else {
      assert(fptr != nullptr);
      EMIT_CALL(a, fptr,
                 V(inputs[leftIsString ? 1 : 0]->location),
                 V(inputs[leftIsString ? 0 : 1]->location));
    }
    if (i.changesPC) {
      fuseBranchSync(t, i);
      prepareForTestAndSmash(kTestImmRegLen, kAlignJccAndJmp);
      a.   testb (1, al);
      fuseBranchAfterBool(t, i, ccNegate(ccBranch));
      return;
    }
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
    if (instrNeg) {
      a.  xor_imm32_reg32(1, rax);
    }
    return;
  }

  m_regMap.allocOutputRegs(i);
  PhysReg src, srcdest;
  getBinaryStackInputs(m_regMap, i, src, srcdest);
  assert(getReg(i.outStack->location) == srcdest);
  if (i.changesPC) {
    fuseBranchSync(t, i);
  }
  if (IS_NULL_TYPE(leftType) || IS_NULL_TYPE(rightType)) {
    prepareForTestAndSmash(kTestRegRegLen, kAlignJccAndJmp);
    if (IS_NULL_TYPE(leftType)) {
      a.   test_reg64_reg64(srcdest, srcdest);
    } else {
      assert(IS_NULL_TYPE(rightType));
      a.   test_reg64_reg64(src, src);
    }
  } else if (leftType  == KindOfBoolean ||
             rightType == KindOfBoolean) {
    // OK to destroy src and srcdest in-place; their stack locations are
    // blown away by this instruction.
    if (leftType != KindOfBoolean)
      emitConvertToBool(a, src, src, false);
    if (rightType != KindOfBoolean)
      emitConvertToBool(a, srcdest, srcdest, false);
    a.   cmp_reg64_reg64(src, srcdest);
  } else if (leftType == KindOfDouble || rightType == KindOfDouble) {
    fpEq(i, src, srcdest);
  } else {
    a.   cmp_reg64_reg64(src, srcdest);
  }
  if (i.changesPC) {
    fuseBranchAfterBool(t, i, ccBranch);
    return;
  }
  if (instrNeg) {
    a.   setnz          (rbyte(srcdest));
  } else {
    a.   setz           (rbyte(srcdest));
  }
  a.     movzbl (rbyte(srcdest), r32(srcdest));
}

void
TranslatorX64::analyzeLtGtOp(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  const RuntimeType& left = i.inputs[0]->rtt;
  DataType leftType = left.outerType();
  DataType rightType = i.inputs[1]->outerType();
  i.m_txFlags = nativePlan(sameDataTypes(leftType, rightType) &&
                           (left.isNull() ||
                            leftType == KindOfBoolean ||
                            left.isInt()));
  if (i.isNative() && IS_NULL_TYPE(left.outerType())) {
    // No inputs. w00t.
    i.manuallyAllocInputs = true;
  }
}

void
TranslatorX64::translateLtGtOp(const Tracelet& t,
                               const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpLt || op == OpLte || op == OpGt || op == OpGte);
  assert(i.inputs.size() == 2);
  assert(i.outStack && !i.outLocal);
  assert(i.inputs[0]->outerType() != KindOfRef);
  assert(i.inputs[1]->outerType() != KindOfRef);
  assert(i.isNative());

  bool fEquals = (op == OpLte || op == OpGte);
  bool fLessThan = (op == OpLt || op == OpLte);

  m_regMap.allocOutputRegs(i);
  if (IS_NULL_TYPE(i.inputs[0]->outerType())) {
    assert(IS_NULL_TYPE(i.inputs[1]->outerType()));
    // null < null is always false, null <= null is always true
    SKTRACE(2, i.source, "straightening null/null comparison\n");
    PhysReg rOut = getReg(i.outStack->location);
    bool resultIsTrue = (op == OpLte || op == OpGte);
    if (i.changesPC) {
      fuseBranchAfterStaticBool(a, t, i, resultIsTrue);
    } else {
      emitImmReg(a, resultIsTrue, rOut);
    }
    return;
  }
  PhysReg src, srcdest;
  getBinaryStackInputs(m_regMap, i, src, srcdest);
  assert(getReg(i.outStack->location) == srcdest);
  if (i.changesPC) {
    fuseBranchSync(t, i);
  }
  a.       cmp_reg64_reg64(src, srcdest);
  static const ConditionCode opToCc[2][2] = {
    // !fEquals fEquals
    { CC_G,     CC_GE }, // !fLessThan
    { CC_L,     CC_LE }, // fLessThan
  };
  ConditionCode cc = opToCc[fLessThan][fEquals];
  if (i.changesPC) {
    // Fuse the coming branch.
    fuseBranchAfterBool(t, i, cc);
    return;
  }
  a.       setcc(cc, rbyte(srcdest));
  a.       movzbl (rbyte(srcdest), r32(srcdest));
}

static TXFlags
planUnaryBooleanOp(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  RuntimeType& rtt = i.inputs[0]->rtt;
  DataType inType = rtt.valueType();
  if (inType == KindOfArray) {
    return Supported;
  }
  if (rtt.isString()) {
    return Simple;
  }
  return nativePlan(rtt.isNull() ||
                    inType == KindOfBoolean || rtt.isInt());
}

void
TranslatorX64::analyzeUnaryBooleanOp(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = planUnaryBooleanOp(i);
}

void
TranslatorX64::translateUnaryBooleanOp(const Tracelet& t,
                                       const NormalizedInstruction& i) {
  const Opcode op = i.op();
  assert(op == OpCastBool || op == OpEmptyL);
  const vector<DynLocation*>& inputs  = i.inputs;
  assert(inputs.size() == 1);
  assert(i.outStack && !i.outLocal);
  bool instrNeg = (op == OpEmptyL);
  DataType inType = inputs[0]->valueType();
  const Location& inLoc = inputs[0]->location;
  bool boxedForm = (inputs[0]->outerType() == KindOfRef);


  switch (inType) {
    NULLCASE(): {
      m_regMap.allocOutputRegs(i);
      PhysReg outReg = getReg(i.outStack->location);
      emitImmReg(a, instrNeg, outReg);
      assert(i.isNative());
    } break;
    case KindOfBoolean: {
      if (op == OpCastBool) {
        // Casting bool to bool is a nop.  CastBool's input must be
        // a cell on the stack as per the bytecode specification.
        assert(inputs[0]->isStack());
        assert(inputs[0]->outerType() != KindOfRef);
        assert(inputs[0]->location.space == Location::Stack);
        assert(i.isNative());
        break;
      }
      m_regMap.allocOutputRegs(i);
      PhysReg reg = getReg(inLoc);
      PhysReg outReg = getReg(i.outStack->location);
      if (boxedForm) {
        emitDerefRef(a, reg, outReg);
      } else {
        emitMovRegReg(a, reg, outReg);
      }
      if (instrNeg) {
        a.  xor_imm32_reg32(1, outReg);
      }
    } break;
    case KindOfInt64: {
      m_regMap.allocOutputRegs(i);
      PhysReg reg = getReg(inLoc);
      PhysReg outReg = getReg(i.outStack->location);
      ScratchReg scratch(m_regMap);
      if (boxedForm) {
        emitDerefRef(a, reg, r(scratch));
        emitConvertToBool(a, r(scratch), outReg, instrNeg);
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
        EMIT_CALL(a, fptr, DEREF(inLoc));
      } else {
        EMIT_CALL(a, fptr, V(inLoc));
      }
      if (!IS_STRING_TYPE(inType)) {
        recordReentrantCall(i);
      }
      if (instrNeg) {
        a.    xor_imm32_reg32(1, rax);
      }
      m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                    RegInfo::DIRTY);
    } break;
    default: {
      assert(false);
    } break;
  }
}

void
TranslatorX64::analyzeBranchOp(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = Supported;
}

// Helper for decoding dests of branch-like instructions at the end of
// a basic block.
static void branchDests(const Tracelet& t,
                        const NormalizedInstruction& i,
                        SrcKey* outTaken, SrcKey* outNotTaken,
                        int immIdx = 0) {
  *outNotTaken = nextSrcKey(t, i);
  int dest = i.imm[immIdx].u_BA;
  *outTaken = SrcKey(curFunc(), i.offset() + dest);
}

void TranslatorX64::branchWithFlagsSet(const Tracelet& t,
                                       const NormalizedInstruction& i,
                                       ConditionCode cc) {
  assert(i.op() == OpJmpNZ || i.op() == OpJmpZ);
  // not_taken
  SrcKey taken, notTaken;
  branchDests(t, i, &taken, &notTaken);
  TRACE(3, "branchWithFlagsSet %d %d cc%02x jmp%sz\n",
        taken.offset(), notTaken.offset(), cc,
        i.isJmpNZ() ? "n" : "");
  emitCondJmp(taken, notTaken, cc);
}

void TranslatorX64::fuseBranchAfterStaticBool(Asm& a,
                                              const Tracelet& t,
                                              const NormalizedInstruction& i,
                                              bool resultIsTrue,
                                              bool doSync) {
  assert(i.breaksTracelet);
  assert(i.next);
  NormalizedInstruction &nexti = *i.next;
  if (doSync) {
    fuseBranchSync(t, i);
  } else {
    assert(m_regMap.branchSynced());
  }
  bool isTaken = (resultIsTrue == nexti.isJmpNZ());
  SrcKey taken, notTaken;
  branchDests(t, nexti, &taken, &notTaken);
  if (isTaken) {
    emitBindJmp(a, taken);
  } else {
    emitBindJmp(a, notTaken);
  }
}

void TranslatorX64::fuseBranchAfterHelper(const Tracelet& t,
                                          const NormalizedInstruction& i) {
  fuseBranchSync(t, i);
  a.test_reg64_reg64(rax, rax);
  fuseBranchAfterBool(t, i, CC_NZ);
}

void TranslatorX64::fuseBranchSync(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  assert(!m_regMap.branchSynced());
  // Don't bother sync'ing the output of this instruction.
  m_regMap.scrubStackEntries(i.outStack->location.offset);
  syncOutputs(t);
  m_regMap.setBranchSynced();
}

void TranslatorX64::fuseBranchAfterBool(const Tracelet& t,
                                        const NormalizedInstruction& i,
                                        ConditionCode cc) {
  assert(m_regMap.branchSynced() && i.breaksTracelet && i.next);
  NormalizedInstruction &nexti = *i.next;
  if (!i.next->isJmpNZ()) cc = ccNegate(cc);
  branchWithFlagsSet(t, nexti, cc);
}

/*
 * Fusing "half" of a branch is useful in situations where you would
 * otherwise emit a jcc to or over a fuseStaticBranch. Pass in the
 * condition code and whether that CC means the branch is taken or
 * not. For example, if %rax == 0 means that your branch is not taken
 * (but %rax != 0 means you have to do more checks), do something like
 * this:
 *
 * a.test_reg64_reg64(rax, rax);
 * fuseHalfBranchAfterBool(t, i, CC_Z, false);
 * // ...more comparisons
 */
void TranslatorX64::fuseHalfBranchAfterBool(const Tracelet& t,
                                            const NormalizedInstruction& i,
                                            ConditionCode cc,
                                            bool taken) {
  assert(m_regMap.branchSynced() && i.breaksTracelet && i.next);
  SrcKey destTaken, destNotTaken;
  branchDests(t, *i.next, &destTaken, &destNotTaken);
  if (!i.next->isJmpNZ()) taken = !taken;
  emitBindJcc(a, cc, taken ? destTaken : destNotTaken);
}

void
TranslatorX64::translateBranchOp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  DEBUG_ONLY const Opcode op = i.op();
  assert(op == OpJmpZ || op == OpJmpNZ);

  bool isZ = !i.isJmpNZ();
  assert(i.inputs.size()  == 1);
  assert(!i.outStack && !i.outLocal && !i.outStack2 && !i.outStack3);
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
    bool inStr = rtt.isString();
    void* fptr = inStr ? (void*)str_to_bool : (void*)arr_to_bool;
    EMIT_CALL(a, fptr, V(inLoc));
    if (!inStr) recordReentrantCall(i);
    src = rax;
    ScratchReg sr(m_regMap, rax);
    syncOutputs(t);
  } else if (inputType != KindOfUninit &&
             inputType != KindOfNull &&
             inputType != KindOfBoolean &&
             !rtt.isInt()) {
    // input might be in-flight
    m_regMap.cleanLoc(inLoc);
    // Cast to a bool.
    if (false) {
      TypedValue *tv = nullptr;
      int64_t ret = tv_to_bool(tv);
      if (ret) {
        printf("zoot");
      }
    }
    TRACE(2, Trace::prettyNode("tv_to_bool", inLoc) + string("\n"));
    // tv_to_bool will decRef for us if appropriate
    EMIT_CALL(a, tv_to_bool, A(inLoc));
    recordReentrantCall(i);
    src = rax;
    ScratchReg sr(m_regMap, rax);
    syncOutputs(t);
  } else {
    syncOutputs(t);
  }

  // not_taken
  SrcKey taken, notTaken;
  branchDests(t, i, &taken, &notTaken);

  // Since null always evaluates to false, we can emit an
  // unconditional jump. OpJmpNZ will never take the branch
  // while OpJmpZ will always take the branch.
  if (IS_NULL_TYPE(inputType)) {
    TRACE(1, "branch on Null -> always Z\n");
    emitBindJmp(isZ ? taken : notTaken);
    return;
  }
  prepareForTestAndSmash(kTestRegRegLen, kAlignJccAndJmp);
  a.    test_reg64_reg64(src, src);
  branchWithFlagsSet(t, i, isZ ? CC_Z : CC_NZ);
}

void
TranslatorX64::analyzeCGetL(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  const RuntimeType& type = i.inputs[0]->rtt;
  i.m_txFlags = (type.isUninit() || GuardType(type).mayBeUninit()) ?
    Supported : Native;
}

void
TranslatorX64::translateCGetL(const Tracelet& t,
                              const NormalizedInstruction& i) {
  const DEBUG_ONLY Opcode op = i.op();
  assert(op == OpFPassL || OpCGetL);
  const vector<DynLocation*>& inputs = i.inputs;
  assert(inputs.size() == 1);
  assert(inputs[0]->isLocal());
  DataType outType = inputs[0]->valueType();
  assert(outType != KindOfInvalid);

  if (GuardType(outType).isRelaxed()) {
    assert(outType == KindOfUncountedInit);
    PhysReg locBase, stackBase;
    int     locDisp, stackDisp;
    locToRegDisp(inputs[0]->location, &locBase, &locDisp);
    locToRegDisp(i.outStack->location, &stackBase, &stackDisp);
    if (i.manuallyAllocInputs && !m_regMap.hasReg(inputs[0]->location)) {
      emitCopyToAligned(a, locBase, locDisp, stackBase, stackDisp);
    } else {
      ScratchReg rTmp(m_regMap);
      PhysReg localReg = getReg(inputs[0]->location);
      a.    storeq (localReg, stackBase[stackDisp + TVOFF(m_data)]);
      emitLoadTVType(a, locBase[locDisp + TVOFF(m_type)], r(rTmp));
      emitStoreTVType(a, r(rTmp), stackBase[stackDisp + TVOFF(m_type)]);
    }
    return;
  }

  // Check for use of an undefined local.
  if (inputs[0]->rtt.isUninit()) {
    assert(!i.outStack || i.outStack->outerType() == KindOfNull);
    outType = KindOfNull;
    assert(inputs[0]->location.offset < curFunc()->numLocals());
    const StringData* name = local_name(inputs[0]->location);
    EMIT_CALL(a, raiseUndefVariable, IMM((uintptr_t)name));
    recordReentrantCall(i);
    if (i.outStack) {
      m_regMap.allocOutputRegs(i);
    }
    return;
  }

  /*
   * we can merge a CGetL with a following InstanceOfD
   * to avoid the incRef/decRef on the result (see
   * analyzeSecondPass).
   *
   * outStack will be clear in that case.
   */
  if (!i.outStack) return;
  assert(outType == i.outStack->outerType());
  m_regMap.allocOutputRegs(i);
  if (IS_NULL_TYPE(outType)) return;
  PhysReg dest = getReg(i.outStack->location);

  if (i.manuallyAllocInputs && !m_regMap.hasReg(inputs[0]->location)) {
    fill(inputs[0]->location, dest);
  } else {
    PhysReg localReg = getReg(inputs[0]->location);
    emitMovRegReg(a, localReg, dest);
  }
  if (inputs[0]->isRef()) {
    emitDerefRef(a, dest, dest);
  }
  assert(outType != KindOfStaticString);
  emitIncRef(dest, outType);
}

void
TranslatorX64::analyzeCGetL2(Tracelet& t,
                             NormalizedInstruction& ni) {
  const int locIdx = 1;
  assert(ni.inputs.size() == 2);
  ni.m_txFlags = ni.inputs[locIdx]->rtt.isUninit() ? Supported : Native;
}

void
TranslatorX64::translateCGetL2(const Tracelet& t,
                               const NormalizedInstruction& ni) {
  const int stackIdx = 0;
  const int locIdx   = 1;

  // Note: even if it's an undefined local we need to move a few
  // values around to have outputs end up in the right place.
  const bool undefinedLocal = ni.inputs[locIdx]->rtt.isUninit();

  if (undefinedLocal) {
    assert(ni.outStack->valueType() == KindOfNull);
    assert(ni.inputs[locIdx]->location.offset < curFunc()->numLocals());
    const StringData* name = local_name(ni.inputs[locIdx]->location);

    EMIT_CALL(a, raiseUndefVariable, IMM((uintptr_t)name));
    recordReentrantCall(ni);

    m_regMap.allocInputRegs(ni);
  }

  m_regMap.allocOutputRegs(ni);
  const PhysReg stackIn  = getReg(ni.inputs[stackIdx]->location);
  const PhysReg localIn  = getReg(ni.inputs[locIdx]->location);
  const PhysReg stackOut = getReg(ni.outStack2->location);
  assert(ni.inputs[stackIdx]->location.isStack());
  assert(ni.inputs[locIdx]->location.isLocal());

  /*
   * These registers overlap a bit, so we can swap a few bindings to
   * avoid a move.
   */
  assert(stackIn == getReg(ni.outStack->location) && localIn != stackOut);
  m_regMap.swapRegisters(stackIn, stackOut);
  const PhysReg cellOut = getReg(ni.outStack->location);
  assert(cellOut != stackIn);
  if (ni.inputs[locIdx]->isRef()) {
    emitDerefRef(a, localIn, cellOut);
  } else if (!undefinedLocal) {
    emitMovRegReg(a, localIn, cellOut);
  }
  emitIncRef(cellOut, ni.inputs[locIdx]->valueType());
}

void
TranslatorX64::analyzeVGetL(Tracelet& t,
                            NormalizedInstruction& i) {
  i.m_txFlags = Native;
}

void
TranslatorX64::translateVGetL(const Tracelet& t,
                              const NormalizedInstruction& i) {
  const DEBUG_ONLY Opcode op = i.op();
  assert(op == OpVGetL || op == OpFPassL);
  const vector<DynLocation*>& inputs = i.inputs;
  assert(inputs.size() == 1);
  assert(i.outStack);
  assert(inputs[0]->isLocal());
  assert(i.outStack->rtt.outerType() == KindOfRef);

  PhysReg localReg = getReg(inputs[0]->location);
  PhysReg dest;
  if (inputs[0]->rtt.outerType() != KindOfRef) {
    emitBox(inputs[0]->rtt.outerType(), localReg);
    m_regMap.bind(rax, inputs[0]->location, KindOfRef,
                  RegInfo::DIRTY);
    m_regMap.allocOutputRegs(i);
    dest = getReg(i.outStack->location);
    emitMovRegReg(a, rax, dest);
  } else {
    m_regMap.allocOutputRegs(i);
    dest = getReg(i.outStack->location);
    emitMovRegReg(a, localReg, dest);
  }
  emitIncRef(dest, KindOfRef);
}

static bool
isSupportedInstrVGetG(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  return (i.inputs[0]->rtt.isString());
}

void
TranslatorX64::analyzeVGetG(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = simplePlan(isSupportedInstrVGetG(i));
}

static TypedValue* lookupAddBoxedGlobal(StringData* name) {
  VarEnv* ve = g_vmContext->m_globalVarEnv;
  TypedValue* r = ve->lookupAdd(name);
  if (r->m_type != KindOfRef) {
    tvBox(r);
  }
  decRefStr(name);
  return r;
}

void
TranslatorX64::translateVGetG(const Tracelet& t,
                              const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(i.outStack);
  assert(i.outStack->isRef());
  assert(i.inputs[0]->location == i.outStack->location);

  using namespace TargetCache;
  const StringData* maybeName = i.inputs[0]->rtt.valueString();
  if (!maybeName) {
    EMIT_CALL(a, lookupAddBoxedGlobal, V(i.inputs[0]->location));
    recordCall(i);
  } else {
    CacheHandle ch = BoxedGlobalCache::alloc(maybeName);

    if (false) { // typecheck
      StringData *key = nullptr;
      TypedValue UNUSED *glob = BoxedGlobalCache::lookupCreate(ch, key);
    }
    SKTRACE(1, i.source, "ch %d\n", ch);
    EMIT_CALL(a, BoxedGlobalCache::lookupCreate,
               IMM(ch),
               V(i.inputs[0]->location));
    recordCall(i);
  }
  m_regMap.bind(rax, i.outStack->location, KindOfRef, RegInfo::DIRTY);
  emitIncRefGeneric(rax, 0);
  emitDeref(a, rax, rax);
}

void
TranslatorX64::analyzeAssignToLocalOp(Tracelet& t,
                                      NormalizedInstruction& ni) {
  const int locIdx = 1;
  ni.m_txFlags = planHingesOnRefcounting(ni.inputs[locIdx]->outerType());
}


void
TranslatorX64::translateAssignToLocalOp(const Tracelet& t,
                                        const NormalizedInstruction& ni) {
  const int rhsIdx  = 0;
  const int locIdx  = 1;
  const Opcode op = ni.op();
  assert(op == OpSetL || op == OpBindL);
  assert(ni.inputs.size() == 2);
  assert((op == OpBindL) ==
         (ni.inputs[rhsIdx]->outerType() == KindOfRef));

  assert(!ni.outStack || ni.inputs[locIdx]->location != ni.outStack->location);
  assert(ni.outLocal);
  assert(ni.inputs[locIdx]->location == ni.outLocal->location);
  assert(ni.inputs[rhsIdx]->isStack());

  const DataType oldLocalType = ni.inputs[locIdx]->outerType();
  const DataType rhsType      = ni.inputs[rhsIdx]->outerType();
  bool rhsTypeRelaxed         = GuardType(rhsType).isRelaxed();
  bool locTypeRelaxed         = GuardType(oldLocalType).isRelaxed();

  m_regMap.allocOutputRegs(ni);

  const PhysReg rhsReg        = getReg(ni.inputs[rhsIdx]->location);
  const PhysReg localReg      = getReg(ni.outLocal->location);
  assert(localReg != rhsReg);

  LazyScratchReg oldLocalReg(m_regMap);
  DataType decRefType;

  // For SetL, when the local is boxed, we need to change the
  // type/value of the inner cell.  If we're doing BindL, we don't
  // want to affect the old inner cell in any case (except to decref
  // it).
  const bool affectInnerCell = op == OpSetL &&
                               oldLocalType == KindOfRef;
  if (affectInnerCell) {
    assert(rhsType != KindOfRef);
    decRefType = ni.inputs[locIdx]->rtt.innerType();
    bool useOldType = (locTypeRelaxed && GuardType(decRefType).isCounted()) ||
      (!locTypeRelaxed && IS_REFCOUNTED_TYPE(decRefType));
    if (useOldType) {
      oldLocalReg.alloc();
      emitDerefRef(a, localReg, r(oldLocalReg));
    }
    if (rhsTypeRelaxed) {
      PhysReg base;
      int disp;
      ScratchReg rTmp(m_regMap);
      locToRegDisp(ni.inputs[rhsIdx]->location, &base, &disp);
      size_t typeOff = RefData::tvOffset() + TVOFF(m_type);
      size_t dataOff = RefData::tvOffset() + TVOFF(m_data);
      emitLoadTVType(a, base[disp + TVOFF(m_type)], r(rTmp));
      a.    storeq (rhsReg, localReg[dataOff]);
      emitStoreTVType(a, r(rTmp), localReg[typeOff]);
    } else {
      emitStoreToRefData(a, rhsType, rhsReg, 0, localReg);
    }
  } else if (rhsTypeRelaxed) {
    PhysReg rhsBase;
    int rhsDisp;
    locToRegDisp(ni.inputs[rhsIdx]->location, &rhsBase, &rhsDisp);
    PhysReg locBase;
    int locDisp;
    locToRegDisp(ni.inputs[locIdx]->location, &locBase, &locDisp);
    ScratchReg rTmp(m_regMap);
    a.    storeq(rhsReg, locBase[locDisp + TVOFF(m_data)]);
    emitLoadTVType(a, rhsBase[rhsDisp + TVOFF(m_type)], r(rTmp));
    emitStoreTVType(a, r(rTmp), locBase[locDisp + TVOFF(m_type)]);
    m_regMap.swapRegisters(rhsReg, localReg);
    decRefType = oldLocalType;
    m_regMap.markAsClean(ni.inputs[locIdx]->location);
  } else {
    /*
     * Instead of emitting a mov, just swap the locations these two
     * registers are mapped to.
     *
     * TODO: this might not be the best idea now that the register
     * allocator has some awareness about what is a local.  (Maybe we
     * should just xchg.)
     */
    m_regMap.swapRegisters(rhsReg, localReg);
    decRefType = oldLocalType;
  }

  // If we're giving stack output, it's important to incref before
  // calling a possible destructor, since the destructor could have
  // access to the local if it is a var.
  if (ni.outStack) {
    if (rhsTypeRelaxed) {
      if (GuardType(rhsType).isCounted()) {
        PhysReg base;
        int disp;
        locToRegDisp(ni.inputs[rhsIdx]->location, &base, &disp);
        emitIncRefGeneric(base, disp); // forces static check
      }
    } else {
      emitIncRef(rhsReg, rhsType);
    }
  } else {
    SKTRACE(3, ni.source, "hoisting Pop* into current instr\n");
  }

  if (locTypeRelaxed) {
    if (GuardType(decRefType).isCounted()) {
      emitDecRef(ni, oldLocalReg.isAllocated() ? r(oldLocalReg) : localReg,
                 decRefType);
    }
  } else {
    emitDecRef(ni, oldLocalReg.isAllocated() ? r(oldLocalReg) : localReg,
               decRefType);
  }

  if (ni.outStack && !IS_NULL_TYPE(ni.outStack->outerType())) {
    assert(!rhsTypeRelaxed);
    PhysReg stackReg = getReg(ni.outStack->location);
    emitMovRegReg(a, rhsReg, stackReg);
  }
}


static void
planPop(NormalizedInstruction& i) {
  DataType type = i.inputs[0]->outerType();
  // Avoid type-prediction guard simply for popping the value out of the stack.
  if (i.prev && i.prev->outputPredicted) {
    i.prev->outputPredicted = false;
    // If the prediction is based on static analysis, the type is either 'type'
    // or null. So if 'type' is not ref-counted, keeping it avoids the dynamic
    // check for decref.
    if (!(i.prev->outputPredictionStatic) || IS_REFCOUNTED_TYPE(type)) {
      i.inputs[0]->rtt = RuntimeType(KindOfInvalid);
      type = KindOfInvalid;
    }
  }
  i.m_txFlags =
    (type == KindOfInvalid || IS_REFCOUNTED_TYPE(type)) ? Supported : Native;
  i.manuallyAllocInputs = true;
}

void TranslatorX64::analyzePopC(Tracelet& t, NormalizedInstruction& i) {
  planPop(i);
}

void TranslatorX64::analyzePopV(Tracelet& t, NormalizedInstruction& i) {
  planPop(i);
}

void TranslatorX64::analyzePopR(Tracelet& t, NormalizedInstruction& i) {
  planPop(i);
}

void
TranslatorX64::translatePopC(const Tracelet& t,
                             const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(!i.outStack && !i.outLocal);
  if (i.inputs[0]->rtt.isVagueValue()) {
    PhysReg base;
    int disp;
    locToRegDisp(i.inputs[0]->location, &base, &disp);
    emitDecRefGeneric(i, base, disp);
  } else {
    emitDecRefInput(a, i, 0);
  }
}

void
TranslatorX64::translatePopV(const Tracelet& t,
                             const NormalizedInstruction& i) {
  assert(i.inputs[0]->rtt.isVagueValue() ||
         i.inputs[0]->isRef());
  translatePopC(t, i);
}

void
TranslatorX64::translatePopR(const Tracelet& t,
                             const NormalizedInstruction& i) {
  translatePopC(t, i);
}

void
TranslatorX64::translateUnboxR(const Tracelet& t,
                               const NormalizedInstruction& i) {
  assert(!i.inputs[0]->rtt.isVagueValue());

  // If the value on the top of a stack is a var, unbox it and
  // leave it on the top of the stack.
  if (i.inputs[0]->isRef()) {
    emitUnboxTopOfStack(i);
  }
}

void
TranslatorX64::translateNull(const Tracelet& t,
                             const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);
  if (i.outStack) {
    assert(i.outStack->outerType() == KindOfNull);

    // We have to mark the output register as dirty to ensure that
    // the type gets spilled at the end of the tracelet
    m_regMap.allocOutputRegs(i);
  }
  /* nop */
}

void
TranslatorX64::translateNullUninit(const Tracelet& t,
                             const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);
  if (i.outStack) {
    assert(i.outStack->outerType() == KindOfUninit);
    // We have to mark the output register as dirty to ensure that
    // the type gets spilled at the end of the tracelet
    m_regMap.allocOutputRegs(i);
  }
}

void
TranslatorX64::translateTrue(const Tracelet& t,
                             const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);
  if (i.outStack) {
    m_regMap.allocOutputRegs(i);
    PhysReg rdest = getReg(i.outStack->location);
    emitImmReg(a, 1, rdest);
  }
}

void
TranslatorX64::translateFalse(const Tracelet& t,
                              const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);
  if (i.outStack) {
    m_regMap.allocOutputRegs(i);
    PhysReg dest = getReg(i.outStack->location);
    emitImmReg(a, false, dest);
  }
}

void
TranslatorX64::translateInt(const Tracelet& t,
                            const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);
  if (i.outStack) {
    assert(i.outStack->isInt());
    m_regMap.allocOutputRegs(i);
    PhysReg dest = getReg(i.outStack->location);
    uint64_t srcImm = i.imm[0].u_I64A;
    emitImmReg(a, srcImm, dest);
  }
}

void
TranslatorX64::translateDouble(const Tracelet& t,
                               const NormalizedInstruction& i) {
  if (i.outStack) {
    assert(i.outStack->isDouble());
    m_regMap.allocOutputRegs(i);
    emitImmReg(a, i.imm[0].u_I64A, getReg(i.outStack->location));
  }
}

void
TranslatorX64::translateString(const Tracelet& t,
                               const NormalizedInstruction& i) {
  assert(i.inputs.size()  == 0);
  assert(!i.outLocal);
  if (!i.outStack) return;
  assert(Translator::typeIsString(i.outStack->outerType()));
  m_regMap.allocOutputRegs(i);
  PhysReg dest = getReg(i.outStack->location);
  uint64_t srcImm = (uintptr_t)curUnit()->lookupLitstrId(i.imm[0].u_SA);
  // XXX: can simplify the lookup here by just fishing it out of the
  // output's valueString().
  // We are guaranteed that the string is static, so we do not need to
  // increment the refcount
  assert(((StringData*)srcImm)->isStatic());
  SKTRACE(2, i.source, "Litstr %d -> %p \"%s\"\n",
      i.imm[0].u_SA, (StringData*)srcImm,
      Util::escapeStringForCPP(((StringData*)srcImm)->data()).c_str());
  emitImmReg(a, srcImm, dest);
}

void
TranslatorX64::translateArray(const Tracelet& t,
                              const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outLocal);
  if (i.outStack) {
    assert(i.outStack->outerType() == KindOfArray);
    m_regMap.allocOutputRegs(i);
    ArrayData* ad = curUnit()->lookupArrayId(i.imm[0].u_AA);
    PhysReg r = getReg(i.outStack->location);
    emitImmReg(a, uint64_t(ad), r);
    // We are guaranteed that the array is static, so we do not need to
    // increment the refcount
    assert(ad->isStatic());
  }
}

void
TranslatorX64::translateNewArray(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(i.outStack && !i.outLocal);
  assert(i.outStack->outerType() == KindOfArray);
  int capacity = i.imm[0].u_IVA;
  if (capacity == 0) {
    m_regMap.allocOutputRegs(i);
    PhysReg r = getReg(i.outStack->location);
    emitImmReg(a, uint64_t(HphpArray::GetStaticEmptyArray()), r);
    // We are guaranteed that the new array is static, so we do not need to
    // increment the refcount
    assert(HphpArray::GetStaticEmptyArray()->isStatic());
  } else {
    // create an empty array with a nonzero capacity
    if (false) {
      ArrayData* a = new_array(42);
      printf("%p", a); // use ret
    }
    EMIT_CALL(a, new_array, IMM(capacity));
    m_regMap.bind(rax, i.outStack->location, KindOfArray, RegInfo::DIRTY);
  }
}

void TranslatorX64::analyzeNewTuple(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = Simple; // the array constructors are not re-entrant.
  i.manuallyAllocInputs = true; // all values passed via stack.
}

void TranslatorX64::translateNewTuple(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  int arity = i.imm[0].u_IVA;
  assert(arity > 0 && i.inputs.size() == unsigned(arity));
  assert(i.outStack && !i.outLocal);
  for (int j = 0; j < arity; j++) {
    assert(i.inputs[j]->outerType() != KindOfRef);
    assert(i.inputs[j]->isStack());
  }

  // We pass the values by address, so we need to sync them back to memory
  for (int j = 0; j < arity; j++) {
    m_regMap.cleanLoc(i.inputs[j]->location);
  }
  if (false) {
    TypedValue* rhs = 0;
    ArrayData* ret = new_tuple(arity, rhs);
    printf("%p", ret); // use ret
  }
  EMIT_CALL(a, new_tuple, IMM(arity), A(i.inputs[0]->location));
  // new_tuple() returns the up-to-date array pointer in rax. Therefore, we
  // can bind rax to the result location and mark it as dirty.
  m_regMap.bind(rax, i.inputs[arity-1]->location, KindOfArray, RegInfo::DIRTY);
}

void
TranslatorX64::translateNewCol(const Tracelet& t,
                               const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(i.outStack && !i.outLocal);
  assert(i.outStack->outerType() == KindOfObject);
  int cType = i.imm[0].u_IVA;
  int nElms = i.imm[1].u_IVA;
  void* fptr = nullptr;
  switch (cType) {
    case Collection::VectorType: fptr = (void*)newVectorHelper; break;
    case Collection::MapType: fptr = (void*)newMapHelper; break;
    case Collection::StableMapType: fptr = (void*)newStableMapHelper; break;
    case Collection::PairType: fptr = (void*)newPairHelper; break;
    default: assert(false); break;
  }
  if (false) {
    ObjectData* obj1 UNUSED = newVectorHelper(42);
    ObjectData* obj2 UNUSED = newMapHelper(42);
    ObjectData* obj3 UNUSED = newStableMapHelper(42);
    ObjectData* obj4 UNUSED = newPairHelper();
  }
  if (cType == Collection::PairType) {
    // newPairHelper does not take any arguments, since Pairs always
    // have exactly two elements
    EMIT_CALL(a, fptr);
  } else {
    EMIT_CALL(a, fptr, IMM(nElms));
  }
  m_regMap.bind(rax, i.outStack->location, KindOfObject, RegInfo::DIRTY);
}

void
TranslatorX64::analyzeNop(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = Native;
}

void
TranslatorX64::translateNop(const Tracelet& t,
                            const NormalizedInstruction& i) {
}

void
TranslatorX64::analyzeAddElemC(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = supportedPlan(i.inputs[2]->outerType() == KindOfArray &&
                              (i.inputs[1]->isInt() ||
                               i.inputs[1]->isString()));
}

void
TranslatorX64::translateAddElemC(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  assert(i.outStack && !i.outLocal);
  assert(i.inputs.size() >= 3);
  const DynLocation& arr = *i.inputs[2];
  const DynLocation& key = *i.inputs[1];
  const DynLocation& val = *i.inputs[0];
  assert(!arr.isRef()); // not handling variants.
  assert(!key.isRef());
  assert(!val.isRef());

  const Location& arrLoc = arr.location;
  const Location& keyLoc = key.location;
  const Location& valLoc = val.location;

  assert(arrLoc.isStack());
  assert(keyLoc.isStack());
  assert(arrLoc.isStack());

  // We will need to pass the rhs by address, so we need to sync it to memory
  m_regMap.cleanLoc(valLoc);

  // The array_setm helpers will decRef any old value that is
  // overwritten if appropriate. If copy-on-write occurs, it will also
  // incRef the new array and decRef the old array for us. Finally,
  // some of the array_setm helpers will decRef the key if it is a
  // string (for cases where the key is not a local), while others do
  // not (for cases where the key is a local).
  assert(key.rtt.isInt() || key.rtt.isString());
  if (false) { // type-check
    RefData* ref = nullptr;
    TypedValue* rhs = nullptr;
    StringData* strkey = nullptr;
    ArrayData* arr = nullptr;
    ArrayData* ret;
    ret = array_setm_ik1_v0(ref, arr, 12, rhs);
    printf("%p", ret); // use ret
    ret = array_setm_sk1_v0(ref, arr, strkey, rhs);
    printf("%p", ret); // use ret
  }
  // Otherwise, we pass the rhs by address
  void* fptr = key.rtt.isString() ? (void*)array_setm_sk1_v0 :
               (void*)array_setm_ik1_v0;
  EMIT_CALL(a, fptr,
             IMM(0),
             V(arrLoc),
             V(keyLoc),
             A(valLoc));
  recordReentrantCall(i);
  // The array value may have changed, so we need to invalidate any
  // register we have associated with arrLoc
  m_regMap.invalidate(arrLoc);
  // The array_setm helper returns the up-to-date array pointer in rax.
  // Therefore, we can bind rax to arrLoc and mark it as dirty.
  m_regMap.bind(rax, arrLoc, KindOfArray, RegInfo::DIRTY);
}

void
TranslatorX64::analyzeAddNewElemC(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  i.m_txFlags = supportedPlan(i.inputs[1]->outerType() == KindOfArray);
}

void
TranslatorX64::translateAddNewElemC(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  assert(i.outStack && !i.outLocal);
  assert(i.inputs[0]->outerType() != KindOfRef);
  assert(i.inputs[1]->outerType() != KindOfRef);
  assert(i.inputs[0]->isStack());
  assert(i.inputs[1]->isStack());

  Location arrLoc = i.inputs[1]->location;
  Location valLoc = i.inputs[0]->location;

  // We pass the rhs by address, so we need to sync it back to memory
  m_regMap.cleanLoc(valLoc);

  // The array_setm_wki_v0 helper will decRef the value if it cannot
  // be stored; otherwise the value is moved (neither incref'd or decref'd).
  // Copy-on-write is expected not to occur since AddNewElemC is used
  // for array initialization.
  if (false) { // type-check
    TypedValue* rhs = nullptr;
    ArrayData* arr = nullptr;
    ArrayData* ret;
    ret = array_setm_wk1_v0(arr, rhs);
    printf("%p", ret); // use ret
  }
  EMIT_CALL(a, array_setm_wk1_v0,
            V(arrLoc), A(valLoc));
  recordReentrantCall(i);
  // The array value may have changed, so we need to invalidate any
  // register we have associated with arrLoc
  m_regMap.invalidate(arrLoc);
  // The array_setm helper returns the up-to-date array pointer in rax.
  // Therefore, we can bind rax to arrLoc and mark it as dirty.
  m_regMap.bind(rax, arrLoc, KindOfArray, RegInfo::DIRTY);
}

void
TranslatorX64::analyzeColAddNewElemC(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  i.m_txFlags = supportedPlan(i.inputs[1]->outerType() == KindOfObject);
}

void
TranslatorX64::translateColAddNewElemC(const Tracelet& t,
                                       const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  assert(i.outStack && !i.outLocal);
  assert(i.inputs[0]->outerType() != KindOfRef);
  assert(i.inputs[1]->outerType() != KindOfRef);
  assert(i.inputs[0]->isStack());
  assert(i.inputs[1]->isStack());
  Location collLoc = i.inputs[1]->location;
  Location valLoc = i.inputs[0]->location;
  m_regMap.cleanLoc(valLoc);
  if (false) { // type-check
    TypedValue* rhs = nullptr;
    ObjectData* coll = nullptr;
    collection_setm_wk1_v0(coll, rhs);
  }
  EMIT_RCALL(a, i, collection_setm_wk1_v0,
             V(collLoc),
             A(valLoc));
}

void
TranslatorX64::analyzeColAddElemC(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = supportedPlan(i.inputs[2]->outerType() == KindOfObject &&
                              (i.inputs[1]->isInt() ||
                               i.inputs[1]->isString()));
}

void
TranslatorX64::translateColAddElemC(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  assert(i.outStack && !i.outLocal);
  assert(i.inputs.size() >= 3);
  const DynLocation& coll = *i.inputs[2];
  const DynLocation& key = *i.inputs[1];
  const DynLocation& val = *i.inputs[0];
  assert(!coll.isRef()); // not handling variants.
  assert(!key.isRef());
  assert(!val.isRef());
  const Location& collLoc = coll.location;
  const Location& keyLoc = key.location;
  const Location& valLoc = val.location;
  assert(collLoc.isStack());
  assert(keyLoc.isStack());
  assert(collLoc.isStack());
  m_regMap.cleanLoc(valLoc);
  assert(key.rtt.isInt() || key.rtt.isString());
  if (false) { // type-check
    TypedValue* rhs = nullptr;
    StringData* strkey = nullptr;
    ObjectData* coll = nullptr;
    collection_setm_ik1_v0(coll, 12, rhs);
    collection_setm_sk1_v0(coll, strkey, rhs);
  }
  void* fptr = key.rtt.isString() ? (void*)collection_setm_sk1_v0 :
               (void*)collection_setm_ik1_v0;
  EMIT_RCALL(a, i, fptr,
             V(collLoc),
             V(keyLoc),
             A(valLoc));
}

static void undefCns(const StringData* nm) {
  VMRegAnchor _;
  TypedValue *cns = g_vmContext->getCns(const_cast<StringData*>(nm));
  if (!cns) {
    if (AutoloadHandler::s_instance->autoloadConstant(StrNR(nm))) {
      cns = g_vmContext->getCns(const_cast<StringData*>(nm));
    }
    if (!cns) {
      raise_notice(Strings::UNDEFINED_CONSTANT, nm->data(), nm->data());
      g_vmContext->getStack().pushStringNoRc(const_cast<StringData*>(nm));
      return;
    }
  }
  Cell* c1 = g_vmContext->getStack().allocC();
  tvReadCell(cns, c1);
}

void TranslatorX64::emitSideExit(Asm& a, const NormalizedInstruction& i,
                                 bool next) {
  const NormalizedInstruction& dest = next ? *i.next : i;

  SKTRACE(3, i.source, "sideexit check %p\n", a.code.frontier);
  Stats::emitInc(a, Stats::Tx64_SideExit);
  if (!m_regMap.hasDirtyRegs(i.stackOff)) {
    Stats::emitInc(a, Stats::Tx64_SideExitClean);
  }
  // NB: if next == true, we are assuming here that stack elements
  // spit out by this instruction are already clean and sync'd back to
  // the top slot of the stack.
  m_regMap.scrubStackEntries(dest.stackOff);
  m_regMap.cleanAll();
  emitRB(a, RBTypeSideExit, i.source);
  int stackDisp = dest.stackOff;
  if (stackDisp != 0) {
    SKTRACE(3, i.source, "stack bump %d => %x\n", stackDisp,
            -cellsToBytes(stackDisp));
    a.   addq (-cellsToBytes(stackDisp), rVmSp);
  }
  emitBindJmp(a, dest.source, REQ_BIND_SIDE_EXIT);
}

void
TranslatorX64::translateCns(const Tracelet& t,
                            const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(i.outStack && !i.outLocal);

  // OK to burn "name" into TC: it was merged into the static string
  // table, so as long as this code is reachable, so should the string
  // be.
  DataType outType = i.outStack->valueType();
  StringData* name = curUnit()->lookupLitstrId(i.imm[0].u_SA);
  const TypedValue* tv = g_vmContext->getCns(name, true, false);
  bool checkDefined = false;
  if (outType != KindOfInvalid && tv == nullptr &&
      !RuntimeOption::RepoAuthoritative) {
    PreConstDepMap::accessor acc;
    tv = findUniquePreConst(acc, name);
    if (tv != nullptr) {
      checkDefined = true;
      acc->second.srcKeys.insert(t.m_sk);
      Stats::emitInc(a, Stats::Tx64_CnsFast);
    } else {
      // We had a unique value while analyzing but don't anymore. This
      // should be rare so just punt to keep things simple.
      punt();
    }
  }
  using namespace TargetCache;
  if (tv && tvIsStatic(tv)) {
    m_regMap.allocOutputRegs(i);
    boost::scoped_ptr<DiamondReturn> astubsRet;
    if (checkDefined) {
      size_t bit = allocCnsBit(name);
      uint8_t mask;
      CacheHandle ch = bitOffToHandleAndMask(bit, mask);
      // The 'test' instruction takes a signed immediate and the mask is
      // unsigned, but everything works out okay because the immediate is
      // the same size as the other operand. However, we have to sign-extend
      // the mask to 64 bits to make the assembler happy.
      int64_t imm = (int64_t)(int8_t)mask;
      a.testb(imm, rVmTl[ch]);
      if (!i.next) astubsRet.reset(new DiamondReturn);
      {
        // If we get to the optimistic translation and the constant
        // isn't defined, our tracelet is ruined because the type may
        // not be what we expect. If we were expecting KindOfString we
        // could theoretically keep going here since that's the type
        // of an undefined constant expression, but it should be rare
        // enough that it's not worth the complexity.
        UnlikelyIfBlock ifZero(CC_Z, a, astubs, astubsRet.get());
        Stats::emitInc(astubs, Stats::Tx64_CnsFast, -1);
        EMIT_CALL(astubs, undefCns, IMM((uintptr_t)name));
        recordReentrantStubCall(i);
        if (i.next) {
          emitSideExit(astubs, i, true);
        } else {
          m_regMap.invalidate(i.outStack->location);
        }
      }
    }
    // Its type and value are known at compile-time.
    assert(tv->m_type == outType ||
           (IS_STRING_TYPE(tv->m_type) && IS_STRING_TYPE(outType)));
    PhysReg r = getReg(i.outStack->location);
    a.   movq (tv->m_data.num, r);
    // tv is static; no need to incref
    return;
  }

  Stats::emitInc(a, Stats::Tx64_CnsSlow);
  CacheHandle ch = allocConstant(name);
  TRACE(2, "Cns: %s -> ch %" PRId64 "\n", name->data(), ch);
  // Load the constant out of the thread-private tl_targetCaches.
  ScratchReg cns(m_regMap);
  a.    lea_reg64_disp_reg64(rVmTl, ch, r(cns));
  emitCmpTVType(a, 0, r(cns)[TVOFF(m_type)]);
  DiamondReturn astubsRet;
  int stackDest = 0 - int(sizeof(Cell)); // popped - pushed
  {
    // It's tempting to dedup these, but not obvious we really can;
    // at least stackDest and tmp are specific to the translation
    // context.
    UnlikelyIfBlock ifb(CC_Z, a, astubs, &astubsRet);
    EMIT_CALL(astubs, undefCns, IMM((uintptr_t)name));
    recordReentrantStubCall(i);
    m_regMap.invalidate(i.outStack->location);
  }

  // Bitwise copy to output area.
  emitCopyToStack(a, i, r(cns), stackDest);
  m_regMap.invalidate(i.outStack->location);
}

void
TranslatorX64::analyzeDefCns(Tracelet& t,
                             NormalizedInstruction& i) {
  StringData* name = curUnit()->lookupLitstrId(i.imm[0].u_SA);
  /* don't bother to translate if it names a builtin constant */
  i.m_txFlags = supportedPlan(!g_vmContext->getCns(name, true, false));
}

typedef void (*defCnsHelper_func_t)(TargetCache::CacheHandle ch, Variant *inout,
                                    StringData *name, size_t bit);
template<bool setBit>
static void defCnsHelper(TargetCache::CacheHandle ch, Variant *inout,
                         StringData *name, size_t bit) {
  using namespace TargetCache;
  TypedValue *tv = (TypedValue*)handleToPtr(ch);
  if (LIKELY(tv->m_type == KindOfUninit &&
             inout->isAllowedAsConstantValue())) {
    inout->setEvalScalar();
    if (LIKELY(g_vmContext->insertCns(name, (TypedValue*)inout))) {
      tvDup((TypedValue*)inout, tv);
      *inout = true;
      if (setBit) {
        DEBUG_ONLY bool alreadyDefined = testAndSetBit(bit);
        assert(!alreadyDefined);
      }
      return;
    }
    tv = (TypedValue*)&false_varNR;
  }

  if (tv->m_type != KindOfUninit) {
    raise_warning(Strings::CONSTANT_ALREADY_DEFINED, name->data());
  } else {
    assert(!inout->isAllowedAsConstantValue());
    raise_warning(Strings::CONSTANTS_MUST_BE_SCALAR);
  }
  *inout = false;
}

void
TranslatorX64::translateDefCns(const Tracelet& t,
                               const NormalizedInstruction& i) {
  StringData* name = curUnit()->lookupLitstrId(i.imm[0].u_SA);

  if (false) {
    TargetCache::CacheHandle ch = 0;
    size_t bit = 0;
    Variant *inout = 0;
    StringData *name = 0;
    defCnsHelper<true>(ch, inout, name, bit);
    defCnsHelper<false>(ch, inout, name, bit);
  }

  using namespace TargetCache;
  CacheHandle ch = allocConstant(name);
  TRACE(2, "DefCns: %s -> ch %" PRId64 "\n", name->data(), ch);

  m_regMap.cleanLoc(i.inputs[0]->location);
  if (RuntimeOption::RepoAuthoritative) {
    EMIT_CALL(a, (defCnsHelper_func_t)defCnsHelper<false>,
               IMM(ch), A(i.inputs[0]->location),
               IMM((uint64_t)name));
  } else {
    EMIT_CALL(a, (defCnsHelper_func_t)defCnsHelper<true>,
               IMM(ch), A(i.inputs[0]->location),
               IMM((uint64_t)name), IMM(allocCnsBit(name)));
  }
  recordReentrantCall(i);
  m_regMap.invalidate(i.outStack->location);
}

void
TranslatorX64::translateClsCnsD(const Tracelet& t,
                                const NormalizedInstruction& i) {
  using namespace TargetCache;
  const NamedEntityPair& namedEntityPair =
    curUnit()->lookupNamedEntityPairId(i.imm[1].u_SA);
  assert(namedEntityPair.second);
  const StringData *clsName = namedEntityPair.first;
  assert(clsName->isStatic());
  StringData* cnsName = curUnit()->lookupLitstrId(i.imm[0].u_SA);
  assert(cnsName->isStatic());
  StringData* fullName = StringData::GetStaticString(
    Util::toLower(clsName->data()) + "::" + cnsName->data());

  Stats::emitInc(a, Stats::TgtCache_ClsCnsHit);
  CacheHandle ch = allocClassConstant(fullName);
  ScratchReg cns(m_regMap);
  a.lea_reg64_disp_reg64(rVmTl, ch, r(cns));
  emitCmpTVType(a, 0, r(cns)[TVOFF(m_type)]);
  {
    UnlikelyIfBlock ifNull(CC_Z, a, astubs);

    if (false) { // typecheck
      TypedValue* tv = nullptr;
      UNUSED TypedValue* ret =
        TargetCache::lookupClassConstant(tv, namedEntityPair.second,
                                         namedEntityPair.first, cnsName);
    }

    EMIT_CALL(astubs, TCA(TargetCache::lookupClassConstant),
              R(cns),
              IMM(uintptr_t(namedEntityPair.second)),
              IMM(uintptr_t(namedEntityPair.first)),
              IMM(uintptr_t(cnsName)));
    recordReentrantStubCall(i);
    // DiamondGuard will restore cns's SCRATCH state but not its
    // contents. lookupClassConstant returns the value we want.
    emitMovRegReg(astubs, rax, r(cns));
  }
  int stackDest = 0 - int(sizeof(Cell)); // 0 popped - 1 pushed
  emitCopyToStack(a, i, r(cns), stackDest);
}

void
TranslatorX64::analyzeConcat(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  const RuntimeType& r = i.inputs[0]->rtt;
  const RuntimeType& l = i.inputs[1]->rtt;
  // The concat translation isn't reentrant; objects that override
  // __toString() can cause reentry.
  i.m_txFlags = supportedPlan(r.valueType() != KindOfObject &&
                              l.valueType() != KindOfObject);
}

void
TranslatorX64::translateConcat(const Tracelet& t,
                               const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  const DynLocation& r = *i.inputs[0];
  const DynLocation& l = *i.inputs[1];
  // We have specialized helpers for concatenating two strings, a
  // string and an int, and an int an a string.
  void* fptr = nullptr;
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
      StringData* v1 = nullptr;
      StringData* v2 = nullptr;
      StringData* retval = concat_ss(v1, v2);
      printf("%p", retval); // use retval
    }

    // The concat helper will decRef the inputs and incRef the output
    // for us if appropriate
    EMIT_RCALL(a, i, fptr, V(l.location), V(r.location));
    assert(i.outStack->rtt.isString());
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);

  } else {
    // Otherwise, use the generic concat helper
    if (false) { // type check
      uint64_t v1 = 0, v2 = 0;
      DataType t1 = KindOfUninit, t2 = KindOfUninit;
      StringData *retval = concat(t1, v1, t2, v2);
      printf("%p", retval); // use retval
    }
    // concat will decRef the two inputs and incRef the output
    // for us if appropriate
    EMIT_RCALL(a, i, concat,
               IMM(l.valueType()), V(l.location),
               IMM(r.valueType()), V(r.location));
    assert(i.outStack->isString());
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
  }
}

TXFlags
planInstrAdd_Int(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  return nativePlan(i.inputs[0]->isInt() && i.inputs[1]->isInt());
}

TXFlags
planInstrAdd_Array(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  return supportedPlan(i.inputs[0]->valueType() == KindOfArray &&
                       i.inputs[1]->valueType() == KindOfArray);
}

TXFlags
planInstrAdd_Double(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  return nativePlan(i.inputs[0]->isDouble() && i.inputs[1]->isDouble());
}

void
TranslatorX64::analyzeAdd(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = TXFlags(planInstrAdd_Int(i) | planInstrAdd_Array(i) |
                        planInstrAdd_Double(i));
}

void
TranslatorX64::translateAdd(const Tracelet& t,
                            const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);

  if (planInstrAdd_Array(i)) {
    // Handle adding two arrays
    assert(i.outStack->outerType() == KindOfArray);
    if (false) { // type check
      ArrayData* v = nullptr;
      v = array_add(v, v);
    }
    // The array_add helper will decRef the inputs and incRef the output
    // for us if appropriate
    EMIT_CALL(a, array_add,
               V(i.inputs[1]->location),
               V(i.inputs[0]->location));
    recordReentrantCall(i);
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
    return;
  }

  assert(planInstrAdd_Int(i) | planInstrAdd_Double(i));
  binaryArithCell(i, OpAdd, *i.inputs[0], *i.outStack);
}

void
TranslatorX64::analyzeXor(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = nativePlan((i.inputs[0]->outerType() == KindOfBoolean ||
                            i.inputs[0]->isInt()) &&
                           (i.inputs[1]->outerType() == KindOfBoolean ||
                            i.inputs[1]->isInt()));
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
  a.   setcc           (CC, rbyte(scratch));
  a.   movzbl (rbyte(scratch), r32(srcdest));
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
  PhysReg src, srcdest;
  getBinaryStackInputs(m_regMap, i, src, srcdest);
  m_regMap.allocOutputRegs(i);
  ScratchReg scr(m_regMap);
  if (i.inputs[0]->isInt()) {
    emitIntToBool(a, src, r(scr));
  }
  if (i.inputs[1]->isInt()) {
    emitIntToBool(a, srcdest, r(scr));
  }
  a.    xor_reg64_reg64(src, srcdest);
}

void
TranslatorX64::analyzeMod(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = nativePlan(false);
}

void TranslatorX64::translateMod(const Tracelet& t, const NormalizedInstruction& i) {
  not_reached();
}

void
TranslatorX64::analyzeNot(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  i.m_txFlags = nativePlan(i.inputs[0]->isInt() ||
                           i.inputs[0]->outerType() == KindOfBoolean);
}

void
TranslatorX64::translateNot(const Tracelet& t,
                            const NormalizedInstruction& i) {
  assert(i.isNative());
  assert(i.outStack && !i.outLocal);
  assert(!i.inputs[0]->isRef());
  m_regMap.allocOutputRegs(i);
  PhysReg srcdest = m_regMap.getReg(i.outStack->location);
  ScratchReg scr(m_regMap);
  emitIntToNegBool(a, srcdest, r(scr));
}

void
TranslatorX64::analyzeBitNot(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = nativePlan(i.inputs[0]->isInt());
}

void
TranslatorX64::translateBitNot(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  assert(i.outStack && !i.outLocal);
  m_regMap.allocOutputRegs(i);
  PhysReg srcdest = m_regMap.getReg(i.outStack->location);
  a.   not  (srcdest);
}

#define TRIVIAL_CAST(Type) \
void \
TranslatorX64::analyzeCast## Type(Tracelet& t, NormalizedInstruction& i) { \
  i.m_txFlags = nativePlan(i.inputs[0]->is## Type()); \
} \
 \
void \
TranslatorX64::translateCast## Type(const Tracelet& t, \
                                    const NormalizedInstruction& i) { \
  assert(i.inputs.size() == 1); \
  assert(i.outStack && !i.outLocal); \
  assert(i.inputs[0]->is## Type()); \
 \
  /* nop */ \
}

TRIVIAL_CAST(Int)
TRIVIAL_CAST(Array)
TRIVIAL_CAST(Object)
#undef TRIVIAL_CAST

void
TranslatorX64::analyzeCastString(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags =
    i.inputs[0]->isArray() || i.inputs[0]->isObject() ? Supported :
    i.inputs[0]->isInt() ? Simple :
    Native;
  i.funcd = nullptr;
}

static void toStringError(StringData *cls) {
  raise_error("Method __toString() must return a string value");
}

static const StringData* stringDataFromInt(int64_t n) {
  StringData* s = buildStringData(n);
  s->incRefCount();
  return s;
}

static const StringData* stringDataFromDouble(int64_t n) {
  StringData* s = buildStringData(*(double*)&n);
  s->incRefCount();
  return s;
}

// returns the prologue address to execute
uint64_t TranslatorX64::toStringHelper(ObjectData *obj) {
  // caller must set r15 to the new ActRec
  static_assert(rStashedAR == r15 &&
                rVmFp == rbp,
                "toStringHelper needs to be updated for ABI changes");
  register ActRec *ar asm("r15");

  const Class* cls = obj->getVMClass();
  const Func* toString = cls->getToString();
  if (!toString) {
    // the unwinder will restore rVmSp to
    // &ar->m_r, so we'd better make sure its
    // got a valid TypedValue there.
    tvWriteUninit(&ar->m_r);
    std::string msg = cls->preClass()->name()->data();
    msg += "::__toString() was not defined";
    throw BadTypeConversionException(msg.c_str());
  }
  ar->m_func = toString;
  // ar->m_soff set by caller
  ar->initNumArgs(0);
  ar->setThis(obj);
  ar->setVarEnv(0);
  return (uint64_t)toString->getPrologue(0);
}

void
TranslatorX64::translateCastString(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(i.outStack && !i.outLocal);

  if (i.inputs[0]->isNull()) {
    m_regMap.allocOutputRegs(i);
    PhysReg dest = m_regMap.getReg(i.outStack->location);
    a.   mov_imm64_reg((uint64_t)empty_string.get(), dest);
  } else if (i.inputs[0]->isBoolean()) {
    static StringData* s_1 = StringData::GetStaticString("1");
    m_regMap.allocOutputRegs(i);
    PhysReg dest = m_regMap.getReg(i.outStack->location);
    a.   cmp_imm32_reg64(0, dest);
    a.   mov_imm64_reg((uint64_t)empty_string.get(), dest);
    ScratchReg scratch(m_regMap);
    a.   mov_imm64_reg((intptr_t)s_1, r(scratch));
    a.   cmov_reg64_reg64(CC_NZ, r(scratch), dest);
  } else if (i.inputs[0]->isInt()) {
    EMIT_CALL(a, stringDataFromInt, V(i.inputs[0]->location));
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
  } else if (i.inputs[0]->isDouble()) {
    EMIT_CALL(a, stringDataFromDouble, V(i.inputs[0]->location));
    m_regMap.bind(rax, i.outStack->location, i.outStack->outerType(),
                  RegInfo::DIRTY);
  } else if (i.inputs[0]->isString()) {
    // nop
  } else if (i.inputs[0]->isArray()) {
    static StringData* s_array = StringData::GetStaticString("Array");
    m_regMap.allocOutputRegs(i);
    PhysReg dest = m_regMap.getReg(i.outStack->location);
    emitDecRef(i, dest, KindOfArray);
    a.   mov_imm64_reg((uint64_t)s_array, dest);
  } else if (i.inputs[0]->isObject()) {
    m_regMap.scrubStackEntries(i.stackOff - 1);
    m_regMap.cleanAll();
    int delta = i.stackOff + kNumActRecCells - 1;
    if (delta) {
      a. add_imm64_reg64(-cellsToBytes(delta), rVmSp);
    }
    a.   store_reg64_disp_reg64(rVmFp, AROFF(m_savedRbp), rVmSp);
    a.   store_imm32_disp_reg(nextSrcKey(t, i).offset() - curFunc()->base(),
                              AROFF(m_soff), rVmSp);
    PhysReg obj = m_regMap.getReg(i.inputs[0]->location);
    if (obj != argNumToRegName[0]) {
      a. mov_reg64_reg64(obj,  argNumToRegName[0]);
    }
    m_regMap.smashRegs(kAllRegs);
    a.   mov_reg64_reg64(rVmSp, rStashedAR);

    EMIT_CALL(a, TCA(toStringHelper));
    recordReentrantCall(i);
    // call to the address returned by toStringHelper
    a.    call(reg::rax);
    if (i.stackOff != 0) {
      a.  add_imm64_reg64(cellsToBytes(i.stackOff), rVmSp);
    }

    PhysReg base;
    int disp;
    locToRegDisp(i.outStack->location, &base, &disp);
    emitStringCheck(a, base, disp + TVOFF(m_type));
    {
      UnlikelyIfBlock ifNotString(CC_Z, a, astubs);
      EMIT_CALL(astubs, toStringError, IMM(0));
      recordReentrantStubCall(i);
    }
  } else {
    NOT_REACHED();
  }
}

void
TranslatorX64::analyzeCastDouble(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = nativePlan(i.inputs[0]->valueType() == KindOfDouble);
}

void
TranslatorX64::translateCastDouble(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  // nop.
}

void
TranslatorX64::analyzePrint(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  const RuntimeType& rtt = i.inputs[0]->rtt;
  DataType type = rtt.outerType();
  i.m_txFlags = simplePlan(
    type == KindOfUninit ||
    type == KindOfNull ||
    type == KindOfBoolean ||
    rtt.isInt() ||
    rtt.isString());
}

void
TranslatorX64::translatePrint(const Tracelet& t,
                              const NormalizedInstruction& i) {
  const vector<DynLocation*>& inputs  = i.inputs;
  assert(inputs.size()   == 1);
  assert(!i.outLocal);
  assert(!i.outStack || i.outStack->isInt());
  Location  loc = inputs[0]->location;
  DataType type = inputs[0]->outerType();
  switch (type) {
    STRINGCASE():       EMIT_CALL(a, print_string,  V(loc)); break;
    case KindOfInt64:   EMIT_CALL(a, print_int,     V(loc)); break;
    case KindOfBoolean: EMIT_CALL(a, print_boolean, V(loc)); break;
    NULLCASE():         /* do nothing */                   break;
    default: {
      // Translation is only supported for Null, Boolean, Int, and String
      assert(false);
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
TranslatorX64::translateJmp(const Tracelet& t,
                            const NormalizedInstruction& i) {
  assert(!i.outStack && !i.outLocal);
  if (i.breaksTracelet) {
    syncOutputs(t);
  }

  // Check the surprise page on all backwards jumps
  if (i.imm[0].u_BA < 0 && !i.noSurprise) {
    if (trustSigSegv) {
      const uint64_t stackMask =
        ~(cellsToBytes(RuntimeOption::EvalVMStackElms) - 1);
      a.mov_reg64_reg64(rVmSp, rScratch);
      a.and_imm64_reg64(stackMask, rScratch);
      TCA surpriseLoad = a.code.frontier;
      a.load_reg64_disp_reg64(rScratch, 0, rScratch);

      if (!m_segvStubs.insert(SignalStubMap::value_type(surpriseLoad,
                                                        astubs.code.frontier)))
        NOT_REACHED();
      /*
       * Note that it is safe not to register unwind information here,
       * because we just called syncOutputs so all registers are
       * already clean.
       */
      astubs.call((TCA)&EventHook::CheckSurprise);
      recordStubCall(i);
      astubs.jmp(a.code.frontier);
    } else {
      emitTestSurpriseFlags(a);
      {
        UnlikelyIfBlock ifSurprise(CC_NZ, a, astubs);
        astubs.call((TCA)&EventHook::CheckSurprise);
        recordStubCall(i);
      }
    }
  }
  if (i.breaksTracelet) {
    SrcKey sk(curFunc(), i.offset() + i.imm[0].u_BA);
    emitBindJmp(sk);
  }
}

void
TranslatorX64::analyzeSwitch(Tracelet& t,
                             NormalizedInstruction& i) {
  RuntimeType& rtt = i.inputs[0]->rtt;
  assert(rtt.outerType() != KindOfRef);
  switch (rtt.outerType()) {
    NULLCASE():
    case KindOfBoolean:
    case KindOfInt64:
      i.m_txFlags = Native;
      break;

    case KindOfDouble:
      i.m_txFlags = Simple;
      break;

    STRINGCASE():
    case KindOfObject:
    case KindOfArray:
      i.m_txFlags = Supported;
      break;

    default:
      not_reached();
  }
}

template <typename T>
static int64_t switchBoundsCheck(T v, int64_t base, int64_t nTargets) {
  // I'm relying on gcc to be smart enough to optimize away the next
  // two lines when T is int64.
  if (int64_t(v) == v) {
    int64_t ival = v;
    if (ival >= base && ival < (base + nTargets)) {
      return ival - base;
    }
  }
  return nTargets + 1;
}

int64_t switchDoubleHelper(int64_t val, int64_t base, int64_t nTargets) {
  union {
    int64_t intbits;
    double dblval;
  } u;
  u.intbits = val;
  return switchBoundsCheck(u.dblval, base, nTargets);
}

int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets) {
  int64_t ival;
  double dval;
  switch (s->isNumericWithVal(ival, dval, 1)) {
    case KindOfNull:
      ival = switchBoundsCheck(0, base, nTargets);
      break;

    case KindOfDouble:
      ival = switchBoundsCheck(dval, base, nTargets);
      break;

    case KindOfInt64:
      ival = switchBoundsCheck(ival, base, nTargets);
      break;

    default:
      not_reached();
  }
  decRefStr(s);
  return ival;
}

int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets) {
  int64_t ival = o->o_toInt64();
  decRefObj(o);
  return switchBoundsCheck(ival, base, nTargets);
}

void
TranslatorX64::translateSwitch(const Tracelet& t,
                               const NormalizedInstruction& i) {
  int64_t base = i.imm[1].u_I64A;
  bool bounded = i.imm[2].u_IVA;
  const ImmVector& iv = i.immVec;
  int nTargets = bounded ? iv.size() - 2 : iv.size();
  int jmptabSize = nTargets;
  assert(nTargets > 0);
  PhysReg valReg = getReg(i.inputs[0]->location);
  DataType inType = i.inputs[0]->outerType();
  assert(IMPLIES(inType != KindOfInt64, bounded));
  assert(IMPLIES(bounded, iv.size() > 2));
  syncOutputs(t); // this will mark valReg as FREE but it still has
                  // its old value

  SrcKey defaultSk(curFunc(), i.offset() + iv.vec32()[iv.size() - 1]);
  SrcKey zeroSk(curFunc(), 0);
  if (0 >= base && 0 < (base + nTargets)) {
    zeroSk.m_offset = i.offset() + iv.vec32()[0 - base];
  } else {
    zeroSk.m_offset = defaultSk.m_offset;
  }

  switch (i.inputs[0]->outerType()) {
    NULLCASE(): {
      emitBindJmp(zeroSk);
      return;
    }

    case KindOfBoolean: {
      SrcKey nonzeroSk(curFunc(), i.offset() + iv.vec32()[iv.size() - 2]);
      a.test_reg64_reg64(valReg, valReg);
      emitCondJmp(nonzeroSk, zeroSk, CC_NZ);
      return;
    }

    case KindOfInt64:
      // No special treatment needed
      break;

    case KindOfDouble:
    STRINGCASE():
    case KindOfObject: {
      // switch(Double|String|Obj)Helper do bounds-checking for us, so
      // we need to make sure the default case is in the jump table,
      // and don't emit our own bounds-checking code
      jmptabSize = iv.size();
      bounded = false;
      if (false) {
        StringData* s = nullptr;
        ObjectData* o = nullptr;
        switchDoubleHelper(0.0, 0, 0);
        switchStringHelper(s, 0, 0);
        switchObjHelper(o, 0, 0);
      }
      EMIT_CALL(a,
                 inType == KindOfDouble ? (TCA)switchDoubleHelper :
                 (IS_STRING_TYPE(inType) ? (TCA)switchStringHelper :
                  (TCA)switchObjHelper),
                 R(valReg), IMM(base), IMM(nTargets));
      recordCall(i);
      valReg = rax;
      break;
    }

    case KindOfArray:
      emitDecRef(a, i, valReg, KindOfArray);
      emitBindJmp(defaultSk);
      return;

    default:
      not_reached();
  }

  if (bounded) {
    if (base) {
      a.sub_imm64_reg64(base, valReg);
    }
    a.cmp_imm64_reg64(nTargets, valReg);
    prepareForSmash(a, kJmpccLen);
    TCA defaultStub =
      emitServiceReq(REQ_BIND_JMPCC_SECOND, 3,
                     a.code.frontier, defaultSk.m_offset, CC_AE);
    // Unsigned comparison: check for < 0 and >= nTargets at the same time
    a.jae(defaultStub);
  }

  TCA* jmptab = m_globalData.alloc<TCA>(sizeof(TCA), jmptabSize);
  TCA afterLea = a.code.frontier + kLeaRipLen;
  ptrdiff_t diff = (TCA)jmptab - afterLea;
  assert(deltaFits(diff, sz::dword));
  a.  lea  (rip[diff], rScratch);
  assert(a.code.frontier == afterLea);
  a.  jmp  (rScratch[valReg*8]);

  for (int idx = 0; idx < jmptabSize; ++idx) {
    SrcKey sk(curFunc(), i.offset() + iv.vec32()[idx]);
    jmptab[idx] = emitServiceReq(SRFlags::SRNone, REQ_BIND_ADDR, 2ull,
                                 &jmptab[idx], uint64_t(sk.offset()));
  }
}

void
TranslatorX64::analyzeSSwitch(Tracelet& t,
                              NormalizedInstruction& i) {
  i.m_txFlags = Supported;
}

static TCA sswitchHelperSlow(TypedValue* val, const StringData** strs,
                             int cases, TCA* jmptab) {
  if (val->m_type == KindOfRef) val = val->m_data.pref->tv();
  for (int i = 0; i < cases; ++i) {
    if (tvAsCVarRef(val).equal(strs[i])) return jmptab[i];
  }
  // default case
  return jmptab[cases];
}

typedef FixedStringMap<TCA, true> SSwitchMap;

HOT_FUNC_VM
TCA sswitchHelperFast(const StringData* val, SSwitchMap* table,
                      TCA* def) {
  TCA* dest = table->find(val);
  if (dest) {
    return *dest;
  } else {
    return *def;
  }
}

void
TranslatorX64::translateSSwitch(const Tracelet& t,
                                const NormalizedInstruction& ni) {
  DynLocation& input = *ni.inputs[0];
  Location& inLoc = input.location;
  const ImmVector& iv = ni.immVec;
  const StrVecItem* strvec = iv.strvec();
  int targets = iv.size();
  assert(targets > 1);
  unsigned cases = targets - 1;
  const Unit* u = curUnit();
  std::vector<const StringData*> strings;
  for (unsigned i = 0; i < cases; ++i) {
    strings.push_back(u->lookupLitstrId(strvec[i].str));
  }

  // We support the fast path if the input is a string and none of the
  // cases are numeric strings
  bool fastPath = IS_STRING_TYPE(input.valueType());
  for (auto s : strings) {
    if (s->isNumeric()) {
      fastPath = false;
      break;
    }
  }

  auto bindAddr = [&](TCA& dest, Offset o) {
    SrcKey sk(curFunc(), ni.offset() + o);
    dest = emitServiceReq(SRFlags::SRNone, REQ_BIND_ADDR, 2ull,
                          &dest, uint64_t(sk.offset()));
  };
  if (fastPath) {
    Stats::emitInc(a, Stats::Tx64_StringSwitchFast);

    SSwitchMap* table = m_globalData.alloc<SSwitchMap>(kX64CacheLineSize);
    table->init(cases);
    TCA* def = m_globalData.alloc<TCA>(sizeof(TCA), 1);
    for (unsigned i = 0; i < cases; ++i) {
      table->add(strings[i], nullptr);
      TCA* addr = table->find(strings[i]);
      assert(addr && *addr == nullptr);
      bindAddr(*addr, strvec[i].dest);
    }
    bindAddr(*def, strvec[targets-1].dest);

    EMIT_RCALL(a, ni, sswitchHelperFast,
               input.isRef() ? DEREF(inLoc) : V(inLoc),
               IMM(int64_t(table)), IMM(int64_t(def)));
  } else {
    Stats::emitInc(a, Stats::Tx64_StringSwitchSlow);
    const StringData** strtab = m_globalData.alloc<const StringData*>(
      sizeof(const StringData*), cases);
    memcpy(strtab, &strings[0], sizeof(const StringData*) * cases);

    // Build the jump table.
    TCA* jmptab = m_globalData.alloc<TCA>(sizeof(TCA), targets);
    for (int i = 0; i < targets; ++i) {
      bindAddr(jmptab[i], strvec[i].dest);
    }

    m_regMap.cleanLoc(inLoc);
    EMIT_RCALL(a, ni, sswitchHelperSlow,
               A(inLoc), IMM(int64_t(strtab)), IMM(cases), IMM(int64_t(jmptab)));
  }
  ScratchReg holdRax(m_regMap, rax);
  m_regMap.allocInputReg(ni, 0);
  emitDecRef(a, ni, getReg(inLoc), input.outerType());
  syncOutputs(t);
  a.jmp(rax);
}

void
TranslatorX64::analyzeRetC(Tracelet& t,
                           NormalizedInstruction& i) {
  i.manuallyAllocInputs = true;
  i.m_txFlags = Supported;
}

void
TranslatorX64::analyzeRetV(Tracelet& t,
                           NormalizedInstruction& i) {
  analyzeRetC(t, i);
}

void TranslatorX64::emitReturnVal(
  Asm& a, const NormalizedInstruction& i,
  PhysReg dstBase, int dstOffset, PhysReg thisBase, int thisOffset,
  PhysReg scratch) {

  if (!i.grouped) return;
  TypedValue tv;
  tvWriteUninit(&tv);
  tv.m_data.num = 0; // to keep the compiler happy

  auto moveRetValIfNeeded = [&] {
    if (thisBase != dstBase ||
        thisOffset != (dstOffset + TVOFF(m_data))) {
      a.  loadq(thisBase[thisOffset], scratch);
      a.  storeq(scratch, dstBase[dstOffset + TVOFF(m_data)]);
    }
  };
  /*
   * We suppressed the write of the (literal) return value
   * to the stack. Figure out what it was.
   */
  NormalizedInstruction* prev = i.prev;
  assert(!prev->outStack);
  switch (prev->op()) {
    case OpNull:
      tv.m_type = KindOfNull;
      break;
    case OpTrue:
    case OpFalse:
      tv.m_type = KindOfBoolean;
      tv.m_data.num = prev->op() == OpTrue;
      break;
    case OpInt:
      tv.m_type = KindOfInt64;
      tv.m_data.num = prev->imm[0].u_I64A;
      break;
    case OpDouble:
      tv.m_type = KindOfDouble;
      tv.m_data.dbl = prev->imm[0].u_DA;
      break;
    case OpString:
      tv.m_type = BitwiseKindOfString;
      tv.m_data.pstr = curUnit()->lookupLitstrId(prev->imm[0].u_SA);
      break;
    case OpArray:
      tv.m_type = KindOfArray;
      tv.m_data.parr = curUnit()->lookupArrayId(prev->imm[0].u_AA);
      break;
    case OpThis: {
      moveRetValIfNeeded();
      emitStoreTVType(a, KindOfObject, dstBase[dstOffset + TVOFF(m_type)]);
      return;
    }
    case OpBareThis: {
      assert(curFunc()->cls());
      moveRetValIfNeeded();
      a.    mov_imm32_reg32(KindOfNull, scratch);
      a.    testb(1, thisBase[thisOffset]);
      {
        JccBlock<CC_NZ> noThis(a);
        a.  mov_imm32_reg32(KindOfObject, scratch);
      }
      emitStoreTVType(a, scratch, dstBase[dstOffset + TVOFF(m_type)]);
      return;
    }
    default:
      not_reached();
  }

  emitStoreTVType(a, tv.m_type, r64(dstBase)[dstOffset + TVOFF(m_type)]);
  if (tv.m_type != KindOfNull) {
    emitStoreImm(a, tv.m_data.num,
                 dstBase, dstOffset + TVOFF(m_data), sz::qword);
  }

}

void TranslatorX64::emitDecRefThis(const ScratchReg& rTmp) {
  // If we grouped a $this into the ret we're returning this, but we
  // didn't incRef it, so we dont have to decRef here.
  const bool mergedThis = m_curNI->wasGroupedWith(OpThis, OpBareThis);
  if (mergedThis) {
    return;
  }

  /*
   * In both of these cases we need to write back a null pointer to
   * the this field in the ActRec, just for the case that a local
   * might do debug_backtrace and access a freed object.
   *
   * In the case of mergedThis it's safe not to do this, because we
   * are returning a reference on $this from the function so it will
   * still be alive in any case.
   */

  // If this is a instance method called on an object or if it is a
  // pseudomain, we need to decRef $this (if there is one)
  if (curFunc()->isMethod() && !curFunc()->isStatic()) {
    // This assert is weaker than it looks; it only checks the invocation
    // we happen to be translating for. The runtime "assert" is the
    // unconditional dereference of m_this we emit; if the frame has
    // neither this nor a class, then m_this will be null and we'll
    // SEGV.
    assert(curFrame()->hasThis() || curFrame()->hasClass());
    // m_this and m_cls share a slot in the ActRec, so we check the
    // lowest bit (0 -> m_this, 1 -> m_cls)
    a.      load_reg64_disp_reg64(rVmFp, AROFF(m_this), r(rTmp));
    a.      store_imm64_disp_reg64(0, AROFF(m_this), rVmFp);
    if (m_curNI->guardedThis) {
      emitDecRef(*m_curNI, r(rTmp), KindOfObject);
    } else {
      a.    testb(1, rbyte(rTmp));
      {
        JccBlock<CC_NZ> ifZero(a);
        emitDecRef(a, *m_curNI, r(rTmp), KindOfObject);
      }
    }
  } else if (curFunc()->isPseudoMain()) {
    a.      load_reg64_disp_reg64(rVmFp, AROFF(m_this), r(rTmp));
    a.      store_imm64_disp_reg64(0, AROFF(m_this), rVmFp);
    a.      shrq(1, r(rTmp)); // sets c (from bit 0) and z
    FreezeRegs ice(m_regMap);
    {
      // tests for Not Zero and Not Carry
      UnlikelyIfBlock ifRealThis(CC_NBE, a, astubs);
      astubs.    shlq(1, r(rTmp));
      emitDecRef(astubs, *m_curNI, r(rTmp), KindOfObject);
    }
  }
}

/*
 * If this function can possibly use variadic arguments or shared
 * variable environment, we need to check for it and clear them if
 * they exist.
 */
void TranslatorX64::emitVVRet(const ScratchReg& rTmp,
                              Label& extraArgsReturn,
                              Label& varEnvReturn) {
  if (!(curFunc()->attrs() & AttrMayUseVV)) return;
  SKTRACE(2, m_curNI->source, "emitting mayUseVV in UnlikelyIf\n");

  a.    load_reg64_disp_reg64(rVmFp, AROFF(m_varEnv), r(rTmp));
  a.    test_reg64_reg64(r(rTmp), r(rTmp));
  {
    // TODO: maybe this should be a semi-likely block when there
    // is a varenv at translation time.
    UnlikelyIfBlock varEnvCheck(CC_NZ, a, astubs);
    auto& a = astubs;

    a.  test_imm32_reg32(ActRec::kExtraArgsBit, r(rTmp));
    jccBlock<CC_Z>(a, [&] {
      guardDiamond(a, [&] {
        EMIT_RCALL(
          a, *m_curNI,
          TCA(static_cast<void (*)(ActRec*)>(ExtraArgs::deallocate)),
          R(rVmFp)
        );
      });
      extraArgsReturn.jmp(a);
    });

    m_regMap.cleanAll();
    EMIT_RCALL(
      a, *m_curNI,
      TCA(getMethodPtr(&VarEnv::detach)),
      R(rTmp),
      R(rVmFp)
    );

    if (!m_curNI->inlineReturn) {
      // If it's not inline, the return we're about to jump to expects
      // the helper has adjusted rVmSp already.
      a.lea_reg64_disp_reg64(rVmFp, AROFF(m_r), rVmSp);
    }
    varEnvReturn.jmp(a);
  }
}

void TranslatorX64::emitInlineReturn(Location retvalSrcLoc,
                                     int retvalSrcDisp) {
  SKTRACE(2, m_curNI->source, "emitting specialized inline return\n");

  assert(int(m_curNI->inputs.size()) == curFunc()->numLocals() -
    int(m_curNI->nonRefCountedLocals.count()));
  for (int k = m_curNI->inputs.size() - 1; k >= 0; --k) {
    assert(m_curNI->inputs[k]->location.space == Location::Local);
    DataType t = m_curNI->inputs[k]->outerType();
    if (GuardType(t).isCounted()) {
      PhysReg reg = m_regMap.allocReg(m_curNI->inputs[k]->location, t,
                                      RegInfo::CLEAN);
      // We currently need to zero the type just in case the event
      // hook throws (see #2088495), or a destructor captures a
      // backtrace
      PhysReg base;
      int disp;
      locToRegDisp(m_curNI->inputs[k]->location, &base, &disp);
      emitStoreTVType(a, KindOfUninit, base[disp + TVOFF(m_type)]);

      emitDecRef(*m_curNI, reg, t);
    }
  }

  // Register map is officially out of commission now.
  m_regMap.scrubLoc(retvalSrcLoc);
  m_regMap.smashRegs(kAllRegs);
}

void TranslatorX64::emitGenericReturn(bool noThis, int retvalSrcDisp) {
  SKTRACE(2, m_curNI->source, "emitting generic return\n");
  assert(m_curNI->inputs.size() == 0);

  m_regMap.cleanAll();
  m_regMap.smashRegs(kAllRegs);

  if (m_curNI->grouped) {
    /*
     * What a pain: EventHook::onFunctionExit needs access
     * to the return value - so we have to write it to the
     * stack anyway. We still win for OpThis, and
     * OpBareThis, since we dont have to do any refCounting
     */
    ScratchReg s(m_regMap);
    emitReturnVal(a, *m_curNI,
                  rVmSp, retvalSrcDisp, rVmFp, AROFF(m_this), r(s));
  }

  // Custom calling convention: the argument is in rVmSp.
  int numLocals = curFunc()->numLocals();
  assert(numLocals > 0);
  a.    subq(0x8, rsp); // For parity.  Callee will do retq $0x8.
  a.    lea(rVmFp[-numLocals * sizeof(TypedValue)], rVmSp);
  if (numLocals > kNumFreeLocalsHelpers) {
    emitCall(a, m_freeManyLocalsHelper);
  } else {
    emitCall(a, m_freeLocalsHelpers[numLocals - 1]);
  }
  recordReentrantCall(a, *m_curNI);
}

void
TranslatorX64::translateRetC(const Tracelet& t,
                             const NormalizedInstruction& i) {
  if (i.skipSync) assert(i.grouped);

  int stackAdjustment = t.m_stackChange;
  if (i.skipSync) {
    SKTRACE(2, i.source, "i.skipSync\n");

    /*
     * getting here means there was nothing to do between
     * a previous reqXXX and this ret. Any spill code we generate
     * here would be broken (because the rbx is wrong), so
     * verify that we don't generate anything...
     */
    TCA s DEBUG_ONLY = a.code.frontier;
    syncOutputs(0);
    assert(s == a.code.frontier);
    stackAdjustment = 0;
  } else {
    /*
     * no need to syncOutputs here... we're going to update
     * rbx at the end of this function anyway, and we may want
     * to use enregistered locals on the fast path below
     */
    m_regMap.scrubStackEntries(t.m_stackChange);
    m_regMap.cleanAll(); // TODO(#1339331): don't.
  }

  const bool noThis = !curFunc()->isPseudoMain() &&
                      (!curFunc()->isMethod() || curFunc()->isStatic());

  /*
   * figure out where to put the return value, and where to get it from
   */
  assert(i.stackOff == t.m_stackChange);
  const Location retvalSrcLoc(Location::Stack, stackAdjustment - 1);

  const Func *callee = curFunc();
  assert(callee);
  int nLocalCells =
    callee == nullptr ? 0 : // This happens for returns from pseudo-main.
    callee->numSlotsInFrame();
  int retvalSrcDisp = cellsToBytes(-stackAdjustment);
  assert(cellsToBytes(locPhysicalOffset(retvalSrcLoc)) == retvalSrcDisp);

  Label varEnvReturn;
  Label extraArgsReturn;
  {
    ScratchReg rTmp(m_regMap);
    emitDecRefThis(rTmp);
    emitVVRet(rTmp, extraArgsReturn, varEnvReturn);
  }
asm_label(a, extraArgsReturn);
  if (m_curNI->inlineReturn) {
    emitInlineReturn(retvalSrcLoc, retvalSrcDisp);
  } else {
    emitGenericReturn(noThis, retvalSrcDisp);
  }
  assert(m_regMap.empty());

  // The (1 + nLocalCells) skips 1 slot for the return value.
  int retvalDestDisp = cellsToBytes(1 + nLocalCells - stackAdjustment) +
    AROFF(m_r);

  if (!m_curNI->inlineReturn) {
    // Compensate for rVmSp already being adjusted by the helper in
    // emitFrameRelease.
    retvalSrcDisp -= sizeof(ActRec) +
      cellsToBytes(nLocalCells - stackAdjustment);
    retvalDestDisp = 0;
  }

asm_label(a, varEnvReturn);
  emitTestSurpriseFlags(a);
  {
    UnlikelyIfBlock ifTracer(CC_NZ, a, astubs);
    if (m_curNI->grouped) {
      // We need to drop the return value on the stack for the event
      // hook, same as in emitGenericReturn.
      ScratchReg s(m_regMap);
      emitReturnVal(astubs, *m_curNI,
                    rVmSp, retvalSrcDisp, rVmFp, AROFF(m_this), r(s));
    }
    astubs.mov_reg64_reg64(rVmFp, argNumToRegName[0]);
    emitCall(astubs, (TCA)&EventHook::FunctionExit, true);
    recordReentrantStubCall(*m_curNI);
  }

  /*
   * We're officially between tracelets now, and the normal register
   * allocator is not being used.
   */
  RegSet scratchRegs = kScratchCrossTraceRegs;
  DumbScratchReg rRetAddr(scratchRegs);

  /*
   * Having gotten everything we care about out of the current frame
   * pointer, smash the return address type and value over it. We don't
   * care about reference counts: as long as this runs to completion, we're
   * refcount-neutral.
   */
  if (i.grouped) {
    DumbScratchReg s(scratchRegs);
    emitReturnVal(a, i, rVmSp, retvalDestDisp,
                  rVmSp, retvalDestDisp - AROFF(m_r) + AROFF(m_this),
                  r(s));
  } else {
    emitCopyToAligned(a, rVmSp, retvalSrcDisp, rVmSp, retvalDestDisp);
  }

  /*
   * Now update the principal hardware registers.
   *
   * Stack pointer has to skip over all the locals as well as the
   * activation record.
   */
  if (m_curNI->inlineReturn) {
    // If we're not freeing inline, the helper took care of this.
    a.  lea_reg64_disp_reg64(rVmFp, AROFF(m_r), rVmSp);
  }
  a.    load_reg64_disp_reg64(rVmFp, AROFF(m_savedRip), r(rRetAddr));
  a.    load_reg64_disp_reg64(rVmFp, AROFF(m_savedRbp), rVmFp);
  emitRB(a, RBTypeFuncExit, curFunc()->fullName()->data(), RegSet(r(rRetAddr)));
  // push the return address and do a ret
  a.    push(r(rRetAddr));
  a.    ret();
  translator_not_reached(a);
}

void
TranslatorX64::translateRetV(const Tracelet& t,
                             const NormalizedInstruction& i) {
  translateRetC(t, i);
}

/*
 * NativeImpl is a special operation in the sense that it must be the
 * only opcode in a function body, and also functions as the return.
 *
 * This function runs between tracelets and does not use m_regMap.
 *
 * if emitSavedRIPReturn is false, it returns the amount by which
 * rVmSp should be adjusted, otherwise, it emits code to perform
 * the adjustment (this allows us to combine updates to rVmSp)
 */
int32_t TranslatorX64::emitNativeImpl(const Func* func,
                                      bool emitSavedRIPReturn) {
  BuiltinFunction builtinFuncPtr = func->builtinFuncPtr();
  if (false) { // typecheck
    ActRec* ar = nullptr;
    builtinFuncPtr(ar);
  }

  TRACE(2, "calling builtin preClass %p func %p\n", func->preClass(),
    builtinFuncPtr);
  /*
   * Call the native implementation. This will free the locals for us in the
   * normal case. In the case where an exception is thrown, the VM unwinder
   * will handle it for us.
   */
  a.   mov_reg64_reg64(rVmFp, argNumToRegName[0]);
  emitCall(a, (TCA)builtinFuncPtr, false /* smash regs */);

  /*
   * We're sometimes calling this while curFunc() isn't really the
   * builtin---make sure to properly record the sync point as if we
   * are inside the builtin.
   *
   * The assumption here is that for builtins, the generated func
   * contains only a single opcode (NativeImpl), and there are no
   * non-argument locals.
   */
  assert(func->numIterators() == 0 && func->isBuiltin());
  assert(func->numLocals() == func->numParams());
  assert(*func->getEntry() == OpNativeImpl);
  assert(instrLen(func->getEntry()) == func->past() - func->base());
  Offset pcOffset = 0;  // NativeImpl is the only instruction in the func
  Offset stackOff = func->numLocals(); // Builtin stubs have no
                                       // non-arg locals
  recordSyncPoint(a, pcOffset, stackOff);

  if (emitSavedRIPReturn) {
    // push the return address to get ready to ret.
    a.   push  (rVmFp[AROFF(m_savedRip)]);
  }

  /*
   * The native implementation already put the return value on the
   * stack for us, and handled cleaning up the arguments.  We have to
   * update the frame pointer and the stack pointer, and load the
   * return value into the return register so the trace we are
   * returning to has it where it expects.
   *
   * TODO(#1273094): we should probably modify the actual builtins to
   * return values via registers (rax:edx) using the C ABI and do a
   * reg-to-reg move.
   */
  int nLocalCells = func->numSlotsInFrame();
  if (emitSavedRIPReturn) {
    a. add_imm64_reg64(sizeof(ActRec) + cellsToBytes(nLocalCells-1), rVmSp);
  }
  a.   load_reg64_disp_reg64(rVmFp, AROFF(m_savedRbp), rVmFp);

  emitRB(a, RBTypeFuncExit, func->fullName()->data());
  if (emitSavedRIPReturn) {
    a. ret();
    translator_not_reached(a);
    return 0;
  }
  return sizeof(ActRec) + cellsToBytes(nLocalCells-1);
}

void
TranslatorX64::translateNativeImpl(const Tracelet& t,
                                   const NormalizedInstruction& ni) {
  /*
   * We assume that NativeImpl is the only instruction in the trace,
   * and the only instruction for the implementation of the function.
   */
  assert(ni.stackOff == 0);
  assert(m_regMap.empty());
  emitNativeImpl(curFunc(), true);
}

// emitClsLocalIndex --
// emitStringToClass --
// emitStringToKnownClass --
// emitObjToClass --
// emitClsAndPals --
//   Helpers for AGetC/AGetL.

const int kEmitClsLocalIdx = 0;

/*
 * Determine if the class is defined, and fatal if not.
 * If reg is not noreg, return the Class* in it
 * If we can statically prove that the class is defined,
 * all checks are omitted (eg its a parent of the current,
 * fixed, context).
 */
void
TranslatorX64::emitKnownClassCheck(const NormalizedInstruction& i,
                                   const StringData* clsName,
                                   RegNumber reg) {
  using namespace TargetCache;
  assert(clsName);
  Class* klass = Unit::lookupUniqueClass(clsName);
  bool guarded = false;
  if (klass) {
    guarded = i.guardedCls;
    if (!guarded) {
      Class *ctx = curFunc()->cls();
      if (ctx && ctx->classof(klass)) {
        guarded = true;
      }
    }
  }
  if (guarded) {
    if (reg != reg::noreg) {
      emitImmReg(a, (uint64_t)klass, r64(reg));
    }
  } else {
    Stats::emitInc(a, Stats::TgtCache_KnownClsHit);
    CacheHandle ch = allocKnownClass(clsName);
    if (reg == reg::noreg) {
      a.          cmp_imm32_disp_reg32(0, ch, rVmTl);
    } else {
      a.          load_reg64_disp_reg64(rVmTl, ch, reg);
      a.          test_reg64_reg64(reg, reg);
    }
    {
      UnlikelyIfBlock ifNull(CC_Z, a, astubs);
      ScratchReg clsPtr(m_regMap);
      astubs.   lea_reg64_disp_reg64(rVmTl, ch, r(clsPtr));
      if (false) { // typecheck
        Class** cache = nullptr;
        UNUSED Class* ret =
          TargetCache::lookupKnownClass<false>(cache, clsName, true);
      }
      // We're only passing two arguments to lookupKnownClass because
      // the third is ignored in the checkOnly == false case
      EMIT_CALL(astubs, ((TargetCache::lookupKnownClass_func_t)
                         TargetCache::lookupKnownClass<false>),
                R(clsPtr), IMM((uintptr_t)clsName));
      recordReentrantStubCall(i);
      if (reg != reg::noreg) {
        emitMovRegReg(astubs, rax, PhysReg(reg));
      }
    }
  }
}

void
TranslatorX64::emitStringToKnownClass(const NormalizedInstruction& i,
                                      const StringData* clsName) {
  ScratchReg cls(m_regMap);
  emitKnownClassCheck(i, clsName, r(cls));
  m_regMap.bindScratch(cls, i.outStack->location, KindOfClass, RegInfo::DIRTY);
}

void
TranslatorX64::emitStringToClass(const NormalizedInstruction& i) {
  using namespace TargetCache;
  if (!i.inputs[kEmitClsLocalIdx]->rtt.valueString()) {
    // Handle the case where we don't know the name of the class
    // at translation time
    const Location& in = i.inputs[kEmitClsLocalIdx]->location;
    const Location& out = i.outStack->location;
    CacheHandle ch = ClassCache::alloc();
    if (false) {
      StringData *name = nullptr;
      const UNUSED Class* cls = ClassCache::lookup(ch, name);
    }
    TRACE(1, "ClassCache @ %d\n", int(ch));
    if (i.inputs[kEmitClsLocalIdx]->rtt.isRef()) {
        EMIT_CALL(a, ClassCache::lookup,
                   IMM(ch),
                   DEREF(in));
    } else {
        EMIT_CALL(a, ClassCache::lookup,
                   IMM(ch),
                   V(in));
    }
    recordReentrantCall(i);
    m_regMap.bind(rax, out, KindOfClass, RegInfo::DIRTY);
    return;
  }
  // We know the name of the class at translation time; use the
  // target cache associated with the name of the class
  const StringData* clsName = i.inputs[kEmitClsLocalIdx]->rtt.valueString();
  emitStringToKnownClass(i, clsName);
}

void
TranslatorX64::emitObjToClass(const NormalizedInstruction& i) {
  m_regMap.allocOutputRegs(i);
  const Location& in = i.inputs[kEmitClsLocalIdx]->location;
  const Location& out = i.outStack->location;
  PhysReg src = getReg(in);
  ScratchReg tmp(m_regMap);
  if (i.inputs[kEmitClsLocalIdx]->rtt.isRef()) {
    emitDerefRef(a, src, r(tmp));
    src = r(tmp);
  }
  assert(i.outStack->valueType() == KindOfClass);
  a.   load_reg64_disp_reg64(src, ObjectData::getVMClassOffset(), getReg(out));
}

void
TranslatorX64::emitClsAndPals(const NormalizedInstruction& ni) {
  if (ni.inputs[kEmitClsLocalIdx]->isString()) {
    emitStringToClass(ni);
  } else {
    emitObjToClass(ni);
  }
}

void
TranslatorX64::analyzeAGetC(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(i.outStack && !i.outLocal);
  assert(i.outStack->valueType() == KindOfClass);
  const RuntimeType& rtt = i.inputs[0]->rtt;
  assert(!rtt.isRef());
  i.m_txFlags = supportedPlan(rtt.isString() ||
                              rtt.valueType() == KindOfObject);
  if (rtt.isString() && rtt.valueString()) i.manuallyAllocInputs = true;
}

void TranslatorX64::translateAGetC(const Tracelet& t,
                                   const NormalizedInstruction& ni) {
  if (ni.outStack) {
    emitClsAndPals(ni);
  }
}

void TranslatorX64::analyzeAGetL(Tracelet& t,
                                 NormalizedInstruction& ni) {
  assert(ni.inputs.size() == 1);
  assert(ni.inputs[0]->isLocal());
  const RuntimeType& rtt = ni.inputs[0]->rtt;
  ni.m_txFlags = supportedPlan(rtt.isString() ||
                               rtt.valueType() == KindOfObject);
}

void TranslatorX64::translateAGetL(const Tracelet& t,
                                   const NormalizedInstruction& ni) {
  emitClsAndPals(ni);
}

void TranslatorX64::translateSelf(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  m_regMap.allocOutputRegs(i);
  PhysReg tmp = getReg(i.outStack->location);
  assert(curFunc()->cls());
  emitImmReg(a, (int64_t)curFunc()->cls(), tmp);
}

void TranslatorX64::translateParent(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  m_regMap.allocOutputRegs(i);
  PhysReg tmp = getReg(i.outStack->location);
  assert(curFunc()->cls() && curFunc()->cls()->parent());
  emitImmReg(a, (int64_t)curFunc()->cls()->parent(), tmp);
}

void TranslatorX64::analyzeSelf(Tracelet& t,NormalizedInstruction& i) {
  Class* clss = curClass();
  if (clss == nullptr) {
    i.m_txFlags = Interp;
    return;
  }
  i.m_txFlags = Supported;
}

void TranslatorX64::analyzeParent(Tracelet& t,NormalizedInstruction& i) {
  Class* clss = curClass();
  if (clss == nullptr) {
    i.m_txFlags = Interp;
    return;
  }
  if (clss->parent() == nullptr) {
    // clss has no parent; interpret to throw fatal
    i.m_txFlags = Interp;
    return;
  }
  i.m_txFlags = Supported;
}

void TranslatorX64::translateDup(const Tracelet& t,
                                 const NormalizedInstruction& ni) {
  assert(ni.inputs.size() == 1);
  assert(ni.outStack);
  assert(!ni.inputs[0]->rtt.isRef());
  m_regMap.allocOutputRegs(ni);
  PhysReg outR = getReg(ni.outStack->location);
  emitMovRegReg(a, getReg(ni.inputs[0]->location), outR);
  emitIncRef(outR, ni.inputs[0]->outerType());
}

typedef std::map<int, int> ParamMap;
/*
 * mapContParams determines if every named local in origFunc has a
 * corresponding named local in genFunc. If this step succeeds and
 * there's no VarEnv at runtime, the continuation's variables can be
 * filled completely inline in the TC (assuming there aren't too
 * many).
 */
bool TranslatorX64::mapContParams(ParamMap& map,
                                  const Func* origFunc, const Func* genFunc) {
  const StringData* const* varNames = origFunc->localNames();
  for (Id i = 0; i < origFunc->numNamedLocals(); ++i) {
    Id id = genFunc->lookupVarId(varNames[i]);
    if (id != kInvalidId) {
      map[i] = id;
    } else {
      return false;
    }
  }
  return true;
}

void TranslatorX64::emitCallFillCont(X64Assembler& a,
                                     const Func* orig,
                                     const Func* gen) {
  if (false) {
    ActRec* fp = nullptr;
    c_Continuation *cont = nullptr;
    cont =
      VMExecutionContext::fillContinuationVars(fp, orig, gen, cont);
  }
  EMIT_CALL(a,
             VMExecutionContext::fillContinuationVars,
             R(rVmFp),
             IMM((intptr_t)orig),
             IMM((intptr_t)gen),
             R(rax));
}

void TranslatorX64::translateCreateCont(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  bool getArgs = i.imm[0].u_IVA;
  const StringData* genName = curUnit()->lookupLitstrId(i.imm[1].u_SA);
  const Func* origFunc = curFunc();
  const Func* genFunc = origFunc->getGeneratorBody(genName);

  if (false) {
    ActRec* fp = nullptr;
    UNUSED c_Continuation* cont =
      VMExecutionContext::createContinuation<true>(fp, getArgs, origFunc,
                                                   genFunc);
    VMExecutionContext::createContinuation<false>(fp, getArgs, origFunc,
                                                 genFunc);
  }

  // Even callee-saved regs need to be clean, because
  // createContinuation will read all locals.
  m_regMap.cleanAll();
  auto helper = origFunc->isMethod() ?
    VMExecutionContext::createContinuation<true> :
    VMExecutionContext::createContinuation<false>;
  EMIT_CALL(a,
             (TCA)helper,
             R(rVmFp),
             IMM(getArgs),
             IMM((intptr_t)origFunc),
             IMM((intptr_t)genFunc));
  ScratchReg holdRax(m_regMap, rax);

  int origLocals = origFunc->numLocals();
  int genLocals = genFunc->numLocals();
  ContParamMap params;
  if (origLocals <= kMaxInlineContLocals &&
      mapContParams(params, origFunc, genFunc)) {
    ScratchReg rScratch(m_regMap);
    a.  load_reg64_disp_reg64(rVmFp, AROFF(m_varEnv), r(rScratch));
    a.  test_reg64_reg64(r(rScratch), r(rScratch));
    DiamondReturn astubsRet;
    {
      UnlikelyIfBlock ifVarEnv(CC_NZ, a, astubs, &astubsRet);
      Stats::emitInc(astubs, Stats::Tx64_ContCreateSlow);
      emitCallFillCont(astubs, origFunc, genFunc);
    }
    // fillContinuationVars returned the continuation in rax and
    // DiamondGuard marked rax as scratch again, so it's safe to keep
    // using it
    Stats::emitInc(a, Stats::Tx64_ContCreateFast);
    static const StringData* thisStr = StringData::GetStaticString("this");
    Id thisId = kInvalidId;
    bool fillThis = origFunc->isNonClosureMethod() && !origFunc->isStatic() &&
      ((thisId = genFunc->lookupVarId(thisStr)) != kInvalidId) &&
      (origFunc->lookupVarId(thisStr) == kInvalidId);
    ScratchReg rDest(m_regMap);
    if (origLocals > 0 || fillThis) {
      a.  load_reg64_disp_reg32(rax, CONTOFF(m_localsOffset), r(rDest));
      a.  add_reg64_reg64(rax, r(rDest));
    }
    for (int i = 0; i < origLocals; ++i) {
      assert(mapContains(params, i));
      int destOff = cellsToBytes(genLocals - params[i] - 1);
      emitCopyTo(a, rVmFp, localOffset(i), r(rDest), destOff, r(rScratch));
      emitIncRefGenericRegSafe(r(rDest), destOff, r(rScratch));
    }

    // Deal with a potential $this local in the generator body
    if (fillThis) {
      assert(thisId != kInvalidId);
      a.    load_reg64_disp_reg64(rax, CONTOFF(m_obj), r(rScratch));
      a.    test_reg64_reg64(r(rScratch), r(rScratch));
      {
        JccBlock<CC_Z> ifObj(a);
        const int thisOff = cellsToBytes(genLocals - thisId - 1);
        // We don't have to check for a static refcount since we
        // know it's an Object
        a.  incl(r(rScratch)[FAST_REFCOUNT_OFFSET]);
        a.  storeq(r(rScratch), r(rDest)[thisOff + TVOFF(m_data)]);
        emitStoreTVType(a, KindOfObject, r(rDest)[thisOff + TVOFF(m_type)]);
      }
    }
  } else {
    Stats::emitInc(a, Stats::Tx64_ContCreateSlow);
    emitCallFillCont(a, origFunc, genFunc);
  }
  m_regMap.bindScratch(holdRax, i.outStack->location, KindOfObject,
                       RegInfo::DIRTY);
}

void TranslatorX64::translateUnpackCont(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  m_regMap.allocOutputRegs(i);
  const int contIdx = 0;
  assert(i.inputs.size() == 1);
  assert(i.inputs[contIdx]->location == Location(Location::Local, 0));
  assert(i.outStack->outerType() == KindOfInt64);

  ScratchReg rScratch(m_regMap);
  a.    load_reg64_disp_reg64(rVmFp, AROFF(m_varEnv), r(rScratch));
  a.    test_reg64_reg64(r(rScratch), r(rScratch));
  {
    UnlikelyIfBlock hasVars(CC_NZ, a, astubs);
    Stats::emitInc(astubs, Stats::Tx64_ContUnpackSlow);
    if (false) {
      ActRec* fp = nullptr;
      VMExecutionContext::unpackContVarEnvLinkage(fp);
    }
    EMIT_CALL(astubs,
              VMExecutionContext::unpackContVarEnvLinkage,
              R(rVmFp));
    // helper can't reenter
  }
  Stats::emitInc(a, Stats::Tx64_ContUnpackFast);

  PhysReg rCont = getReg(i.inputs[contIdx]->location);
  PhysReg rLabel = getReg(i.outStack->location);

  a.    load_reg64_disp_reg64(rCont, CONTOFF(m_label), rLabel);
}

void TranslatorX64::emitCallPack(X64Assembler& a,
                                 const NormalizedInstruction& i) {
  if (false) {
    ActRec* fp = nullptr;
    VMExecutionContext::packContVarEnvLinkage(fp);
  }
  EMIT_CALL(a,
            VMExecutionContext::packContVarEnvLinkage,
            R(rVmFp));
  recordCall(a, i);
}

void TranslatorX64::translatePackCont(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  const int valIdx = 0;
  const int contIdx = 1;

  ScratchReg rScratch(m_regMap);
  a.    load_reg64_disp_reg64(rVmFp, AROFF(m_varEnv), r(rScratch));
  a.    test_reg64_reg64(r(rScratch), r(rScratch));
  {
    // TODO: Task #1132976: We can probably prove that this is impossible in
    // most cases using information from hphpc
    UnlikelyIfBlock varEnv(CC_NZ, a, astubs);
    Stats::emitInc(astubs, Stats::Tx64_ContPackSlow);
    emitCallPack(astubs, i);
  }
  Stats::emitInc(a, Stats::Tx64_ContPackFast);

  PhysReg rCont = getReg(i.inputs[contIdx]->location);
  PhysReg rValue = getReg(i.inputs[valIdx]->location);

  // We're moving our reference to the value from the stack to the
  // continuation object, so we don't have to incRef or decRef
  emitTvSet(i, rValue, i.inputs[valIdx]->outerType(),
            rCont, CONTOFF(m_value), false);

  emitImmReg(a, i.imm[0].u_IVA, r(rScratch));
  a.    store_reg64_disp_reg64(r(rScratch), CONTOFF(m_label), rCont);
}

static void continuationRaiseHelper(c_Continuation* cont) {
  cont->t_raised();
  not_reached();
}

void TranslatorX64::emitContRaiseCheck(X64Assembler& a,
                                       const NormalizedInstruction& i) {
  const int contIdx = 0;
  assert(i.inputs[contIdx]->location == Location(Location::Local, 0));
  PhysReg rCont = getReg(i.inputs[contIdx]->location);
  a.    testb(0x1, rCont[CONTOFF(m_should_throw)]);
  {
    UnlikelyIfBlock ifThrow(CC_NZ, a, astubs);
    if (false) {
      c_Continuation* c = nullptr;
      continuationRaiseHelper(c);
    }
    EMIT_CALL(astubs,
               continuationRaiseHelper,
               R(rCont));
    recordReentrantStubCall(i);
    translator_not_reached(astubs);
  }
}

void TranslatorX64::translateContReceive(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  const int contIdx = 0;
  emitContRaiseCheck(a, i);
  PhysReg rCont = getReg(i.inputs[contIdx]->location);
  ScratchReg rScratch(m_regMap);
  emitLea(a, rCont, CONTOFF(m_received), r(rScratch));
  emitCopyToStack(a, i, r(rScratch), -1 * (int)sizeof(Cell));
  emitStoreUninitNull(a, CONTOFF(m_received), rCont);
}

void TranslatorX64::translateContEnter(const Tracelet& t,
                                       const NormalizedInstruction& i) {
  // We're about to execute the generator body, which uses regs
  syncOutputs(i);

  a.    loadq  (rVmFp[AROFF(m_this)], rStashedAR);
  a.    loadq  (rStashedAR[CONTOFF(m_arPtr)], rStashedAR);

  // Frame linkage.
  int32_t returnOffset = nextSrcKey(t, i).offset() - curFunc()->base();
  a.    storel (returnOffset, rStashedAR[AROFF(m_soff)]);
  a.    storeq (rVmFp, rStashedAR[AROFF(m_savedRbp)]);

  // We're between tracelets; hardcode the register
  a.    loadq  (rStashedAR[AROFF(m_func)], rax);
  a.    loadq  (rax[Func::prologueTableOff() + sizeof(TCA)], rax);

  a.    call   (rax);
}

void TranslatorX64::emitContExit() {
  emitTestSurpriseFlags(a);
  {
    UnlikelyIfBlock ifTracer(CC_NZ, a, astubs);
    astubs.mov_reg64_reg64(rVmFp, argNumToRegName[0]);
    emitCall(astubs, (TCA)&EventHook::FunctionExit, true);
    recordReentrantStubCall(*m_curNI);
  }
  a.    push  (rVmFp[AROFF(m_savedRip)]);
  a.    loadq (rVmFp[AROFF(m_savedRbp)], rVmFp);
  a.    ret   ();
}

void TranslatorX64::translateContExit(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  syncOutputs(i);
  emitContExit();
}

void TranslatorX64::translateContRetC(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  PhysReg valueReg = getReg(i.inputs[0]->location);
  PhysReg contReg = getReg(i.inputs[1]->location);
  a.    store_imm8_disp_reg(0x1, CONTOFF(m_done), contReg);

  // m_value = $1
  emitTvSet(i, valueReg, i.inputs[0]->outerType(),
            contReg, CONTOFF(m_value), false);

  // transfer control
  syncOutputs(i.stackOff - 1);
  emitContExit();
}

static void contPreNextThrowHelper(c_Continuation* c) {
  c->preNext();
  not_reached();
}

void TranslatorX64::emitContPreNext(const NormalizedInstruction& i,
                                    ScratchReg& rCont) {
  const Offset doneOffset = CONTOFF(m_done);
  static_assert((doneOffset + 1) == CONTOFF(m_running),
                "m_done should immediately precede m_running");
  // Check m_done and m_running at the same time
  a.    test_imm32_disp_reg32(0x0101, doneOffset, r(rCont));
  {
    UnlikelyIfBlock ifThrow(CC_NZ, a, astubs);
    EMIT_CALL(astubs, contPreNextThrowHelper, R(rCont));
    recordReentrantStubCall(i);
    translator_not_reached(astubs);
  }

  // ++m_index
  a.    incq(r(rCont)[CONTOFF(m_index)]);
  // m_running = true
  a.    store_imm8_disp_reg(0x1, CONTOFF(m_running), r(rCont));
}

void TranslatorX64::translateContNext(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  ScratchReg rCont(m_regMap);
  a.    load_reg64_disp_reg64(rVmFp, AROFF(m_this), r(rCont));
  emitContPreNext(i, rCont);

  // m_received.setNull()
  emitTvSet(i, InvalidReg, KindOfNull, r(rCont), CONTOFF(m_received), false);
}

static void contNextCheckThrowHelper(c_Continuation* cont) {
  cont->startedCheck();
  not_reached();
}

void TranslatorX64::emitContStartedCheck(const NormalizedInstruction& i,
                                         ScratchReg& cont) {
  // if (m_index < 0)
  a.    cmpq (0x0, r(cont)[CONTOFF(m_index)]);
  {
    UnlikelyIfBlock whoops(CC_L, a, astubs);
    EMIT_CALL(astubs, contNextCheckThrowHelper, r(cont));
    recordReentrantStubCall(i);
    translator_not_reached(astubs);
  }
}

template<bool raise>
void TranslatorX64::translateContSendImpl(const NormalizedInstruction& i) {
  const int valIdx = 0;
  assert(i.inputs[valIdx]->location == Location(Location::Local, 0));

  ScratchReg rCont(m_regMap);
  a.    load_reg64_disp_reg64(rVmFp, AROFF(m_this), r(rCont));
  emitContStartedCheck(i, rCont);
  emitContPreNext(i, rCont);

  // m_received = value
  PhysReg valReg = getReg(i.inputs[valIdx]->location);
  DataType valType = i.inputs[valIdx]->outerType();
  emitTvSet(i, valReg, valType, r(rCont), CONTOFF(m_received), true);

  // m_should_throw = true (maybe)
  if (raise) {
    a.  store_imm8_disp_reg(0x1, CONTOFF(m_should_throw), r(rCont));
  }
}

void TranslatorX64::translateContSend(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  translateContSendImpl<false>(i);
}

void TranslatorX64::translateContRaise(const Tracelet& t,
                                       const NormalizedInstruction& i) {
  translateContSendImpl<true>(i);
}

void TranslatorX64::translateContValid(const Tracelet& t,
                                       const NormalizedInstruction& i) {
  ScratchReg rCont(m_regMap);
  a.    loadq (rVmFp[AROFF(m_this)], r64(rCont));

  m_regMap.allocOutputRegs(i);
  PhysReg validReg = getReg(i.outStack->location);
  // !m_done
  a.    loadzbl (r(rCont)[CONTOFF(m_done)], r32(validReg));
  a.    xorl (0x1, r32(validReg));
}

void TranslatorX64::translateContCurrent(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  ScratchReg rCont(m_regMap);
  a.   load_reg64_disp_reg64(rVmFp, AROFF(m_this), r(rCont));
  emitContStartedCheck(i, rCont);

  emitLea(a, r(rCont), CONTOFF(m_value), r(rCont));
  emitIncRefGeneric(r(rCont), 0);
  emitCopyToStack(a, i, r(rCont), -1 * (int)sizeof(Cell));
}

void TranslatorX64::translateContStopped(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  ScratchReg rCont(m_regMap);
  a.    load_reg64_disp_reg64(rVmFp, AROFF(m_this), r(rCont));
  a.    store_imm8_disp_reg(0x0, CONTOFF(m_running), r(rCont));
}

void TranslatorX64::translateContHandle(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  // Always interpreted
  not_reached();
}

void TranslatorX64::analyzeStrlen(Tracelet& t,
                                  NormalizedInstruction& i) {
  switch (i.inputs[0]->rtt.valueType()) {
    NULLCASE() :
    case KindOfBoolean:
      i.m_txFlags = Native;
      break;
    STRINGCASE() :
      // May have to destroy a StringData, but can't reenter
      i.m_txFlags = Simple;
      break;
    case KindOfArray:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfObject:
      i.m_txFlags = Interp;
      break;
    default:
      not_reached();
  }
}

void TranslatorX64::translateStrlen(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  PhysReg rInput = getReg(i.inputs[0]->location);
  DataType inType = i.inputs[0]->rtt.valueType();

  switch (inType) {
    NULLCASE(): {
      m_regMap.allocOutputRegs(i);
      PhysReg rOutput = getReg(i.outStack->location);
      a.  xor_reg64_reg64(rOutput, rOutput);
      break;
    }
    case KindOfBoolean:
      m_regMap.allocOutputRegs(i);
      // Nothing. strlen(true) == 1, strlen(false) == 0
      break;
    STRINGCASE(): {
      ScratchReg rScratch(m_regMap);
      a.  load_reg64_disp_reg32(rInput, StringData::sizeOffset(), r(rScratch));
      emitDecRef(a, i, rInput, inType);
      m_regMap.bindScratch(rScratch, i.outStack->location, KindOfInt64,
                           RegInfo::DIRTY);
      assert(m_regMap.regIsFree(rInput));
      break;
    }
    case KindOfArray:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfObject:
    default:
      not_reached();
  }
}

void TranslatorX64::translateIncStat(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  int32_t counter = i.imm[0].u_IVA;
  int32_t value = i.imm[1].u_IVA;
  Stats::emitInc(a, Stats::StatCounter(counter), value);
}

static void analyzeClassExistsImpl(NormalizedInstruction& i) {
  const int nameIdx = 1;
  const int autoIdx = 0;
  assert(!i.inputs[nameIdx]->isRef() && !i.inputs[autoIdx]->isRef());
  i.m_txFlags = supportedPlan(i.inputs[nameIdx]->isString() &&
                              i.inputs[autoIdx]->isBoolean());
  i.fuseBranch = (i.m_txFlags & Supported) &&
    i.inputs[nameIdx]->rtt.valueString() &&
    i.inputs[autoIdx]->rtt.valueBoolean() != RuntimeType::UnknownBool;
}

void TranslatorX64::analyzeClassExists(Tracelet& t,
                                       NormalizedInstruction& i) {
  analyzeClassExistsImpl(i);
}

void TranslatorX64::analyzeInterfaceExists(Tracelet& t,
                                           NormalizedInstruction& i) {
  analyzeClassExistsImpl(i);
}

void TranslatorX64::analyzeTraitExists(Tracelet& t,
                                       NormalizedInstruction& i) {
  analyzeClassExistsImpl(i);
}

static int64_t classExistsSlow(const StringData* name, bool autoload,
                             Attr typeAttr) {
  bool ret = Unit::classExists(name, autoload, typeAttr);
  // XXX: do we need to decref this during an exception?
  decRefStr(const_cast<StringData*>(name));
  return ret;
}

void TranslatorX64::translateClassExistsImpl(const Tracelet& t,
                                             const NormalizedInstruction& i,
                                             Attr typeAttr) {
  const int nameIdx = 1;
  const int autoIdx = 0;
  const StringData* name = i.inputs[nameIdx]->rtt.valueString();
  assert(IMPLIES(name, name->isStatic()));
  const int autoload = i.inputs[autoIdx]->rtt.valueBoolean();

  ScratchReg scratch(m_regMap);
  if (name != nullptr && autoload != RuntimeType::UnknownBool) {
    assert(i.fuseBranch);
    const Attr attrNotClass = Attr(AttrTrait | AttrInterface);
    const bool isClass = typeAttr == AttrNone;
    using namespace TargetCache;
    Stats::emitInc(a, Stats::Tx64_ClassExistsFast);
    CacheHandle ch = allocKnownClass(name);

    {
      DiamondReturn astubsRet;
      a.  load_reg64_disp_reg64(rVmTl, ch, r(scratch));
      a.  test_reg64_reg64(r(scratch), r(scratch));
      if (autoload) {
        UnlikelyIfBlock ifNull(CC_Z, a, astubs, &astubsRet);
        if (false) {
          Class** c = nullptr;
          UNUSED Class* ret = lookupKnownClass<true>(c, name, false);
        }
        Stats::emitInc(astubs, Stats::TgtCache_ClassExistsMiss);
        // If the class exists after autoloading, the helper will
        // return the Class's flags. Otherwise, it will return a set
        // of flags such that our flag check at the join point below
        // will fail.
        EMIT_CALL(astubs, (lookupKnownClass_func_t)lookupKnownClass<true>,
                   RPLUS(rVmTl, ch),
                   IMM((uintptr_t)name),
                   IMM(isClass));
        recordReentrantStubCall(i);
        emitMovRegReg(astubs, rax, r(scratch));
      } else {
        UnlikelyIfBlock ifNull(CC_Z, a, astubs, &astubsRet);
        // This isn't really a traditional slow path, count as a hit
        Stats::emitInc(astubs, Stats::TgtCache_ClassExistsHit);
        // Provide flags so the check back in a fails
        emitImmReg(astubs, isClass ? attrNotClass : AttrNone, r(scratch));
      }
      // If we don't take the slow/NULL path, load the Class's attrs
      // into *scratch to prepare for the flag check.
      Stats::emitInc(a, Stats::TgtCache_ClassExistsHit);
      a.  load_reg64_disp_reg64(r(scratch), Class::preClassOff(),
                                r(scratch));
      a.  load_reg64_disp_reg32(r(scratch), PreClass::attrsOffset(),
                                r(scratch));
    }

    if (i.changesPC) {
      fuseBranchSync(t, i);
    }
    prepareForTestAndSmash(kTestImmRegLen, kAlignJccAndJmp);
    a.    test_imm32_reg32(isClass ? attrNotClass : typeAttr, r(scratch));
    ConditionCode cc = isClass ? CC_Z : CC_NZ;
    if (i.changesPC) {
      fuseBranchAfterBool(t, i, cc);
    } else {
      a.  setcc(cc, rbyte(scratch));
      a.  movzbl(rbyte(scratch), r32(scratch));
      m_regMap.bindScratch(scratch, i.outStack->location, KindOfBoolean,
                           RegInfo::DIRTY);
    }
  } else {
    assert(!i.fuseBranch);
    Stats::emitInc(a, Stats::Tx64_ClassExistsSlow);
    if (false) {
      UNUSED bool ret = false;
      ret = classExistsSlow(name, ret, typeAttr);
    }
    EMIT_CALL(a, classExistsSlow,
               V(i.inputs[nameIdx]->location),
               V(i.inputs[autoIdx]->location),
               IMM(typeAttr));
    recordReentrantCall(i);
    // Our helper decrefs the string
    m_regMap.bind(rax, i.outStack->location, KindOfBoolean, RegInfo::DIRTY);
  }
}

void TranslatorX64::translateClassExists(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  translateClassExistsImpl(t, i, AttrNone);
}

void TranslatorX64::translateInterfaceExists(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  translateClassExistsImpl(t, i, AttrInterface);
}

void TranslatorX64::translateTraitExists(const Tracelet& t,
                                         const NormalizedInstruction& i) {
  translateClassExistsImpl(t, i, AttrTrait);
}

// Helper function for static property access.  This function emits code
// which leaves a pointer to the static property for clsInput::$propInput in
// register scr. We destroy scr early on, yet do not consume inputs until
// later, so scr must not alias an input register.  This also handles
// the decref for the case where prop is not a static string.
void TranslatorX64::emitStaticPropInlineLookup(const NormalizedInstruction& i,
                                               int classInputIdx,
                                               const DynLocation& propInput,
                                               PhysReg scr) {
  auto const& clsInput = *i.inputs[classInputIdx];
  const Class* cls = clsInput.rtt.valueClass();
  const StringData* propName = propInput.rtt.valueString();
  using namespace TargetCache;
  CacheHandle ch;

  assert(cls && propName);
  // Use the uniquely known cls / prop to generate a single cache per prop
  const StringData* clsName = cls->preClass()->name();
  string sds(Util::toLower(clsName->data()) + ":" +
             string(propName->data(), propName->size()));
  StackStringData sd(sds.c_str(), sds.size(), AttachLiteral);
  ch = SPropCache::alloc(&sd);
  SKTRACE(1, i.source, "SPropInlineLookup %s %d\n", sd.data(), int(ch));

  Stats::emitInc(a, Stats::TgtCache_SPropHit);

  // For the simple case of statically known class and prop name, we inline
  // the target cache lookup, and outline the miss case.
  // Load the TV pointer out of the thread-private tl_targetCaches.
  BOOST_STATIC_ASSERT((offsetof(SPropCache, m_tv) == 0));
  a.   load_reg64_disp_reg64(rVmTl, ch, scr);
  a.   test_reg64_reg64(scr, scr);

  // Call the slow path.
  {
    UnlikelyIfBlock shucks(CC_Z, a, astubs);

    // Precondition for this lookup - we don't need to pass the preClass,
    // as we only translate in class lookups.
    assert(cls == curFunc()->cls());
    if (false) { // typecheck
      StringData *data = nullptr;
      SPropCache::lookup(ch, cls, data);
    }

    std::vector<int> args(i.inputs.size(), ArgDontAllocate);
    args[classInputIdx] = 1;
    allocInputsForCall(i, &args[0]);

    EMIT_CALL(astubs, (TCA)SPropCache::lookup,
               IMM(ch), V(clsInput.location), IMM(uint64_t(propName)));
    recordReentrantStubCall(i);
    emitMovRegReg(astubs, rax, scr);

    // We're consuming the name as input, but it is static, no decref needed
    assert(propInput.rtt.valueString()->isStatic());
    // astubs.  jmp(a.code.frontier); -- implicit
  }
}

void TranslatorX64::analyzeCGetS(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  assert(i.inputs[0]->valueType() == KindOfClass);
  assert(i.outStack);
  const Class* cls = i.inputs[0]->rtt.valueClass();
  const StringData* propName = i.inputs[1]->rtt.valueString();
  i.m_txFlags = supportedPlan(cls && propName && curFunc()->cls() == cls);
  i.manuallyAllocInputs = true;
}

void TranslatorX64::translateCGetS(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  const int kClassIdx = 0;
  const int kPropIdx = 1;

  ScratchReg sprop(m_regMap);
  emitStaticPropInlineLookup(i, kClassIdx, *i.inputs[kPropIdx], r(sprop));
  emitDerefIfVariant(a, r(sprop));
  emitIncRefGeneric(r(sprop), 0);
  // Finally copy the thing to the stack
  int stackDest = 2 * sizeof(Cell) - sizeof(Cell); // popped - pushed
  emitCopyToStack(a, i, r(sprop), stackDest);
}

void TranslatorX64::analyzeSetS(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 3);
  assert(i.inputs[1]->valueType() == KindOfClass);
  assert(i.outStack);
  const Class* cls = i.inputs[1]->rtt.valueClass();
  const StringData* propName = i.inputs[2]->rtt.valueString();
  // Might be able to broaden this: if cls is an ancestor of the current context,
  // the context is Fixed, and the property is not private
  // Also if the m_hoistable in cls is set to AlwaysHoistable, defined in
  // the same unit as context, and the property is public
  i.m_txFlags = supportedPlan(cls && propName && curFunc()->cls() == cls);
  i.manuallyAllocInputs = true;
}

void TranslatorX64::translateSetS(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  const int kClassIdx = 1;

  ScratchReg sprop(m_regMap);
  const RuntimeType& rhsType = i.inputs[0]->rtt;
  emitStaticPropInlineLookup(i, kClassIdx, *i.inputs[2], r(sprop));

  assert(m_regMap.getInfo(r(sprop))->m_state == RegInfo::SCRATCH);
  assert(!rhsType.isRef());

  m_regMap.allocInputReg(i, 0);
  m_regMap.allocOutputRegs(i);
  PhysReg rhsReg = getReg(i.inputs[0]->location);
  PhysReg outReg = getReg(i.outStack->location);
  emitTvSet(i, rhsReg, rhsType.outerType(), r(sprop));
  assert(i.inputs[2]->location == i.outStack->location);
  emitMovRegReg(a, rhsReg, outReg);
}

void TranslatorX64::analyzeSetG(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  i.m_txFlags = supportedPlan(
    i.inputs[1]->isString() &&
    !i.inputs[0]->isRef()
  );
  if (i.m_txFlags) i.manuallyAllocInputs = true;
}

void TranslatorX64::translateSetG(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  assert(i.outStack && !i.outLocal);
  assert(i.inputs.size() == 2);
  assert(i.inputs[1]->isString());
  assert(i.inputs[1]->location == i.outStack->location);

  const DataType type = i.inputs[0]->rtt.outerType();

  /*
   * Grab the global from the target cache; rax will get a pointer to
   * the TypedValue in the globals array, maybe newly created as a
   * null.
   */
  emitGetGlobal(i, 1, true /* allowCreate */);
  ScratchReg raxSaver(m_regMap, rax);
  m_regMap.allocInputReg(i, 0);
  PhysReg src = getReg(i.inputs[0]->location);
  m_regMap.allocOutputRegs(i);
  PhysReg out = getReg(i.outStack->location);

  emitTvSet(i, src, type, rax);
  emitMovRegReg(a, src, out);
}

static TypedValue* lookupGlobal(StringData* name) {
  VarEnv* ve = g_vmContext->m_globalVarEnv;
  TypedValue* r = ve->lookup(name);
  // If the global didn't exist, we need to leave name un-decref'd for
  // the caller to raise warnings.
  if (r) {
    decRefStr(name);
    if (r->m_type == KindOfRef) r = r->m_data.pref->tv();
  }
  return r;
}

static TypedValue* lookupAddGlobal(StringData* name) {
  VarEnv* ve = g_vmContext->m_globalVarEnv;
  TypedValue* r = ve->lookupAdd(name);
  if (r->m_type == KindOfRef) r = r->m_data.pref->tv();
  decRefStr(name);
  return r;
}

/*
 * Look up a global in the TargetCache with the name
 * i.inputs[nameIdx].  If `allowCreate' is true, also creates it.  If
 * we don't create the global, the input name is not decref'd yet.
 */
void
TranslatorX64::emitGetGlobal(const NormalizedInstruction& i, int nameIdx,
    bool allowCreate) {
  using namespace TargetCache;
  assert(i.inputs.size() > size_t(nameIdx));
  assert(i.inputs[nameIdx]->isString());

  const StringData *maybeName = i.inputs[nameIdx]->rtt.valueString();
  if (!maybeName) {
    m_regMap.allocInputReg(i, nameIdx, argNumToRegName[0]);
    // Always do a lookup when there's no statically-known name.
    // There's not much we can really cache here right now anyway.
    EMIT_CALL(a, allowCreate ? lookupAddGlobal : lookupGlobal,
                  V(i.inputs[nameIdx]->location));
    recordCall(i);
    return;
  }

  CacheHandle ch = GlobalCache::alloc(maybeName);
  if (false) { // typecheck
    StringData* UNUSED key = nullptr;
    TypedValue* UNUSED glob = GlobalCache::lookup(ch, key);
    TypedValue* UNUSED glob2 = GlobalCache::lookupCreate(ch, key);
  }
  SKTRACE(1, i.source, "ch %d\n", ch);
  EMIT_CALL(a, allowCreate ? GlobalCache::lookupCreate
                           : GlobalCache::lookup,
            IMM(ch),
            IMM((uint64_t)maybeName));
  recordCall(i);
}

static bool
isSupportedInstrCGetG(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  return (i.inputs[0]->rtt.isString());
}

void
TranslatorX64::analyzeCGetG(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = simplePlan(isSupportedInstrCGetG(i));
  if (i.m_txFlags) i.manuallyAllocInputs = true;
}

void
TranslatorX64::translateCGetG(const Tracelet& t,
                              const NormalizedInstruction& i) {
  assert(i.outStack && !i.outLocal);
  assert(i.inputs.size() == 1);
  assert(i.inputs[0]->isString());

  emitGetGlobal(i, 0, false /* allowCreate */);
  ScratchReg raxHolder(m_regMap, rax);

  // If non-null, rax now points to the in-memory location of the
  // object of unknown type. lookup() has already decref'd the name.
  a.  test_reg64_reg64(rax, rax);
  DiamondReturn astubsRet;
  {
    UnlikelyIfBlock ifNotRax(CC_Z, a, astubs, &astubsRet);
    if (!i.inputs[0]->rtt.valueString()) {
      m_regMap.allocInputReg(i, 0);
      PhysReg reg = getReg(i.inputs[0]->location);
      emitDecRef(astubs, i, reg, BitwiseKindOfString);
    }
    // TODO: if (MoreWarnings) raise a undefined variable warning.
    // (Note: when changing this remember to change the Simple flag to
    // Supported in analyze.)
    emitStoreNull(astubs, vstackOffset(i, 0), rVmSp);
    m_regMap.invalidate(i.outStack->location);
  }

  emitCopyToStack(a, i, rax, 0);
  emitIncRefGeneric(rax, 0);
  m_regMap.invalidate(i.outStack->location);
}

void TranslatorX64::analyzeFPassL(Tracelet& t,
                                  NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    analyzeVGetL(t, ni);
  } else {
    analyzeCGetL(t, ni);
  }
}

void TranslatorX64::translateFPassL(const Tracelet& t,
                                    const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    translateVGetL(t, ni);
  } else {
    translateCGetL(t, ni);
  }
}

void TranslatorX64::analyzeFPassS(Tracelet& t,
                                  NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    // We need a VGetS translation.
    ni.m_txFlags = Interp;
  } else {
    analyzeCGetS(t, ni);
  }
}

void TranslatorX64::translateFPassS(const Tracelet& t,
                                    const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    assert(false);
  } else {
    translateCGetS(t, ni);
  }
}

void TranslatorX64::analyzeFPassG(Tracelet& t,
                                  NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    analyzeVGetG(t, ni);
  } else {
    analyzeCGetG(t, ni);
  }
}

void TranslatorX64::translateFPassG(const Tracelet& t,
                                    const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    translateVGetG(t, ni);
  } else {
    translateCGetG(t, ni);
  }
}

void TranslatorX64::analyzeCheckTypeOp(Tracelet& t,
                                       NormalizedInstruction& ni) {
  assert(ni.inputs.size() == 1);

  if (ni.op() == OpIsObjectL || ni.op() == OpIsObjectC) {
    // is_object is weird because it's supposed to return false for
    // things where ObjectData::isResource() is true.  For now we only
    // translate when it is not an object.
    if (ni.inputs[0]->valueType() == KindOfObject) {
      ni.m_txFlags = Interp;
      return;
    }
  }

  if (ni.inputs[0]->isLocal()) {
    ni.manuallyAllocInputs = true;
    if (ni.op() != OpIssetL && ni.inputs[0]->rtt.isUninit()) {
      ni.m_txFlags = Supported;
    } else {
      ni.m_txFlags = Native;
    }
    return;
  }

  ni.m_txFlags = planHingesOnRefcounting(ni.inputs[0]->valueType());
}

static bool checkTypeHelper(Opcode op, DataType dt) {
  switch (op) {
  case OpIssetL:    return !IS_NULL_TYPE(dt);
  case OpIsNullL:   case OpIsNullC:   return IS_NULL_TYPE(dt);
  case OpIsStringL: case OpIsStringC: return IS_STRING_TYPE(dt);
  case OpIsArrayL:  case OpIsArrayC:  return IS_ARRAY_TYPE(dt);
  case OpIsIntL:    case OpIsIntC:    return IS_INT_TYPE(dt);
  case OpIsBoolL:   case OpIsBoolC:   return IS_BOOL_TYPE(dt);
  case OpIsDoubleL: case OpIsDoubleC: return IS_DOUBLE_TYPE(dt);

  case OpIsObjectL: case OpIsObjectC:
    // Note: this is because we refused to translate if it was
    // actually an object for now.  (We'd need to emit some kind of
    // call to ObjectData::isResource or something.)
    return 0;
  }
  assert(false);
  NOT_REACHED();
}

static void warnNullThis() { raise_notice(Strings::WARN_NULL_THIS); }

void
TranslatorX64::translateCheckTypeOp(const Tracelet& t,
                                    const NormalizedInstruction& ni) {
  assert(ni.inputs.size() == 1);
  assert(ni.outStack);

  bool isType;

  if (ni.wasGroupedWith(OpThis, OpBareThis)) {
    assert(ni.op() == OpIsNullC);
    if (ni.prev->op() == OpThis) {
      isType = false;
    } else {
      if (ni.changesPC) {
        fuseBranchSync(t, ni);
        a.   testb(1, rVmFp[AROFF(m_this)]);
        if (ni.prev->imm[0].u_OA) {
          UnlikelyIfBlock nullThis(CC_NZ, a, astubs);
          EMIT_CALL(astubs, warnNullThis);
          recordReentrantStubCall(ni);
          nullThis.reconcileEarly();
          astubs.testb(1, rVmFp[AROFF(m_this)]);
        }
        fuseBranchAfterBool(t, ni, ni.invertCond ? CC_Z : CC_NZ);
      } else {
        m_regMap.allocOutputRegs(ni);
        PhysReg res = getReg(ni.outStack->location);
        a.   testb(1, rVmFp[AROFF(m_this)]);
        a.   setcc(ni.invertCond ? CC_Z : CC_NZ, rbyte(res));
        if (ni.prev->imm[0].u_OA) {
          UnlikelyIfBlock nullThis(CC_NZ, a, astubs);
          EMIT_CALL(astubs, warnNullThis);
          recordReentrantStubCall(ni);
        }
        a.   movzbl (rbyte(res), r32(res));
      }
      return;
    }
  } else {
    const DataType dt    = ni.inputs[0]->valueType();
    const bool isLocalOp = ni.inputs[0]->isLocal();

    isType = checkTypeHelper(ni.op(), dt) != ni.invertCond;
    if (!isLocalOp) {
      emitDecRef(ni, getReg(ni.inputs[0]->location), dt);
    }
    if (isLocalOp &&
        ni.op() != OpIssetL &&
        ni.inputs[0]->rtt.isUninit()) {
      const StringData* name = local_name(ni.inputs[0]->location);
      assert(name->isStatic());
      EMIT_CALL(a, raiseUndefVariable, IMM((uintptr_t)name));
      recordReentrantCall(ni);
    }
  }

  m_regMap.allocOutputRegs(ni);
  if (ni.changesPC) {
    // Don't bother driving an output reg. Just take the branch
    // where it leads.
    Stats::emitInc(a, Stats::Tx64_FusedTypeCheck);
    fuseBranchAfterStaticBool(a, t, ni, isType);
    return;
  }
  Stats::emitInc(a, Stats::Tx64_UnfusedTypeCheck);
  emitImmReg(a, isType, getReg(ni.outStack->location));
}

static void badArray() {
  throw_bad_type_exception("array_key_exists expects an array or an object; "
                           "false returned.");
}

static void badKey() {
  raise_warning("Array key should be either a string or an integer");
}

static inline int64_t ak_exist_string_helper(StringData* key, ArrayData* arr) {
  int64_t n;
  if (key->isStrictlyInteger(n)) {
    return arr->exists(n);
  }
  return arr->exists(StrNR(key));
}

static int64_t ak_exist_string(StringData* key, ArrayData* arr) {
  int64_t res = ak_exist_string_helper(key, arr);
  decRefArr(arr);
  decRefStr(key);
  return res;
}

static int64_t ak_exist_int(int64_t key, ArrayData* arr) {
  bool res = arr->exists(key);
  decRefArr(arr);
  return res;
}

static int64_t ak_exist_string_obj(StringData* key, ObjectData* obj) {
  CArrRef arr = obj->o_toArray();
  int64_t res = ak_exist_string_helper(key, arr.get());
  decRefObj(obj);
  decRefStr(key);
  return res;
}

static int64_t ak_exist_int_obj(int64_t key, ObjectData* obj) {
  CArrRef arr = obj->o_toArray();
  bool res = arr.get()->exists(key);
  decRefObj(obj);
  return res;
}

void
TranslatorX64::analyzeAKExists(Tracelet& t, NormalizedInstruction& i) {
  const int keyIx = 1;
  const int arrIx = 0;

  const DataType dta = i.inputs[arrIx]->valueType();
  const DataType dtk = i.inputs[keyIx]->valueType();

  bool reentrant = (dta != KindOfArray && dta != KindOfObject) ||
    (!IS_STRING_TYPE(dtk) && dtk != KindOfInt64 && dtk != KindOfNull);

  i.m_txFlags = reentrant ? Supported : Simple;
  i.manuallyAllocInputs = true;
}

void
TranslatorX64::translateAKExists(const Tracelet& t,
                                 const NormalizedInstruction& ni) {
  assert(ni.inputs.size() == 2);
  assert(ni.outStack);

  const int keyIx = 1;
  const int arrIx = 0;

  const DataType dta = ni.inputs[arrIx]->valueType();
  const DataType dtk = ni.inputs[keyIx]->valueType();
  TCA string_func = (TCA)ak_exist_string;
  TCA int_func = (TCA)ak_exist_int;

  int result = -1;
  int args[2];
  args[keyIx] = 0;
  args[arrIx] = 1;
  switch (dta) {
    case KindOfObject:
      string_func = (TCA)ak_exist_string_obj;
      int_func = (TCA)ak_exist_int_obj;
    case KindOfArray:
      switch (dtk) {
        case BitwiseKindOfString:
        case KindOfStaticString:
        case KindOfInt64: {
          allocInputsForCall(ni, args);
          PhysReg rk = getReg(ni.inputs[keyIx]->location);
          PhysReg ra = getReg(ni.inputs[arrIx]->location);
          m_regMap.scrubStackEntries(ni.outStack->location.offset);
          EMIT_CALL(a, dtk == KindOfInt64 ? int_func : string_func,
                    R(rk), R(ra));
          recordCall(ni);
          break;
        }
        case KindOfNull:
          if (dta == KindOfArray) {
            args[keyIx] = ArgDontAllocate;
            allocInputsForCall(ni, args);
            PhysReg ra = getReg(ni.inputs[arrIx]->location);
            m_regMap.scrubStackEntries(ni.outStack->location.offset);
            EMIT_CALL(a, string_func,
                      IMM((uint64_t)empty_string.get()), R(ra));
            recordCall(ni);
          } else {
            result = ni.invertCond;
          }
          break;
        default:
          EMIT_CALL(a, badKey);
          recordReentrantCall(ni);
          result = ni.invertCond;
          break;
      }
      break;
    default:
      EMIT_CALL(a, badArray);
      recordReentrantCall(ni);
      result = ni.invertCond;
      break;
  }

  if (result >= 0) {
    if (ni.changesPC) {
      fuseBranchAfterStaticBool(a, t, ni, result);
      return;
    } else {
      m_regMap.allocOutputRegs(ni);
      emitImmReg(a, result, getReg(ni.outStack->location));
    }
  } else {
    ScratchReg res(m_regMap, rax);
    if (ni.changesPC) {
      fuseBranchSync(t, ni);
      prepareForTestAndSmash(kTestRegRegLen, kAlignJccAndJmp);
      a.    test_reg64_reg64(r(res), r(res));
      fuseBranchAfterBool(t, ni, ni.invertCond ? CC_Z : CC_NZ);
    } else {
      if (ni.invertCond) {
        a.  xor_imm32_reg64(1, r(res));
      }
      m_regMap.bindScratch(res, ni.outStack->location, KindOfBoolean,
                           RegInfo::DIRTY);
    }
  }
}

void
TranslatorX64::analyzeSetOpL(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  const SetOpOp subOp = SetOpOp(i.imm[1].u_OA);
  Opcode arithOp = setOpOpToOpcodeOp(subOp);
  i.m_txFlags = nativePlan(i.inputs[0]->isInt() &&
                           i.inputs[1]->valueType() == KindOfInt64 &&
                           (arithOp == OpAdd || arithOp == OpSub ||
                            arithOp == OpMul ||
                            arithOp == OpBitAnd || arithOp == OpBitOr ||
                            arithOp == OpBitXor));
  if (!i.m_txFlags) {
    i.m_txFlags = nativePlan(mathEquivTypes(i.inputs[0]->rtt,
                                            i.inputs[1]->rtt) &&
                             (arithOp == OpAdd || arithOp == OpSub ||
                              arithOp == OpMul));
    return;
  }
}

void
TranslatorX64::translateSetOpL(const Tracelet& t,
                               const NormalizedInstruction& i) {
  const vector<DynLocation*>& inputs  = i.inputs;
  assert(inputs.size() >= 2);
  assert(i.outStack && i.outLocal);
  const int valIdx   = 0;
  const int localIdx = 1;
  assert(inputs[localIdx]->isLocal());
  assert(inputs[valIdx]->isStack());
  assert(inputs[valIdx]->outerType() != KindOfRef);

  const SetOpOp subOp = SetOpOp(i.imm[1].u_OA);
  Opcode arithOp = setOpOpToOpcodeOp(subOp);
  m_regMap.allocOutputRegs(i);
  binaryArithLocal(i, arithOp, *inputs[valIdx], *inputs[localIdx],
                   *i.outStack);
}

void
TranslatorX64::analyzeIncDecL(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = nativePlan(i.inputs[0]->isInt());
}

void
TranslatorX64::translateIncDecL(const Tracelet& t,
                                const NormalizedInstruction& i) {
  const vector<DynLocation*>& inputs = i.inputs;
  assert(inputs.size() == 1);
  assert(i.outLocal);
  assert(inputs[0]->isLocal());
  const IncDecOp oplet = IncDecOp(i.imm[1].u_OA);
  assert(oplet == PreInc || oplet == PostInc || oplet == PreDec ||
         oplet == PostDec);
  assert(inputs[0]->isInt() && (!i.outStack || i.outStack->isInt()));
  bool post = (oplet == PostInc || oplet == PostDec);
  bool pre  = !post;
  bool inc  = (oplet == PostInc || oplet == PreInc);

  m_regMap.allocOutputRegs(i);
  PhysReg localVal = getReg(inputs[0]->location);
  if (i.outStack && post) { // $a++, $a--
    PhysReg output   = getReg(i.outStack->location);
    emitMovRegReg(a, localVal, output);
  }
  if (inc) {
    a.    incq(localVal);
  } else {
    a.    decq(localVal);
  }
  if (i.outStack && pre) { // --$a, ++$a
    PhysReg output   = getReg(i.outStack->location);
    emitMovRegReg(a, localVal, output);
  }
}

void
TranslatorX64::translateUnsetL(const Tracelet& t,
                               const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(!i.outStack && i.outLocal);
  const int locIdx = 0;
  const DynLocation& localDl = *i.inputs[locIdx];
  assert(localDl.isLocal());

  // We have to mark the output register as dirty to ensure that
  // the type gets spilled at the tend of the tracelet
  m_regMap.allocOutputRegs(i);

  DataType type = localDl.outerType();
  // decRef the value that currently lives in the local if appropriate.
  emitDecRef(i, getReg(localDl.location), type);
}


void
TranslatorX64::analyzeReqLit(Tracelet& t, NormalizedInstruction& i,
                             InclOpFlags flags) {
  assert(i.inputs.size() == 1);
  Eval::PhpFile* efile = g_vmContext->lookupIncludeRoot(
                                (StringData*)i.inputs[0]->rtt.valueString(),
                                flags, nullptr);
  i.m_txFlags = supportedPlan(i.inputs[0]->isString() &&
                              i.inputs[0]->rtt.valueString() != nullptr &&
                              efile &&
                              (RuntimeOption::RepoAuthoritative ||
                               RuntimeOption::ServerStatCache));
  if (efile && efile->unit()->getMainReturn()->m_type != KindOfUninit) {
    i.outStack->rtt = RuntimeType(efile->unit()->getMainReturn()->m_type);
  }

  // We don't need the reference lookupIncludeRoot made for us.
  if (efile) efile->decRef();
  i.manuallyAllocInputs = true;
}

void
TranslatorX64::analyzeReqDoc(Tracelet& t, NormalizedInstruction& i) {
  analyzeReqLit(t, i, InclOpDocRoot);
}

void
TranslatorX64::translateReqLit(const Tracelet& t,
                               const NormalizedInstruction& i,
                               InclOpFlags flags) {
  bool local = flags & InclOpLocal;
  StringData *s = const_cast<StringData*>(i.inputs[0]->rtt.valueString());
  HPHP::Eval::PhpFile* efile =
    g_vmContext->lookupIncludeRoot(s, flags, nullptr);
  /*
   * lookupIncludeRoot increments the refcount for us. This reference is
   * going to be burned into the translation cache. We will remove it only
   * when the file changes (via invalidateFile), and we're sure that no
   * outstanding requests are using the old code (via the Treadmill
   * module).
   */
  TRACE(1, "lookupIncludeRoot: %s -> %p c %d\n", s->data(), efile,
        efile->getRef());
  /*
   * Remember that this tracelet (not just this instruction) now depends on the
   * contents of the required file.
   */
  m_srcDB.recordDependency(efile, t.m_sk);
  Unit *unit = efile->unit();
  Func *func = unit->getMain(local ? nullptr : curClass());

  const Offset after = nextSrcKey(t, i).offset();
  TRACE(1, "requireHelper: efile %p offset %d%s\n", efile, after,
           i.skipSync ? " [skipsync]" : "");

  if (i.skipSync) {
    /*
     * getting here means there was nothing to do between
     * the previous req and this one. Any spill code we generate
     * here would be broken (because the rbx is wrong), so
     * verify that we don't generate anything...
     */
    TCA s DEBUG_ONLY = a.code.frontier;
    syncOutputs(0);
    assert(s == a.code.frontier);
  } else {
    syncOutputs(i);
  }
  ReqLitStaticArgs* args = m_globalData.alloc<ReqLitStaticArgs>();
  emitImmReg(a, (uint64_t)args, argNumToRegName[0]);
  emitCall(a, (TCA)reqLitHelper, true);

  args->m_efile = efile;
  args->m_pseudoMain = emitServiceReq(SRFlags::SRNone, REQ_BIND_REQUIRE, 3,
                                      uint64_t(args),
                                      uint64_t(func), uint64_t(func->base()));
  args->m_pcOff = after;
  args->m_local = local;

  if (i.breaksTracelet) {
    SrcKey fallThru(curFunc(), after);
    emitBindJmp(fallThru);
  } else {
    /*
     * When we get here, rVmSp points to the actual top of stack,
     * but the rest of this tracelet assumes that rVmSp is set to
     * the top of the stack at the beginning of the tracelet, so we
     * have to fix it up here.
     *
     */
    if (!i.outStack) {
      /* as a special case, if we're followed by a pop, and
         we return a non-refcounted type, and then followed
         by another require, we can avoid the add here and the sub
         in the following require
      */
    } else {
      int delta = i.stackOff + getStackDelta(i);
      if (delta != 0) {
        // i.stackOff is in negative Cells, not bytes.
        a.    add_imm64_reg64(cellsToBytes(delta), rVmSp);
      }
    }
  }
}

void
TranslatorX64::translateReqDoc(const Tracelet& t,
                               const NormalizedInstruction& i) {
  translateReqLit(t, i, InclOpDocRoot);
}

TCA
TranslatorX64::emitNativeTrampoline(TCA helperAddr) {
  auto& a = atrampolines;

  if (!a.code.canEmit(m_trampolineSize)) {
    // not enough space to emit a trampoline, so just return the
    // helper address and emitCall will the emit the right sequence
    // to call it indirectly
    TRACE(1, "Ran out of space to emit a trampoline for %p\n", helperAddr);
    assert(false);
    return helperAddr;
  }
  uint32_t index = m_numNativeTrampolines++;
  TCA trampAddr = a.code.frontier;
  if (Stats::enabled()) {
    Stats::emitInc(a, &Stats::tl_helper_counters[0], index);
    char* name = Util::getNativeFunctionName(helperAddr);
    const size_t limit = 50;
    if (strlen(name) > limit) {
      name[limit] = '\0';
    }
    Stats::helperNames[index] = name;
  }

  /*
   * For stubs that take arguments in rScratch, we need to make sure
   * we're not damaging its contents here.  (If !jmpDeltaFits, the jmp
   * opcode will need to movabs the address into rScratch before
   * jumping.)
   */
  auto UNUSED stubUsingRScratch = [&](TCA tca) {
    return tca == m_dtorGenericStubRegs;
  };

  assert(IMPLIES(stubUsingRScratch(helperAddr), a.jmpDeltaFits(helperAddr)));
  a.    jmp    (helperAddr);
  a.    ud2    ();

  trampolineMap[helperAddr] = trampAddr;
  if (m_trampolineSize == 0) {
    m_trampolineSize = a.code.frontier - trampAddr;
    assert(m_trampolineSize >= kMinPerTrampolineSize);
  }
  recordBCInstr(OpNativeTrampoline, a, trampAddr);
  return trampAddr;
}

TCA
TranslatorX64::getNativeTrampoline(TCA helperAddr) {
  if (!RuntimeOption::EvalJitTrampolines && !Stats::enabled()) {
    return helperAddr;
  }
  TCA trampAddr = (TCA)mapGet<PointerMap>(trampolineMap, helperAddr);
  if (trampAddr) {
    return trampAddr;
  }
  return emitNativeTrampoline(helperAddr);
}

void TranslatorX64::analyzeDefCls(Tracelet& t,
                                  NormalizedInstruction& i) {
  i.m_txFlags = Supported;
}

static void defClsHelper(PreClass *preClass) {
  assert(tl_regState == REGSTATE_DIRTY);
  tl_regState = REGSTATE_CLEAN;
  Unit::defClass(preClass);

  /*
   * m_defClsHelper sync'd the registers for us already.  This means
   * if an exception propagates we want to leave things as
   * REGSTATE_CLEAN, since we're still in sync.  Only set it to dirty
   * if we are actually returning to run in the TC again.
   */
  tl_regState = REGSTATE_DIRTY;
}

void TranslatorX64::translateDefCls(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  int cid = i.imm[0].u_IVA;
  const Opcode* after = curUnit()->at(i.source.offset());
  PreClass* c = curFunc()->unit()->lookupPreClassId(cid);

  assert(m_defClsHelper);

  /*
     compute the corrected stack ptr as a pseudo-param to m_defClsHelper
     which it will store in g_vmContext, in case of fatals, or __autoload
  */
  m_regMap.cleanReg(rax);
  m_regMap.smashReg(rax);
  ScratchReg offset(m_regMap, rax);
  emitLea(a, rVmSp, -cellsToBytes(i.stackOff), rax);

  EMIT_CALL(a, m_defClsHelper, IMM((uint64_t)c), IMM((uint64_t)after));
}

void TranslatorX64::analyzeDefFunc(Tracelet& t,
                                   NormalizedInstruction& i) {
  i.m_txFlags = Supported;
}

void defFuncHelper(Func *f) {
  f->setCached();
}

void TranslatorX64::translateDefFunc(const Tracelet& t,
                                     const NormalizedInstruction& i) {
  int fid = i.imm[0].u_IVA;
  Func* f = curFunc()->unit()->lookupFuncId(fid);

  EMIT_CALL(a, defFuncHelper, IMM((uint64_t)f));
  recordReentrantCall(i);
}

void
TranslatorX64::analyzeFPushFunc(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() >= 1);
  // The input might be an object implementing __invoke()
  i.m_txFlags = simplePlan(i.inputs[0]->isString());
}

void
TranslatorX64::translateFPushFunc(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  using namespace TargetCache;
  CacheHandle ch = FuncCache::alloc();
  assert(i.inputs.size() == 1);
  Location& inLoc = i.inputs[0]->location;

  m_regMap.allocOutputRegs(i);
  m_regMap.scrubStackRange(i.stackOff - 1,
                           i.stackOff - 1 + kNumActRecCells);
  // Popped one cell, pushed an actrec
  int startOfActRec = int(sizeof(Cell)) - int(sizeof(ActRec));
  size_t funcOff = AROFF(m_func) + startOfActRec;
  size_t thisOff = AROFF(m_this) + startOfActRec;
  if (false) { // typecheck
    StackStringData sd("foo");
    const UNUSED Func* f = FuncCache::lookup(ch, &sd);
  }
  SKTRACE(1, i.source, "ch %d\n", ch);
  EMIT_CALL(a, FuncCache::lookup, IMM(ch), V(inLoc));
  recordCall(i);
  emitVStackStore(a, i, rax, funcOff, sz::qword);
  emitVStackStoreImm(a, i, 0, thisOff, sz::qword, &m_regMap);
  emitPushAR(i, nullptr, sizeof(Cell) /* bytesPopped */);
}

void
TranslatorX64::analyzeFPushClsMethodD(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = supportedPlan(true);
}

void
TranslatorX64::translateFPushClsMethodD(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  using namespace TargetCache;
  const StringData* meth = curUnit()->lookupLitstrId(i.imm[1].u_SA);
  const NamedEntityPair& np = curUnit()->lookupNamedEntityPairId(i.imm[2].u_SA);
  const StringData* cls = np.first;
  assert(meth && meth->isStatic() &&
         cls && cls->isStatic());
  assert(i.inputs.size() == 0);

  const Class* baseClass = Unit::lookupUniqueClass(np.second);
  bool magicCall = false;
  const Func* func = lookupImmutableMethod(baseClass, meth, magicCall,
                                           true /* staticLookup */);

  m_regMap.scrubStackRange(i.stackOff,
                           i.stackOff + kNumActRecCells);

  int startOfActRec = -int(sizeof(ActRec));
  SKTRACE(2, i.source, "FPushClsMethodD %s :: %s\n",
          cls->data(), meth->data());

  size_t clsOff  = AROFF(m_cls) + startOfActRec;
  if (func) {
    emitKnownClassCheck(i, cls, reg::noreg);
    Stats::emitInc(a, Stats::TgtCache_StaticMethodBypass);
    emitPushAR(i, func, 0 /*bytesPopped*/,
               false /* isCtor */, false /* clearThis */,
               magicCall ? uintptr_t(meth) | ActRec::kInvNameBit : 0);

    setupActRecClsForStaticCall(i, func, baseClass, clsOff, false);
  } else {
    Stats::emitInc(a, Stats::TgtCache_StaticMethodHit);
    CacheHandle ch = StaticMethodCache::alloc(cls, meth, getContextName());
    ScratchReg rFunc(m_regMap);
    // Unconditionally set rCls; if we miss, the miss path will clean it up for
    // us. The fill path has already |'ed in the necessary 1.
    ScratchReg rCls(m_regMap);
    a.    load_reg64_disp_reg64(rVmTl,
                                ch + offsetof(StaticMethodCache, m_cls),
                                r(rCls));
    emitVStackStore(a, i, r(rCls), clsOff);
    TCA stubsSkipRet;
    a.    load_reg64_disp_reg64(rVmTl, ch, r(rFunc));
    a.    test_reg64_reg64(r(rFunc), r(rFunc));
    {
      UnlikelyIfBlock miss(CC_Z, a, astubs);
      if (false) { // typecheck
        const UNUSED Func* f = StaticMethodCache::lookup(ch, np.second,
                                                         cls, meth);
      }
      EMIT_CALL(astubs,
                 StaticMethodCache::lookup,
                 IMM(ch),
                 IMM(int64_t(np.second)),
                 IMM(int64_t(cls)),
                 IMM(int64_t(meth)));
      recordReentrantStubCall(i);
      emitMovRegReg(astubs, rax, r(rFunc));
      // NULL return means our work is done; see also
      // translateFPushClsMethodF.
      miss.reconcileEarly();
      astubs.test_reg64_reg64(r(rFunc), r(rFunc));
      stubsSkipRet = astubs.code.frontier;
      astubs.jz(a.code.frontier); // 1f to be patched later
    }

    {
      FreezeRegs ice(m_regMap);
      emitPushAR(i, nullptr);
      size_t funcOff = AROFF(m_func) + startOfActRec;
      emitVStackStore(a, i, r(rFunc), funcOff, sz::qword);
    }
    // 1:
    astubs.patchJcc(stubsSkipRet, a.code.frontier);
  }
}

void
TranslatorX64::analyzeFPushClsMethodF(Tracelet& t,
                                      NormalizedInstruction& i) {
  assert(i.inputs[0]->valueType() == KindOfClass);
  i.m_txFlags = supportedPlan(
    i.inputs[1]->isString() &&
    i.inputs[1]->rtt.valueString() != nullptr && // We know the method name
    i.inputs[0]->valueType() == KindOfClass &&
    i.inputs[0]->rtt.valueClass() != nullptr // We know the class name
  );
}

void
TranslatorX64::translateFPushClsMethodF(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  using namespace TargetCache;
  assert(!curFunc()->isPseudoMain());
  assert(curFunc()->cls() != nullptr); // self:: and parent:: should only
                                    // appear in methods
  DynLocation* clsLoc = i.inputs[0];
  DynLocation* nameLoc = i.inputs[1];
  const StringData* name = nameLoc->rtt.valueString();
  assert(name && name->isStatic());

  // Even though we know the Class* at compile time, it's not
  // guaranteed to be the same between requests. The name, however, is
  // fixed, so we can use that.
  const Class* cls = clsLoc->rtt.valueClass();
  assert(cls);
  bool magicCall = false;
  const Func* func = lookupImmutableMethod(cls, name, magicCall,
                                           true /* staticLookup */);

  const int bytesPopped = 2 * sizeof(Cell); // [A C] popped
  const int startOfActRec = -int(sizeof(ActRec)) + bytesPopped;
  const Offset clsOff = startOfActRec + AROFF(m_cls);

  UNUSED ActRec* fp = curFrame();
  assert(!fp->hasThis() || fp->getThis()->instanceof(cls));
  if (func) {
    Stats::emitInc(a, Stats::TgtCache_StaticMethodFBypass);
    emitPushAR(i, func, bytesPopped,
               false /* isCtor */, false /* clearThis */,
               magicCall ? uintptr_t(name) | 1 : 0 /* varEnvInvName */);

    setupActRecClsForStaticCall(i, func, cls, clsOff, true);
    m_regMap.scrubStackRange(i.stackOff - 2,
                             i.stackOff - 2 + kNumActRecCells);
  } else {
    const StringData* clsName = cls->name();
    CacheHandle ch = StaticMethodFCache::alloc(clsName, name, getContextName());

    Stats::emitInc(a, Stats::TgtCache_StaticMethodFHit);
    TCA stubsSkipRet;
    ScratchReg rFunc(m_regMap);
    a.    load_reg64_disp_reg64(rVmTl, ch, r(rFunc));
    a.    test_reg64_reg64(r(rFunc), r(rFunc));
    {
      UnlikelyIfBlock miss(CC_Z, a, astubs);
      if (false) { // typecheck
        const UNUSED Func* f = StaticMethodFCache::lookup(ch, cls, name);
      }
      EMIT_CALL(astubs,
                 StaticMethodFCache::lookup,
                 IMM(ch),
                 V(clsLoc->location),
                 V(nameLoc->location));
      recordReentrantStubCall(i);
      emitMovRegReg(astubs, rax, r(rFunc));
      // if rax == NULL, the helper interpreted the entire
      // instruction for us. Skip over the rest of the emitted code in
      // a, but we don't want to skip the branch spill/fill code.
      miss.reconcileEarly();
      astubs.test_reg64_reg64(r(rFunc), r(rFunc));
      stubsSkipRet = astubs.code.frontier;
      astubs.jz(a.code.frontier); // to be patched later
    }

    const Offset funcOff = startOfActRec + AROFF(m_func);
    m_regMap.scrubStackRange(i.stackOff - 2,
                             i.stackOff - 2 + kNumActRecCells);
    {
      FreezeRegs ice(m_regMap);
      emitPushAR(i, nullptr, bytesPopped);
      emitVStackStore(a, i, r(rFunc), funcOff);

      // We know we're in a method so we don't have to worry about
      // rVmFp->m_cls being NULL. We just have to figure out if it's a
      // Class* or $this, and whether or not we should pass along $this or
      // its class.
      PhysReg rCls = r(rFunc); // no need to allocate another scratch
      a.    load_reg64_disp_reg64(rVmFp, AROFF(m_cls), rCls);
      a.    testb(1, rbyte(rCls));
      {
        JccBlock<CC_NZ> ifThis(a);
        // rCls is holding $this. Should we pass it to the callee?
        a.  cmp_imm32_disp_reg32(1, ch + offsetof(StaticMethodFCache, m_static),
                                 rVmTl);
        {
          IfElseBlock<CC_NE> ifStatic(a);
          // We're calling a static method. Load (this->m_cls | 0x1) into rCls.
          a.load_reg64_disp_reg64(rCls, ObjectData::getVMClassOffset(), rCls);
          a.or_imm32_reg64(1, rCls);

          ifStatic.Else();
          // We're calling an instance method. incRef $this.
          emitIncRef(rCls, KindOfObject);
        }
      }
      emitVStackStore(a, i, rCls, clsOff);
    }

    astubs.patchJcc(stubsSkipRet, a.code.frontier);
    // No need to decref our inputs: one was KindOfClass and the other's
    // a static string.
  }
}

void
TranslatorX64::analyzeFPushObjMethodD(Tracelet& t,
                                      NormalizedInstruction &i) {
  DynLocation* objLoc = i.inputs[0];
  i.m_txFlags = supportedPlan(objLoc->valueType() == KindOfObject);
}

void
TranslatorX64::translateFPushObjMethodD(const Tracelet &t,
                                        const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  Location& objLoc = i.inputs[0]->location;
  assert(i.inputs[0]->valueType() == KindOfObject);
  int id = i.imm[1].u_IVA;
  const StringData* name = curUnit()->lookupLitstrId(id);

  const Class* baseClass = i.inputs[0]->rtt.valueClass();
  bool magicCall = false;
  const Func* func = lookupImmutableMethod(baseClass, name, magicCall,
                                           false /* staticLookup */);
  m_regMap.scrubStackRange(i.stackOff - 1,
                           i.stackOff - 1 + kNumActRecCells);
  // Popped one cell, pushed an actrec
  int startOfActRec = int(sizeof(Cell)) - int(sizeof(ActRec));
  size_t thisOff = AROFF(m_this) + startOfActRec;
  size_t funcOff = AROFF(m_func) + startOfActRec;
  emitPushAR(i, func, sizeof(Cell) /*bytesPopped*/,
             false /* isCtor */, false /* clearThis */,
             func && magicCall ? uintptr_t(name) | 1 : 0 /* varEnvInvName */);

  if (!func) {
    if (baseClass && !(baseClass->attrs() & AttrInterface)) {
      MethodLookup::LookupResult res =
        g_vmContext->lookupObjMethod(func, baseClass, name, false);
      if ((res == MethodLookup::MethodFoundWithThis ||
           res == MethodLookup::MethodFoundNoThis) &&
          !func->isAbstract()) {
        /*
         * if we found the func in baseClass, then either:
         *  - its private, and this is always going to be the
         *    called function, or
         *  - any derived class must have a func that matches in
         *    staticness, and is at least as accessible (and in
         *    particular, you can't override a public/protected
         *    method with a private method)
         */
        if (func->attrs() & AttrPrivate) {
          emitVStackStoreImm(a, i, uintptr_t(func), funcOff, sz::qword);
        } else {
          Offset methodsOff = Class::getMethodsOffset();
          Offset vecOff = methodsOff + Class::MethodMap::vecOff();
          ScratchReg scratch(m_regMap);
          // get the object's class into *scratch
          a.   load_reg64_disp_reg64(getReg(objLoc),
                                     ObjectData::getVMClassOffset(),
                                     r(scratch));
          if (res == MethodLookup::MethodFoundNoThis) {
            emitDecRef(a, i, getReg(objLoc), KindOfObject);
            a.   lea_reg64_disp_reg64(r(scratch), 1, getReg(objLoc));
          }
          emitVStackStore(a, i, getReg(objLoc), thisOff, sz::qword);

          // get the method vector into *scratch
          a.   load_reg64_disp_reg64(r(scratch), vecOff, r(scratch));
          // get the func
          a.   load_reg64_disp_reg64(r(scratch),
                                     func->methodSlot() * sizeof(Func*),
                                     r(scratch));
          emitVStackStore(a, i, r(scratch), funcOff, sz::qword);
          Stats::emitInc(a, Stats::TgtCache_MethodFast);
          return;
        }
      } else {
        func = nullptr;
      }
    }
  }

  if (func) {
    if (func->attrs() & AttrStatic && !func->isClosureBody()) {
      if (func->attrs() & AttrPrivate) {
        emitVStackStoreImm(a, i, uintptr_t(curFunc()->cls()) | 1,
                           thisOff, sz::qword);
      } else {
        ScratchReg scratch(m_regMap);
        a.   load_reg64_disp_reg64(getReg(objLoc),
                                   ObjectData::getVMClassOffset(),
                                   r(scratch));
        a.   or_imm32_reg64(1, r(scratch));
        emitVStackStore(a, i, r(scratch), thisOff, sz::qword);
      }
      emitDecRef(a, i, getReg(objLoc), KindOfObject);
    } else {
      emitVStackStore(a, i, getReg(objLoc), thisOff, sz::qword);
    }
    Stats::emitInc(a, Stats::TgtCache_MethodBypass);
  } else {
    emitVStackStore(a, i, getReg(objLoc), thisOff, sz::qword);
    using namespace TargetCache;
    CacheHandle ch = MethodCache::alloc();
    if (false) { // typecheck
      ActRec* ar = nullptr;
      MethodCache::lookup(ch, ar, name);
    }
    int arOff = vstackOffset(i, startOfActRec);
    SKTRACE(1, i.source, "ch %d\n", ch);
    EMIT_CALL(a, MethodCache::lookup, IMM(ch),
               RPLUS(rVmSp, arOff), IMM(uint64_t(name)));
    recordReentrantCall(i);
  }
}

static inline ALWAYS_INLINE Class* getKnownClass(Class** classCache,
                                                 const StringData* clsName) {
  Class* cls = *classCache;
  if (UNLIKELY(cls == nullptr)) {
    // lookupKnownClass does its own VMRegAnchor'ing.
    cls = TargetCache::lookupKnownClass<false>(classCache, clsName, true);
    assert(*classCache && *classCache == cls);
  }
  assert(cls);
  return cls;
}

Instance*
HOT_FUNC_VM
newInstanceHelper(Class* cls, int numArgs, ActRec* ar, ActRec* prevAr) {
  const Func* f = cls->getCtor();
  Instance* ret = nullptr;
  if (UNLIKELY(!(f->attrs() & AttrPublic))) {
    VMRegAnchor _;
    UNUSED MethodLookup::LookupResult res =
      g_vmContext->lookupCtorMethod(f, cls, true /*raise*/);
    assert(res == MethodLookup::MethodFoundWithThis);
  }
  // Don't start pushing the AR until newInstance returns; it may reenter.
  ret = newInstance(cls);
  f->validate();
  ar->m_func = f;
  ar->initNumArgs(numArgs, true /*fromCtor*/);
  // Count stack and this.
  ret->incRefCount();
  ret->incRefCount();
  ar->setThis(ret);
  ar->setVarEnv(nullptr);
  arSetSfp(ar, prevAr);
  TRACE(2, "newInstanceHelper: AR %p: f %p, savedRbp %#lx, savedRip %#lx"
        " this %p\n",
        ar, ar->m_func, ar->m_savedRbp, ar->m_savedRip, ar->m_this);
  return ret;
}

void TranslatorX64::translateFPushCtor(const Tracelet& t,
                                       const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  int arOff = vstackOffset(i, -int(sizeof(ActRec)));
  m_regMap.scrubStackRange(i.stackOff, i.stackOff + kNumActRecCells);
  EMIT_CALL(a, newInstanceHelper,
            V(i.inputs[0]->location),
            IMM(numArgs),
            RPLUS(rVmSp, arOff),
            R(rVmFp));
  recordReentrantCall(i);

  m_regMap.bind(rax, i.outStack->location, KindOfObject, RegInfo::DIRTY);
}

static Instance*
HOT_FUNC_VM
newInstanceHelperNoCtor(Class* cls) {
  Instance* ret = newInstance(cls);
  ret->incRefCount();
  return ret;
}

Instance*
HOT_FUNC_VM
newInstanceHelperNoCtorCached(Class** classCache, const StringData* clsName) {
  Class* cls = getKnownClass(classCache, clsName);
  return newInstanceHelperNoCtor(cls);
}

Instance*
HOT_FUNC_VM
newInstanceHelperCached(Class** classCache,
                        const StringData* clsName, int numArgs,
                        ActRec* ar, ActRec* prevAr) {
  Class* cls = getKnownClass(classCache, clsName);
  return newInstanceHelper(cls, numArgs, ar, prevAr);
}

void TranslatorX64::translateFPushCtorD(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  using namespace TargetCache;
  int numArgs = i.imm[0].u_IVA;
  const StringData* clsName = curUnit()->lookupLitstrId(i.imm[1].u_SA);
  Class* cls = Unit::lookupUniqueClass(clsName);
  bool fastPath = !RuntimeOption::EnableObjDestructCall &&
    classIsPersistent(cls) &&
    !(cls->attrs() & (AttrAbstract | AttrInterface | AttrTrait)) &&
    (cls->getCtor()->attrs() & AttrPublic);
  int arOff = -int(sizeof(ActRec)) - cellsToBytes(1);
  m_regMap.scrubStackRange(i.stackOff, i.stackOff + kNumActRecCells + 1);

  LazyScratchReg clsCache(m_regMap);
  if (fastPath) {
    emitFPushCtorDFast(i, cls, arOff);
  } else {
    CacheHandle classCh = allocKnownClass(clsName);
    clsCache.alloc();
    a.   lea_reg64_disp_reg64(rVmTl, classCh, r(clsCache));

    if (i.noCtor) {
      Stats::emitInc(a, Stats::Tx64_NewInstanceNoCtor);
      EMIT_RCALL(a, i, newInstanceHelperNoCtorCached,
                 R(clsCache), IMM(uintptr_t(clsName)));
    } else {
      arOff = vstackOffset(i, arOff);
      Stats::emitInc(a, Stats::Tx64_NewInstanceGeneric);
      EMIT_RCALL(a, i, newInstanceHelperCached,
                 R(clsCache),
                 IMM(uintptr_t(clsName)),
                 IMM(numArgs),
                 RPLUS(rVmSp, arOff),     // ActRec
                 R(rVmFp));               // prevAR
    }
  }
  m_regMap.bind(rax, i.outStack->location, KindOfObject, RegInfo::DIRTY);
}

void
TranslatorX64::emitFPushCtorDFast(const NormalizedInstruction& i,
                                  Class* cls, int arOff) {
  size_t size = Instance::sizeForNProps(cls->numDeclProperties());
  int allocator = object_alloc_size_to_index(size);
  if (i.noCtor) {
    Stats::emitInc(a, Stats::Tx64_NewInstanceNoCtorFast);
  } else {
    Stats::emitInc(a, Stats::Tx64_NewInstanceFast);
  }

  // First, make sure our property init vectors are all set up
  bool props = cls->pinitVec().size() > 0;
  bool sprops = cls->numStaticProperties() > 0;
  assert((props || sprops) == cls->needInitialization());
  if (cls->needInitialization()) {
    if (props) {
      cls->initPropHandle();
      Stats::emitInc(a, Stats::Tx64_NewInstancePropCheck);
      a.test_imm64_disp_reg64(-1, cls->propHandle(), rVmTl);
      {
        UnlikelyIfBlock ifZero(CC_Z, a, astubs);
        Stats::emitInc(a, Stats::Tx64_NewInstancePropInit);
        EMIT_RCALL(astubs, i, getMethodPtr(&Class::initProps),
                   IMM(int64_t(cls)));
      }
    }
    if (sprops) {
      cls->initSPropHandle();
      Stats::emitInc(a, Stats::Tx64_NewInstanceSPropCheck);
      a.test_imm64_disp_reg64(-1, cls->sPropHandle(), rVmTl);
      {
        UnlikelyIfBlock ifZero(CC_Z, a, astubs);
        Stats::emitInc(a, Stats::Tx64_NewInstanceSPropInit);
        EMIT_RCALL(astubs, i, getMethodPtr(&Class::initSProps),
                   IMM(int64_t(cls)));
      }
    }
  }

  // Next, allocate the object
  if (cls->instanceCtor()) {
    EMIT_RCALL(a, i, cls->instanceCtor(), IMM(int64_t(cls)));
  } else {
    assert(allocator != -1);
    EMIT_RCALL(a, i, getMethodPtr(&Instance::newInstanceRaw),
               IMM(int64_t(cls)), IMM(allocator));
  }
  ScratchReg holdRax(m_regMap, rax);

  // Set the attributes, if any
  int odAttrs = cls->getODAttrs();
  if (odAttrs) {
    // o_attribute is 16 bits but the fact that we're or-ing a mask makes
    // it ok
    assert(!(odAttrs & 0xffff0000));
    a.or_imm32_disp_reg32(odAttrs, ObjectData::attributeOff(), rax);
  }

  // Initialize the properties
  size_t nProps = cls->numDeclProperties();
  if (nProps > 0) {
    ScratchReg propVec(m_regMap);
    a.lea_reg64_disp_reg64(rax,
                           sizeof(ObjectData) + cls->builtinPropSize(),
                           r(propVec));
    a.push(rax);
    a.sub_imm32_reg64(8, rsp); // rsp alignment to keep memcpy happy
    if (cls->pinitVec().size() == 0) {
      // Fast case: copy from a known address in the Class
      EMIT_CALL(a, memcpy,
                R(propVec),
                IMM(int64_t(&cls->declPropInit()[0])),
                IMM(cellsToBytes(nProps)));
    } else {
      // Slower case: we have to load the src address from the targetcache
      ScratchReg propData(m_regMap);
      // Load the Class's propInitVec from the targetcache
      a.load_reg64_disp_reg64(rVmTl, cls->propHandle(), r(propData));
      // propData holds the PropInitVec. We want &(*propData)[0]
      a.load_reg64_disp_reg64(r(propData), Class::PropInitVec::dataOff(),
                              r(propData));
      if (!cls->hasDeepInitProps()) {
        EMIT_CALL(a, memcpy,
                  R(propVec),
                  R(propData),
                  IMM(cellsToBytes(nProps)));
      } else {
        EMIT_CALL(a, deepInitHelper,
                  R(propVec),
                  R(propData),
                  IMM(nProps));
      }
    }
    a.add_imm32_reg64(8, rsp);
    a.pop(rax);
  }
  if (cls->callsCustomInstanceInit()) {
    // callCustomInstanceInit returns the instance in rax
    if (false) {
      UNUSED Instance* ret = ret->callCustomInstanceInit();
    }
    EMIT_RCALL(a, i,
               getMethodPtr(&Instance::callCustomInstanceInit),
               R(rax));
  }

  // We're done with what Instance's constructor would've done. Set up the
  // ActRec if needed.
  if (i.noCtor) {
    // If we're not running the constructor, just incref the object once and
    // don't set up the ActRec.
    a.incl(rax[FAST_REFCOUNT_OFFSET]);
    return;
  } else {
    // Incref the object twice: once for the stack and once for $this in the
    // ActRec.
    a.add_imm32_disp_reg32(2, FAST_REFCOUNT_OFFSET, rax);
  }
  emitVStackStore(a, i, rVmFp, arOff + AROFF(m_savedRbp));
  emitVStackStoreImm(a, i, int64_t(cls->getCtor()), arOff + AROFF(m_func));
  emitVStackStoreImm(a, i, ActRec::encodeNumArgs(i.imm[0].u_IVA, true),
                     arOff + AROFF(m_numArgsAndCtorFlag), sz::dword);
  emitVStackStoreImm(a, i, 0, arOff + AROFF(m_varEnv));
  emitVStackStore(a, i, rax, arOff + AROFF(m_this));
}

void
TranslatorX64::translateCreateCl(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  int getArgs = i.imm[0].u_IVA;
  const StringData* clsName = curUnit()->lookupLitstrId(i.imm[1].u_SA);

  LazyScratchReg clsCache(m_regMap);

  TargetCache::CacheHandle classCh = TargetCache::allocKnownClass(clsName);
  clsCache.alloc();
  a.    lea_reg64_disp_reg64(rVmTl, classCh, r(clsCache));
  EMIT_RCALL(a, i,
             newInstanceHelperNoCtorCached,
             R(clsCache),
             IMM(uintptr_t(clsName)));

  EMIT_RCALL(a, i,
             getMethodPtr(&c_Closure::init),
             R(rax),
             IMM(getArgs),
             R(rVmFp),
             RPLUS(rVmSp, vstackOffset(i, 0)));

  m_regMap.bind(rax, i.outStack->location, KindOfObject, RegInfo::DIRTY);
}


static void fatalNullThis() {
  raise_error(Strings::FATAL_NULL_THIS);
}

void
TranslatorX64::emitThisCheck(const NormalizedInstruction& i,
                             PhysReg reg) {
  if (curFunc()->cls() == nullptr) {  // Non-class
    a.test_reg64_reg64(reg, reg);
    a.jz(astubs.code.frontier); // jz if_null
  }

  a.  testb(1, rbyte(reg));
  {
    UnlikelyIfBlock ifThisNull(CC_NZ, a, astubs);
    // if_null:
    EMIT_CALL(astubs, fatalNullThis);
    recordReentrantStubCall(i);
  }
}

void
TranslatorX64::translateThis(const Tracelet &t,
                             const NormalizedInstruction &i) {
  if (!i.outStack) {
    assert(i.next && i.next->grouped);
    return;
  }

  assert(!i.outLocal);
  assert(curFunc()->isPseudoMain() || curFunc()->cls() ||
         curFunc()->isClosureBody());
  m_regMap.allocOutputRegs(i);
  PhysReg out = getReg(i.outStack->location);
  a.   loadq(rVmFp[AROFF(m_this)], out);

  if (!i.guardedThis) {
    emitThisCheck(i, out);
  }
  emitIncRef(out, KindOfObject);
}

void
TranslatorX64::translateBareThis(const Tracelet &t,
                                const NormalizedInstruction &i) {
  if (!i.outStack) {
    assert(i.next && i.next->grouped);
    return;
  }
  assert(!i.outLocal);
  assert(curFunc()->cls() || curFunc()->isClosureBody());
  ScratchReg outScratch(m_regMap);
  PhysReg out = r(outScratch);
  PhysReg base;
  int offset;
  locToRegDisp(i.outStack->location, &base, &offset);
  if (i.outStack->rtt.isVagueValue()) {
    m_regMap.scrubLoc(i.outStack->location);
  }
  a.   load_reg64_disp_reg64(rVmFp, AROFF(m_this), out);
  a.   testb(1, rbyte(out));
  DiamondReturn astubsRet;
  {
    UnlikelyIfBlock ifThisNull(CC_NZ, a, astubs, &astubsRet);
    emitStoreTVType(astubs, KindOfNull, base[offset + TVOFF(m_type)]);
    if (i.imm[0].u_OA) {
      EMIT_CALL(astubs, warnNullThis);
      recordReentrantStubCall(i);
    }
    if (i.next && !i.outStack->rtt.isVagueValue()) {
      // To handle the case where we predict that
      // the bare this will have type Object.
      // Using the normal type prediction mechanism
      // would require writing the object to the stack
      // anyway.
      // This is currently dead, however - I couldnt
      // find a win.
      emitSideExit(astubs, i, true);
      astubsRet.kill();
    }
  }
  emitIncRef(out, KindOfObject);
  if (i.outStack->rtt.isVagueValue()) {
    emitStoreTVType(a, KindOfObject, base[offset + TVOFF(m_type)]);
    a. storeq(out, base[TVOFF(m_data) + offset]);
  } else {
    assert(i.outStack->isObject());
    m_regMap.bindScratch(outScratch, i.outStack->location, KindOfObject,
                         RegInfo::DIRTY);
  }
}

void
TranslatorX64::translateCheckThis(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1 &&
         i.inputs[0]->location == Location(Location::This));
  if (i.guardedThis) return;
  emitThisCheck(i, getReg(i.inputs[0]->location));
}

void
TranslatorX64::translateInitThisLoc(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  assert(i.outLocal && !i.outStack);
  assert(curFunc()->isPseudoMain() || curFunc()->cls());

  PhysReg base;
  int offset;
  locToRegDisp(i.outLocal->location, &base, &offset);
  assert(base == rVmFp);

  ScratchReg thiz(m_regMap);
  a.    load_reg64_disp_reg64(rVmFp, AROFF(m_this), r(thiz));
  if (curFunc()->cls() == nullptr) {
    // If we're in a pseudomain, m_this could be NULL
    a.  testq (r(thiz), r(thiz));
    a.  jz (astubs.code.frontier); // jz if_null
  }
  // Ok, it's not NULL but it might be a Class which should be treated
  // equivalently
  a.    testb(1, rbyte(thiz));
  a.    jnz(astubs.code.frontier); // jnz if_null

  // We have a valid $this!
  emitStoreTVType(a, KindOfObject, base[offset + TVOFF(m_type)]);
  a.    storeq(r(thiz), base[offset + TVOFF(m_data)]);
  emitIncRef(r(thiz), KindOfObject);

  // if_null:
  emitStoreUninitNull(astubs, offset, base);
  astubs.jmp(a.code.frontier);

  m_regMap.invalidate(i.outLocal->location);
}

void
TranslatorX64::analyzeFPushFuncD(Tracelet& t, NormalizedInstruction& i) {
  Id funcId = i.imm[1].u_SA;
  const NamedEntityPair nep = curUnit()->lookupNamedEntityPairId(funcId);
  const Func* func = Unit::lookupFunc(nep.second, nep.first);
  i.m_txFlags = supportedPlan(func != nullptr);
}

void
TranslatorX64::translateFPushFuncD(const Tracelet& t,
                                   const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outStack && !i.outLocal);
  Id funcId = i.imm[1].u_SA;
  const NamedEntityPair& nep = curUnit()->lookupNamedEntityPairId(funcId);
  const StringData* name = nep.first;
  const Func* func       = Unit::lookupFunc(nep.second, name);

  // Translation is only supported if function lookup succeeds
  func->validate();
  if (Trace::enabled && !func) {
    TRACE(1, "Attempt to invoke undefined function %s\n", name->data());
  }

  // Inform the register allocator that we just annihilated a range of
  // possibly-dirty stack entries.
  m_regMap.scrubStackRange(i.stackOff,
                           i.stackOff + kNumActRecCells);

  size_t thisOff = AROFF(m_this) - sizeof(ActRec);
  bool funcCanChange = !func->isNameBindingImmutable(curUnit());
  if (funcCanChange) {
    // Look it up in a FuncCache.
    using namespace TargetCache;
    CacheHandle ch = allocFixedFunction(nep.second, false);
    size_t funcOff = AROFF(m_func) - sizeof(ActRec);
    size_t funcCacheOff = ch + offsetof(FixedFuncCache, m_func);

    SKTRACE(1, i.source, "ch %d\n", ch);

    Stats::emitInc(a, Stats::TgtCache_FuncDHit);
    ScratchReg scratch(m_regMap);
    a.load_reg64_disp_reg64(rVmTl, funcCacheOff, r(scratch));
    a.test_reg64_reg64(r(scratch), r(scratch));
    {
      UnlikelyIfBlock ifNull(CC_Z, a, astubs);

      if (false) { // typecheck
        StackStringData sd("foo");
        FixedFuncCache::lookupUnknownFunc(&sd);
      }

      EMIT_CALL(astubs, TCA(FixedFuncCache::lookupUnknownFunc),
                        IMM(uintptr_t(name)));
      recordReentrantStubCall(i);
      emitMovRegReg(astubs, rax, r(scratch));
    }
    emitVStackStore(a, i, r(scratch), funcOff, sz::qword);
  }
  // delay writing the ActRec until after calling lookupUnknownFunc
  // since it can re-enter and overwrite anything we had written...
  emitVStackStoreImm(a, i, 0, thisOff, sz::qword, &m_regMap);
  emitPushAR(i, funcCanChange ? nullptr : func, 0, false, false);
}

const Func*
TranslatorX64::findCuf(const NormalizedInstruction& ni,
                       Class*& cls, StringData*& invName, bool& forward) {
  forward = (ni.op() == OpFPushCufF);
  cls = nullptr;
  invName = nullptr;

  DynLocation* callable = ni.inputs[ni.op() == OpFPushCufSafe ? 1 : 0];

  const StringData* str =
    callable->isString() ? callable->rtt.valueString() : nullptr;
  const ArrayData* arr =
    callable->isArray() ? callable->rtt.valueArray() : nullptr;

  StringData* sclass = nullptr;
  StringData* sname = nullptr;
  if (str) {
    Func* f = HPHP::VM::Unit::lookupFunc(str);
    if (f) return f;
    String name(const_cast<StringData*>(str));
    int pos = name.find("::");
    if (pos <= 0 || pos + 2 >= name.size() ||
        name.find("::", pos + 2) != String::npos) {
      return nullptr;
    }
    sclass = StringData::GetStaticString(name.substr(0, pos).get());
    sname = StringData::GetStaticString(name.substr(pos + 2).get());
  } else if (arr) {
    if (arr->size() != 2) return nullptr;
    CVarRef e0 = arr->get(int64_t(0), false);
    CVarRef e1 = arr->get(int64_t(1), false);
    if (!e0.isString() || !e1.isString()) return nullptr;
    sclass = e0.getStringData();
    sname = e1.getStringData();
    String name(sname);
    if (name.find("::") != String::npos) return nullptr;
  } else {
    return nullptr;
  }

  Class* ctx = curFunc()->cls();

  if (sclass->isame(s_self.get())) {
    if (!ctx) return nullptr;
    cls = ctx;
    forward = true;
  } else if (sclass->isame(s_parent.get())) {
    if (!ctx || !ctx->parent()) return nullptr;
    cls = ctx->parent();
    forward = true;
  } else if (sclass->isame(s_static.get())) {
    return nullptr;
  } else {
    cls = VM::Unit::lookupUniqueClass(sclass);
    if (!cls) return nullptr;
  }

  bool magicCall = false;
  const Func* f = lookupImmutableMethod(cls, sname, magicCall, true);
  if (!f || (forward && !ctx->classof(f->cls()))) {
    /*
     * To preserve the invariant that the lsb class
     * is an instance of the context class, we require
     * that f's class is an instance of the context class.
     * This is conservative, but without it, we would need
     * a runtime check to decide whether or not to forward
     * the lsb class
     */
    return nullptr;
  }
  if (magicCall) invName = sname;
  return f;
}

void
TranslatorX64::analyzeFPushCufOp(Tracelet& t,
                                 NormalizedInstruction& ni) {
  Class* cls = nullptr;
  StringData* invName = nullptr;
  bool forward = false;
  const Func* func = findCuf(ni, cls, invName, forward);
  ni.m_txFlags = supportedPlan(func != nullptr);
  ni.manuallyAllocInputs = true;
}

void
TranslatorX64::setupActRecClsForStaticCall(const NormalizedInstruction &i,
                                           const Func* func, const Class* cls,
                                           size_t clsOff, bool forward) {
  if (forward) {
    ScratchReg rClsScratch(m_regMap);
    PhysReg rCls = r(rClsScratch);
    a.    load_reg64_disp_reg64(rVmFp, AROFF(m_cls), rCls);
    if (!(curFunc()->attrs() & AttrStatic)) {
      assert(curFunc()->cls() &&
             curFunc()->cls()->classof(cls));
      /* the context is non-static, so we have to deal
         with passing in $this or getClass($this) */
      a.  testb(1, rbyte(rCls));
      {
        JccBlock<CC_NZ> ifThis(a);
        // rCls is holding a real $this.
        if (func->attrs() & AttrStatic) {
          // but we're a static method, so pass getClass($this)|1
          a.load_reg64_disp_reg64(rCls, ObjectData::getVMClassOffset(), rCls);
          a.or_imm32_reg64(1, rCls);
        } else {
          // We should pass $this to the callee
          emitIncRef(rCls, KindOfObject);
        }
      }
    }
    emitVStackStore(a, i, rCls, clsOff);
  } else {
    if (!(func->attrs() & AttrStatic) &&
        !(curFunc()->attrs() & AttrStatic) &&
        curFunc()->cls() &&
        curFunc()->cls()->classof(cls)) {
      /* might be a non-static call */
      ScratchReg rClsScratch(m_regMap);
      PhysReg rCls = r(rClsScratch);
      a.    load_reg64_disp_reg64(rVmFp, AROFF(m_cls), rCls);
      a.    testb(1, rbyte(rCls));
      {
        IfElseBlock<CC_NZ> ifThis(a);
        // rCls is holding $this. We should pass it to the callee
        emitIncRef(rCls, KindOfObject);
        emitVStackStore(a, i, rCls, clsOff);
        ifThis.Else();
        emitVStackStoreImm(a, i, uintptr_t(cls)|1, clsOff);
      }
    } else {
      emitVStackStoreImm(a, i, uintptr_t(cls)|1, clsOff);
    }
  }
}

int64_t checkClass(TargetCache::CacheHandle ch, StringData* clsName,
                   ActRec *ar) {
  VMRegAnchor _;
  AutoloadHandler::s_instance->invokeHandler(clsName->data());
  if (*(Class**)TargetCache::handleToPtr(ch)) return true;
  ar->m_func = SystemLib::GetNullFunction();
  if (ar->hasThis()) {
    // cannot hit zero, we just inc'ed it
    ar->getThis()->decRefCount();
  }
  ar->setThis(0);
  return false;
}

static const Func* autoloadMissingFunc(const StringData* funcName,
                                       TargetCache::CacheHandle ch,
                                       bool safe) {
  VMRegAnchor _;
  AutoloadHandler::s_instance->autoloadFunc(funcName->data());
  Func* toCall = *(Func**)TargetCache::handleToPtr(ch);
  /* toCall could be a different function due to renaming */
  if (toCall) {
    return toCall;
  }
  if (!safe) {
    throw_invalid_argument("function: method '%s' not found",
                           funcName->data());
  }
  return SystemLib::GetNullFunction();
}

void
TranslatorX64::translateFPushCufOp(const Tracelet& t,
                                   const NormalizedInstruction& ni) {
  Class* cls = nullptr;
  StringData* invName = nullptr;
  bool forward = false;
  const Func* func = findCuf(ni, cls, invName, forward);
  assert(func);

  int numPopped = ni.op() == OpFPushCufSafe ? 0 : 1;
  m_regMap.scrubStackRange(ni.stackOff - numPopped,
                           ni.stackOff - numPopped + kNumActRecCells);

  int startOfActRec = int(numPopped * sizeof(Cell)) - int(sizeof(ActRec));

  emitPushAR(ni, cls ? func : nullptr, numPopped * sizeof(Cell),
             false /* isCtor */, false /* clearThis */,
             invName ? uintptr_t(invName) | ActRec::kInvNameBit : 0);

  bool safe = (ni.op() == OpFPushCufSafe);
  size_t clsOff  = AROFF(m_cls) + startOfActRec;
  size_t funcOff  = AROFF(m_func) + startOfActRec;
  LazyScratchReg flag(m_regMap);
  if (safe) {
    flag.alloc();
    emitImmReg(a, true, r(flag));
  }
  if (cls) {
    setupActRecClsForStaticCall(ni, func, cls, clsOff, forward);
    TargetCache::CacheHandle ch = cls->m_cachedOffset;
    if (!TargetCache::isPersistentHandle(ch)) {
      a.          cmp_imm32_disp_reg32(0, ch, rVmTl);
      {
        UnlikelyIfBlock ifNull(CC_Z, a, astubs);
        if (false) {
          checkClass(0, nullptr, nullptr);
        }
        EMIT_CALL(astubs, TCA(checkClass),
                  IMM(ch), IMM(uintptr_t(cls->name())),
                  RPLUS(rVmSp, vstackOffset(ni, startOfActRec)));
        recordReentrantStubCall(ni, true);
        if (safe) {
          astubs.  mov_reg64_reg64(rax, r(flag));
        }
      }
    }
  } else {
    TargetCache::CacheHandle ch = func->getCachedOffset();
    if (TargetCache::isPersistentHandle(ch)) {
      emitVStackStoreImm(a, ni, uintptr_t(func), funcOff, sz::qword);
      emitVStackStoreImm(a, ni, 0, clsOff, sz::qword, &m_regMap);
    } else {
      ScratchReg funcReg(m_regMap);
      a.          load_reg64_disp_reg64(rVmTl, ch, r(funcReg));
      emitVStackStore(a, ni, r(funcReg), funcOff);
      emitVStackStoreImm(a, ni, 0, clsOff, sz::qword, &m_regMap);
      a.          test_reg64_reg64(r(funcReg), r(funcReg));
      {
        UnlikelyIfBlock ifNull(CC_Z, a, astubs);
        EMIT_CALL(astubs, TCA(autoloadMissingFunc),
                  IMM(uintptr_t(func->name())),
                  IMM(ch),
                  IMM(safe));
        recordReentrantStubCall(ni, true);
        emitVStackStore(astubs, ni, rax, funcOff);
        if (safe) {
          astubs.xorq(r(flag), r(flag));
          astubs.cmpq(SystemLib::GetNullFunction(), rax);
          astubs.setne(rbyte(flag));
        }
      }
    }
  }

  if (safe) {
    DynLocation* outFlag = ni.outStack2;
    DynLocation* outDef = ni.outStack;

    DynLocation* inDef = ni.inputs[0];
    if (!m_regMap.hasReg(inDef->location)) {
      m_regMap.scrubStackRange(ni.stackOff - 2, ni.stackOff - 2);
      PhysReg base1, base2;
      int disp1, disp2;
      locToRegDisp(inDef->location, &base1, &disp1);
      locToRegDisp(outDef->location, &base2, &disp2);
      ScratchReg tmp(m_regMap);
      a.   load_reg64_disp_reg64(base1, TVOFF(m_data) + disp1, r(tmp));
      a.   store_reg64_disp_reg64(r(tmp), TVOFF(m_data) + disp2, base2);
      if (!inDef->rtt.isVagueValue()) {
        emitStoreTVType(a, inDef->outerType(), base2[disp2 + TVOFF(m_type)]);
      } else {
        emitLoadTVType(a,  base1[TVOFF(m_type) + disp1], r(tmp));
        emitStoreTVType(a, r(tmp), base2[disp2 + TVOFF(m_type)]);
      }
    } else {
      PhysReg reg = m_regMap.getReg(inDef->location);
      m_regMap.scrubStackRange(ni.stackOff - 1, ni.stackOff - 1);
      m_regMap.bind(reg, outDef->location, inDef->rtt.outerType(),
                    RegInfo::DIRTY);
    }
    m_regMap.bindScratch(flag, outFlag->location, KindOfBoolean,
                         RegInfo::DIRTY);
  }
}

void
TranslatorX64::analyzeFPassCOp(Tracelet& t, NormalizedInstruction& i) {
  i.m_txFlags = nativePlan(!i.preppedByRef);
}

void
TranslatorX64::translateFPassCOp(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outStack && !i.outLocal);
  assert(!i.preppedByRef);
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
  assert(!i.inputs[0]->rtt.isVagueValue());

  assert(i.inputs.size() == 1);
  const RuntimeType& inRtt = i.inputs[0]->rtt;
  if (inRtt.isRef() && !i.preppedByRef) {
    emitUnboxTopOfStack(i);
  }
}

void
TranslatorX64::translateFCall(const Tracelet& t,
                              const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  const Opcode* after = curUnit()->at(nextSrcKey(t, i).offset());
  const Func* srcFunc = curFunc();

  // Sync all dirty registers and adjust rVmSp to point to the
  // top of stack at the beginning of the current instruction
  syncOutputs(i);

  // We are "between" tracelets and don't use the register map
  // anymore.  (Note that the currently executing trace may actually
  // continue past the FCall, but it will have to resume with a fresh
  // register map.)
  RegSet scratchRegs = kScratchCrossTraceRegs;
  DumbScratchReg retIPReg(scratchRegs);

  // Caller-specific fields: return addresses and the frame pointer
  // offset.
  assert(sizeof(Cell) == 1 << 4);

  // The kooky offset here a) gets us to the current ActRec,
  // and b) accesses m_soff.
  int32_t callOffsetInUnit = srcFunc->unit()->offsetOf(after - srcFunc->base());
  a.    storel  (callOffsetInUnit,
                 rVmSp[cellsToBytes(numArgs) + AROFF(m_soff)]);

  int32_t adjust = emitBindCall(i.source, i.funcd, numArgs);

  if (i.breaksTracelet) {
    if (adjust) {
      a.    addq (adjust, rVmSp);
    }
    SrcKey fallThru(curFunc(), after);
    emitBindJmp(fallThru);
  } else {
    /*
     * Before returning, the callee restored rVmSp to point to the
     * current top of stack but the rest of this tracelet assumes that
     * rVmSp is set to the top of the stack at the beginning of the
     * tracelet, so we have to fix it up here.
     *
     * TODO: in the case of an inlined NativeImpl, we're essentially
     * emitting two adds to rVmSp in a row, which we can combine ...
     */
    int delta = cellsToBytes(i.stackOff + getStackDelta(i)) + adjust;
    if (delta != 0) {
      // i.stackOff is in negative Cells, not bytes.
      a.    addq (delta, rVmSp);
    }
  }
}

void TranslatorX64::analyzeFCallArray(Tracelet& t,
                                      NormalizedInstruction& i) {
  i.m_txFlags = Supported;
}

void TranslatorX64::translateFCallArray(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  const Offset after = nextSrcKey(t, i).offset();

  syncOutputs(i);

  FCallArrayArgs* args = m_globalData.alloc<FCallArrayArgs>();
  emitImmReg(a, (uint64_t)args, argNumToRegName[0]);
  emitCall(a, (TCA)fCallArrayHelper, true);

  args->m_pcOff = i.offset();
  args->m_pcNext = after;

  if (i.breaksTracelet) {
    SrcKey fallThru(curFunc(), after);
    emitBindJmp(fallThru);
  } else {
    /*
     * When we get here, rVmSp points to the actual top of stack,
     * but the rest of this tracelet assumes that rVmSp is set to
     * the top of the stack at the beginning of the tracelet, so we
     * have to fix it up here.
     *
     */
    assert(i.outStack);
    int delta = i.stackOff + getStackDelta(i);
    if (delta != 0) {
      // i.stackOff is in negative Cells, not bytes.
      a.    add_imm64_reg64(cellsToBytes(delta), rVmSp);
    }
  }
}

void TranslatorX64::analyzeFCallBuiltin(Tracelet& t,
                                        NormalizedInstruction& i) {
  Id funcId = i.imm[2].u_SA;
  const NamedEntityPair nep = curUnit()->lookupNamedEntityPairId(funcId);
  const Func* func = Unit::lookupFunc(nep.second, nep.first);
  i.m_txFlags = supportedPlan(func != nullptr);
}

void TranslatorX64::translateFCallBuiltin(const Tracelet& t,
                                          const NormalizedInstruction& ni) {
  int numArgs = ni.imm[0].u_IVA;
  int numNonDefault = ni.imm[1].u_IA;
  Id funcId = ni.imm[2].u_SA;
  const NamedEntityPair& nep = curUnit()->lookupNamedEntityPairId(funcId);
  const StringData* name = nep.first;
  const Func* func = Unit::lookupFunc(nep.second, name);
  PhysReg base;
  int disp;
  assert(ni.outStack);
  assert(numArgs == func->numParams());
  assert(numArgs <= kMaxBuiltinArgs);

  func->validate();

  // Sync all dirty registers
  m_regMap.scrubStackEntries(ni.stackOff);
  m_regMap.cleanAll();

  // Emit typecasts if needed
  for (int i = 0; i < numNonDefault; i++) {
    const Func::ParamInfo& pi = func->params()[i];
    const Location& in = ni.inputs[numArgs - i - 1]->location;
    RuntimeType& rtt = ni.inputs[numArgs - i - 1]->rtt;

#define CSE(type) case KindOf ## type : do { \
  if (!rtt.is ## type ()) { \
    EMIT_CALL(a, tvCastTo ## type ## InPlace, A(in)); \
    recordCall(ni); \
  } \
} while(0); break;

    switch (pi.builtinType()) {
      CSE(Boolean)
      case KindOfInt64 : {
        if (!rtt.isInt()) {
          EMIT_CALL(a, tvCastToInt64InPlace, A(in), IMM(10));
          recordCall(ni);
        }
      } break;
      CSE(Double)
      CSE(Array)
      CSE(Object)
      case BitwiseKindOfString : {
        if (!rtt.isString()) {
          EMIT_CALL(a, tvCastToStringInPlace, A(in));
          recordCall(ni);
        }
      } break;
      case KindOfUnknown: break;
      default:        not_reached();
    }
  }
#undef CSE

  int refReturn = 0;
  PhysReg returnBase = rsp;
  int returnOffset = offsetof(MInstrState, tvBuiltinReturn);

  auto returnType = func->returnType();
  if (isCppByRef(returnType)) {
    if (isSmartPtrRef(returnType)) returnOffset += TVOFF(m_data);
    emitLea(a, returnBase, returnOffset, argNumToRegName[0]);
    refReturn = 1;
  }

  // Load args into registers
  for (int i = 0; i < numArgs; i++) {
    const Func::ParamInfo& pi = func->params()[i];
    locToRegDisp(ni.inputs[numArgs - i - 1]->location, &base, &disp);
    auto argReg = argNumToRegName[i + refReturn];
    switch (pi.builtinType()) {
      case KindOfDouble:
        assert(false);
      case KindOfBoolean:
      case KindOfInt64:
        // pass by value
        a.   loadq  (base[disp + TVOFF(m_data.num)], argReg);
        break;
      STRINGCASE():
      case KindOfArray:
      case KindOfObject:
        // pass ptr to TV.m_data as String&, Array&, or Object&
        emitLea(a, base, disp + TVOFF(m_data), argReg);
        break;
      default:
        // pass ptr to TV as Variant&
        emitLea(a, base, disp, argReg);
        break;
    }
  }
  // Call builtin
  BuiltinFunction nativeFuncPtr = func->nativeFuncPtr();
  emitCall(a, (TCA)nativeFuncPtr, true);
  recordReentrantCall(ni);

  // Bind return value to a scratch reg so that decref helpers
  // don't throw it away
  ScratchReg ret(m_regMap, rax);

  // Decref and free arguments
  for (int i = 0; i < numNonDefault; i++) {
    const Func::ParamInfo& pi = func->params()[i];
    locToRegDisp(ni.inputs[numArgs - i - 1]->location, &base, &disp);
    if (pi.builtinType() == KindOfUnknown) {
      emitDecRefGeneric(ni, base, disp);
    } else if (IS_REFCOUNTED_TYPE(pi.builtinType())) {
      a.  loadq  (base[disp + TVOFF(m_data)], rScratch);
      emitDecRef(ni, rScratch, pi.builtinType());
    }
  }

  // invalidate return value
  m_regMap.invalidate(ni.outStack->location);

  // copy return value
  locToRegDisp(ni.outStack->location, &base, &disp);

  switch (returnType) {
    // For bool return value, get the %al byte
    case KindOfBoolean:
      a.  movzbl (al, eax);  // sign extend byte->qword
      emitStoreTypedValue(a, returnType, rax, disp, base, true);
      break;
    case KindOfNull:  /* void return type */
    case KindOfInt64:
      emitStoreTypedValue(a, returnType, rax, disp, base, true);
      break;
    STRINGCASE():
    case KindOfArray:
    case KindOfObject:
      // returnOffset already has TVOFF(m_data) added if necessary.
      a.   loadq  (returnBase[returnOffset], rax);
      a.   testq  (rax, rax);
      {
        IfElseBlock<CC_Z> ifNotZero(a);
        emitStoreTypedValue(a, returnType, rax, disp, base, true);

        ifNotZero.Else();
        emitStoreTVType(a, KindOfNull, base[disp + TVOFF(m_type)]);
      }
      break;
    case KindOfUnknown: // return type was Variant
      emitLea(a, returnBase, returnOffset, rax);
      emitCmpTVType(a, KindOfUninit, rax[TVOFF(m_type)]);
      {
        IfElseBlock<CC_Z> ifNotUninit(a);
        // copy 16-byte TypedValue
        emitCopyToAligned(a, rax, 0, base, disp);

        ifNotUninit.Else();
        // result was KindOfUninit; convert to KindOfNull
        emitStoreTVType(a, KindOfNull, base[disp + TVOFF(m_type)]);
      }
      break;
    default:
      not_reached();
  }
}

template <bool UseTC>
static TypedValue*
staticLocHelper(StringData* name, ActRec* fp, TypedValue* sp,
                TargetCache::CacheHandle ch) {
  if (UseTC) {
    Stats::inc(Stats::TgtCache_StaticMiss);
    Stats::inc(Stats::TgtCache_StaticHit, -1);
  }
  HphpArray* map = get_static_locals(fp);
  TypedValue* retval = map->nvGet(name); // Local to num
  if (!retval) {
    // Read the initial value off the stack.
    TypedValue tv = *sp;
    map->nvSet(name, &tv, false);
    retval = map->nvGet(name);
  }
  assert(retval);
  if (retval->m_type != KindOfRef) {
    tvBox(retval);
  }
  assert(retval->m_type == KindOfRef);
  if (UseTC) {
    TypedValue** chTv = (TypedValue**)TargetCache::handleToPtr(ch);
    assert(*chTv == nullptr);
    return (*chTv = retval);
  } else {
    return retval;
  }
}

void
TranslatorX64::emitCallStaticLocHelper(X64Assembler& as,
                                       const NormalizedInstruction& i,
                                       ScratchReg& output,
                                       TargetCache::CacheHandle ch) {
  // The helper is going to read the value from memory, so record it.  We
  // could also pass type/value as parameters, but this is hopefully a
  // rare path.
  m_regMap.cleanLoc(i.inputs[0]->location);
  if (false) { // typecheck
    StringData* sd = nullptr;
    ActRec* fp = nullptr;
    TypedValue* sp = nullptr;
    sp = staticLocHelper<true>(sd, fp, sp, ch);
    sp = staticLocHelper<false>(sd, fp, sp, ch);
  }
  const StringData* name = curFunc()->unit()->lookupLitstrId(i.imm[1].u_SA);
  assert(name->isStatic());
  if (ch) {
    EMIT_CALL(as, (TCA)staticLocHelper<true>, IMM(uintptr_t(name)), R(rVmFp),
              RPLUS(rVmSp, -cellsToBytes(i.stackOff)), IMM(ch));
  } else {
    EMIT_CALL(as, (TCA)staticLocHelper<false>, IMM(uintptr_t(name)), R(rVmFp),
              RPLUS(rVmSp, -cellsToBytes(i.stackOff)));
  }
  recordCall(as, i);
  emitMovRegReg(as, rax, r(output));
}

void
TranslatorX64::translateStaticLocInit(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  using namespace TargetCache;
  ScratchReg output(m_regMap);
  const Location& outLoc = i.outLocal->location;

  // Closures and generators from closures don't satisfy the "one
  // static per source location" rule that the inline fastpath
  // requires
  if (!curFunc()->isClosureBody() &&
      !curFunc()->isGeneratorFromClosure()) {
    // Miss path explicitly decrements.
    Stats::emitInc(a, Stats::TgtCache_StaticHit);
    Stats::emitInc(a, Stats::Tx64_StaticLocFast);

    CacheHandle ch = allocStatic();
    assert(ch);
    a.  load_reg64_disp_reg64(rVmTl, ch, r(output));
    a.  test_reg64_reg64(r(output), r(output));
    {
      UnlikelyIfBlock fooey(CC_Z, a, astubs);
      emitCallStaticLocHelper(astubs, i, output, ch);
    }
  } else {
    Stats::emitInc(a, Stats::Tx64_StaticLocSlow);
    emitCallStaticLocHelper(a, i, output, 0);
  }
  // Now we've got the outer variant in *output. Get the address of the
  // inner cell, since that's the enregistered representation of a variant.
  emitDeref(a, r(output), r(output));
  emitIncRef(r(output), KindOfRef);
  // Turn output into the local we just initialized.
  m_regMap.bindScratch(output, outLoc, KindOfRef, RegInfo::DIRTY);
}

void
TranslatorX64::analyzeVerifyParamType(Tracelet& t, NormalizedInstruction& i) {
  int param = i.imm[0].u_IVA;
  const TypeConstraint& tc = curFunc()->params()[param].typeConstraint();
  if (!tc.isObjectOrTypedef()) {
    // We are actually using the translation-time value of this local as a
    // prediction; if the param check failed at compile-time, we predict it
    // will continue failing.
    bool compileTimeCheck = tc.check(frame_local(curFrame(), param), curFunc());
    i.m_txFlags = nativePlan(compileTimeCheck);
    i.manuallyAllocInputs = true;
  } else {
    bool trace = i.inputs[0]->isObject() ||
                 (i.inputs[0]->isNull() && tc.nullable());
    i.m_txFlags = supportedPlan(trace);
  }
}

/*
 * This function will happily give you a Class* to a Class that hasn't been
 * defined in your request yet. Make sure code using it is tolerant of that.
 */
static void
emitClassToReg(X64Assembler& a, const StringData* name, PhysReg r) {
  if (!name) {
    emitImmReg(a, 0, r);
    return;
  }

  Class* cls = Unit::lookupClass(name);
  if (classIsUniqueOrCtxParent(cls)) {
    emitImmReg(a, int64_t(cls), r);
  } else {
    TargetCache::CacheHandle ch = TargetCache::allocKnownClass(name);
    a.  load_reg64_disp_reg64(rVmTl, ch, r);
  }
}

static void
VerifyParamCallable(ObjectData* obj, int param) {
  TypedValue tv;
  tvWriteObject(obj, &tv);
  if (!UNLIKELY(f_is_callable(tvAsCVarRef(&tv)))) {
    VerifyParamTypeFail(param);
  }
  tvDecRef(&tv);
}

void
TranslatorX64::translateVerifyParamType(const Tracelet& t,
                                        const NormalizedInstruction& i) {
  int param = i.imm[0].u_IVA;
  const TypeConstraint& tc = curFunc()->params()[param].typeConstraint();
  // not quite a nop. The guards should have verified that the m_type field
  // is compatible, but for objects we need to go one step further and
  // ensure that we're dealing with the right class.
  // NULL inputs only get traced when constraint is nullable.
  assert(i.inputs.size() == 1);
  if (!i.inputs[0]->isObject()) return; // nop.

  // Get the input's class from ObjectData->m_cls
  const Location& in = i.inputs[0]->location;
  PhysReg src = getReg(in);
  ScratchReg inCls(m_regMap);
  if (i.inputs[0]->rtt.isRef()) {
    emitDerefRef(a, src, r(inCls));
    src = r(inCls);
  }
  a.  load_reg64_disp_reg64(src, ObjectData::getVMClassOffset(), r(inCls));

  ScratchReg cls(m_regMap);
  // Constraint may not be in the class-hierarchy of the method being traced,
  // look up the class handle and emit code to put the Class* into a reg.
  bool isSpecial = tc.isSelf() || tc.isParent() || tc.isCallable();
  const Class* constraint = nullptr;
  const StringData* clsName;
  if (!isSpecial) {
    clsName = tc.typeName();
    constraint = Unit::lookupUniqueClass(clsName);
  } else {
    if (tc.isSelf()) {
      tc.selfToClass(curFunc(), &constraint);
    } else if (tc.isParent()) {
      tc.parentToClass(curFunc(), &constraint);
    } else {
      assert(tc.isCallable());
      EMIT_RCALL(a, i, VerifyParamCallable, R(src), IMM(param));
      return;
    }
    clsName = constraint ? constraint->preClass()->name() : nullptr;
  }
  Class::initInstanceBits();
  bool haveBit = Class::haveInstanceBit(clsName);
  // See the first big comment in emitInstanceCheck for the contract here
  if (!haveBit || !classIsUniqueOrCtxParent(constraint)) {
    emitClassToReg(a, clsName, r(cls));
  }

  if (haveBit || classIsUniqueNormalClass(constraint)) {
    LazyScratchReg dummy(m_regMap);
    Stats::emitInc(a, Stats::Tx64_VerifyParamTypeFast);
    emitInstanceCheck(t, i, clsName, constraint, inCls, cls, dummy);
  } else {
    // Compare this class to the incoming object's class. If the
    // typehint's class is not present, can not be an instance, unless
    // this is a typedef.  The slow path handles that case.
    Stats::emitInc(a, Stats::Tx64_VerifyParamTypeSlowShortcut);
    a.  cmp_reg64_reg64(r(inCls), r(cls));
    {
      JccBlock<CC_E> subclassCheck(a);
      // Call helper since ObjectData::instanceof is a member function
      if (false) {
        VerifyParamTypeSlow(constraint, constraint, param, &tc);
      }
      EMIT_RCALL(a, i, VerifyParamTypeSlow, R(inCls), R(cls),
                 IMM(param),
                 IMM(uintptr_t(&tc)));
    }
  }
}

void
TranslatorX64::analyzeInstanceOfD(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(i.outStack && !i.outLocal);
  i.m_txFlags = planHingesOnRefcounting(i.inputs[0]->outerType());
}

// Helpers for InstanceOfD. They return uint64_t so the translated
// code calling them doesn't have to zero-extend the lower byte.
static uint64_t
InstanceOfDSlow(const Class* cls, const Class* constraint) {
  Stats::inc(Stats::Tx64_InstanceOfDSlow);
  return constraint && cls->classof(constraint);
}

static uint64_t
InstanceOfDSlowInterface(const Class* cls, const Class* parent) {
  Stats::inc(Stats::Tx64_InstanceOfDInterface);
  return parent && cls->classof(parent->preClass());
}

void
TranslatorX64::translateInstanceOfD(const Tracelet& t,
                                    const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  assert(i.outStack && !i.outLocal);

  DynLocation* input0 = i.inputs[0];
  bool input0IsLoc = input0->isLocal();
  DataType type = input0->valueType();
  PhysReg srcReg;
  LazyScratchReg result(m_regMap);
  LazyScratchReg srcScratch(m_regMap);
  TCA patchAddr = nullptr;
  boost::scoped_ptr<DiamondReturn> retFromNullThis;

  if (!i.changesPC) {
    result.alloc();
  } else {
    Stats::emitInc(a, Stats::Tx64_InstanceOfDFused);
  }

  if (i.wasGroupedWith(OpThis, OpBareThis)) {
    assert(curFunc()->cls());
    srcScratch.alloc();
    srcReg = r(srcScratch);
    a.    load_reg64_disp_reg64(rVmFp, AROFF(m_this), srcReg);
    if (i.prev->op() == OpThis) {
      assert(i.prev->guardedThis);
    } else {
      if (i.prev->imm[0].u_OA) {
        // Warn on null $this
        if (!i.changesPC) {
          retFromNullThis.reset(new DiamondReturn);
        }
        a.  testb(1, rbyte(srcReg));
        {
          UnlikelyIfBlock ifNull(CC_NZ, a, astubs, retFromNullThis.get());
          EMIT_RCALL(astubs, i, warnNullThis);
          if (i.changesPC) {
            fuseBranchAfterStaticBool(astubs, t, i, false);
          } else {
            emitImmReg(astubs, false, r(result));
          }
        }
      } else {
        if (!i.changesPC) {
          emitImmReg(a, false, r(result));
        }
        a.  testb(1, rbyte(srcReg));
        if (i.changesPC) {
          JccBlock<CC_Z> ifNull(a);
          fuseBranchAfterStaticBool(a, t, i, false);
        } else {
          patchAddr = a.code.frontier;
          a.  jcc(CC_NZ, patchAddr);
        }
      }
    }
    input0IsLoc = true; // we dont want a decRef
    type = KindOfObject;
  } else {
    srcReg = getReg(input0->location);
  }

  if (type != KindOfObject) {
    Stats::emitInc(a, Stats::Tx64_InstanceOfDBypass);
    // All non-object inputs are not instances
    if (!input0IsLoc) {
      assert(!input0->isRef());
      emitDecRef(i, srcReg, type);
    }
    if (i.changesPC) {
      fuseBranchAfterStaticBool(a, t, i, false);
      assert(!patchAddr);
      return;
    } else {
      emitImmReg(a, false, r(result));
    }
  } else {
    // Get the input's class from ObjectData->m_cls
    ScratchReg inCls(m_regMap);
    PhysReg baseReg = srcReg;
    if (input0->rtt.isRef()) {
      assert(input0IsLoc);
      emitDerefRef(a, srcReg, r(inCls));
      baseReg = r(inCls);
    }
    a.  load_reg64_disp_reg64(baseReg, ObjectData::getVMClassOffset(),
                              r(inCls));
    if (!input0IsLoc) {
      emitDecRef(i, srcReg, type);
    }

    const StringData* clsName = curUnit()->lookupLitstrId(i.imm[0].u_SA);
    Class* maybeCls = Unit::lookupUniqueClass(clsName);

    // maybeInterface is just used as a hint: If it's a trait/interface now but
    // a class at runtime, InstanceOfDSlowInterface will still do the right
    // thing but more slowly. fastPath is guaranteed to be correct.
    Class::initInstanceBits();
    bool haveBit = Class::haveInstanceBit(clsName);
    bool maybeInterface = maybeCls && !haveBit &&
      (maybeCls->attrs() & (AttrTrait | AttrInterface));
    bool fastPath = !maybeInterface &&
      (classIsUniqueNormalClass(maybeCls) || haveBit);
    auto afterHelper = [&] {
      if (i.changesPC) fuseBranchAfterHelper(t, i);
      else emitMovRegReg(a, rax, r(result));
    };

    ScratchReg cls(m_regMap);
    // See the first big comment in emitInstanceCheck for the contract here
    if (!haveBit || !classIsUniqueOrCtxParent(maybeCls)) {
      emitClassToReg(a, clsName, r(cls));
    }
    if (maybeInterface) {
      EMIT_CALL(a, InstanceOfDSlowInterface, R(inCls), R(cls));
      afterHelper();
    } else if (fastPath) {
      Stats::emitInc(a, Stats::Tx64_InstanceOfDFast);
      emitInstanceCheck(t, i, clsName, maybeCls, inCls, cls, result);
    } else {
      EMIT_CALL(a, InstanceOfDSlow, R(inCls), R(cls));
      afterHelper();
    }
    if (i.changesPC) {
      assert(!patchAddr && !retFromNullThis);
      return;
    }
  }

  assert(!patchAddr || !retFromNullThis);
  assert(IMPLIES(retFromNullThis, !i.changesPC));
  if (patchAddr) {
    a. patchJcc(patchAddr, a.code.frontier);
  } else {
    retFromNullThis.reset();
  }

  // Bind result and destination
  assert(!i.changesPC);
  m_regMap.bindScratch(result, i.outStack->location, i.outStack->outerType(),
                       RegInfo::DIRTY);
}

void
TranslatorX64::emitInstanceCheck(const Tracelet& t,
                                 const NormalizedInstruction& i,
                                 const StringData* clsName,
                                 const Class* klass,
                                 const ScratchReg& inCls,
                                 const ScratchReg& cls,
                                 const LazyScratchReg& result) {
  LazyScratchReg one(m_regMap);
  bool verifying = i.op() == OpVerifyParamType;
  bool haveBit = Class::haveInstanceBit(clsName);
  assert(IMPLIES(verifying, !i.changesPC));

  TCA equalJe = nullptr;
  TCA parentJmp = nullptr;
  TCA parentFailJe = nullptr;

  if (i.changesPC) {
    fuseBranchSync(t, i);
  } else if (!verifying) {
    one.alloc();
    emitImmReg(a, 1, r(one));
  }
  std::unique_ptr<FreezeRegs> ice;
  if (!verifying) ice.reset(new FreezeRegs(m_regMap));

  if (haveBit) {
    Stats::emitInc(a, verifying ? Stats::Tx64_VerifyParamTypeBit
                                : Stats::Tx64_InstanceOfDBit);
    translatorAssert(a, CC_NZ, "Class instance bits must be initialized", [&]{
      a.testb(0x1, r(inCls)[Class::instanceBitsOff()]);
    });
  }

  // Are the Class*s the exact same class? If we have a bit then this is the
  // only part of the translation that needs a pointer to the class. If the
  // class is also unique (or a parent class of the current context), we can
  // burn its value into the translation, so it won't be in *cls and we use an
  // immediate.
  if (haveBit && classIsUniqueOrCtxParent(klass)) {
    a.    cmp_imm64_reg64(int64_t(klass), r(inCls));
  } else {
    a.    cmp_reg64_reg64(r(inCls), r(cls));
  }
  {
    std::unique_ptr<IfElseBlock<CC_NE>> ifElse;
    if (verifying) {
      equalJe = a.code.frontier;
      a.  je8(equalJe);
    } else {
      Stats::emitInc(a, Stats::Tx64_InstanceOfDEqual, 1, CC_E);
      if (i.changesPC) {
        fuseHalfBranchAfterBool(t, i, CC_E, true);
      } else {
        ifElse.reset(new IfElseBlock<CC_NE>(a));
        a.  mov_reg64_reg64(r(one), r(result));
        ifElse->Else();
      }
    }

    // Default to false and override if all the checks succeed
    if (!i.changesPC && !verifying) {
      emitImmReg(a, 0, r(result));
    }

    int offset;
    uint8_t mask;
    if (Class::getInstanceBitMask(clsName, offset, mask)) {
      // We don't need to check that the parent class exists: if it doesn't
      // exist then it's impossible for this object to be an instance of it,
      // and the corresponding bit won't be set.
      a.  testb((int8_t)mask, r(inCls)[offset]);
      if (verifying) {
        {
          UnlikelyIfBlock fail(CC_Z, a, astubs);
          EMIT_RCALL(astubs, i, VerifyParamTypeFail, IMM(i.imm[0].u_IVA));
        }
        a.patchJcc8(equalJe, a.code.frontier);
      } else if (i.changesPC) {
        fuseBranchAfterBool(t, i, CC_NZ);
      } else {
        a.cmov_reg64_reg64(CC_NZ, r(one), r(result));
      }
      return;
    }

    assert(klass);
    // Is our inheritence hierarchy no shorter than the candidate?
    unsigned parentVecLen = klass->classVecLen();
    a.  cmp_imm32_disp_reg32(parentVecLen, Class::classVecLenOff(),
                             r(inCls));
    {
      JccBlock<CC_B> veclen(a);

      // Is the spot in our inheritance hierarchy corresponding to the
      // candidate equal to the candidate? *cls might still be NULL here
      // (meaning the class isn't defined yet) but that's ok: if it is null
      // the cmp will always fail.
      int offset = Class::classVecOff() + sizeof(Class*) * (parentVecLen-1);
      if (Class::alwaysLowMem()) {
        a.cmp_reg32_disp_reg64(r(cls), offset, r(inCls));
      } else {
        a.cmp_reg64_disp_reg64(r(cls), offset, r(inCls));
      }
      if (verifying) {
        parentFailJe = a.code.frontier;
        a.jne8(parentFailJe);
        parentJmp = a.code.frontier;
        a.jmp8(parentJmp);
      } else {
        Stats::emitInc(a, Stats::Tx64_InstanceOfDFinalTrue, 1, CC_E);
        Stats::emitInc(a, Stats::Tx64_InstanceOfDFinalFalse, 1, CC_NE);
        if (i.changesPC) {
          // The decision is done here but if we fallthrough it's to the
          // failure case, so it's ok to only bind half the branch.
          fuseHalfBranchAfterBool(t, i, CC_E, true);
        } else {
          a.cmov_reg64_reg64(CC_E, r(one), r(result));
        }
      }
    }

    // If execution makes it here the check has failed
    if (i.changesPC) {
      fuseBranchAfterStaticBool(a, t, i, false, false);
    } else if (verifying) {
      a.patchJcc8(parentFailJe, a.code.frontier);
      stubBlock(a, astubs, [&]{
          EMIT_RCALL(astubs, i, VerifyParamTypeFail, IMM(i.imm[0].u_IVA));
        });
    }
  }

  if (verifying) {
    a.patchJcc8(equalJe, a.code.frontier);
    Stats::emitInc(a, Stats::Tx64_VerifyParamTypeEqual);
    a.patchJmp8(parentJmp, a.code.frontier);
    Stats::emitInc(a, Stats::Tx64_VerifyParamTypePass);
  }
}

static void translatorAssertFail(const char* msg) {
  VMExecutionContext::PrintTCCallerInfo();
  std::cerr << "Failed assertion in translated code: " << msg << std::endl;
  not_reached();
}

template<typename L>
void TranslatorX64::translatorAssert(X64Assembler& a, ConditionCode cc,
                                     const char* msg, L setup) {
  if (!debug) return;
  setup();
  TCA jmp = a.code.frontier;
  a.jcc8(cc, jmp);
  emitImmReg(a, int64_t(msg), rdi);
  a.call((TCA)translatorAssertFail);
  recordCall(a, *m_curNI);
  a.patchJcc8(jmp, a.code.frontier);
}

// note: this is ok for all the iterkey/itervalue stuff too
void
TranslatorX64::analyzeIterInit(Tracelet& t, NormalizedInstruction& ni) {
  DataType inType = ni.inputs[0]->valueType();
  ni.m_txFlags = supportedPlan(inType == KindOfArray || inType == KindOfObject);
}

void
TranslatorX64::analyzeIterInitK(Tracelet& t, NormalizedInstruction& ni) {
  DataType inType = ni.inputs[0]->valueType();
  ni.m_txFlags = supportedPlan(inType == KindOfArray || inType == KindOfObject);
}

void TranslatorX64::translateBasicIterInit(const Tracelet& t,
                                           const NormalizedInstruction& ni) {
  const int kValIdx = 0;
  DynLocation* in = ni.inputs[kValIdx];
  assert(in->outerType() != KindOfRef);
  SKTRACE(1, ni.source, "IterInit: committed to translation\n");
  PhysReg src = getReg(in->location);
  SrcKey taken, notTaken;
  branchDests(t, ni, &taken, &notTaken, 1 /* immIdx */);
  Location iterLoc(Location::Iter, ni.imm[0].u_IVA);
  switch (in->valueType()) {
  case KindOfArray: {
    if (false) { // typecheck
      Iter *dest = nullptr;
      HphpArray *arr = nullptr;
      TypedValue *val = nullptr;
      TypedValue *key = nullptr;
      new_iter_array(dest, arr, val);
      new_iter_array_key(dest, arr, val, key);
    }
    if (ni.outLocal2) {
      EMIT_RCALL(a, ni, new_iter_array_key, A(iterLoc), R(src),
                 A(ni.outLocal->location), A(ni.outLocal2->location));
    } else {
      EMIT_RCALL(a, ni, new_iter_array, A(iterLoc), R(src),
                 A(ni.outLocal->location));
    }
    break;
  }
  case KindOfObject: {
    if (false) { // typecheck
      Iter *dest = nullptr;
      ObjectData *obj = nullptr;
      Class *ctx = nullptr;
      TypedValue *val = nullptr;
      TypedValue *key = nullptr;
      new_iter_object(dest, obj, ctx, val, key);
    }
    Class* ctx = arGetContextClass(curFrame());
    m_regMap.scrubLoc(in->location);
    EMIT_CALL(a, new_iter_object, A(iterLoc), R(src),
              IMM((uintptr_t)ctx),
              A(ni.outLocal->location),
              ni.outLocal2 ? A(ni.outLocal2->location) : IMM(0));
    recordReentrantCall(a, ni, false, -1);
    break;
  }
  default: not_reached();
  }
  syncOutputs(t); // Ends BB
  // If a new iterator is created, new_iter_* will not adjust the refcount of
  // the input. If a new iterator is not created, new_iter_* will decRef the
  // input for us.  new_iter_* returns 0 if an iterator was not created,
  // otherwise it returns 1.
  prepareForTestAndSmash(kTestRegRegLen, kAlignJccAndJmp);
  a.    test_reg64_reg64(rax, rax);
  emitCondJmp(taken, notTaken, CC_Z);
}

void TranslatorX64::translateIterInit(const Tracelet& t,
                                      const NormalizedInstruction& ni) {
  assert(ni.inputs.size() == 1);
  assert(ni.outLocal);
  assert(!ni.outStack && !ni.outLocal2);
  translateBasicIterInit(t, ni);
}

void TranslatorX64::translateIterInitK(const Tracelet& t,
                                       const NormalizedInstruction& ni) {
  assert(ni.inputs.size() == 1);
  assert(ni.outLocal && ni.outLocal2);
  assert(!ni.outStack);
  translateBasicIterInit(t, ni);
}

void
TranslatorX64::analyzeIterNext(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  i.m_txFlags = Supported;
}

void
TranslatorX64::analyzeIterNextK(Tracelet& t, NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  i.m_txFlags = Supported;
}

void
TranslatorX64::translateBasicIterNext(const Tracelet& t,
                                      const NormalizedInstruction& i) {
  if (false) { // type check
    Iter* it = nullptr;
    TypedValue* val = nullptr;
    TypedValue* key = nullptr;
    int64_t ret = iter_next(it, val);
    ret = iter_next_key(it, val, key);
    if (ret) printf("\n");
  }
  m_regMap.cleanAll(); // input might be in-flight
  // If the iterator reaches the end, iter_next will handle
  // freeing the iterator and it will decRef the array
  Location iterLoc(Location::Iter, i.imm[0].u_IVA);
  if (i.outLocal2) {
    EMIT_CALL(a, iter_next_key, A(iterLoc),
              A(i.outLocal->location), A(i.outLocal2->location));
  } else {
    EMIT_CALL(a, iter_next, A(iterLoc),
              A(i.outLocal->location));
  }
  recordReentrantCall(a, i);
  ScratchReg raxScratch(m_regMap, rax);

  // syncOutputs before we handle the branch.
  syncOutputs(t);
  SrcKey taken, notTaken;
  branchDests(t, i, &taken, &notTaken, 1 /* destImmIdx */);

  prepareForTestAndSmash(kTestRegRegLen, kAlignJccAndJmp);
  a.   test_reg64_reg64(rax, rax);
  emitCondJmp(taken, notTaken, CC_NZ);
}

void
TranslatorX64::translateIterNext(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outStack && !i.outLocal2);
  assert(i.outLocal);
  translateBasicIterNext(t, i);
}

void
TranslatorX64::translateIterNextK(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);
  assert(!i.outStack);
  assert(i.outLocal && i.outLocal2);
  translateBasicIterNext(t, i);
}

// PSEUDOINSTR_DISPATCH is a switch() fragment that routes opcodes to their
// shared handlers, as per the PSEUDOINSTRS macro.
#define PSEUDOINSTR_DISPATCH(func)              \
  case OpBitAnd:                                \
  case OpBitOr:                                 \
  case OpBitXor:                                \
  case OpSub:                                   \
  case OpMul:                                   \
    func(BinaryArithOp, t, i)                   \
  case OpSame:                                  \
  case OpNSame:                                 \
    func(SameOp, t, i)                          \
  case OpEq:                                    \
  case OpNeq:                                   \
    func(EqOp, t, i)                            \
  case OpLt:                                    \
  case OpLte:                                   \
  case OpGt:                                    \
  case OpGte:                                   \
    func(LtGtOp, t, i)                          \
  case OpEmptyL:                                \
  case OpCastBool:                              \
    func(UnaryBooleanOp, t, i)                  \
  case OpJmpZ:                                  \
  case OpJmpNZ:                                 \
    func(BranchOp, t, i)                        \
  case OpSetL:                                  \
  case OpBindL:                                 \
    func(AssignToLocalOp, t, i)                 \
  case OpFPassC:                                \
  case OpFPassCW:                               \
  case OpFPassCE:                               \
    func(FPassCOp, t, i)                        \
  case OpFPushCuf:                              \
  case OpFPushCufF:                             \
  case OpFPushCufSafe:                          \
    func(FPushCufOp, t, i)                      \
  case OpIssetL:                                \
  case OpIsNullL:                               \
  case OpIsStringL:                             \
  case OpIsArrayL:                              \
  case OpIsIntL:                                \
  case OpIsObjectL:                             \
  case OpIsBoolL:                               \
  case OpIsDoubleL:                             \
  case OpIsNullC:                               \
  case OpIsStringC:                             \
  case OpIsArrayC:                              \
  case OpIsIntC:                                \
  case OpIsObjectC:                             \
  case OpIsBoolC:                               \
  case OpIsDoubleC:                             \
    func(CheckTypeOp, t, i)

void
TranslatorX64::analyzeInstr(Tracelet& t,
                            NormalizedInstruction& i) {
  const Opcode op = i.op();
  switch (op) {
#define CASE(iNm) \
  case Op ## iNm: { \
    analyze ## iNm(t, i); \
  } break;
#define ANALYZE(a, b, c) analyze ## a(b, c); break;
  INSTRS
  PSEUDOINSTR_DISPATCH(ANALYZE)

#undef ANALYZE
#undef CASE
    default: {
      assert(i.m_txFlags == Interp);
    }
  }
  SKTRACE(1, i.source, "translation plan: %x\n", i.m_txFlags);
}

bool
TranslatorX64::dontGuardAnyInputs(Opcode op) {
  switch (op) {
#define CASE(iNm) case Op ## iNm:
#define NOOP(a, b, c)
  INSTRS
    PSEUDOINSTR_DISPATCH(NOOP)
    return false;
  }
  return true;
#undef NOOP
#undef CASE
}

void TranslatorX64::emitOneGuard(const Tracelet& t,
                                 const NormalizedInstruction& i,
                                 PhysReg reg, int disp, DataType type,
                                 TCA &sideExit) {
  bool isFirstInstr = (&i == t.m_instrStream.first);
  bool regsClean = !m_regMap.hasDirtyRegs(i.stackOff);
  ConditionCode cc = IS_STRING_TYPE(type) ? CC_Z : CC_NZ;
  emitTypeCheck(a, type, reg, disp);
  if (isFirstInstr) {
    SrcRec& srcRec = *getSrcRec(t.m_sk);
    // If it's the first instruction, we haven't made any forward
    // progress yet, so this is really a tracelet-level guard rather
    // than a side exit. If we tried to "side exit", we'd come right
    // back to this check!
    //
    // We need to record this as a fallback branch.
    emitFallbackJmp(srcRec, cc);
  } else if (!sideExit || regsClean) {
    if (regsClean) {
      // If we have no dirty regs and no stack offset at our destination, we
      // can do this with a single jnz. If the destination has a translation
      // already we'd emit an unlikely backwards jne, so use semiLikelyIfBlock
      // in that case.
      if (i.stackOff == 0 && !lookupTranslation(i.source)) {
        Stats::emitInc(a, Stats::Tx64_OneGuardShort);
        emitBindJcc(a, cc, i.source, REQ_BIND_SIDE_EXIT);
      } else {
        Stats::emitInc(a, Stats::Tx64_OneGuardLong);
        semiLikelyIfBlock(cc, a, [&]{
          emitSideExit(a, i, false /*next*/);
        });
      }
    } else {
      UnlikelyIfBlock ifFail(cc, a, astubs);
      sideExit = astubs.code.frontier;
      emitSideExit(astubs, i, false /*next*/);
    }
  } else {
    a.jcc(cc, sideExit);
  }
}

// Emit necessary guards for variants and pseudo-main locals before instr i.
// For HHIR, this only inserts guards for pseudo-main locals.  Variants are
// guarded in a different way.
void
TranslatorX64::emitVariantGuards(const Tracelet& t,
                                 const NormalizedInstruction& i) {
  bool pseudoMain = Translator::liveFrameIsPseudoMain();
  bool isFirstInstr = (&i == t.m_instrStream.first);
  TCA sideExit = nullptr;
  const NormalizedInstruction *base = &i;
  while (base->grouped) {
    base = base->prev;
    assert(base);
  }
  for (size_t in = 0; in < i.inputs.size(); ++in) {
    DynLocation* input = i.inputs[in];
    if (!input->isValue()) continue;
    bool isRef = input->isRef() &&
      !i.ignoreInnerType &&
      input->rtt.innerType() != KindOfInvalid;
    bool modifiableLocal = pseudoMain && input->isLocal() &&
      !input->rtt.isVagueValue();

    if (!modifiableLocal && !isRef) continue;

    SKTRACE(1, i.source, "guarding %s: (%s:%d) :: %d!\n",
            modifiableLocal ? "pseudoMain local" : "variant inner",
            input->location.spaceName(),
            input->location.offset,
            input->rtt.valueType());
    // TODO task 1122807: don't check the inner type if we've already
    // checked it and have executed no possibly-aliasing instructions in
    // the meanwhile.
    if (modifiableLocal) {
      if (m_useHHIR) {
        RuntimeType& rtt = input->rtt;
        JIT::Type type = JIT::Type::fromRuntimeType(rtt);
        if (isFirstInstr) {
          m_hhbcTrans->guardTypeLocal(input->location.offset, type);
        } else {
          m_hhbcTrans->checkTypeLocal(input->location.offset, type);
        }
      } else {
        PhysReg reg;
        int disp;
        locToRegDisp(input->location, &reg, &disp);
        emitOneGuard(t, *base, reg, disp,
                     input->rtt.outerType(), sideExit);
      }
    }
    if (isRef && !m_useHHIR) {
      m_regMap.allocInputReg(i, in);
      emitOneGuard(t, *base, getReg(input->location), RefData::tvOffset(),
                   input->rtt.innerType(), sideExit);
    }
  }
}

void
TranslatorX64::emitPredictionGuards(const NormalizedInstruction& i) {
  if (!i.outputPredicted || i.breaksTracelet) return;
  NormalizedInstruction::OutputUse u = i.getOutputUsage(i.outStack);

  switch (u) {
    case NormalizedInstruction::OutputUsed:
      break;
    case NormalizedInstruction::OutputUnused:
      return;
    case NormalizedInstruction::OutputInferred:
      Stats::emitInc(a, Stats::TC_TypePredOverridden);
      return;
    case NormalizedInstruction::OutputDoesntCare:
      Stats::emitInc(a, Stats::TC_TypePredUnneeded);
      return;
  }

  assert(i.outStack);
  PhysReg base;
  int disp;
  locToRegDisp(i.outStack->location, &base, &disp);
  assert(base == rVmSp);
  TRACE(1, "PREDGUARD: %p dt %d offset %d voffset %" PRId64 "\n",
        a.code.frontier, i.outStack->outerType(), disp,
        i.outStack->location.offset);
  DataType type = i.outStack->outerType();
  emitTypeCheck(a, type, rVmSp, disp);
  ConditionCode cc = IS_STRING_TYPE(type) ? CC_Z : CC_NZ;
  {
    UnlikelyIfBlock branchToSideExit(cc, a, astubs);
    Stats::emitInc(astubs, Stats::TC_TypePredMiss);
    emitSideExit(astubs, i, true);
  }
  Stats::emitInc(a, Stats::TC_TypePredHit);
}

static void failedTypePred() {
  raise_error("A type prediction was incorrect");
}

void
TranslatorX64::translateInstrWork(const Tracelet& t,
                                  const NormalizedInstruction& i) {
  const Opcode op = i.op();
  switch (op) {
#define CASE(iNm)                               \
  case Op ## iNm:                             \
                  translate ## iNm(t, i);     \
    break;
#define TRANSLATE(a, b, c) translate ## a(b, c); break;
    INSTRS
      PSEUDOINSTR_DISPATCH(TRANSLATE)
#undef TRANSLATE
#undef CASE
  default:
    assert(false);
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
   *      accessible locations. This also means that all refcounts must be
   *      up to date.
   */
  assert(!m_useHHIR);
  assert(!(RuntimeOption::EvalJitUseIR && RuntimeOption::EvalHHIRDisableTx64));
  assert(!i.outStack || i.outStack->isStack());
  assert(!i.outLocal || i.outLocal->isLocal());
  const char *opNames[] = {
#define O(name, imm, push, pop, flags) \
#name,
  OPCODES
#undef O
  };
  SpaceRecorder sr(opNames[i.op()], a);
  SKTRACE(1, i.source, "translate %#lx\n", long(a.code.frontier));
  const Opcode op = i.op();

  TCA start = a.code.frontier;
  TCA astart = astubs.code.frontier;

  m_regMap.bumpEpoch();
  // Allocate the input regs upfront unless instructed otherwise
  // or the instruction is interpreted
  if (!i.manuallyAllocInputs && i.m_txFlags) {
    m_regMap.allocInputRegs(i);
  }

  if (debug) {
    for (unsigned j = 0; j < i.inputs.size(); j++) {
      if (i.inputWasInferred(j)) {
        DynLocation* dl = i.inputs[j];
        assert(dl->rtt.isValue() &&
               !dl->rtt.isVagueValue() &&
               dl->outerType() != KindOfInvalid);
        PhysReg base;
        int disp;
        locToRegDisp(dl->location, &base, &disp);
        DataType type = dl->rtt.typeCheckValue();
        emitTypeCheck(a, type, base, disp);
        ConditionCode cc = IS_STRING_TYPE(type) ? CC_Z : CC_NZ;
        {
          UnlikelyIfBlock typePredFailed(cc, a, astubs);
          EMIT_CALL(astubs, failedTypePred);
          recordReentrantStubCall(i);
        }
      }
    }
  }

  if (!i.grouped) {
    emitVariantGuards(t, i);
    const NormalizedInstruction* n = &i;
    while (n->next && n->next->grouped) {
      n = n->next;
      emitVariantGuards(t, *n);
    }
  }

  // Allocate the input regs upfront unless instructed otherwise
  // or the instruction is interpreted
  if (!i.manuallyAllocInputs && i.m_txFlags) {
    m_regMap.allocInputRegs(i);
  }

  if (i.m_txFlags == Interp || RuntimeOption::EvalThreadingJit) {
    // If the problem is local to this instruction, just call out to
    // the interpreter. emitInterpOne will perform end-of-tracelet duties
    // if this instruction ends the tracelet.
    SKTRACE(1, i.source, "Interp\n");
    emitInterpOne(t, i);
  } else {
    // Actually translate the instruction's body.
    Stats::emitIncTranslOp(a, op, RuntimeOption::EnableInstructionCounts);
    translateInstrWork(t, i);
  }

  // Invalidate locations that are no longer live
  for (unsigned k = 0; k < i.deadLocs.size(); ++k) {
    const Location& l = i.deadLocs[k];
    m_regMap.invalidate(l);
  }

  // Kill any live regs that won't be of further use in this trace.
  RegSet live = m_regMap.getRegsLike(RegInfo::DIRTY) |
    m_regMap.getRegsLike(RegInfo::CLEAN);
  PhysReg pr;
  while (live.findFirst(pr)) {
    live.remove(pr);
    const RegInfo* ri = m_regMap.getInfo(pr);
    assert(ri->m_state == RegInfo::CLEAN || ri->m_state == RegInfo::DIRTY);
    bool dirty = ri->m_state == RegInfo::DIRTY;
    if (ri->m_cont.m_kind != RegContent::Loc) continue;
    const Location loc = ri->m_cont.m_loc;
    // These heuristics do poorly on stack slots, which are more like
    // ephemeral temps.
    if (loc.space != Location::Local) continue;
    if (false && dirty && !t.isWrittenAfterInstr(loc, i)) {
      // This seems plausible enough: the intuition is that carrying aroud
      // a register we'll read, but not write, in a dirty state, has a cost
      // because any control-flow diamonds will have to spill it and then
      // refill it. It appears to hurt performance today, though.
      m_regMap.cleanLoc(loc);
    }
    if (t.isLiveAfterInstr(loc, i)) continue;
    SKTRACE(1, i.source, "killing %s reg %d for (%s, %d)\n",
            dirty ? "dirty" : "clean", (int)pr, loc.spaceName(), loc.offset);
    if (dirty) {
       m_regMap.cleanLoc(loc);
    }
    assert(ri->m_state == RegInfo::CLEAN);
    m_regMap.smashLoc(loc);
  }

  emitPredictionGuards(i);
  recordBCInstr(op, a, start);
  recordBCInstr(op + Op_count, astubs, astart);

  if (i.breaksTracelet && !i.changesPC) {
    // If this instruction's opcode always ends the tracelet then the
    // instruction case is responsible for performing end-of-tracelet
    // duties. Otherwise, we handle ending the tracelet here.
    syncOutputs(t);
    emitBindJmp(t.m_nextSk);
  }

  m_regMap.assertNoScratch();
}

bool
TranslatorX64::checkTranslationLimit(SrcKey sk,
                                     const SrcRec& srcRec) const {
  if (srcRec.translations().size() == SrcRec::kMaxTranslations) {
    INC_TPC(max_trans);
    if (debug && Trace::moduleEnabled(Trace::tx64, 2)) {
      const vector<TCA>& tns = srcRec.translations();
      TRACE(1, "Too many (%" PRId64 ") translations: %s, BC offset %d\n",
            tns.size(), curUnit()->filepath()->data(),
            sk.offset());
      SKTRACE(2, sk, "{\n", tns.size());
      TCA topTrans = srcRec.getTopTranslation();
      for (size_t i = 0; i < tns.size(); ++i) {
        const TransRec* rec = getTransRec(tns[i]);
        assert(rec);
        SKTRACE(2, sk, "%d %p\n", i, tns[i]);
        if (tns[i] == topTrans) {
          SKTRACE(2, sk, "%d: *Top*\n", i);
        }
        if (rec->kind == TransAnchor) {
          SKTRACE(2, sk, "%d: Anchor\n", i);
        } else {
          SKTRACE(2, sk, "%d: guards {\n", i);
          for (unsigned j = 0; j < rec->dependencies.size(); ++j) {
            TRACE(2, rec->dependencies[j]);
          }
          SKTRACE(2, sk, "%d } guards\n", i);
        }
      }
      SKTRACE(2, sk, "} /* Too many translations */\n");
    }
    return true;
  }

  return false;
}

void
TranslatorX64::emitGuardChecks(X64Assembler& a,
                               SrcKey sk,
                               const ChangeMap& dependencies,
                               const RefDeps& refDeps,
                               SrcRec& fail) {
  if (Trace::moduleEnabled(Trace::stats, 2)) {
    Stats::emitInc(a, Stats::TraceletGuard_enter);
  }

  bool pseudoMain = Translator::liveFrameIsPseudoMain();

  emitRB(a, RBTypeTraceletGuards, sk);
  for (DepMap::const_iterator dep = dependencies.begin();
       dep != dependencies.end();
       ++dep) {
    if (!pseudoMain || !dep->second->isLocal() || !dep->second->isValue()) {
      checkType(a, dep->first, dep->second->rtt, fail);
    } else {
      TRACE(3, "Skipping tracelet guard for %s %d\n",
            dep->second->location.pretty().c_str(),
            (int)dep->second->rtt.outerType());
    }
  }

  if (m_useHHIR) {
    irEmitLoadDeps();
  }

  checkRefs(a, sk, refDeps, fail);

  if (Trace::moduleEnabled(Trace::stats, 2)) {
    Stats::emitInc(a, Stats::TraceletGuard_execute);
  }
}


void dumpTranslationInfo(const Tracelet& t, TCA postGuards) {
  if (!debug) return;

  SrcKey sk = t.m_sk;

  TRACE(3, "----------------------------------------------\n");
  TRACE(3, "  Translating from file %s:%d %s at %p:\n",
        curUnit()->filepath()->data(),
        curUnit()->getLineNumber(sk.offset()),
        curFunc()->name()->data(),
        postGuards);
  TRACE(3, "  preconds:\n");
  TRACE(3, "    types:\n");
  for (DepMap::const_iterator i = t.m_dependencies.begin();
       i != t.m_dependencies.end(); ++i) {
    TRACE(3, "      %-5s\n", i->second->pretty().c_str());
  }
  if (t.m_refDeps.size() != 0) {
    TRACE(3, "    refs:\n");
    for (RefDeps::ArMap::const_iterator i = t.m_refDeps.m_arMap.begin();
        i != t.m_refDeps.m_arMap.end();
        ++i) {
      TRACE(3, "      (ActRec %" PRId64 " : %-5s)\n", i->first,
        i->second.pretty().c_str());
    }
  }
  TRACE(3, "  postconds:\n");
  for (ChangeMap::const_iterator i = t.m_changes.begin();
       i != t.m_changes.end(); ++i) {
    TRACE(3, "    %-5s\n", i->second->pretty().c_str());
  }
  for (auto ni = t.m_instrStream.first; ni; ni = ni->next) {
    TRACE(3, "  %6d: %s\n", ni->source.offset(),
      instrToString(ni->pc()).c_str());
    if (ni->breaksTracelet) break;
  }
  TRACE(3, "----------------------------------------------\n");
  if (Trace::moduleEnabled(Trace::tx64, 5)) {
    // prettyStack() expects to use vmpc(). Leave it in the state we
    // found it since this code is debug-only, and we don't want behavior
    // to vary across the optimized/debug builds.
    PC oldPC = vmpc();
    vmpc() = curUnit()->at(sk.offset());
    TRACE(3, g_vmContext->prettyStack(string(" tx64 ")));
    vmpc() = oldPC;
    TRACE(3, "----------------------------------------------\n");
  }
}

void
TranslatorX64::translateTracelet(SrcKey sk, bool considerHHIR/*=true*/,
                                bool dryRun /*= false */) {
  std::unique_ptr<Tracelet> tp = analyze(sk);
  Tracelet& t = *tp;
  m_curTrace = &t;
  Nuller<Tracelet> ctNuller(&m_curTrace);

  SKTRACE(1, sk, "translateTracelet\n");
  assert(m_srcDB.find(sk));
  assert(m_regMap.pristine());

  TCA                     start = a.code.frontier;
  TCA                     stubStart = astubs.code.frontier;
  TCA                     counterStart = 0;
  uint8_t                 counterLen = 0;
  SrcRec&                 srcRec = *getSrcRec(sk);
  vector<TransBCMapping>  bcMapping;
  TransKind               transKind = TransNormal;


  if (m_useHHIR) {
    TranslateTraceletResult result;
    do {
      hhirTraceStart(sk.offset());
      SKTRACE(1, sk, "retrying irTranslateTracelet\n");
      result = irTranslateTracelet(t, start, stubStart, &bcMapping);
    } while (result == Retry);
    m_useHHIR = false;
    if (result == Success) {
      m_irAUsage += (a.code.frontier - start);
      m_irAstubsUsage += (astubs.code.frontier - stubStart);
      transKind = TransNormalIR;
    }
  }
  if (transKind == TransNormal) { // Regular old tx64.
    assert(m_pendingFixups.size() == 0);
    assert(srcRec.inProgressTailJumps().size() == 0);
    assert(!m_useHHIR);
    bcMapping.clear();
    transKind = TransNormal;
    try {
      if (t.m_analysisFailed || checkTranslationLimit(t.m_sk, srcRec)) {
        punt();
      }
      // If we failed to IR-translate the tracelet, either reanalyze
      // with more aggressive assumptions, or fall back to the
      // interpreter.
      if (considerHHIR) {
        if (RuntimeOption::EvalHHIRDisableTx64) {
          punt();
        }
        // Recur. We need to re-analyze. Since m_useHHIR is clear, we
        // won't go down this path again.
        return translateTracelet(sk, false);
      }

      emitGuardChecks(a, t.m_sk, t.m_dependencies, t.m_refDeps, srcRec);
      dumpTranslationInfo(t, a.code.frontier);

      // after guards, add a counter for the translation if requested
      if (RuntimeOption::EvalJitTransCounters) {
        emitTransCounterInc(a);
      }

      // emit a counter for the hhir punt that got us here, if any
      if (Trace::moduleEnabled(Trace::punt, 1) && !m_lastHHIRPunt.empty()) {
        emitRecordPunt(a, m_lastHHIRPunt);
      }

      emitRB(a, RBTypeTraceletBody, t.m_sk);
      Stats::emitInc(a, Stats::Instr_TC, t.m_numOpcodes);
      recordBCInstr(OpTraceletGuard, a, start);

      // Translate each instruction in the tracelet
      for (auto ni = t.m_instrStream.first; ni; ni = ni->next) {
        if (isTransDBEnabled()) {
          bcMapping.push_back((TransBCMapping){ni->offset(),
                                               a.code.frontier,
                                               astubs.code.frontier});
        }

        m_curNI = ni;
        Nuller<NormalizedInstruction> niNuller(&m_curNI);
        translateInstr(t, *ni);
        assert(ni->source.offset() >= curFunc()->base());
        // We sometimes leave the tail of a truncated tracelet in place to aid
        // analysis, but breaksTracelet is authoritative.
        if (ni->breaksTracelet) break;
      }
    } catch (TranslationFailedExc& tfe) {
      // The whole translation failed; give up on this BB. Since it is not
      // linked into srcDB yet, it is guaranteed not to be reachable.
      m_regMap.reset();
      // Permanent reset; nothing is reachable yet.
      a.code.frontier = start;
      astubs.code.frontier = stubStart;
      bcMapping.clear();
      // Discard any pending fixups.
      m_pendingFixups.clear();
      srcRec.clearInProgressTailJumps();
      TRACE(1,
            "emitting %d-instr interp request for failed translation @%s:%d\n",
            int(t.m_numOpcodes), tfe.m_file, tfe.m_line);
      // Add a counter for the translation if requested
      if (RuntimeOption::EvalJitTransCounters) {
        emitTransCounterInc(a);
      }
      a.    jmp(emitServiceReq(REQ_INTERPRET, 2ull, uint64_t(t.m_sk.offset()),
                               uint64_t(t.m_numOpcodes)));
      // Fall through.
    }
  }

  m_regMap.reset();

  if (dryRun) {
    m_pendingFixups.clear();
    bcMapping.clear();
    srcRec.clearInProgressTailJumps();
    return;
  }

  for (uint i = 0; i < m_pendingFixups.size(); i++) {
    TCA tca = m_pendingFixups[i].m_tca;
    assert(isValidCodeAddress(tca));
    m_fixupMap.recordFixup(tca, m_pendingFixups[i].m_fixup);
  }
  m_pendingFixups.clear();

  addTranslation(TransRec(t.m_sk, curUnit()->md5(), transKind, t, start,
                          a.code.frontier - start, stubStart,
                          astubs.code.frontier - stubStart,
                          counterStart, counterLen,
                          bcMapping));

  recordGdbTranslation(sk, curFunc(), a, start,
                       false, false);
  recordGdbTranslation(sk, curFunc(), astubs, stubStart,
                       false, false);
  // SrcRec::newTranslation() makes this code reachable. Do this last;
  // otherwise there's some chance of hitting in the reader threads whose
  // metadata is not yet visible.
  TRACE(1, "newTranslation: %p  sk: (func %d, bcOff %d)\n",
      start, sk.getFuncId(), sk.m_offset);
  srcRec.newTranslation(a, astubs, start);
  TRACE(1, "tx64: %zd-byte tracelet\n", a.code.frontier - start);
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease(getUsage().c_str());
  }


  if (transKind == TransNormalIR && RuntimeOption::EvalJitCompareHHIR) {
    m_useHHIR = false;
    Disasm disasm(Disasm::Options().relativeOffset(true));
    TCA irEnd = a.code.frontier;
    TCA irStubsEnd = astubs.code.frontier;
    TCA tx64Start = a.code.frontier;
    translateTracelet(sk, false, true);
    TCA tx64End = a.code.frontier;
    size_t irSize = irEnd - start;
    size_t tx64Size = tx64End - tx64Start;

    double ratio = (double)irSize / tx64Size;
    if (ratio > RuntimeOption::EvalJitCompareHHIR) {
      std::ostringstream irOut, tx64Out, out;
      out << folly::format("{:-^140}\n",
                           folly::format(" New translation - hhir/tx64 = {}% ",
                                         int(100 * ratio)));
      t.print(out);
      out << '\n';
#     define IRCOL "{:<90}"
#     define TXCOL "{:<50}"
      out << folly::format(TXCOL "  " TXCOL "\n",
                           folly::format("Translation from tx64 ({} bytes)",
                                         tx64Size),
                           folly::format("Translation from hhir ({} bytes)",
                                         irSize));

      disasm.disasm(irOut, start, irEnd);
      disasm.disasm(tx64Out, tx64Start, tx64End);
      std::string irAsm = irOut.str(), tx64Str = tx64Out.str();
      std::istringstream irAsmIn(irAsm), irPrettyIn(m_lastHHIRDump),
                         tx64In(tx64Str);
      std::string irAsmLine, irPrettyLine, tx64Line;

      // || without short-circuiting
      auto or = [](bool a, bool b) { return a || b; };
      while (or(std::getline(irAsmIn, irAsmLine),
                or(std::getline(irPrettyIn, irPrettyLine),
                   std::getline(tx64In, tx64Line)))) {
        out << folly::format("  " TXCOL TXCOL IRCOL "\n",
                             tx64Line, irAsmLine, irPrettyLine);
        irAsmLine.clear();
        irPrettyLine.clear();
        tx64Line.clear();
      }
#     undef IRCOL
#     undef TXCOL
      out << '\n';

      Trace::traceRelease("%s", out.str().c_str());
    }

    a.code.frontier = irEnd;
    astubs.code.frontier = irStubsEnd;
  }
}

/*
 * Defines functions called by emitGenericReturn, and
 * cgGenericRetDecRefs.
 */
void TranslatorX64::emitFreeLocalsHelpers() {
  Label doRelease;
  Label release;
  Label loopHead;

  /*
   * Note: the IR currently requires that we preserve r14/r15 across
   * calls to these free locals helpers.
   */
  static_assert(rVmSp == rbx, "");
  auto const rIter     = rbx;
  auto const rFinished = r13;
  auto const rType     = esi;
  auto const rData     = rdi;

  moveToAlign(a, kNonFallthroughAlign);

  TRACE(1, "HOTSTUB: freeLocalsHelpers starts %lx\n", uintptr_t(a.code.frontier));

asm_label(a, release);
  a.    loadq  (rIter[TVOFF(m_data)], rData);
  a.    cmpl   (RefCountStaticValue, rData[FAST_REFCOUNT_OFFSET]);
  jccBlock<CC_Z>(a, [&] {
    a.  decl   (rData[FAST_REFCOUNT_OFFSET]);
    a.  jz8    (doRelease);
  });
  a.    ret    ();
asm_label(a, doRelease);
  emitStoreTVType(a, KindOfUninit, rIter[TVOFF(m_type)]);
  jumpDestructor(a, PhysReg(rType), rax);

  m_freeManyLocalsHelper = a.code.frontier;
  a.    lea    (rVmFp[-cellsToBytes(kNumFreeLocalsHelpers)], rFinished);

  auto emitDecLocal = [&] {
    Label skipDecRef;

    emitLoadTVType(a, rIter[TVOFF(m_type)], rType);
    emitCmpTVType(a, KindOfRefCountThreshold, rType);
    a.  jle8   (skipDecRef);
    a.  call   (release);
    recordIndirectFixup(a.code.frontier, 0);
  asm_label(a, skipDecRef);
  };

  // Loop for the first few locals, but unroll the final
  // kNumFreeLocalsHelpers.
asm_label(a, loopHead);
  emitDecLocal();
  a.    addq   (sizeof(TypedValue), rIter);
  a.    cmpq   (rIter, rFinished);
  a.    jnz8   (loopHead);

  for (int i = 0; i < kNumFreeLocalsHelpers; ++i) {
    m_freeLocalsHelpers[kNumFreeLocalsHelpers - i - 1] = a.code.frontier;
    TRACE(1, "HOTSTUB: m_freeLocalsHelpers[%d] = %p\n",
          kNumFreeLocalsHelpers - i - 1, a.code.frontier);
    emitDecLocal();
    if (i != kNumFreeLocalsHelpers - 1) {
      a.addq (sizeof(TypedValue), rIter);
    }
  }

  a.    addq   (AROFF(m_r) + sizeof(TypedValue), rVmSp);
  a.    ret    (8);

  TRACE(1, "STUB freeLocals helpers: %zu bytes\n",
           size_t(a.code.frontier - m_freeManyLocalsHelper));
}

TranslatorX64::TranslatorX64()
: m_numNativeTrampolines(0),
  m_trampolineSize(0),
  m_spillFillCode(&a),
  m_interceptHelper(0),
  m_defClsHelper(0),
  m_funcPrologueRedispatch(0),
  m_irAUsage(0),
  m_irAstubsUsage(0),
  m_numHHIRTrans(0),
  m_regMap(kCallerSaved, kCalleeSaved, this),
  m_interceptsEnabled(false),
  m_unwindRegMap(128),
  m_curTrace(0),
  m_curNI(0),
  m_curFile(nullptr),
  m_curLine(0),
  m_curFunc(nullptr),
  m_vecState(nullptr)
{
  const size_t kAHotSize = RuntimeOption::VMTranslAHotSize;
  const size_t kASize = RuntimeOption::VMTranslASize;
  const size_t kAStubsSize = RuntimeOption::VMTranslAStubsSize;
  const size_t kGDataSize = RuntimeOption::VMTranslGDataSize;
  m_totalSize = kAHotSize + kASize + kAStubsSize +
    kTrampolinesBlockSize + kGDataSize;

  TRACE(1, "TranslatorX64@%p startup\n", this);
  tx64 = this;

  if ((kAHotSize < (2 << 20)) ||
      (kASize < (10 << 20)) ||
      (kAStubsSize < (10 << 20)) ||
      (kGDataSize < (2 << 20))) {
    fprintf(stderr, "Allocation sizes ASize, AStubsSize, and GlobalDataSize "
                    "are too small.\n");
    exit(1);
  }

  if (m_totalSize > (2ul << 30)) {
    fprintf(stderr,"Combined size of ASize, AStubSize, and GlobalDataSize "
                   "must be < 2GiB to support 32-bit relative addresses\n");
    exit(1);
  }

  static bool profileUp = false;
  if (!profileUp) {
    profileInit();
    profileUp = true;
  }

  // We want to ensure that the block for "a", "astubs",
  // "atrampolines", and "m_globalData" are nearby so that we can
  // short jump/point between them. Thus we allocate one slab and
  // divide it between "a", "astubs", and "atrampolines".

  // Using sbrk to ensure its in the bottom 2G, so we avoid
  // the need for trampolines, and get to use shorter
  // instructions for tc addresses.
  static const size_t kRoundUp = 2 << 20;
  const size_t allocationSize = m_totalSize + kRoundUp - 1;
  uint8_t *base = (uint8_t*)sbrk(allocationSize);
  if (base == (uint8_t*)-1) {
    base = (uint8_t*)low_malloc(allocationSize);
    if (!base) {
      base = (uint8_t*)malloc(allocationSize);
    }
    if (!base) {
      fprintf(stderr, "could not allocate %zd bytes for translation cache\n",
              allocationSize);
      exit(1);
    }
  }
  assert(base);
  base += -(uint64_t)base & (kRoundUp - 1);
  if (RuntimeOption::EvalMapTCHuge) {
    hintHuge(base, m_totalSize);
  }
  TRACE(1, "init atrampolines @%p\n", base);
  atrampolines.init(base, kTrampolinesBlockSize);
  base += kTrampolinesBlockSize;

  m_unwindRegistrar = register_unwind_region(base, m_totalSize);
  TRACE(1, "init ahot @%p\n", base);
  ahot.init(base, kAHotSize);
  base += kAHotSize;
  TRACE(1, "init a @%p\n", base);
  a.init(base, kASize);
  base += kASize;
  TRACE(1, "init astubs @%p\n", base);
  astubs.init(base, kAStubsSize);
  base += kAStubsSize;
  TRACE(1, "init gdata @%p\n", base);
  m_globalData.init(base, kGDataSize);

  // put the stubs into ahot, rather than a
  AHotSelector ahs(this, true);

  // Emit some special helpers that are shared across translations.

  // Emit a byte of padding. This is a kind of hacky way to
  // avoid hitting an assert in recordGdbStub when we call
  // it with m_callToExit - 1 as the start address.
  astubs.emitNop(1);

  // Call to exit with whatever value the program leaves on
  // the return stack.
  m_callToExit = emitServiceReq(SRFlags(SRAlign | SRJmpInsteadOfRet),
                                REQ_EXIT, 0ull);
  m_retHelper = emitRetFromInterpretedFrame();
  m_genRetHelper = emitRetFromInterpretedGeneratorFrame();

  moveToAlign(astubs);
  m_resumeHelperRet = astubs.code.frontier;
  emitPopRetIntoActRec(astubs);
  m_resumeHelper = astubs.code.frontier;
  emitGetGContext(astubs, rax);
  astubs.   load_reg64_disp_reg64(rax, offsetof(VMExecutionContext, m_fp),
                                       rVmFp);
  astubs.   load_reg64_disp_reg64(rax, offsetof(VMExecutionContext, m_stack) +
                                       Stack::topOfStackOffset(), rVmSp);
  emitServiceReq(SRFlags::SRInline, REQ_RESUME, 0ull);

  // Helper for DefCls, in astubs.
  {
    auto& a = astubs;
    if (false) {
      PreClass *preClass = 0;
      defClsHelper(preClass);
    }
    m_defClsHelper = TCA(a.code.frontier);
    PhysReg rEC = argNumToRegName[2];
    emitGetGContext(a, rEC);
    a.   storeq (rVmFp, rEC[offsetof(VMExecutionContext, m_fp)]);
    a.   storeq (argNumToRegName[1],
                    rEC[offsetof(VMExecutionContext, m_pc)]);
    a.   storeq (rax, rEC[offsetof(VMExecutionContext, m_stack) +
                      Stack::topOfStackOffset()]);
    a.   jmp    (TCA(defClsHelper));
  }

  // The decRef helper for when we bring the count down to zero. Callee needs to
  // bring the value into rdi. These can be burned in for all time, and for all
  // translations.
  typedef void* vp;

  TCA strDtor, arrDtor, objDtor, refDtor;
  strDtor = emitUnaryStub(astubs, Call(getMethodPtr(&StringData::release)));
  arrDtor = emitUnaryStub(astubs, Call(getVTableOffset(&HphpArray::release)));
  objDtor = emitUnaryStub(astubs, Call(getMethodPtr(&ObjectData::release)));
  refDtor = emitUnaryStub(astubs, Call(vp(getMethodPtr(&RefData::release))));

  m_dtorStubs[typeToDestrIndex(BitwiseKindOfString)] = strDtor;
  m_dtorStubs[typeToDestrIndex(KindOfArray)]         = arrDtor;
  m_dtorStubs[typeToDestrIndex(KindOfObject)]        = objDtor;
  m_dtorStubs[typeToDestrIndex(KindOfRef)]           = refDtor;

  // Hot helper stubs in A:
  emitGenericDecRefHelpers();
  emitFreeLocalsHelpers();
  m_funcPrologueRedispatch = emitPrologueRedispatch(a);
  TRACE(1, "HOTSTUB: all stubs finished: %lx\n",
        uintptr_t(a.code.frontier));

  if (trustSigSegv) {
    // Install SIGSEGV handler for timeout exceptions
    struct sigaction sa;
    struct sigaction old_sa;
    sa.sa_sigaction = &TranslatorX64::SEGVHandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, &old_sa) != 0) {
      throw std::runtime_error(
        std::string("Failed to install SIGSEGV handler: ") +
          strerror(errno));
    }
    m_segvChain = old_sa.sa_flags & SA_SIGINFO ?
      old_sa.sa_sigaction : (sigaction_t)old_sa.sa_handler;
  }

  moveToAlign(astubs);
  m_stackOverflowHelper = astubs.code.frontier;
  // We are called from emitStackCheck, with the new stack frame in
  // rStashedAR. Get the caller's PC into rdi and save it off.
  astubs.    load_reg64_disp_reg64(rVmFp, AROFF(m_func), rax);
  astubs.    load_reg64_disp_reg32(rStashedAR, AROFF(m_soff), rdi);
  astubs.    load_reg64_disp_reg64(rax, Func::sharedOffset(), rax);
  astubs.    load_reg64_disp_reg32(rax, Func::sharedBaseOffset(), rax);
  astubs.    add_reg32_reg32(rax, rdi);
  emitEagerVMRegSave(astubs, SaveFP | SavePC);
  emitServiceReq(SRFlags::SRInline, REQ_STACK_OVERFLOW, 0ull);
}

// do gdb specific initialization. This has to happen after
// the TranslatorX64 constructor is called, because gdb initialization
// calls backs into TranslatorX64::Get()
void TranslatorX64::initGdb() {
  // On a backtrace, gdb tries to locate the calling frame at address
  // returnRIP-1. However, for the first VM frame, there is no code at
  // returnRIP-1, since the AR was set up manually. For this frame,
  // record the tracelet address as starting from callToExit-1, so gdb
  // does not barf
  recordGdbStub(astubs, m_callToExit - 1, "HHVM::callToExit");

  recordBCInstr(OpRetFromInterp, astubs, m_retHelper);
  recordGdbStub(astubs, m_retHelper - 1, "HHVM::retHelper");
  recordBCInstr(OpResumeHelper, astubs, m_resumeHelper);
  recordBCInstr(OpDefClsHelper, astubs, m_defClsHelper);
  recordBCInstr(OpDtorStub, astubs,
                m_dtorStubs[typeToDestrIndex(BitwiseKindOfString)]);
  recordGdbStub(astubs, m_dtorStubs[typeToDestrIndex(BitwiseKindOfString)],
                        "HHVM::destructorStub");
}

TranslatorX64*
TranslatorX64::Get() {
  /*
   * Called from outrageously early, pre-main code, and will
   * allocate the first translator space.
   */
  if (!nextTx64) {
    nextTx64 = new TranslatorX64();
    nextTx64->initGdb();
  }
  if (!tx64) {
    tx64 = nextTx64;
  }
  assert(tx64);
  return tx64;
}

template<int Arity>
TCA TranslatorX64::emitNAryStub(X64Assembler& a, Call c) {
  BOOST_STATIC_ASSERT((Arity < kNumRegisterArgs));

  // The callNAryStub has already saved these regs on a.
  RegSet alreadySaved;
  for (size_t i = 0; i < Arity; ++i) {
    alreadySaved |= RegSet(argNumToRegName[i]);
  }

  /*
   * We've made a call instruction, and pushed Arity args on the
   * stack.  So the stack address will be odd coming into the stub if
   * Arity + 1 (for the call) is odd.  We need to correct for this
   * when saving other registers below to keep SSE-friendly alignment
   * of the stack.
   */
  const int Parity = (Arity + 1) % 2;

  // These dtor stubs are meant to be called with the call
  // instruction, unlike most translator code.
  moveToAlign(a);
  TCA start = a.code.frontier;
  /*
   * Preserve most caller-saved regs. The calling code has already
   * preserved regs in `alreadySaved'; we push the rest of the caller
   * saved regs and rbp.  It should take 9 qwords in total, and the
   * incoming call instruction made it 10.  This is an even number of
   * pushes, so we preserve the SSE-friendliness of our execution
   * environment (without real intervention from PhysRegSaverParity).
   *
   * Note that we don't need to clean all registers because the only
   * reason we could need those locations written back is if stack
   * unwinding were to happen.  These stubs can re-enter due to user
   * destructors, but exceptions are not allowed to propagate out of
   * those, so it's not a problem.
   */
  a.    push (rbp); // {
  a.    movq (rsp, rbp);
  {
    RegSet s = kCallerSaved - alreadySaved;
    PhysRegSaverParity<Parity> rs(a, s);
    emitCall(a, c);
  }
  a.    pop  (rbp);  // }
  a.    ret  ();
  return start;
}

TCA TranslatorX64::emitUnaryStub(X64Assembler& a, Call c) {
  return emitNAryStub<1>(a, c);
}

/*
 * Both callUnaryStubImpl and callBinaryStub assume that the stub they
 * are calling cannot throw an exception.
 */

template <bool reentrant>
void
TranslatorX64::callUnaryStubImpl(X64Assembler& a,
                                 const NormalizedInstruction& i,
                                 TCA stub, PhysReg arg, int disp/*=0*/) {
  // Call the generic dtor stub. They all take one arg.
  a.    push (rdi);
  if (arg == rsp) {
    // Account for pushing rdi.
    disp += 8;
  }
  emitLea(a, arg, disp, rdi);
  assert(isValidCodeAddress(stub));
  emitCall(a, stub);
  recordCallImpl<reentrant>(a, i);
  a.    pop  (rdi);
}

void
TranslatorX64::callBinaryStub(X64Assembler& a, const NormalizedInstruction& i,
                              TCA stub, PhysReg arg1, PhysReg arg2) {
  a.    push (rdi);
  a.    push (rsi);

  // We need to be careful not to clobber our arguments when moving
  // them into the appropriate registers.  (If we ever need ternary
  // stubs, this should probably be converted to use ArgManager.)
  if (arg2 == rdi && arg1 == rsi) {
    a.  xchgq(rdi, rsi);
  } else if (arg2 == rdi) {
    emitMovRegReg(a, arg2, rsi);
    emitMovRegReg(a, arg1, rdi);
  } else {
    emitMovRegReg(a, arg1, rdi);
    emitMovRegReg(a, arg2, rsi);
  }

  assert(isValidCodeAddress(stub));
  emitCall(a, stub);
  recordReentrantCall(a, i);
  a.    pop  (rsi);
  a.    pop  (rdi);
}

namespace {

struct DeferredFileInvalidate : public DeferredWorkItem {
  Eval::PhpFile* m_f;
  DeferredFileInvalidate(Eval::PhpFile* f) : m_f(f) {
    TRACE(2, "DeferredFileInvalidate @ %p, m_f %p\n", this, m_f); }
  void operator()() {
    TRACE(2, "DeferredFileInvalidate: Firing @ %p , m_f %p\n", this, m_f);
    tx64->invalidateFileWork(m_f);
  }
};

struct DeferredPathInvalidate : public DeferredWorkItem {
  const std::string m_path;
  DeferredPathInvalidate(const std::string& path) : m_path(path) {
    assert(m_path.size() >= 1 && m_path[0] == '/');
  }
  void operator()() {
    String spath(m_path);
    /*
     * inotify saw this path change. Now poke the file repository;
     * it will notice the underlying PhpFile* has changed, and notify
     * us via ::invalidateFile.
     *
     * We don't actually need to *do* anything with the PhpFile* from
     * this lookup; since the path has changed, the file we'll get out is
     * going to be some new file, not the old file that needs invalidation.
     */
    UNUSED Eval::PhpFile* f =
      g_vmContext->lookupPhpFile(spath.get(), "");
    // We don't keep around the extra ref.
    if (f) f->decRefAndDelete();
  }
};

}

void
TranslatorX64::requestInit() {
  TRACE(1, "in requestInit(%" PRId64 ")\n", g_vmContext->m_currentThreadIdx);
  tl_regState = REGSTATE_CLEAN;
  PendQ::drain();
  requestResetHighLevelTranslator();
  Treadmill::startRequest(g_vmContext->m_currentThreadIdx);
  memset(&s_perfCounters, 0, sizeof(s_perfCounters));
  initJmpProfile();
  Stats::init();
}

void
TranslatorX64::requestExit() {
  if (s_writeLease.amOwner()) {
    s_writeLease.drop();
  }
  TRACE_MOD(txlease, 2, "%" PRIx64 " write lease stats: %15" PRId64
            " kept, %15" PRId64 " grabbed\n",
            pthread_self(), s_writeLease.m_hintKept,
            s_writeLease.m_hintGrabbed);
  PendQ::drain();
  Treadmill::finishRequest(g_vmContext->m_currentThreadIdx);
  TRACE(1, "done requestExit(%" PRId64 ")\n", g_vmContext->m_currentThreadIdx);
  Stats::dump();
  Stats::clear();
  dumpJmpProfile();

  if (Trace::moduleEnabledRelease(Trace::tx64stats, 1)) {
    Trace::traceRelease("TranslatorX64 perf counters for %s:\n",
                        g_context->getRequestUrl(50).c_str());
    for (int i = 0; i < tpc_num_counters; i++) {
      Trace::traceRelease("%-20s %10lld\n",
                          kPerfCounterNames[i], s_perfCounters[i]);
    }
    Trace::traceRelease("\n");
  }
}

bool
TranslatorX64::isPseudoEvent(const char* event) {
  for (auto name : kPerfCounterNames) {
    if (!strcmp(event, name)) {
      return true;
    }
  }
  return false;
}

void
TranslatorX64::getPerfCounters(Array& ret) {
  for (int i = 0; i < tpc_num_counters; i++) {
    // Until Perflab can automatically scale the values we give it to
    // an appropriate range, we have to fudge these numbers so they
    // look more like reasonable hardware counter values.
    ret.set(kPerfCounterNames[i], s_perfCounters[i] * 1000);
  }

  if (RuntimeOption::EnableInstructionCounts) {
    auto doCounts = [&](unsigned begin, const char* const name) {
      int64_t count = 0;
      for (; begin < Stats::Instr_InterpOneHighInvalid;
           begin += STATS_PER_OPCODE) {
        count += Stats::tl_counters[Stats::StatCounter(begin)];
      }
      ret.set(name, count);
    };

    doCounts(Stats::Instr_TranslLowInvalid + STATS_PER_OPCODE,
             kInstrCountTx64Name);
    doCounts(Stats::Instr_TranslIRPostLowInvalid + STATS_PER_OPCODE,
             kInstrCountIRName);
  }
}

TranslatorX64::~TranslatorX64() {
  freeSlab(atrampolines.code.base, m_totalSize);
}

static Debug::TCRange rangeFrom(const X64Assembler& a, const TCA addr,
                                bool isAstubs) {
  assert(a.code.isValidAddress(addr));
  return Debug::TCRange(addr, a.code.frontier, isAstubs);
}

void TranslatorX64::recordBCInstr(uint32_t op,
                                  const X64Assembler& a,
                                  const TCA addr) {
  if (addr != a.code.frontier) {
    m_debugInfo.recordBCInstr(Debug::TCRange(addr, a.code.frontier,
                                             &a == &astubs ? true : false), op);
  }
}

void TranslatorX64::recordGdbTranslation(SrcKey sk,
                                         const Func* srcFunc,
                                         const X64Assembler& a,
                                         const TCA start,
                                         bool exit,
                                         bool inPrologue) {
  if (start != a.code.frontier) {
    assert(s_writeLease.amOwner());
    if (!RuntimeOption::EvalJitNoGdb) {
      m_debugInfo.recordTracelet(rangeFrom(a, start,
                                           &a == &astubs ? true : false),
                                 srcFunc,
                                 srcFunc->unit() ?
                                   srcFunc->unit()->at(sk.offset()) : nullptr,
                                 exit, inPrologue);
    }
    if (RuntimeOption::EvalPerfPidMap) {
      m_debugInfo.recordPerfMap(rangeFrom(a, start,
                                          &a == &astubs ? true : false),
                                srcFunc, exit, inPrologue);
    }
  }
}

void TranslatorX64::recordGdbStub(const X64Assembler& a,
                                  const TCA start, const char* name) {
  if (!RuntimeOption::EvalJitNoGdb) {
    m_debugInfo.recordStub(rangeFrom(a, start, &a == &astubs ? true : false),
                           name);
  }
}

void TranslatorX64::defineCns(StringData* name) {
  TargetCache::fillConstant(name);
}

size_t TranslatorX64::getCodeSize() {
  return a.code.frontier - a.code.base;
}

size_t TranslatorX64::getStubSize() {
  return astubs.code.frontier - astubs.code.base;
}

size_t TranslatorX64::getTargetCacheSize() {
  return TargetCache::s_frontier;
}

std::string TranslatorX64::getUsage() {
  std::string usage;
  size_t aHotUsage = ahot.code.frontier - ahot.code.base;
  size_t aUsage = a.code.frontier - a.code.base;
  size_t stubsUsage = astubs.code.frontier - astubs.code.base;
  size_t dataUsage = m_globalData.frontier - m_globalData.base;
  size_t tcUsage = TargetCache::s_frontier;
  size_t persistentUsage =
    TargetCache::s_persistent_frontier - TargetCache::s_persistent_start;
  Util::string_printf(
    usage,
    "tx64: %9zd bytes (%" PRId64 "%%) in ahot.code\n"
    "tx64: %9zd bytes (%" PRId64 "%%) in a.code\n"
    "tx64: %9zd bytes (%" PRId64 "%%) in astubs.code\n"
    "tx64: %9zd bytes (%" PRId64 "%%) in a.code from ir\n"
    "tx64: %9zd bytes (%" PRId64 "%%) in astubs.code from ir\n"
    "tx64: %9zd bytes (%" PRId64 "%%) in m_globalData\n"
    "tx64: %9zd bytes (%" PRId64 "%%) in targetCache\n"
    "tx64: %9zd bytes (%" PRId64 "%%) in persistentCache\n",
    aHotUsage,  100 * aHotUsage / ahot.code.size,
    aUsage,     100 * aUsage / a.code.size,
    stubsUsage, 100 * stubsUsage / astubs.code.size,
    m_irAUsage,     100 * m_irAUsage / a.code.size,
    m_irAstubsUsage, 100 * m_irAstubsUsage / astubs.code.size,
    dataUsage, 100 * dataUsage / m_globalData.size,
    tcUsage,
    400 * tcUsage / RuntimeOption::EvalJitTargetCacheSize / 3,
    persistentUsage,
    400 * persistentUsage / RuntimeOption::EvalJitTargetCacheSize);
  return usage;
}

bool TranslatorX64::addDbgGuards(const Unit* unit) {
  // TODO refactor
  // It grabs the write lease and iterating through whole SrcDB...
  bool locked = s_writeLease.acquire(true);
  if (!locked) {
    return false;
  }
  struct timespec tsBegin, tsEnd;
  gettime(CLOCK_MONOTONIC, &tsBegin);
  // Doc says even find _could_ invalidate iterator, in pactice it should
  // be very rare, so go with it now.
  for (SrcDB::iterator it = m_srcDB.begin(); it != m_srcDB.end(); ++it) {
    SrcKey const sk = SrcKey::fromAtomicInt(it->first);
    SrcRec& sr = *it->second;
    if (sr.unitMd5() == unit->md5() &&
        !sr.hasDebuggerGuard() &&
        isSrcKeyInBL(unit, sk)) {
      addDbgGuardImpl(sk, sr);
    }
  }
  s_writeLease.drop();
  gettime(CLOCK_MONOTONIC, &tsEnd);
  int64_t elapsed = gettime_diff_us(tsBegin, tsEnd);
  if (Trace::moduleEnabledRelease(Trace::tx64, 5)) {
    Trace::traceRelease("addDbgGuards got lease for %" PRId64 " us\n", elapsed);
  }
  return true;
}

bool TranslatorX64::addDbgGuard(const Func* func, Offset offset) {
  SrcKey sk(func, offset);
  {
    if (SrcRec* sr = m_srcDB.find(sk)) {
      if (sr->hasDebuggerGuard()) {
        return true;
      }
    } else {
      // no translation yet
      return true;
    }
  }
  if (debug) {
    if (!isSrcKeyInBL(func->unit(), sk)) {
      TRACE(5, "calling addDbgGuard on PC that is not in blacklist");
      return false;
    }
  }
  bool locked = s_writeLease.acquire(true);
  if (!locked) {
    return false;
  }
  {
    if (SrcRec* sr = m_srcDB.find(sk)) {
      addDbgGuardImpl(sk, *sr);
    }
  }
  s_writeLease.drop();
  return true;
}

void TranslatorX64::addDbgGuardImpl(SrcKey sk, SrcRec& srcRec) {
  TCA dbgGuard = a.code.frontier;
  // Emit the checks for debugger attach
  emitTLSLoad<ThreadInfo>(a, ThreadInfo::s_threadInfo, rScratch);
  static COff dbgOff = offsetof(ThreadInfo, m_reqInjectionData) +
                       offsetof(RequestInjectionData, debugger);
  a.   load_reg64_disp_reg32(rScratch, dbgOff, rScratch);
  a.   testb((int8_t)0xff, rbyte(rScratch));
  // Branch to a special REQ_INTERPRET if attached
  {
    TCA fallback = emitServiceReq(REQ_INTERPRET, 2, uint64_t(sk.offset()), 0);
    a. jnz(fallback);
  }
  // Emit a jump to the actual code
  TCA realCode = srcRec.getTopTranslation();
  prepareForSmash(kJmpLen);
  TCA dbgBranchGuardSrc = a.code.frontier;
  a.   jmp(realCode);
  // Add it to srcRec
  srcRec.addDebuggerGuard(a, astubs, dbgGuard, dbgBranchGuardSrc);
}

bool TranslatorX64::dumpTCCode(const char* filename) {
  string aFilename = string(filename).append("_a");
  string astubFilename = string(filename).append("_astub");
  FILE* aFile = fopen(aFilename.c_str(),"wb");
  if (aFile == nullptr)
    return false;
  FILE* astubFile = fopen(astubFilename.c_str(),"wb");
  if (astubFile == nullptr) {
    fclose(aFile);
    return false;
  }
  string helperAddrFilename = string(filename).append("_helpers_addrs.txt");
  FILE* helperAddrFile = fopen(helperAddrFilename.c_str(),"wb");
  if (helperAddrFile == nullptr) {
    fclose(aFile);
    fclose(astubFile);
    return false;
  }
  // dump starting from the trampolines; this assumes processInit() places
  // trampolines before the translation cache
  size_t count = a.code.frontier-atrampolines.code.base;
  bool result = (fwrite(atrampolines.code.base, 1, count, aFile) == count);
  if (result) {
    count = astubs.code.frontier - astubs.code.base;
    result = (fwrite(astubs.code.base, 1, count, astubFile) == count);
  }
  if (result) {
    for(PointerMap::iterator iter = trampolineMap.begin();
        iter != trampolineMap.end();
        iter++) {
      void* helperAddr = iter->first;
      void* trampAddr = iter->second;
      char* functionName = Util::getNativeFunctionName(helperAddr);
      fprintf(helperAddrFile,"%10p %10p %s\n",
              trampAddr, helperAddr,
              functionName);
      free(functionName);
    }
  }
  fclose(aFile);
  fclose(astubFile);
  fclose(helperAddrFile);
  return result;
}

// Returns true on success
bool TranslatorX64::dumpTC(bool ignoreLease) {
  if (!ignoreLease && !s_writeLease.acquire(true)) return false;
  bool success = dumpTCData();
  if (success) {
    success = dumpTCCode("/tmp/tc_dump");
  }
  if (!ignoreLease) s_writeLease.drop();
  return success;
}

// Returns true on success
bool tc_dump(void) {
  return TranslatorX64::Get()->dumpTC();
}

// Returns true on success
bool TranslatorX64::dumpTCData() {
  gzFile tcDataFile = gzopen("/tmp/tc_data.txt.gz", "w");
  if (!tcDataFile) return false;

  if (!gzprintf(tcDataFile,
                "repo_schema     = %s\n"
                "a.base          = %p\n"
                "a.frontier      = %p\n"
                "astubs.base     = %p\n"
                "astubs.frontier = %p\n\n",
                kRepoSchemaId,
                atrampolines.code.base, a.code.frontier,
                astubs.code.base, astubs.code.frontier)) {
    return false;
  }

  if (!gzprintf(tcDataFile, "total_translations = %zu\n\n",
                m_translations.size())) {
    return false;
  }

  for (size_t t = 0; t < m_translations.size(); t++) {
    if (gzputs(tcDataFile,
               m_translations[t].print(getTransCounter(t)).c_str()) == -1) {
      return false;
    }
  }

  gzclose(tcDataFile);
  return true;
}

#define NATIVE_OP(X) PLAN(X, Native)
#define SUPPORTED_OP(X) PLAN(X, Supported)
#define SIMPLE_OP(X) PLAN(X, Simple)
#define INTERP_OP(X) PLAN(X, Interp)

#define SUPPORTED_OPS() \
  /*
   * Translations with no callouts to C++ whatsoever.
   */ \
  NATIVE_OP(Null) \
  NATIVE_OP(NullUninit) \
  NATIVE_OP(True) \
  NATIVE_OP(False) \
  NATIVE_OP(Int) \
  NATIVE_OP(Double) \
  NATIVE_OP(String) \
  NATIVE_OP(Array) \
  NATIVE_OP(NewArray) \
  NATIVE_OP(InitThisLoc) \
  NATIVE_OP(Dup) \
  NATIVE_OP(ContEnter) \
  NATIVE_OP(ContValid) \
  NATIVE_OP(ContStopped) \
  NATIVE_OP(IncStat) \
  /*
   * Translations with non-reentrant helpers.
   */ \
  SIMPLE_OP(Jmp) \
  SIMPLE_OP(UnpackCont) \
  SIMPLE_OP(CreateCont) \
  SIMPLE_OP(NewCol) \
  SIMPLE_OP(FCall) \
  /*
   * Translations with a reentrant helper.
   *
   * TODO: neither UnboxR nor FPassR can actually call destructors.
   */ \
  SUPPORTED_OP(ContExit) \
  SUPPORTED_OP(UnboxR) \
  SUPPORTED_OP(FPassR) \
  SUPPORTED_OP(NativeImpl) \
  SUPPORTED_OP(UnsetL) \
  SUPPORTED_OP(Cns) \
  SUPPORTED_OP(ClsCnsD) \
  SUPPORTED_OP(This) \
  SUPPORTED_OP(BareThis) \
  SUPPORTED_OP(CheckThis) \
  SUPPORTED_OP(PackCont) \
  SUPPORTED_OP(ContReceive) \
  SUPPORTED_OP(ContRetC) \
  SUPPORTED_OP(ContNext) \
  SUPPORTED_OP(ContSend) \
  SUPPORTED_OP(ContRaise) \
  SUPPORTED_OP(ContCurrent) \
  SUPPORTED_OP(FPushCtor) \
  SUPPORTED_OP(FPushCtorD) \
  SUPPORTED_OP(CreateCl) \
  SUPPORTED_OP(StaticLocInit) \
  /*
   * Always-interp instructions,
   */ \
  INTERP_OP(ContHandle) \


// Define the trivial analyze methods
#define PLAN(Op, Spt) \
void \
TranslatorX64::analyze ## Op(Tracelet& t, NormalizedInstruction& i) { \
  i.m_txFlags = Spt; \
}

SUPPORTED_OPS()

#undef NATIVE_OP
#undef SUPPORTED_OP
#undef SIMPLE_OP
#undef INTERP_OP
#undef SUPPORTED_OPS

void TranslatorX64::invalidateSrcKey(SrcKey sk) {
  assert(!RuntimeOption::RepoAuthoritative);
  assert(s_writeLease.amOwner());
  /*
   * Reroute existing translations for SrcKey to an as-yet indeterminate
   * new one.
   */
  SrcRec* sr = m_srcDB.find(sk);
  assert(sr);
  /*
   * Since previous translations aren't reachable from here, we know we
   * just created some garbage in the TC. We currently have no mechanism
   * to reclaim this.
   */
  sr->replaceOldTranslations(a, astubs);
}

void TranslatorX64::invalidateFileWork(Eval::PhpFile* f) {
  class FileInvalidationTrigger : public Treadmill::WorkItem {
    Eval::PhpFile* m_f;
    int m_nRefs;
  public:
    FileInvalidationTrigger(Eval::PhpFile* f, int n) : m_f(f), m_nRefs(n) { }
    virtual void operator()() {
      if (m_f->decRef(m_nRefs) == 0) {
        Eval::FileRepository::onDelete(m_f);
      }
    }
  };
  size_t nSmashed = m_srcDB.invalidateCode(f);
  if (nSmashed) {
    // The srcDB found an entry for this file. The entry's dependency
    // on this file was counted as a reference, and the code is no longer
    // reachable. We need to wait until the last outstanding request
    // drains to know that we can really remove the reference.
    Treadmill::WorkItem::enqueue(new FileInvalidationTrigger(f, nSmashed));
  }
}

bool TranslatorX64::invalidateFile(Eval::PhpFile* f) {
  // This is called from high rank, but we'll need the write lease to
  // invalidate code.
  if (!RuntimeOption::EvalJit) return false;
  assert(f != nullptr);
  PendQ::defer(new DeferredFileInvalidate(f));
  return true;
}

void TranslatorX64::invalidateOutStack(const NormalizedInstruction& ni) {
  if (ni.outStack) {
    m_regMap.invalidate(ni.outStack->location);
  }
}

void TranslatorX64::cleanOutLocal(const NormalizedInstruction& ni) {
  if (ni.outLocal) {
    m_regMap.cleanLoc(ni.outLocal->location);
  }
}
void TranslatorX64::invalidateOutLocal(const NormalizedInstruction& ni) {
  if (ni.outLocal) {
    m_regMap.invalidate(ni.outLocal->location);
  }
}

} // HPHP::VM::Transl

static const Trace::Module TRACEMOD = Trace::tx64;

void invalidatePath(const std::string& path) {
  TRACE(1, "invalidatePath: abspath %s\n", path.c_str());
  PendQ::defer(new DeferredPathInvalidate(path));
}

void ArgManager::addImm(uint64_t imm) {
  TRACE(6, "ArgManager: push arg %zd imm:%" PRIu64 "\n",
        m_args.size(), imm);
  m_args.push_back(ArgContent(ArgContent::ArgImm, InvalidReg, imm));
}

void ArgManager::addLoc(const Location &loc) {
  TRACE(6, "ArgManager: push arg %zd loc:(%s, %" PRId64 ")\n",
        m_args.size(), loc.spaceName(), loc.offset);
  m_args.push_back(ArgContent(ArgContent::ArgLoc, loc));
}

void ArgManager::addLocRef(const Location &loc) {
  TRACE(6, "ArgManager: push arg ref %zd loc:(%s, %" PRId64 ")\n",
        m_args.size(), loc.spaceName(), loc.offset);
  m_args.push_back(ArgContent(ArgContent::ArgLocRef, loc));
}

void ArgManager::addLocAddr(const Location &loc) {
  TRACE(6, "ArgManager: push arg %zd addr:(%s, %" PRId64 ")\n",
        m_args.size(), loc.spaceName(), loc.offset);
  assert(!loc.isLiteral());
  m_args.push_back(ArgContent(ArgContent::ArgLocAddr, loc));
}

void ArgManager::addDeref(const Location &loc) {
  TRACE(6, "ArgManager: push arg %zd deref:(%s, %" PRId64 ")\n",
        m_args.size(), loc.spaceName(), loc.offset);
  m_args.push_back(ArgContent(ArgContent::ArgDeref, loc));
}

void ArgManager::addReg(PhysReg reg) {
  TRACE(6, "ArgManager: push arg %zd reg:r%d\n",
        m_args.size(), int(reg));
  m_args.push_back(ArgContent(ArgContent::ArgReg, reg, 0));
}

void ArgManager::addRegPlus(PhysReg reg, int32_t off) {
  TRACE(6, "ArgManager: push arg %zd regplus:r%d+%d\n",
        m_args.size(), int(reg), off);
  m_args.push_back(ArgContent(ArgContent::ArgRegPlus, reg, off));
}


} } // HPHP::VM
