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
#include "hphp/runtime/vm/jit/recycle-tc.h"
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
struct IRGS;

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

#define punt() do { \
  throw TranslationFailedExc(__FILE__, __LINE__); \
} while(0)

#define throwUnknownInput() do { \
  throw UnknownInputExc(__FILE__, __LINE__); \
} while(0);


///////////////////////////////////////////////////////////////////////////////
// Translator auxiliary types.

using BlockIdToIRBlockMap = hphp_hash_map<RegionDesc::BlockId, Block*>;

/*
 * The information about the context a translation is occurring in.
 *
 * These fields are fixed for the whole translation.  Many objects in the JIT
 * need access to this.
 */
struct TransContext {
  TransContext(TransID id, SrcKey sk, FPInvOffset spOff);

  /*
   * The SrcKey for this translation.
   */
  SrcKey srcKey() const;

  /*
   * Data members.
   *
   * The contents of SrcKey are re-laid out to avoid func table lookups.
   */
  TransID transID;  // May be kInvalidTransID if not for a real translation.
  FPInvOffset initSpOffset;
  const Func* func;
  Offset initBcOffset;
  bool prologue;
  bool resumed;
};

/*
 * Arguments for the translate() entry points in Translator.
 *
 * These include a variety of flags that help decide what to translate, or what
 * to do after we're done, so it's distinct from the TransContext above.
 */
struct TranslArgs {
  TranslArgs(SrcKey sk, bool align) : sk{sk}, align{align} {}

  SrcKey sk;
  bool align;
  bool setFuncBody{false};
  TransFlags flags{0};
  TransID transId{kInvalidTransID};
  RegionDescPtr region{nullptr};
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
  // Accessors.

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
  SrcRec* getSrcRec(SrcKey sk);

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
  bool isSrcKeyInBL(SrcKey sk);


  /////////////////////////////////////////////////////////////////////////////
  // Static data.

  static Lease& WriteLease();

  static const int MaxJmpsTracedThrough = 5;


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

public:
  UniqueStubs uniqueStubs;

private:
  int64_t m_createdTime;

  TransKind m_mode;
  std::unique_ptr<ProfData> m_profData;
  bool m_useAHot;

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
 * Get input location info and flags for a NormalizedInstruction.  Some flags
 * on `ni' may be updated.
 */
InputInfoVec getInputs(NormalizedInstruction&);

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
 * If this returns true, we dont generate guards for any of the inputs to this
 * instruction.
 *
 * This is used to avoid generating guards for interpreted instructions.
 */
bool dontGuardAnyInputs(Op op);

///////////////////////////////////////////////////////////////////////////////
// Other instruction information.

/*
* Some bytecodes are always no-ops but kept around for various reasons (mostly
* stack flavor safety).
 */
bool isAlwaysNop(Op op);

/*
 * Could `inst' clobber the locals in the environment of `caller'?
 *
 * This occurs, e.g., if `inst' is a call to extract().
 */
bool callDestroysLocals(const NormalizedInstruction& inst,
                        const Func* caller);

/*
 * Could the CPP builtin function `callee` destroy the locals
 * in the environment of its caller?
 *
 * This occurs, e.g., if `func' is extract().
 */
bool builtinFuncDestroysLocals(const Func* callee);

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
                                  const Class* ctx);

/*
 * If possible find the constructor for cls that would be run from the context
 * ctx if a new instance of cls were created there. If the class fails to be
 * unique, or in non-repo-authoritative mode this function will always return
 * nullptr. Additionally if the constructor is inaccessible from the given
 * context this function will return nullptr.
 */
const Func* lookupImmutableCtor(const Class* cls, const Class* ctx);

/*
 * Return true if type is passed in/out of C++ as String&/Array&/Object&.
 */
inline bool isReqPtrRef(MaybeDataType t) {
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
 * The offset, in cells, of this location from the frame pointer.
 */
int locPhysicalOffset(int32_t localIndex);

/*
 * Take a NormalizedInstruction and turn it into a call to the appropriate ht
 * functions.  Updates the bytecode marker, handles interp one flags, etc.
 */
void translateInstr(
  IRGS&,
  const NormalizedInstruction&,
  bool checkOuterTypeOnly,
  bool needsExitPlaceholder
);

extern bool tc_dump();

///////////////////////////////////////////////////////////////////////////////
}}

#define incl_HPHP_TRANSLATOR_INL_H_
#include "hphp/runtime/vm/jit/translator-inl.h"
#undef incl_HPHP_TRANSLATOR_INL_H_

#endif
