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
#ifndef incl_HPHP_TRANSLATOR_H_
#define incl_HPHP_TRANSLATOR_H_

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <memory>
#include <map>
#include <vector>
#include <set>

#include "hphp/util/md5.h"
#include "hphp/util/hash.h"
#include "hphp/util/timer.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/prof-src-key.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/translator-instrs.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/write-lease.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/srckey.h"

/* Translator front-end. */
namespace HPHP {
namespace JIT {
struct HhbcTranslator;
struct IRTranslator;
}
namespace Debug {
struct DebugInfo;
}
namespace JIT {


static const uint32_t transCountersPerChunk = 1024 * 1024 / 8;

struct NormalizedInstruction;

struct TranslationFailedExc : std::runtime_error {
  TranslationFailedExc(const char* file, int line)
    : std::runtime_error(folly::format("TranslationFailedExc @ {}:{}",
                                       file, line).str())
  {}
};

struct UnknownInputExc : std::runtime_error {
  UnknownInputExc(const char* file, int line)
    : std::runtime_error(folly::format("UnknownInputExc @ {}:{}",
                                       file, line).str())
    , m_file(file)
    , m_line(line)
  {}

  const char* m_file; // must be static
  const int m_line;
};

struct ControlFlowFailedExc : std::runtime_error {
  ControlFlowFailedExc(const char* file, int line)
    : std::runtime_error(folly::format("ControlFlowFailedExc @ {}:{}",
                                       file, line).str())
    {}
};

#define punt() do { \
  throw JIT::TranslationFailedExc(__FILE__, __LINE__); \
} while(0)

#define throwUnknownInput() do { \
  throw JIT::UnknownInputExc(__FILE__, __LINE__); \
} while(0);

typedef hphp_hash_map<RegionDesc::BlockId, Block*> BlockIdToIRBlockMap;
typedef hphp_hash_map<RegionDesc::BlockId,
                      RegionDesc::Block*> BlockIdToRegionBlockMap;

/*
 * The information about the context a translation is ocurring
 * in---these fields are fixed for the whole translation.  Many
 * objects in the JIT need access to this.
 */
struct TransContext {
  TransID transID;     // may be kInvalidTransID if not for a real translation
  Offset initBcOffset;
  Offset initSpOffset;
  bool resumed;
  const Func* func;
};

/*
 * Arguments for the translate() entry points in the translator.
 * These include a variety of flags that help decide what to
 * translate, or what to do after we're done, so it's distinct from
 * the TransContext above.
 */
struct TranslArgs {
  TranslArgs(const SrcKey& sk, bool align)
    : m_sk(sk)
    , m_align(align)
    , m_interp(false)
    , m_dryRun(false)
    , m_setFuncBody(false)
    , m_transId(kInvalidTransID)
    , m_region(nullptr)
  {}

  TranslArgs& sk(const SrcKey& sk) {
    m_sk = sk;
    return *this;
  }
  TranslArgs& align(bool align) {
    m_align = align;
    return *this;
  }
  TranslArgs& interp(bool interp) {
    m_interp = interp;
    return *this;
  }
  TranslArgs& dryRun(bool dry) {
    m_dryRun = dry;
    return *this;
  }
  TranslArgs& setFuncBody() {
    m_setFuncBody = true;
    return *this;
  }
  TranslArgs& flags(TransFlags flags) {
    m_flags = flags;
    return *this;
  }
  TranslArgs& transId(TransID transId) {
    m_transId = transId;
    return *this;
  }
  TranslArgs& region(JIT::RegionDescPtr region) {
    m_region = region;
    return *this;
  }

  SrcKey m_sk;
  bool m_align;
  bool m_interp;
  bool m_dryRun;
  bool m_setFuncBody;
  TransFlags m_flags;
  TransID m_transId;
  JIT::RegionDescPtr m_region;
};

/*
 * Translator annotates a tracelet with input/output locations/types.
 */
struct Translator {
  static const int MaxJmpsTracedThrough = 5;

  JIT::UniqueStubs uniqueStubs;

private:
  void createBlockMaps(const RegionDesc&        region,
                       BlockIdToIRBlockMap&     blockIdToIRBlock,
                       BlockIdToRegionBlockMap& blockIdToRegionBlock);

  void setSuccIRBlocks(const RegionDesc&              region,
                       RegionDesc::BlockId            srcBlockId,
                       const BlockIdToIRBlockMap&     blockIdToIRBlock,
                       const BlockIdToRegionBlockMap& blockIdToRegionBlock);

  void setIRBlock(RegionDesc::BlockId            blockId,
                  const BlockIdToIRBlockMap&     blockIdToIRBlock,
                  const BlockIdToRegionBlockMap& blockIdToRegionBlock);

public:
  enum TranslateResult {
    Failure,
    Retry,
    Success
  };
  static const char* translateResultName(TranslateResult r);
  void traceStart(TransContext);
  void traceEnd();
  void traceFree();

public:
  /* translateRegion reads from the RegionBlacklist to determine when
   * to interpret an instruction, and adds failed instructions to the
   * blacklist so they're interpreted on the next attempt. */
  typedef ProfSrcKeySet RegionBlacklist;
  TranslateResult translateRegion(const RegionDesc& region,
                                  bool bcControlFlow,
                                  RegionBlacklist& interp,
                                  TransFlags trflags = TransFlags{});

private:
  typedef std::map<TCA, TransID> TransDB;
  TransDB m_transDB;
  std::vector<TransRec> m_translations;
  std::vector<uint64_t*> m_transCounters;

  int64_t m_createdTime;

  std::unique_ptr<JIT::IRTranslator> m_irTrans;

public:
  JIT::IRTranslator* irTrans() {
    return m_irTrans.get();
  }

private:
  SrcDB m_srcDB;

  static Lease s_writeLease;

public:

  Translator();

  static Lease& WriteLease() {
    return s_writeLease;
  }

  const TransDB& getTransDB() const {
    return m_transDB;
  }

  const TransRec* getTransRec(TCA tca) const {
    if (!isTransDBEnabled()) return nullptr;

    TransDB::const_iterator it = m_transDB.find(tca);
    if (it == m_transDB.end()) {
      return nullptr;
    }
    if (it->second >= m_translations.size()) {
      return nullptr;
    }
    return &m_translations[it->second];
  }

  const TransRec* getTransRec(TransID transId) const {
    if (!isTransDBEnabled()) return nullptr;

    always_assert(transId < m_translations.size());
    return &m_translations[transId];
  }

  TransID getCurrentTransID() const {
    return m_translations.size();
  }

  uint64_t* getTransCounterAddr();
  uint64_t getTransCounter(TransID transId) const;

  void addTranslation(const TransRec& transRec);

  // helpers for srcDB.
  SrcRec* getSrcRec(SrcKey sk) {
    // TODO: add a insert-or-find primitive to THM
    if (SrcRec* r = m_srcDB.find(sk)) return r;
    assert(s_writeLease.amOwner());
    return m_srcDB.insert(sk);
  }

  const SrcDB& getSrcDB() const {
    return m_srcDB;
  }

  static bool liveFrameIsPseudoMain();

  inline bool isTransDBEnabled() const {
    return debug || RuntimeOption::EvalDumpTC;
  }

private:
  PCFilter m_dbgBLPC;
  hphp_hash_set<SrcKey,SrcKey::Hasher> m_dbgBLSrcKey;
  Mutex m_dbgBlacklistLock;

public:
  bool isSrcKeyInBL(const SrcKey& sk);

private:
  TransKind m_mode;
  std::unique_ptr<ProfData> m_profData;

private:
  bool m_useAHot;

public:
  void clearDbgBL();
  bool addDbgBLPC(PC pc);

  ProfData* profData() const {
    return m_profData.get();
  }

  TransKind mode() const {
    return m_mode;
  }
  void setMode(TransKind mode) {
    m_mode = mode;
  }

  bool useAHot() const {
    return m_useAHot;
  }

  void setUseAHot(bool val) {
    m_useAHot = val;
  }

  // Start a new translation space. Returns true IFF this thread created
  // a new space.
  bool replace();
};

int getStackDelta(const NormalizedInstruction& ni);
int64_t getStackPopped(PC pc);
int64_t getStackPushed(PC pc);

enum class ControlFlowInfo {
  None,
  ChangesPC,
  BreaksBB
};

inline ControlFlowInfo
opcodeControlFlowInfo(const Op instr) {
  switch (instr) {
    case Op::Jmp:
    case Op::JmpNS:
    case Op::JmpZ:
    case Op::JmpNZ:
    case Op::Switch:
    case Op::SSwitch:
    case Op::CreateCont:
    case Op::Yield:
    case Op::YieldK:
    case Op::Await:
    case Op::RetC:
    case Op::RetV:
    case Op::Exit:
    case Op::Fatal:
    case Op::IterNext:
    case Op::IterNextK:
    case Op::MIterNext:
    case Op::MIterNextK:
    case Op::WIterNext:
    case Op::WIterNextK:
    case Op::IterInit: // May branch to fail case.
    case Op::IterInitK: // Ditto
    case Op::MIterInit: // Ditto
    case Op::MIterInitK: // Ditto
    case Op::WIterInit: // Ditto
    case Op::WIterInitK: // Ditto
    case Op::DecodeCufIter: // Ditto
    case Op::IterBreak:
    case Op::Throw:
    case Op::Unwind:
    case Op::Eval:
    case Op::NativeImpl:
    case Op::BreakTraceHint:
      return ControlFlowInfo::BreaksBB;
    case Op::FCall:
    case Op::FCallD:
    case Op::FCallArray:
    case Op::FCallUnpack:
    case Op::ContEnter:
    case Op::ContRaise:
    case Op::Incl:
    case Op::InclOnce:
    case Op::Req:
    case Op::ReqOnce:
    case Op::ReqDoc:
      return ControlFlowInfo::ChangesPC;
    default:
      return ControlFlowInfo::None;
  }
}

/*
 * opcodeChangesPC --
 *
 *   Returns true if the instruction can potentially set PC to point
 *   to something other than the next instruction in the bytecode
 */
inline bool
opcodeChangesPC(const Op instr) {
  return opcodeControlFlowInfo(instr) >= ControlFlowInfo::ChangesPC;
}

/*
 * opcodeBreaksBB --
 *
 *   Returns true if the instruction always breaks a tracelet. Most
 *   instructions that change PC will break the tracelet, though some
 *   do not (ex. FCall).
 */
inline bool
opcodeBreaksBB(const Op instr) {
  return opcodeControlFlowInfo(instr) == ControlFlowInfo::BreaksBB;
}

/*
 * instrBreaksProfileBB --
 *
 * Similar to opcodeBreaksBB but more strict. We break profiling blocks after
 * any instruction that can side exit, including instructions with predicted
 * output, and before any control flow merge point.
 */
bool instrBreaksProfileBB(const NormalizedInstruction* instr);

/*
 * If this returns true, we dont generate guards for any of the inputs
 * to this instruction (this is essentially to avoid generating guards
 * on behalf of interpreted instructions).
 */
bool dontGuardAnyInputs(Op op);
bool outputDependsOnInput(const Op instr);

extern bool tc_dump();

/*
 * This routine attempts to find the Func* that will be called for a
 * given target Class and function name, from a given context.  This
 * function determines if a given Func* will be called in a
 * request-insensitive way (i.e. suitable for burning into the TC as a
 * pointer).  The class we are targeting is assumed to be a subclass
 * of `cls', not exactly `cls'.
 *
 * This function should not be used in a context where the call may
 * involve late static binding (i.e. FPushClsMethod), since it assumes
 * static functions will be resolved as targeting on cls regardless of
 * whether they are overridden.
 *
 * Returns nullptr if we can't be sure this would always call this
 * function.
 */
const Func* lookupImmutableMethod(const Class* cls, const StringData* name,
                                  bool& magicCall, bool staticLookup,
                                  Class* ctx);

// This is used to check that return types of builtins are not simple
// types. This is different from IS_REFCOUNTED_TYPE because builtins
// can return Variants, and we use KindOfUnknown to denote these
// return types.
static inline bool isCppByRef(DataType t) {
  return t != KindOfBoolean && t != KindOfInt64 &&
         t != KindOfNull && t != KindOfDouble;
}

// return true if type is passed in/out of C++ as String&/Array&/Object&
static inline bool isSmartPtrRef(DataType t) {
  return t == KindOfString || t == KindOfStaticString ||
         t == KindOfArray || t == KindOfObject ||
         t == KindOfResource;
}

void populateImmediates(NormalizedInstruction&);
bool instrMustInterp(const NormalizedInstruction&);
bool isAlwaysNop(Op op);

struct InputInfo {
  explicit InputInfo(const Location &l)
    : loc(l)
    , dontBreak(false)
    , dontGuard(l.isLiteral())
    , dontGuardInner(false)
  {}

  std::string pretty() const {
    std::string p = loc.pretty();
    if (dontBreak) p += ":dc";
    if (dontGuard) p += ":dg";
    if (dontGuardInner) p += ":dgi";
    return p;
  }
  Location loc;
  /*
   * if an input is unknowable, dont break the tracelet
   * just to find its type. But still generate a guard
   * if that will tell us its type.
   */
  bool     dontBreak;
  /*
   * never break the tracelet, or generate a guard on
   * account of this input.
   */
  bool     dontGuard;
  /*
   * never guard the inner type if this input is KindOfRef
   */
  bool     dontGuardInner;
};

class InputInfos : public std::vector<InputInfo> {
 public:
  InputInfos() : needsRefCheck(false) {}

  std::string pretty() const {
    std::string retval;
    for (size_t i = 0; i < size(); i++) {
      retval += (*this)[i].pretty();
      if (i != size() - 1) {
        retval += std::string(" ");
      }
    }
    return retval;
  }
  bool  needsRefCheck;
};

typedef std::function<Type(int)> LocalTypeFn;
void getInputs(SrcKey startSk, NormalizedInstruction& inst, InputInfos& infos,
               const Func* func, const LocalTypeFn& localType);
void getInputsImpl(SrcKey startSk, NormalizedInstruction* inst,
                   int& currentStackOffset, InputInfos& inputs,
                   const Func* func, const LocalTypeFn& localType);
bool outputIsPredicted(NormalizedInstruction& inst);
bool callDestroysLocals(const NormalizedInstruction& inst,
                        const Func* caller);
int locPhysicalOffset(Location l, const Func* f = nullptr);

struct PropInfo {
  PropInfo()
    : offset(-1)
    , repoAuthType{}
  {}
  explicit PropInfo(int offset, RepoAuthType repoAuthType)
    : offset(offset)
    , repoAuthType{repoAuthType}
  {}

  int offset;
  RepoAuthType repoAuthType;
};
PropInfo getPropertyOffset(const NormalizedInstruction& ni,
                           Class* contextClass,
                           const Class*& baseClass,
                           const MInstrInfo& mii,
                           unsigned mInd, unsigned iInd);
PropInfo getFinalPropertyOffset(const NormalizedInstruction&,
                                Class* contextClass,
                                const MInstrInfo&);

namespace InstrFlags {
enum OutTypeConstraints {
  OutNull,
  OutNullUninit,
  OutString,
  OutStringImm,         // String w/ precisely known immediate.
  OutDouble,
  OutBoolean,
  OutBooleanImm,
  OutInt64,
  OutArray,
  OutArrayImm,
  OutObject,
  OutResource,
  OutThisObject,        // Object from current environment
  OutFDesc,             // Blows away the current function desc

  OutUnknown,           // Not known at tracelet compile-time
  OutPred,              // Unknown, but give prediction a whirl.
  OutPredBool,          // Boolean value predicted to be True or False
  OutCns,               // Constant; may be known at compile-time
  OutVUnknown,          // type is V(unknown)

  OutSameAsInput,       // type is the same as the first stack inpute
  OutCInput,            // type is C(input)
  OutVInput,            // type is V(input)
  OutCInputL,           // type is C(type) of local input
  OutVInputL,           // type is V(type) of local input
  OutFInputL,           // type is V(type) of local input if current param is
                        //   by ref, else type is C(type) of local input
  OutFInputR,           // Like FInputL, but for R's on the stack.

  OutArith,             // For Add, Sub, Mul
  OutArithO,            // For AddO, SubO, MulO
  OutBitOp,             // For BitAnd, BitOr, BitXor
  OutSetOp,             // For SetOpL
  OutIncDec,            // For IncDecL
  OutStrlen,            // OpStrLen
  OutClassRef,          // KindOfClass
  OutFPushCufSafe,      // FPushCufSafe pushes two values of different
                        // types and an ActRec

  OutIsTypeL,           // output for IsTypeL instructions

  OutNone,
};

/*
 * Input codes indicate what an instruction reads, and some other
 * things about their behavior.  The order these show up in the inputs
 * vector is given in getInputs(), and is relevant in a few cases
 * (e.g. instructions taking both stack inputs and MVectors).
 */
enum Operands {
  None            = 0,
  Stack3          = 1 << 0,
  Stack2          = 1 << 1,
  Stack1          = 1 << 2,
  StackIns1       = 1 << 3,  // Insert an element under top of stack
  StackIns2       = 1 << 4,  // Insert an element under top 2 of stack
  FuncdRef        = 1 << 5,  // Input to FPass*
  FStack          = 1 << 6,  // output of FPushFuncD and friends
  Local           = 1 << 7,  // Writes to a local
  MVector         = 1 << 8,  // Member-vector input
  Iter            = 1 << 9,  // Iterator in imm[0]
  AllLocals       = 1 << 10, // All locals (used by RetC)
  DontGuardStack1 = 1 << 11, // Dont force a guard on behalf of stack1 input
  IgnoreInnerType = 1 << 12, // Instruction doesnt care about the inner types
  DontGuardAny    = 1 << 13, // Dont force a guard for any input
  This            = 1 << 14, // Input to CheckThis
  StackN          = 1 << 15, // pop N cells from stack; n = imm[0].u_IVA
  BStackN         = 1 << 16, // consume N cells from stack for builtin call;
                             // n = imm[0].u_IVA
  StackTop2 = Stack1 | Stack2,
  StackTop3 = Stack1 | Stack2 | Stack3,
};

inline Operands operator|(const Operands& l, const Operands& r) {
  return Operands(int(r) | int(l));
}
}

struct InstrInfo {
  InstrFlags::Operands           in;
  InstrFlags::Operands           out;
  InstrFlags::OutTypeConstraints type; // How are outputs related to inputs?
  int numPushed;
};

const InstrInfo& getInstrInfo(Op op);

typedef const int COff; // Const offsets

inline bool isNativeImplCall(const Func* funcd, int numArgs) {
  return funcd && funcd->methInfo() && numArgs == funcd->numParams();
}

} } // HPHP::JIT

#endif
