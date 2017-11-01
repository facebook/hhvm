/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/prof-src-key.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/types.h"

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
namespace irgen { struct IRGS; }

constexpr uint32_t transCountersPerChunk = 1024 * 1024 / 8;


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
  TransContext(TransID id, TransKind kind, TransFlags flags,
               SrcKey sk, FPInvOffset spOff, Op callerFPushOp = Op::Nop);

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
  TransKind kind{TransKind::Invalid};
  TransFlags flags;
  FPInvOffset initSpOffset;
  Op callerFPushOp;
  const Func* func;
  Offset initBcOffset;
  bool hasThis;
  bool prologue;
  ResumeMode resumeMode;
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
  explicit InputInfo(const Location& l) : loc(l) {}

  std::string pretty() const;

public:
  // Location specifier for the input.
  Location loc;

  // If an input is unknowable, don't break the tracelet just to find its
  // type---but still generate a guard if that will tell us its type.
  bool dontBreak{false};

  // Never break the tracelet nor generate a guard on account of this input.
  bool dontGuard{false};

  // Never guard the inner type if this input is KindOfRef.
  bool dontGuardInner{false};
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
InputInfoVec getInputs(NormalizedInstruction&, FPInvOffset bcSPOff);

/*
 * Return the index of op's local immediate.
 */
size_t localImmIdx(Op op);

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
  OutVec,
  OutVecImm,
  OutDict,
  OutDictImm,
  OutKeyset,
  OutKeysetImm,
  OutObject,
  OutResource,
  OutThisObject,        // Object from current environment
  OutFDesc,             // Blows away the current function desc

  OutUnknown,           // Not known at tracelet compile-time
  OutPredBool,          // Boolean value predicted to be True or False
  OutCns,               // Constant; may be known at compile-time
  OutVUnknown,          // type is V(unknown)

  OutSameAsInput1,      // type is the same as the first stack input
  OutModifiedInput2,    // type is the same as the second stack input, but
                        // counted and unspecialized
  OutModifiedInput3,    // type is the same as the third stack input, but
                        // counted and unspecialized
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
  FuncdRef        = 1 << 4,  // Input to FPass*
  FStack          = 1 << 5,  // output of FPushFuncD and friends
  Local           = 1 << 6,  // Writes to a local
  Iter            = 1 << 7,  // Iterator in imm[0]
  AllLocals       = 1 << 8, // All locals (used by RetC)
  DontGuardStack1 = 1 << 9, // Dont force a guard on behalf of stack1 input
  IgnoreInnerType = 1 << 10, // Instruction doesnt care about the inner types
  DontGuardAny    = 1 << 11, // Dont force a guard for any input
  This            = 1 << 12, // Input to CheckThis
  StackN          = 1 << 13, // pop N cells from stack; n = imm[0].u_IVA
  BStackN         = 1 << 14, // consume N cells from stack for builtin call;
                             // n = imm[0].u_IVA
  StackI          = 1 << 15, // consume 1 cell at index imm[0].u_IVA
  MBase           = 1 << 16, // member operation base
  MKey            = 1 << 17, // member lookup key
  LocalRange      = 1 << 18, // read range of locals given in imm[1].u_LAR
  DontGuardBase   = 1 << 19, // Dont force a guard for the base
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

///////////////////////////////////////////////////////////////////////////////
// Completely unrelated functionality.

/*
 * Initialize the instruction table queried by the various functions in this
 * module.
 */
void initInstrInfo();

/*
 * This routine attempts to find the Func* that will be called for a given
 * target Class and function name, when called from ctxFunc.  This function
 * determines if a given Func* will be called in a request-insensitive way
 * (i.e. suitable for burning into the TC as a pointer).
 *
 * If exactClass is true, the class we are targeting is assumed to be
 * exactly `cls', and the returned Func* is definitely the one called.
 *
 * If exactClass is false, the class we are targeting may be a subclass of
 * cls, and the returned Func* may be overridden in a subclass.
 *
 * Its the caller's responsibility to ensure that the Class* is usable -
 * is AttrUnique, an instance of the ctx or guarded in some way.
 *
 * Returns nullptr if we can't determine the Func*.
 */
const Func* lookupImmutableMethod(const Class* cls, const StringData* name,
                                  bool& magicCall, bool staticLookup,
                                  const Func* ctxFunc, bool exactClass);

/*
 * If possible find the constructor for cls that would be run from the
 * context ctx if a new instance of cls were created there.  If the
 * constructor is inaccessible from the given context this function
 * will return nullptr. It is the caller's responsibility to ensure
 * that cls is the right Class* (ie its AttrUnique or bound to the
 * ctx, or otherwise guaranteed by guards).
 */
const Func* lookupImmutableCtor(const Class* cls, const Class* ctx);

/*
 * Find a function which always uniquely maps to the given name in the context
 * of the given unit. A function so returned can be used directly in the TC as
 * it will not change.
 *
 * This generally includes persistent functions, but can also include
 * non-persistent functions in certain situations. Note that even if the
 * function is immutable, the unit it is defined in may need loading. In that
 * case, the function is safe to use, but you have to emit code to ensure the
 * unit is loaded first.
 */
struct ImmutableFuncLookup {
  const Func* func;
  // Does any use of this function require a check to ensure its unit is loaded?
  bool needsUnitLoad;
};
ImmutableFuncLookup lookupImmutableFunc(const Unit* unit,
                                        const StringData* name);

/*
 * The offset, in cells, of this location from the frame pointer.
 */
int locPhysicalOffset(int32_t localIndex);

/*
 * Take a NormalizedInstruction and turn it into a call to the appropriate ht
 * functions.  Updates the bytecode marker, handles interp one flags, etc.
 */
void translateInstr(
  irgen::IRGS&,
  const NormalizedInstruction&,
  bool checkOuterTypeOnly,
  bool firstInst
);

///////////////////////////////////////////////////////////////////////////////
}}

#define incl_HPHP_TRANSLATOR_INL_H_
#include "hphp/runtime/vm/jit/translator-inl.h"
#undef incl_HPHP_TRANSLATOR_INL_H_

#endif
