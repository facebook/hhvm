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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/prof-src-key.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/util/hash-map-typedefs.h"
#include "hphp/util/mutex.h"

#include <folly/Format.h>

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Class;
struct Func;

namespace Debug { struct DebugInfo; }

namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Block;
struct IRTranslator;
struct NormalizedInstruction;
struct ProfData;

static const uint32_t transCountersPerChunk = 1024 * 1024 / 8;


///////////////////////////////////////////////////////////////////////////////
// Translator exceptions.

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
  throw TranslationFailedExc(__FILE__, __LINE__); \
} while(0)

#define throwUnknownInput() do { \
  throw UnknownInputExc(__FILE__, __LINE__); \
} while(0);


///////////////////////////////////////////////////////////////////////////////
// Translator auxiliary types.

using BlockIdToRegionBlockMap = hphp_hash_map<RegionDesc::BlockId,
                                              RegionDesc::Block*>;
using BlockIdToIRBlockMap = hphp_hash_map<RegionDesc::BlockId, Block*>;

/*
 * The information about the context a translation is occurring in.
 *
 * These fields are fixed for the whole translation.  Many objects in the JIT
 * need access to this.
 */
struct TransContext {
  TransID transID;  // May be kInvalidTransID if not for a real translation.
  Offset initBcOffset;
  Offset initSpOffset;
  bool resumed;
  const Func* func;
};

/*
 * Arguments for the translate() entry points in Translator.
 *
 * These include a variety of flags that help decide what to translate, or what
 * to do after we're done, so it's distinct from the TransContext above.
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
  TranslArgs& region(RegionDescPtr region) {
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
  RegionDescPtr m_region;
};


///////////////////////////////////////////////////////////////////////////////
// Translator.

/*
 * Module for converting a RegionDesc into an IR instruction stream.
 *
 * There is only ever one single Translator, owned by the global MCGenerator,
 * whose state is reset in between translations.
 */
struct Translator {

  Translator();

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  /*
   * Blacklisted instruction set.
   *
   * Used by translateRegion() to track instructions that must be interpreted.
   */
  typedef ProfSrcKeySet RegionBlacklist;


  /////////////////////////////////////////////////////////////////////////////
  // Main translation API.

  enum TranslateResult {
    Failure,
    Retry,
    Success
  };
  static const char* ResultName(TranslateResult r);

  /*
   * Start, end, or free a trace of code to be translated.
   */
  void traceStart(TransContext context);
  void traceEnd();
  void traceFree();

  /*
   * Translate `region'.
   *
   * The `toInterp' RegionBlacklist is a set of instructions which must be
   * interpreted.  When an instruction fails in translation, Retry is returned,
   * and the instruction is added to `interp' so that it will be interpreted on
   * the next attempt.
   */
  TranslateResult translateRegion(const RegionDesc& region,
                                  bool bcControlFlow,
                                  RegionBlacklist& interp,
                                  TransFlags trflags = TransFlags{});


  /////////////////////////////////////////////////////////////////////////////
  // Accessors.

  /*
   * Get the IRTranslator for the current translation.
   *
   * This is reset whenever traceStart() is called.
   */
  IRTranslator* irTrans() const;

  /*
   * Get the Translator's ProfData.
   */
  ProfData* profData() const;

  /*
   * Get the SrcDB.
   *
   * This is the one true SrcDB, since Translator is used as a singleton.
   */
  const SrcDB& getSrcDB() const;

  /*
   * Get the SrcRec for `sk'.
   *
   * If no SrcRec exists, insert one into the SrcDB.
   */
  SrcRec* getSrcRec(const SrcKey& sk);


  /*
   * Current region being translated, if any.
   */
  const RegionDesc* region() const;

  /////////////////////////////////////////////////////////////////////////////
  // Configuration.

  /*
   * We call the TransKind `mode' for some reason.
   */
  TransKind mode() const;
  void setMode(TransKind mode);

  /*
   * Whether to use ahot.
   *
   * This defaults to runtime option values, and is only changed if we're using
   * ahot and it runs out of space.
   */
  bool useAHot() const;
  void setUseAHot(bool val);


  /////////////////////////////////////////////////////////////////////////////
  // Translation DB.
  //
  // We maintain mappings from TCAs and TransIDs to translation information,
  // for debugging purposes only.  Outside of debug builds and processes with
  // TC dumps enabled, these routines do no work, and their corresponding data
  // structures are unused.
  //
  // Note that PGO always has a coherent notion of TransID---the so-called
  // `profTransID', which is just the region block ID (which are globally
  // unique).  This is completely distinct from the Translator's TransID.

  /*
   * Whether the TransDB structures should be used.
   *
   * True only for debug builds or when TC dumps are enabled.
   */
  static bool isTransDBEnabled();

  /*
   * Get a TransRec by TCA or TransID.
   *
   * Return nullptr if the TransDB is not enabled.
   */
  const TransRec* getTransRec(TCA tca) const;
  const TransRec* getTransRec(TransID transId) const;

  /*
   * Add a translation.
   *
   * Does nothing but trace if the TransDB is not enabled.
   */
  void addTranslation(const TransRec& transRec);

  /*
   * Get the TransID of the current (or next, if there is no current)
   * translation.
   */
  TransID getCurrentTransID() const;

  /*
   * Get the translation counter for `transId'.
   *
   * Return -1 if the TransDB is not enabled.
   */
  uint64_t getTransCounter(TransID transId) const;

  /*
   * Get a pointer to the translation counter for getCurrentTransID().
   *
   * Return nullptr if the TransDB is not enabled.
   */
  uint64_t* getTransCounterAddr();


  /////////////////////////////////////////////////////////////////////////////
  // Debug blacklist.
  //
  // The set of PC's and SrcKey's we refuse to JIT because they contain hphpd
  // breakpoints.

  /*
   * Atomically clear all entries from the debug blacklist.
   */
  void clearDbgBL();

  /*
   * Add `pc' to the debug blacklist.
   *
   * Return whether we actually performed an insertion.
   */
  bool addDbgBLPC(PC pc);

  /*
   * Check if `sk' is in the debug blacklist.
   *
   * Lazily populates m_dbgBLSrcKey from m_dbgBLPC if we don't find the entry.
   */
  bool isSrcKeyInBL(const SrcKey& sk);


  /////////////////////////////////////////////////////////////////////////////
  // Static data.

  static Lease& WriteLease();

  static const int MaxJmpsTracedThrough = 5;


  /////////////////////////////////////////////////////////////////////////////
  // Other methods.

public:
  static bool liveFrameIsPseudoMain();

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


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

public:
  UniqueStubs uniqueStubs;

private:
  int64_t m_createdTime;

  TransKind m_mode;
  const RegionDesc* m_region{nullptr};
  std::unique_ptr<ProfData> m_profData;
  bool m_useAHot;

  std::unique_ptr<IRTranslator> m_irTrans;
  SrcDB m_srcDB;

  // Translation DB.
  typedef std::map<TCA, TransID> TransDB;
  TransDB m_transDB;
  std::vector<TransRec> m_translations;
  std::vector<uint64_t*> m_transCounters;

  // Debug blacklist.
  PCFilter m_dbgBLPC;
  hphp_hash_set<SrcKey,SrcKey::Hasher> m_dbgBLSrcKey;
  Mutex m_dbgBlacklistLock;

  // Write lease.
  static Lease s_writeLease;
};


///////////////////////////////////////////////////////////////////////////////
// Stack information.

/*
 * Number of stack values popped by the opcode at `pc'.
 */
int64_t getStackPopped(PC pc);

/*
 * Number of stack values pushed by the opcode at `pc'.
 */
int64_t getStackPushed(PC pc);

/*
 * Change in stack depth made by `ni'.
 */
int getStackDelta(const NormalizedInstruction& ni);


///////////////////////////////////////////////////////////////////////////////
// Control flow information.

enum class ControlFlowInfo {
  None,
  ChangesPC,
  BreaksBB
};

/*
 * Return the ControlFlowInfo for `instr'.
 */
ControlFlowInfo opcodeControlFlowInfo(const Op op);

/*
 * Return true if the instruction can potentially set PC to point to something
 * other than the next instruction in the bytecode.
 */
bool opcodeChangesPC(const Op op);

/*
 * Return true if the instruction always breaks a tracelet.
 *
 * Most instructions that change PC will break the tracelet, though some do not
 * (e.g., FCall).
 */
bool opcodeBreaksBB(const Op op);

/*
 * Similar to opcodeBreaksBB but more strict.  We break profiling blocks after
 * any instruction that can side exit, including instructions with predicted
 * output, and before any control flow merge point.
 */
bool instrBreaksProfileBB(const NormalizedInstruction* inst);


///////////////////////////////////////////////////////////////////////////////
// Input and output information.

/*
 * Location and metadata for an instruction's input.
 */
struct InputInfo {
  explicit InputInfo(const Location& l)
    : loc(l)
    , dontBreak(false)
    , dontGuard(l.isLiteral())
    , dontGuardInner(false)
  {}

  std::string pretty() const;

public:
  // Location tag for the input.
  Location loc;

  // If an input is unknowable, don't break the tracelet just to find its
  // type---but still generate a guard if that will tell us its type.
  bool dontBreak;

  // Never break the tracelet nor generate a guard on account of this input.
  bool dontGuard;

  // Never guard the inner type if this input is KindOfRef.
  bool dontGuardInner;
};

/*
 * Vector of InputInfo with some flags and a pretty-printer.
 */
struct InputInfoVec : public std::vector<InputInfo> {
  InputInfoVec()
    : needsRefCheck(false)
  {}

  std::string pretty() const;

public:
  bool needsRefCheck;
};

/*
 * Callback used by getInputs() to get the type of a local variable with a
 * given index.
 */
using LocalTypeFn = std::function<Type(int)>;

/*
 * Get input location info and flags for `ni'.
 *
 * The result is returned via `infos'.  Some flags on `ni' may be updated.
 *
 * `startSk' should be the SrcKey for the first instruction in the region
 * containing `ni'.
 */
void getInputs(SrcKey startSk,
               NormalizedInstruction& inst,
               InputInfoVec& infos,
               const LocalTypeFn& localType);

namespace InstrFlags {
///////////////////////////////////////////////////////////////////////////////

/*
 * Type of the output(s) of an instruction.
 *
 * May be dependent on the input type.
 */
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

  OutSameAsInput,       // type is the same as the first stack input
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
 * Input codes indicate what an instruction reads, and some other things about
 * their behavior.
 *
 * The order these show up in the inputs vector is given in getInputs(), and is
 * relevant in a few cases (e.g. instructions taking both stack inputs and
 * MVectors).
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

Operands operator|(const Operands& l, const Operands& r);

///////////////////////////////////////////////////////////////////////////////
}

/*
 * Metadata describing an instruction's inputs and outputs, including their
 * number and constraints.
 *
 * Encoded in a sparse table in translator.cpp.
 */
struct InstrInfo {
  InstrFlags::Operands in;
  InstrFlags::Operands out;
  InstrFlags::OutTypeConstraints type; // How are outputs related to inputs?
  int numPushed;
};

/*
 * Get the InstrInfo for `op'.
 */
const InstrInfo& getInstrInfo(Op op);

/*
 * Is the output of `instr' dependent on its input?
 */
bool outputDependsOnInput(const Op op);

/*
 * Is the output of `inst' predicted?
 *
 * Flags on `inst' may be updated.
 */
bool outputIsPredicted(NormalizedInstruction& inst);

/*
 * If this returns true, we dont generate guards for any of the inputs to this
 * instruction.
 *
 * This is used to avoid generating guards for interpreted instructions.
 */
bool dontGuardAnyInputs(Op op);


///////////////////////////////////////////////////////////////////////////////
// Property information.

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
                           Class* ctx, const Class*& baseClass,
                           const MInstrInfo& mii,
                           unsigned mInd, unsigned iInd);

PropInfo getFinalPropertyOffset(const NormalizedInstruction& ni,
                                Class* ctx, const MInstrInfo& mii);


///////////////////////////////////////////////////////////////////////////////
// Other instruction information.

/*
* Some bytecodes are always no-ops but kept around for various reasons (mostly
* stack flavor safety).
 */
bool isAlwaysNop(Op op);

/*
 * Return true if we have absolutely no JIT support for `inst'.
 *
 * Always returns true if JitAlwaysInterpOne is set.
 */
bool instrMustInterp(const NormalizedInstruction& inst);

/*
 * Could `inst' clobber the locals in the environment of `caller'?
 *
 * This occurs, e.g., if `inst' is a call to extract().
 */
bool callDestroysLocals(const NormalizedInstruction& inst,
                        const Func* caller);


///////////////////////////////////////////////////////////////////////////////
// Completely unrelated functionality.

/*
 * This routine attempts to find the Func* that will be called for a given
 * target Class and function name, from a given context.  This function
 * determines if a given Func* will be called in a request-insensitive way
 * (i.e. suitable for burning into the TC as a pointer).  The class we are
 * targeting is assumed to be a subclass of `cls', not exactly `cls'.
 *
 * This function should not be used in a context where the call may involve
 * late static binding (i.e. FPushClsMethod), since it assumes static functions
 * will be resolved as targeting on cls regardless of whether they are
 * overridden.
 *
 * Returns nullptr if we can't be sure this would always call this function.
 */
const Func* lookupImmutableMethod(const Class* cls, const StringData* name,
                                  bool& magicCall, bool staticLookup,
                                  Class* ctx);

/*
 * Check whether return types of builtins are not simple types.
 *
 * This is different from IS_REFCOUNTED_TYPE because builtins can return
 * Variants, and we use KindOfUnknown to denote these return types.
 */
inline bool isCppByRef(DataType t) {
  return t != KindOfBoolean && t != KindOfInt64 &&
         t != KindOfNull && t != KindOfDouble;
}

/*
 * Return true if type is passed in/out of C++ as String&/Array&/Object&.
 */
inline bool isSmartPtrRef(DataType t) {
  return t == KindOfString || t == KindOfStaticString ||
         t == KindOfArray || t == KindOfObject ||
         t == KindOfResource;
}

/*
 * Is a call to `funcd' with `numArgs' arguments a NativeImpl call?
 */
inline bool isNativeImplCall(const Func* funcd, int numArgs) {
  return funcd && funcd->methInfo() && numArgs == funcd->numParams();
}

/*
 * The offset, in cells, of this location from its base pointer.
 *
 * The Func* is needed to see how many locals to skip for iterators.  If the
 * current frame pointer is not the context you're looking for, be sure to pass
 * in a non-default `f'.
 */
int locPhysicalOffset(Location l, const Func* f = nullptr);

extern bool tc_dump();

///////////////////////////////////////////////////////////////////////////////
}}

#define incl_HPHP_TRANSLATOR_INL_H_
#include "hphp/runtime/vm/jit/translator-inl.h"
#undef incl_HPHP_TRANSLATOR_INL_H_

#endif
