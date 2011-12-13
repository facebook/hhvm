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

// Translator front-end: parse instruction stream into basic blocks, decode
// and normalize instructions. Propagate run-time type info to instructions
// to annotate their inputs and outputs with types.
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>

#include <vector>
#include <string>

#include <runtime/base/runtime_option.h>
#include <runtime/base/types.h>
#include <runtime/base/tv_macros.h>
#include <util/trace.h>
#include <util/biased_coin.h>
#include <runtime/vm/hhbc.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/runtime.h>

using namespace std;

namespace HPHP {

namespace VM {
namespace Transl {

using namespace HPHP::VM;

TRACE_SET_MOD(trans)

static __thread BiasedCoin *dbgTranslateCoin;

void InstrStream::append(NormalizedInstruction* ni) {
  if (last) {
    ASSERT(first);
    last->next = ni;
    ni->prev = last;
    ni->next = NULL;
    last = ni;
    return;
  }
  ASSERT(!first);
  first = ni;
  last = ni;
  ni->prev = NULL;
  ni->next = NULL;
}

void InstrStream::remove(NormalizedInstruction* ni) {
  if (ni->prev) {
    ni->prev->next = ni->next;
  } else {
    first = ni->next;
  }
  if (ni->next) {
    ni->next->prev = ni->prev;
  } else {
    last = ni->prev;
  }
  ni->prev = NULL;
  ni->next = NULL;
}

Tracelet::~Tracelet() {
  // XXXkma: scoped_ptr
  for (unsigned i = 0; i < m_instrs.size(); ++i) {
    delete m_instrs[i];
  }
  for (unsigned i = 0; i < m_dynlocs.size(); ++i) {
    delete m_dynlocs[i];
  }
}

NormalizedInstruction* Tracelet::newNormalizedInstruction() {
  NormalizedInstruction* ni = new NormalizedInstruction();
  m_instrs.push_back(ni);
  return ni;
}

DynLocation* Tracelet::newDynLocation(Location l, DataType t) {
  DynLocation* dl = new DynLocation(l, t);
  m_dynlocs.push_back(dl);
  return dl;
}

DynLocation* Tracelet::newDynLocation(Location l, RuntimeType t) {
  DynLocation* dl = new DynLocation(l, t);
  m_dynlocs.push_back(dl);
  return dl;
}

DynLocation* Tracelet::newDynLocation() {
  DynLocation* dl = new DynLocation();
  m_dynlocs.push_back(dl);
  return dl;
}

void
SrcKey::trace(const char *fmt, ...) const {
  if (!Trace::enabled) {
    return;
  }
  // We don't want to print string literals, so don't pass the unit
  string s = instrToString(curUnit()->at(m_offset));
  const char *filepath = "*anonFile*";
  if (curUnit()->m_filepath && curUnit()->m_filepath->data() &&
      strlen(curUnit()->m_filepath->data()) > 0)
    filepath = curUnit()->m_filepath->data();
  Trace::trace("%s:%llx %llx%6d: %20s ",
               filepath, m_md5.q[0], m_md5.q[1],
               m_offset, s.c_str());
  va_list a;
  va_start(a, fmt);
  Trace::vtrace(fmt, a);
  va_end(a);
}

// advance --
//
//  Move over the current instruction pointer.
void
Translator::advance(const Opcode** instrs) {
  (*instrs) += instrLen(*instrs);
}


/*
 * locPhysicalOffset --
 *
 *   Return offset, in cells, of this location from its base
 *   pointer. It needs a function descriptor to see how many locals
 *   to skip for iterators; if the current frame pointer is not the context
 *   you're looking for, be sure to pass in a non-default f.
 */
int
Translator::locPhysicalOffset(Location l, const Func* f) {
  f = f ? f : curFunc();
  assert_not_implemented(l.space == Location::Stack ||
                         l.space == Location::Local ||
                         l.space == Location::Iter);
  int localsToSkip = l.space == Location::Iter ? f->m_numLocals : 0;
  int iterInflator = l.space == Location::Iter ? kNumIterCells : 1;
  return -((l.offset + 1) * iterInflator + localsToSkip);
}

// liveType --
//   Return the live type of a location. If we've already plucked
//   out the cell ...
RuntimeType Translator::liveType(Location l, const Unit& u) {
  Cell *outer;
  switch(l.space) {
    case Location::Stack:
      // Stack accesses must be to addresses pushed before
      // translation time; if they are to addresses pushed after,
      // they should be hitting in the changemap.
      ASSERT(locPhysicalOffset(l) >= 0);
      // fallthru
    case Location::Local: {
      Cell *base;
      int offset = locPhysicalOffset(l);
      base    = l.space == Location::Stack ? vmsp() : vmfp();
      outer = &base[offset];
    } break;
    case Location::Iter: {
      const Iter *it = FP2ITER(curFrame(), l.offset);
      TRACE(1, "Iter input: fp %p, iter %p, offset %d\n", vmfp(),
            it, l.offset);
      return RuntimeType(it);
    } break;
    default: {
      NOT_REACHED();
    } break;
  }
  return liveType(outer, l);
}

RuntimeType
Translator::liveType(const Cell* outer, const Location& l) {
  if (!outer) {
    // An undefined global; starts out as a variant null
    return RuntimeType(KindOfVariant, KindOfNull);
  }
  DataType outerType = (DataType)outer->m_type;
  if (outerType == KindOfHome) {
    // Home. Get the live type of the referent, and mark it as a home.
    Cell *pval = outer->m_data.ptv;
    Location l = tvToLocation(pval);
    RuntimeType retval = RuntimeType(l);
    TRACE(2, "liveType home (%p) at (%s, %d)\n", pval,
          l.spaceName(), l.offset);
    return retval;
  }
  DataType valueType = outerType;
  DataType innerType = KindOfInvalid;
  const Cell* valCell = outer;
  if (outerType == KindOfVariant) {
    // Variant. Pick up the inner type, too.
    valCell = outer->m_data.ptv;
    DataType innerType = valCell->m_type;
    valueType = innerType;
    ASSERT(innerType != KindOfVariant);
    TRACE(2, "liveType Var -> %d\n", innerType);
    return RuntimeType(KindOfVariant, innerType);
  }
  const Class *klass = NULL;
  if (valueType == KindOfObject) {
    // TODO: Infer the class, too.
    if (false && LIKELY(valCell->m_data.pobj->isInstance())) {
      klass = valCell->m_data.pobj->getVMClass();
    }
  }
  TRACE(2, "liveType %d\n", outerType);
  RuntimeType retval = RuntimeType(outerType, innerType, klass);
  return retval;
}

bool Translator::liveFrameIsPseudoMain() {
  ActRec* ar = (ActRec*)vmfp();
  return ar->m_func->isPseudoMain();
}

Location
Translator::tvToLocation(const TypedValue* tv) {
  Cell *arg0 = vmfp() + locPhysicalOffset(Location(Location::Local, 0));
  // Physical stack offsets grow downwards from the frame pointer. See
  // locPhysicalOffset.
  int offset = -(tv - arg0);
  ASSERT(offset >= 0);
  ASSERT(offset < curFunc()->m_numLocals);
  TRACE(2, "tvToLocation: %p -> L:%d\n", tv, offset);
  return Location(Location::Local, offset);
}

/* Opcode type-table. */
enum OutTypeConstraints {
  OutNull,
  OutString,
  OutStringImm,         // String w/ precisely known immediate.
  OutDouble,
  OutBoolean,
  OutInt64,
  OutArray,
  OutObject,
  OutThisObject,        // Object from current environment
  OutHome,              // type is H(n)
  OutFDesc,             // Blows away the current function desc

  OutUnknown,           // Not known at tracelet compile-time
  OutCns,               // Constant; may be known at compile-time
  OutVUnknown,          // type is V(unknown)

  OutSameAsInput,       // type of sole input
  OutCInput,            // type is C(input)
  OutVInput,            // type is V(input)
  OutCInputH,           // type is C(type) of home input
  OutVInputH,           // type is V(type) of home input
  OutFInputH,           // type is V(type) of home input if current param is
                        //   by ref, else type is C(type) of home input
  OutFInputR,           // Like FInputH, but for R's on the stack.

  OutArith,             // For Add, Sub, Mul
  OutBitOp,             // For BitAnd, BitOr, BitXor
  OutSetOp,             // For SetOpH
  OutClassRef,          // KindOfClass
  OutNone
};

/*
 * inputs and outputs will appear in this order in NormalizedInstruction's
 * input/output vectors. "Stack first" seems to be the only one that comes
 * up in practice: e.g., SetLoc both pushes the assigned value to the
 * evaluation stack and writes the local variable; outputs[0] is the stack,
 * outputs[1] is the local.
 */
enum Operands {
  None          = 0,
  Stack3        = 1 << 0,
  Stack2        = 1 << 1,
  Stack1        = 1 << 2,
  FuncdRef      = 1 << 3,  // Input to Prep*, output of PassParam*
  FStack        = 1 << 4,  // output of FPushFuncD and friends
  StackPeek     = 1 << 5,  // Element under top element on stack
  StackPeekImm  = 1 << 6,  // n'th element from top of stack
  Local         = 1 << 7,  // Writes to a local
  MVector       = 1 << 8,  // Member-vector input
  Iter          = 1 << 9,  // Iterator in imm[0]
  AllLocals     = 1 << 10, // All locals (used by RetC)

  StackTop2 = Stack1 | Stack2,
  StackTop3 = Stack1 | Stack2 | Stack3,
};

Operands
operator|(const Operands& l, const Operands& r) {
  return Operands(int(r) | int(l));
}

static int64 typeToMask(DataType t) {
  // KindOfInvalid == -1 and KindOfHome == -2, so we have to add 2 to make
  // sure t is nonnegative.
  ASSERT((t+2) >= 0 && (t+2) <= 63);
  return (1 << (t+2));
}

struct InferenceRule {
  int64 mask;
  DataType result;
};

static DataType inferType(const InferenceRule* rules,
                          const vector<DynLocation*>& inputs) {
  int inputMask = 0;
  // We generate the inputMask by ORing together the mask for each input's
  // type.
  for (unsigned int i = 0; i < inputs.size(); ++i) {
    DataType inType = inputs[i]->rtt.valueType();
    inputMask |= typeToMask(inType);
  }
  // This loop checks each rule in order, looking for the first rule that
  // applies. Note that we assume there's a "catch-all" at the end.
  for (unsigned int i = 0; ; ++i) {
    if (rules[i].mask == 0 || (rules[i].mask & inputMask) != 0) {
      return rules[i].result;
    }
  }
  // We return KindOfInvalid by default if none of the rules applied.
  return KindOfInvalid;
}

/*
 * Inference rules used for OutArith. These are applied in order
 * row-by-row.
 */

static const int64 StringMask = typeToMask(KindOfString) |
                                typeToMask(KindOfStaticString);
static const int64 IntMask = typeToMask(KindOfInt32) | typeToMask(KindOfInt64);
static const int64 ArrayMask  = typeToMask(KindOfArray);
static const int64 DoubleMask = typeToMask(KindOfDouble);
static const int64 InvalidMask = typeToMask(KindOfInvalid);

static const InferenceRule ArithRules[] = {
  { DoubleMask, KindOfDouble },
  { ArrayMask, KindOfArray },
  // If one of the inputs is known to be a String or if one of the input
  // types is unknown, the output type is Unknown
  { StringMask | InvalidMask, KindOfInvalid },
  // Default to Int64
  { 0, KindOfInt64 },
};

static const int NumArithRules = sizeof(ArithRules) / sizeof(InferenceRule);

/**
 * Returns the type of the output of a bitwise operator on the two DynLocs
 */
static const InferenceRule BitOpRules[] = {
  { IntMask, KindOfInt64 }, // One int is enough to force an int.
  { StringMask, KindOfString },
  { 0, KindOfInvalid },
};

static RuntimeType bitOpType(DynLocation* a, DynLocation* b) {
  vector<DynLocation*> ins;
  ins.push_back(a);
  if (b) ins.push_back(b);
  return inferType(BitOpRules, ins);
}

/**
 * Returns the type of the value a SetOpH will store into the home.
 */
static RuntimeType setOpOutputType(NormalizedInstruction* ni,
                                   const vector<DynLocation*>& inputs) {
  ASSERT(inputs.size() == 2);
  const int kValIdx = 0;
  const int kHomeIdx = 1;
  unsigned char op = ni->imm[0].u_OA;
  DynLocation homeLocation(inputs[kHomeIdx]->location,
                           inputs[kHomeIdx]->rtt.unbox());
  ASSERT(inputs[kHomeIdx]->location.isLocal());
  switch (op) {
    case SetOpPlusEqual:
    case SetOpMinusEqual:
    case SetOpMulEqual: {
      // Same as OutArith, except we have to fiddle with inputs a bit.
      vector<DynLocation*> arithInputs;
      arithInputs.push_back(&homeLocation);
      arithInputs.push_back(inputs[kValIdx]);
      return RuntimeType(inferType(ArithRules, arithInputs));
    }
    case SetOpConcatEqual: return RuntimeType(KindOfString);
    case SetOpDivEqual:
    case SetOpModEqual:    return RuntimeType(KindOfInvalid);
    case SetOpAndEqual:
    case SetOpOrEqual:
    case SetOpXorEqual:    return bitOpType(&homeLocation, inputs[kValIdx]);
    case SetOpSlEqual:
    case SetOpSrEqual:     return RuntimeType(KindOfInt64);
    default:
      ASSERT(false);
  }
  // not reached
  return RuntimeType(KindOfInvalid);
}

static RuntimeType
getDynLocType(const vector<DynLocation*>& inputs,
              const Tracelet& t,
              Opcode opcode,
              NormalizedInstruction* ni,
              Operands op,
              OutTypeConstraints constraint,
              DynLocation* outDynLoc) {
  ASSERT(constraint != OutFInputH);

  switch (constraint) {
#define CS(OutXLike, KindOfX) \
    case OutXLike:            \
      return RuntimeType(KindOfX);
    CS(OutInt64,       KindOfInt64);
    CS(OutBoolean,     KindOfBoolean);
    CS(OutDouble,      KindOfDouble);
    CS(OutString,      KindOfString);
    CS(OutNull,        KindOfNull);
    CS(OutUnknown,     KindOfInvalid); // Subtle interaction with BB-breaking.
    CS(OutFDesc,       KindOfInvalid); // Unclear if OutFDesc has a purpose.
    CS(OutArray,       KindOfArray);
    CS(OutObject,      KindOfObject);
#undef CS
    case OutClassRef: {
      Op op = Op(ni->op());
      if (op == OpCls && inputs[0]->isString()) {
        const StringData *sd = inputs[0]->rtt.valueString();
        if (sd) {
          Class *klass = g_context->lookupClass(sd);
          TRACE(3, "KindOfClass: derived class \"%s\" from string literal\n",
                klass ? klass->m_preClass->m_name->data() : "NULL");
          return RuntimeType(klass);
        }
      }
      return RuntimeType(KindOfClass);
    }
    case OutCns: {
      // If it's a ClassInfo constant, remember its type. Otherwise fall
      // back to the targetCache mechanism.
      StringData *sd = curUnit()->lookupLitstrId(ni->imm[0].u_SA);
      ASSERT(sd);
      TypedValue* tv = g_context->getCns(sd, true, false);
      if (tv) {
        return RuntimeType(tv->m_type);
      }
      return RuntimeType(KindOfInvalid);
    }
    case OutStringImm: {
      StringData *sd = curUnit()->lookupLitstrId(ni->imm[0].u_SA);
      ASSERT(sd);
      return RuntimeType(sd);
    }
    case OutThisObject: {
      // Use current environment's class as a constraint. Note that
      // since this is fp, not ar, we don't need to worry about fp having
      // moved during the tracelet; fp is invariant during tracelets today.
      const PreClass *ctx = arGetContextPreClass(curFrame());
      puntUnless(ctx); // $this in non-method
      // Fish out $this from the current environment ...
      puntUnless(curFrame()->hasThis()); // null $this
      const ObjectData* that = curFrame()->getThis();
      puntUnless(that->isInstance());
      const Class *klass = that->getVMClass()->classof(ctx);
      // Use a conservative typing of the object; use the least-derived
      // class we might be in.
      ASSERT_NOT_IMPLEMENTED(klass);
      TRACE(2, "KindOfThis: object of class \"%s\", "
            "derived from Class \"%s\"\n",
            that->getVMClass()->m_preClass->m_name->data(),
            klass->m_preClass->m_name->data());
      return RuntimeType(KindOfObject, KindOfInvalid, klass);
    }
    case OutVUnknown: {
      return RuntimeType(KindOfVariant, KindOfInvalid);
    }

    case OutArith: {
      return RuntimeType(inferType(ArithRules, inputs));
    }

    case OutSameAsInput: {
      // Relies on getInputs() pushing top of stack first for multi-stack
      // consumers.
      ASSERT(inputs.size() >= 1);
      int idx = inputs.size() - 1;
      Opcode op = ni->op();
      if (op == OpSetH || op == OpSetM || op == OpSetG || op == OpBindH ||
          op == OpSetS) {
        idx = 0;
      }
      if (op == OpBindH) {
        ASSERT(inputs[idx]->rtt.isVariant() &&
               !inputs[idx]->isLocal());
        return inputs[idx]->rtt;
      }
      return inputs[idx]->rtt.valueType();
    }

    case OutHome: {
      int localId = ni->imm[0].u_IVA;
      ASSERT(ni->op() == OpLoc);
      Location l(Location::Local, localId);
      RuntimeType retval = RuntimeType(l);
      TRACE(2, "Local id %d homed\n", localId);
      return retval;
    }

    case OutCInputH: {
      ASSERT(inputs.size() >= 1);
      const DynLocation* in = inputs[inputs.size() - 1];
      RuntimeType retval = in->rtt.unbox();
      TRACE(2, "Input (%d, %d) home(%d) -> unhomed (%d, %d)\n",
            in->rtt.outerType(), in->rtt.innerType(), in->rtt.isHome(),
            retval.outerType(), retval.innerType());
      return retval;
    }

    case OutCInput: {
      ASSERT(inputs.size() >= 1);
      const DynLocation* in = inputs[inputs.size() - 1];
      if (in->rtt.outerType() == KindOfVariant) {
        return in->rtt.unbox();
      }
      return in->rtt;
    }

    case OutBitOp: {
      ASSERT(inputs.size() == 2 ||
             (inputs.size() == 1 && opcode == OpBitNot));
      if (inputs.size() == 2) {
        return bitOpType(inputs[0], inputs[1]);
      } else {
        return bitOpType(inputs[0], NULL);
      }
    }

    case OutSetOp: {
      return setOpOutputType(ni, inputs);
    }

    case OutNone:
    default:
      return RuntimeType(KindOfInvalid);
  }
}

/*
 * NB: this opcode structure is sparse; it cannot just be indexed by
 * opcode.
 */
struct InstrInfo {
  Operands           in;
  Operands           out;
  OutTypeConstraints type;       // How are outputs related to inputs?
  int                stackDelta; // Impact on stack: # cells *pushed*
};

static const struct {
  Opcode    op;
  InstrInfo info;
} instrInfoSparse [] = {

  // Op             Inputs            Outputs       OutputTypes    Stack delta
  // --             ------            -------       -----------    -----------

  /*** 1. Basic instructions ***/

  { OpNop,         {None,             None,         OutNone,           0 }},
  { OpPopC,        {Stack1,           None,         OutNone,          -1 }},
  { OpPopV,        {Stack1,           None,         OutNone,          -1 }},
  { OpPopR,        {Stack1,           None,         OutNone,          -1 }},
  { OpBox,         {Stack1,           Stack1,       OutVInput,         0 }},
  { OpUnbox,       {Stack1,           Stack1,       OutCInput,         0 }},
  { OpBoxR,        {Stack1,           Stack1,       OutVInput,         0 }},
  { OpUnboxR,      {Stack1,           Stack1,       OutCInput,         0 }},

  /*** 2. Literal and constant instructions ***/

  { OpNull,        {None,             Stack1,       OutNull,           1 }},
  { OpTrue,        {None,             Stack1,       OutBoolean,        1 }},
  { OpFalse,       {None,             Stack1,       OutBoolean,        1 }},
  { OpInt,         {None,             Stack1,       OutInt64,          1 }},
  { OpDouble,      {None,             Stack1,       OutDouble,         1 }},
  { OpString,      {None,             Stack1,       OutStringImm,      1 }},
  { OpArray,       {None,             Stack1,       OutArray,          1 }},
  { OpNewArray,    {None,             Stack1,       OutArray,          1 }},
  { OpAddElemC,    {StackTop3,        Stack1,       OutArray,         -2 }},
  { OpAddElemV,    {StackTop3,        Stack1,       OutArray,         -2 }},
  { OpAddNewElemC, {StackTop2,        Stack1,       OutArray,         -1 }},
  { OpAddNewElemV, {StackTop2,        Stack1,       OutArray,         -1 }},
  { OpCns,         {None,             Stack1,       OutCns,            1 }},
  { OpClsCns,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpClsCnsD,     {None,             Stack1,       OutUnknown,        1 }},

  /*** 3. Operator instructions ***/

  /* Binary string */
  { OpConcat,      {StackTop2,        Stack1,       OutString,        -1 }},
  /* Arithmetic ops */
  { OpAdd,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpSub,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpMul,         {StackTop2,        Stack1,       OutArith,         -1 }},
  /* Div and mod might return boolean false. Sigh. */
  { OpDiv,         {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpMod,         {StackTop2,        Stack1,       OutUnknown,       -1 }},
  /* Logical ops */
  { OpAnd,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpOr,          {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpXor,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpNot,         {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpSame,        {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpNSame,       {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpEq,          {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpNeq,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpLt,          {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpLte,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpGt,          {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpGte,         {StackTop2,        Stack1,       OutBoolean,       -1 }},
  /* Bitwise ops */
  { OpBitAnd,      {StackTop2,        Stack1,       OutBitOp,         -1 }},
  { OpBitOr,       {StackTop2,        Stack1,       OutBitOp,         -1 }},
  { OpBitXor,      {StackTop2,        Stack1,       OutBitOp,         -1 }},
  { OpBitNot,      {Stack1,           Stack1,       OutBitOp,          0 }},
  { OpShl,         {StackTop2,        Stack1,       OutInt64,         -1 }},
  { OpShr,         {StackTop2,        Stack1,       OutInt64,         -1 }},
  /* Cast instructions */
  { OpCastBool,    {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpCastInt,     {Stack1,           Stack1,       OutInt64,          0 }},
  { OpCastDouble,  {Stack1,           Stack1,       OutDouble,         0 }},
  { OpCastString,  {Stack1,           Stack1,       OutString,         0 }},
  { OpCastArray,   {Stack1,           Stack1,       OutArray,          0 }},
  { OpCastObject,  {Stack1,           Stack1,       OutObject,         0 }},
  { OpInstanceOf,  {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpInstanceOfD, {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpPrint,       {Stack1,           Stack1,       OutInt64,          0 }},
  { OpClone,       {Stack1,           Stack1,       OutObject,         0 }},
  { OpExit,        {Stack1,           None,         OutNone,          -1 }},
  { OpRaise,       {StackTop2,        None,         OutNone,          -2 }},
  { OpFatal,       {Stack1,           None,         OutNone,          -1 }},

  /*** 4. Control flow instructions ***/

  { OpJmp,         {None,             None,         OutNone,           0 }},
  { OpJmpZ,        {Stack1,           None,         OutNone,          -1 }},
  { OpJmpNZ,       {Stack1,           None,         OutNone,          -1 }},
  /*
   * RetC and RetV are special. Their manipulation of the runtime stack are
   * outside the boundaries of the tracelet abstraction; since they always end
   * a basic block, they behave more like "glue" between BBs than the
   * instructions in the body of a BB.
   *
   * RetC and RetV consume a value from the stack, and this value's type needs
   * to be known at compile-time.
   */
  { OpRetC,        {AllLocals,        None,         OutNone,           0 }},
  { OpRetV,        {AllLocals,        None,         OutNone,           0 }},
  { OpThrow,       {Stack1,           None,         OutNone,          -1 }},

  /*** 5. Location instructions ***/

  { OpLoc,         {None,             Stack1,       OutHome,           1 }},
  { OpCls,         {StackPeekImm,     StackPeekImm, OutClassRef,       0 }},
  { OpClsH,        {StackPeekImm,     StackPeekImm, OutClassRef,       0 }},

  /*** Instruction groups 6 - 12 ***/

  { OpCGetH,       {Stack1,           Stack1,       OutCInputH,        0 }},
  { OpCGetH2,      {StackPeek,        StackPeek,    OutCInputH,        0 }},
  { OpCGetN,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetG,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetS,       {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpCGetM,       {MVector,          Stack1,       OutUnknown,        1 }},
  { OpVGetH,       {Stack1,           Stack1,       OutVInputH,        0 }},
  // TODO: In pseudo-main, the VGetG instruction invalidates what we know
  // about the types of the locals because it could cause any one of the
  // local variables to become "boxed". We need to add logic to tracelet
  // analysis to deal with this properly.
  { OpVGetG,       {Stack1,           Stack1,       OutVUnknown,       0 }},
  { OpVGetM,       {MVector,          Stack1|Local, OutVUnknown,       1 }},

  { OpIssetH,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIssetG,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIssetM,      {MVector,          Stack1,       OutBoolean,        1 }},
  { OpEmptyH,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpEmptyM,      {MVector,          Stack1,       OutBoolean,        1 }},

  { OpSetH,        {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpSetG,        {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpSetS,        {StackTop3,        Stack1,       OutSameAsInput,   -2 }},
  { OpSetM,        {MVector|Stack1,   Stack1|Local, OutSameAsInput,    0 }},
  { OpSetOpH,      {StackTop2,        Stack1|Local, OutSetOp,         -1 }},
  { OpIncDecH,     {Stack1,           Stack1|Local, OutCInputH,        0 }},
  { OpBindH,       {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpUnsetH,      {Stack1,           Local,        OutNone,          -1 }},
  { OpUnsetM,      {MVector,          None,         OutNone,           0 }},

  { OpFPushFuncD,  {None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushFunc,   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushCtorD,  {None,             Stack1|FStack,OutObject,
                                                     kNumActRecCells + 1 }},
  { OpFPushObjMethodD,
                   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushClsMethodD,
                   {None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushClsMethod,
                   {StackTop2,        FStack,       OutFDesc,
                                                     kNumActRecCells - 2 }},
  { OpFPassC,      {FuncdRef,         None,         OutNull,           0 }},
  { OpFPassCW,     {FuncdRef,         None,         OutNull,           0 }},
  { OpFPassCE,     {FuncdRef,         None,         OutNull,           0 }},
  { OpFPassR,      {Stack1|FuncdRef,  Stack1,       OutFInputR,        0 }},
  { OpFPassG,      {Stack1|FuncdRef,  Stack1,       OutFInputR,        0 }},
  { OpFPassH,      {Stack1|FuncdRef,  Stack1,       OutFInputH,        0 }},
  { OpFPassM,      {MVector|FuncdRef, Stack1,       OutUnknown,        1 }},

  /*
   * FCall is special. Like the Ret* instructions, its manipulation of the
   * runtime stack are outside the boundaries of the tracelet abstraction.
   */
  { OpFCall,       {FStack,           None,         OutNone,           0 }},

  { OpIterInit,    {Stack1,           None,         OutNull,          -1 }},
  { OpIterInitM,   {Stack1,           None,         OutNull,          -1 }},
  { OpIterValueC,  {Iter,             Stack1,       OutUnknown,        1 }},
  { OpIterValueV,  {Iter,             Stack1,       OutVUnknown,       1 }},
  { OpIterKey,     {Iter,             Stack1,       OutUnknown,        1 }},
  { OpIterNext,    {Iter,             None,         OutNull,           0 }},

  /*** Instruction group 13 ***/
  { OpIncl,        {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpInclOnce,    {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReq,         {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqOnce,     {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpEval,        {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpDefFunc,     {None,             None,         OutNone,           0 }},
  { OpDefCls,      {None,             None,         OutNone,           0 }},

  /*** Instruction group 14: Miscellaneous instructions ***/
  { OpThis,        {None,             Stack1,       OutThisObject,     1 }},

  /**
   * TODO: Add entries for the following instructions:
   *   Dup
   *   Unwind
   *   VGetN
   *   VGetS
   *   IssetN
   *   IssetS
   *   EmptyN
   *   EmptyG
   *   EmptyS
   *   SetN
   *   SetOpN
   *   SetOpG
   *   SetOpS
   *   SetOpM
   *   IncDecN
   *   IncDecG
   *   IncDecS
   *   IncDecM
   *   BindN
   *   BindG
   *   BindS
   *   BindM
   *   UnsetN
   *   UnsetG
   *   UnsetS
   *   FPushFunc
   *   FPushObjMethod
   *   FPushCtor
   *   FPassV
   *   FPassN
   *   FPassS
   *   IterFree
   *   InitThisLoc
   *   StaticLoc
   *   StaticLocInit
   *   Catch
   *   LateBoundCls
   */
};

static hphp_hash_map<Opcode, InstrInfo> instrInfo;
static bool instrInfoInited;
static void initInstrInfo() {
  if (!instrInfoInited) {
    for (size_t i = 0; i < sizeof(instrInfoSparse) / sizeof(instrInfoSparse[0]);
         i++) {
      instrInfo[instrInfoSparse[i].op] = instrInfoSparse[i].info;
    }

    instrInfoInited = true;
  }
}

static int numHiddenStackInputs(const Opcode *instr,
                                const InstrInfo& info) {
  ImmVector* iv = getImmVector(instr);
  return iv->numValues();
}

static int getStackDelta(const NormalizedInstruction& ni) {
  int hiddenStackInputs = 0;
  initInstrInfo();
  Opcode op = ni.op();
  const InstrInfo& info = instrInfo[op];
  if (info.in & MVector) {
    hiddenStackInputs = numHiddenStackInputs(ni.pc(), info);
    SKTRACE(2, ni.source, "Has %d hidden stack inputs\n", hiddenStackInputs);
  }
  int delta = instrInfo[op].stackDelta - hiddenStackInputs;
  return delta;
}

// Number to subtract from current stack depth given an operand mask
// and an instruction.
static int getStackPeekDelta(int opMask,
                             const NormalizedInstruction* ni) {
  ASSERT(!(opMask & StackPeek) || !(opMask & StackPeekImm));
  return (opMask & StackPeek) ?
    (1 + 1) :
    (1 + ni->imm[0].u_IVA);
}

/*
 * getInputs --
 *   Returns locations for this instruction's inputs.
 *
 * Throws:
 *   TranslationFailedExc:
 *     Unimplemented functionality, probably an opcode.
 *
 *   UnknownInputExc:
 *     Consumed a datum whose type or value could not be constrained at
 *     translation time, because the tracelet has already modified it.
 *     Truncate the tracelet at the preceding instruction, which must
 *     exists because *something* modified something in it.
 */
void Translator::getInputs(Tracelet& t,
                           NormalizedInstruction* ni,
                           int& currentStackOffset,
                           vector<Location>& inputs) { // out
#ifdef DEBUG
  const SrcKey& sk = ni->source;
#endif
  ASSERT(inputs.empty());
  puntUnless(mapContains(instrInfo, ni->op()));
  const InstrInfo& info = instrInfo[ni->op()];
  Operands input = info.in;
  if (input & FuncdRef) {
    // Drive the arState machine; if it is going to throw an input exception,
    // do so here.
    int argNum = ni->imm[0].u_IVA;
    // instrSpToArDelta() returns the delta relative to the sp at the
    // beginning of the instruction, but getReffiness() wants the delta
    // relative to the sp at the beginning of the tracelet, so we adjust
    // by subtracting ni->stackOff
    int entryArDelta = instrSpToArDelta(ni->pc()) - ni->stackOff;
    ni->preppedByRef = t.m_arState.getReffiness(argNum,
                                                entryArDelta,
                                                &t.m_refDeps);
    ASSERT(t.m_arState.m_state != ActRecState::UNKNOWABLE);
    SKTRACE(1, sk, "passing arg%d by %s\n", argNum,
            ni->preppedByRef ? "reference" : "value");
  }
  if (input & MVector) {
    int n = numHiddenStackInputs(ni->pc(), info);
    SKTRACE(2, sk, "Has %d hidden stack inputs\n", n);
    for (int i = 0; i < n; i++) {
      inputs.push_back(Location(Location::Stack, --currentStackOffset));
    }
  }
  if (input & Iter) {
    inputs.push_back(Location(Location::Iter, ni->imm[0].u_IVA));
  }
  if (input & (StackPeek | StackPeekImm)) {
    int depth = getStackPeekDelta(input, ni);
    SKTRACE(1, sk, "getInputs: just-under-top-of-stack: %d\n",
            currentStackOffset - depth);
    inputs.push_back(Location(Location::Stack, currentStackOffset - depth));
  }
  if (input & Stack1) {
    SKTRACE(1, sk, "getInputs: stack1 %d\n", currentStackOffset - 1);
    inputs.push_back(Location(Location::Stack, --currentStackOffset));
    if (input & Stack2) {
      SKTRACE(1, sk, "getInputs: stack2 %d\n", currentStackOffset - 1);
      inputs.push_back(Location(Location::Stack, --currentStackOffset));
      if (input & Stack3) {
        SKTRACE(1, sk, "getInputs: stack3 %d\n", currentStackOffset - 1);
        inputs.push_back(Location(Location::Stack, --currentStackOffset));
      }
    }
  }
  if ((input & AllLocals) &&
      curFunc()->m_numLocals <= Translator::kFewLocals) {
    int n = curFunc()->m_numLocals;
    for (int i = 0; i < n; ++i) {
      inputs.push_back(Location(Location::Local, i));
    }
  }
  SKTRACE(1, sk, "stack args: virtual sfo now %d\n", currentStackOffset);
  TRACE(1,Trace::prettyNode("Inputs", inputs));
}

/*
 * getOutputs --
 *   Builds a vector describing this instruction's outputs. Also
 *   records any write to a value that *might* alias a local.
 *
 * Throws:
 *   TranslationFailedExc:
 *     Unimplemented functionality, probably an opcode.
 */
void Translator::getOutputs(/*inout*/ Tracelet& t,
                            /*inout*/ NormalizedInstruction* ni,
                            /*inout*/ int& currentStackOffset,
                            /*out*/   bool& writeAlias) {
  writeAlias = false;

  const vector<DynLocation*>& inputs = ni->inputs;
  Opcode op = ni->op();

  initInstrInfo();
  assert_not_implemented(instrInfo.find(op) != instrInfo.end());
  const Operands outLocs = instrInfo[op].out;
  const OutTypeConstraints typeInfo = instrInfo[op].type;

  SKTRACE(1, ni->source, "output flavor %d\n", typeInfo);
  if (typeInfo == OutFInputH || typeInfo == OutFInputR ||
      typeInfo == OutVInputH) {
    // Variable number of outputs. If we box the loc we're reading,
    // we need to write out its boxed-ness.
    ASSERT(inputs.size() >= 1);
    const DynLocation* in = inputs[inputs.size() - 1];
    DynLocation* outDynLoc = t.newDynLocation(in->location, in->rtt);
    outDynLoc->location = Location(Location::Stack, currentStackOffset++);
    bool isRef;
    if (typeInfo == OutVInputH) {
      isRef = true;
    } else {
      ASSERT(typeInfo == OutFInputH || typeInfo == OutFInputR);
      isRef = ni->preppedByRef;
    }
    if (isRef) {
      outDynLoc->rtt = in->rtt.box();
      SKTRACE(1, ni->source, "boxed type: %d -> %d\n",
              outDynLoc->rtt.outerType(), outDynLoc->rtt.innerType());
    } else {
      outDynLoc->rtt = outDynLoc->rtt.unbox();
      SKTRACE(1, ni->source, "unboxed type: %d\n",
              outDynLoc->rtt.outerType());
    }
    ASSERT(outDynLoc->location.isStack());
    ni->outStack = outDynLoc;

    if (isRef && in->rtt.outerType() != KindOfVariant &&
        typeInfo != OutFInputR &&
        in->location.isLocal()) {
      // VGetH or FPassH boxing a local
      DynLocation* smashedLocal =
          t.newDynLocation(in->location, outDynLoc->rtt);
      ASSERT(smashedLocal->location.isLocal());
      ni->outLocal = smashedLocal;
    }
    // Other things that might be getting boxed here include globals
    // and array values; since we don't attempt to track these things'
    // types in symbolic execution anyway, we can ignore them.
    return;
  }

  int opnd = None;
  for (int outLocsCopy = (int)outLocs;
       outLocsCopy != (int)None;
       outLocsCopy &= ~opnd) {
    opnd = 1 << (ffs(outLocsCopy) - 1);
    ASSERT(opnd != None && opnd != Stack3);  // no instr produces 3 values
    ASSERT(opnd != FuncdRef);                // reffiness is immutable
    Location loc;
    switch (opnd) {
      // Pseudo-outputs that affect translator state
      case FStack: {
        currentStackOffset += kNumActRecCells;
        if (op == OpFPushFuncD) {
          const Unit& cu = *ni->unit();
          const StringData* name = cu.lookupLitstrId(ni->imm[1].u_SA);
          const Func* f = g_context->lookupFunc(name);
          puntUnless(f);
          if (f->isNameBindingImmutable(&cu)) {
            t.m_arState.pushFuncD(f);
          } else {
            t.m_arState.pushDynFunc();
          }
        } else {
          // Non-deterministic in some way
          t.m_arState.pushDynFunc();
        }
      } continue; // no instr-associated output

      case Local: {
        ASSERT_NOT_IMPLEMENTED(op == OpSetH || op == OpSetOpH ||
                               op == OpIncDecH || op == OpSetM ||
                               op == OpBindH || op == OpUnsetH ||
                               op == OpSetG || op == OpVGetM);
        if (op == OpIncDecH) {
          ASSERT(ni->inputs.size() == 1);
          puntUnless(ni->inputs[0]->rtt.isInt());
          DynLocation* incDecLoc =
              t.newDynLocation(ni->inputs[0]->location,
                               ni->inputs[0]->rtt);
          ASSERT(incDecLoc->location.isLocal());
          ni->outLocal = incDecLoc;
          continue; // Doesn't mutate a loc's types for int. Carry on.
        }
        if (op == OpUnsetH) {
          ASSERT(ni->inputs.size() == 1);
          DynLocation* inHome = ni->inputs[0];
          ASSERT(inHome->location.isLocal());
          RuntimeType newLhsRtt = RuntimeType(KindOfUninit);
          Location inLoc = inHome->location;
          SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                  inLoc.spaceName(), inLoc.offset, newLhsRtt.valueType());
          DynLocation* unsetLoc = t.newDynLocation(inLoc, newLhsRtt);
          ASSERT(unsetLoc->location.isLocal());
          ni->outLocal = unsetLoc;
          continue;
        }
        if (op == OpSetG) {
          writeAlias = true;
          continue;
        }
        if (op == OpSetM || op == OpVGetM) {
          const int kHomeIdx = ni->inputs.size()-1;
          DynLocation* inHome = ni->inputs[kHomeIdx];
          // TODO: This code assumes that the location is LH. We need to
          // figure out how to handle cases where the location is LN or LG
          // or LR. Also, this code is also assuming that the first member
          // is always E or W, which promotes Null to Array. However, the
          // first member could be P, which promotes Null to Object.
          if (!inHome->location.isLocal()) {
            TRACE(2, *inHome);
            // A SetM to something that is not a home (e.g., $this->x). We
            // don't have type info cached for object members at present,
            // so let it slide.
          } else {
            Location inLoc = inHome->location;
            if (inHome->rtt.isString() ||
                inHome->rtt.valueType() == KindOfBoolean) {
              // Strings and bools produce value-dependent results; "" and
              // false upgrade to an array successfully, while other values
              // fail and leave the lhs unmodified.
              DynLocation* baseLoc = t.newDynLocation(inLoc, KindOfInvalid);
              ASSERT(baseLoc->isLocal());
              ni->outLocal = baseLoc;
            } else if (inHome->rtt.valueType() == KindOfUninit ||
                       inHome->rtt.valueType() == KindOfNull) {
              RuntimeType newLhsRtt = inHome->rtt.setValueType(KindOfArray);
              SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                      inLoc.spaceName(), inLoc.offset, newLhsRtt.valueType());
              DynLocation* baseLoc = t.newDynLocation(inLoc, newLhsRtt);
              ASSERT(baseLoc->location.isLocal());
              ni->outLocal = baseLoc;
            }
            // A SetM in pseudo-main might alias a home whose type we're
            // remembering:
            //
            //   $GLOBALS['a'] = 123; // $a :: Int
            //
            // and more deviously:
            //
            //   $loc['b'][17] = $GLOBALS; $x = 'b'; $y = 17;
            //   $loc[$x][$y]['a'] = 123; // $a :: Int
            //
            // XXX: analogous garbage needed for OpSetOpM.
            writeAlias = true;
          }
          continue;
        }
        if (op == OpSetOpH) {
          const int kHomeIdx = 1;
          DynLocation* inHome = ni->inputs[kHomeIdx];
          puntUnless(inHome->location.isLocal());
          DynLocation* dl = t.newDynLocation();
          dl->location = inHome->location;
          dl->rtt = setOpOutputType(ni, ni->inputs);
          if (inHome->isVariant()) {
            dl->rtt = dl->rtt.box();
          }
          SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                  inHome->location.spaceName(), inHome->location.offset,
                  dl->rtt.valueType());
          ASSERT(dl->location.isLocal());
          ni->outLocal = dl;
          continue;
        }
        ASSERT(ni->inputs.size() == 2);
        const int kValIdx  = 0;
        const int kHomeIdx = 1;
        DynLocation* inHome = ni->inputs[kHomeIdx];
        DynLocation* inVal  = ni->inputs[kValIdx];
        Location inLoc = inHome->location;
        // Variant RHS possible in a few cases.
        ASSERT((op == OpBindH) == (inVal->rtt.outerType() == KindOfVariant));
        ASSERT(!inVal->location.isLocal());
        ASSERT(inHome->location.isLocal());
        RuntimeType newLhsRtt = op == OpBindH ?
          inVal->rtt :
          inHome->rtt.setValueType(inVal->rtt.outerType());
        if (inHome->rtt.outerType() == KindOfVariant) {
          ASSERT(newLhsRtt.outerType() == KindOfVariant);
          if (op != OpBindH) {
            writeAlias = true;
          }
        } else {
          ASSERT(op == OpBindH || newLhsRtt.outerType() != KindOfVariant);
        }
        // SetH on an object can cause the destructor to be called,
        // which can modify any variable in pseudomain, or
        // any variant inside a function. The destructor may be
        // called directly or indirectly (through a SetH on an array
        // or variant). rtt->valueType() checks for innerType if outerType
        // is a variant.
        if (op == OpSetH && (inHome->rtt.valueType() == KindOfObject
                         ||  inHome->rtt.valueType() == KindOfArray)) {
          writeAlias = true;
        }
        SKTRACE(2, ni->source, "(%s, %d) <- type %d\n",
                inLoc.spaceName(), inLoc.offset, inVal->rtt.valueType());
        DynLocation* outLhsLoc = t.newDynLocation(inLoc, newLhsRtt);
        ASSERT(outLhsLoc->location.isLocal());
        ni->outLocal = outLhsLoc;
      } continue; // already pushed an output for the local

      case Stack1:
      case Stack2:
        loc = Location(Location::Stack, currentStackOffset++);
        break;
      case StackPeek:
      case StackPeekImm: {
        int depth = getStackPeekDelta(opnd, ni);
        loc = Location(Location::Stack, currentStackOffset - depth);
      } break;
      default:
        not_reached();
    }
    DynLocation* dl = t.newDynLocation();
    dl->location = loc;
    dl->rtt = getDynLocType(inputs, t, op, ni, (Operands)opnd,
                            typeInfo, dl);
    if (dl->rtt.isHome()) {
      SKTRACE(2, ni->source, "recording output home #(%s, %d)\n",
              dl->location.spaceName(), dl->location.offset);
    } else {
      SKTRACE(2, ni->source, "recording output t(%d->%d) #(%s, %d)\n",
              dl->rtt.outerType(), dl->rtt.innerType(),
              dl->location.spaceName(), dl->location.offset);
    }
    ASSERT(dl->location.isStack());
    ni->outStack = dl;
  }
}

void
Translator::findImmable(ImmStack &stack,
                        NormalizedInstruction* ni) {
  switch (ni->op()) {
  case OpInt:
    stack.pushInt(getImm(ni->pc(), 0).u_I64A);
    return;

  case OpString:
    stack.pushLitstr(getImm(ni->pc(), 0).u_SA);
    return;

  // For binary ops we assume that only one of the two is an immediate
  //   because if both were immediates then hopefully the peephole
  //   optimizer already optimized away this instruction. However, if a
  //   binary op has two immediates, we won't generate incorrect code:
  //   instead it will merely be suboptimal.

  // we can only handle an immediate if it's the second immediate
  case OpAdd:
  case OpSub:
    if (stack.isInt(0)) {
      // the 0 above is stack depth, but the 1 below is argument order
      SKTRACE(1, ni->source, "marking for immediate elision\n");
      ni->constImmPos = 1;
      ni->constImm.u_I64A = stack.get(0).i64a;
      // We don't currently remove the OpInt instruction that produced
      // this integer. We should update the translator to correctly support
      // removing instructions from the tracelet.
    }
    break;

  case OpFPassM:
  case OpCGetM:
  case OpIssetM: {
    // If this is "<VecInstr>M <... E>"
    ImmVector* iv = ni->immVecPtr;
    ASSERT(iv);
    if (iv->len >= 2 && iv->get(iv->len - 1) == ME) {
      // If the operand is a statically known string and it's not strictly an
      // integer, we can call into array-access helper functions that don't
      // bother with the integer check.
      if (stack.isLitstr(0)) {
        Id str_id = stack.get(0).sa;
        StringData* str = curUnit()->lookupLitstrId(str_id);
        int64 lval;
        if (LIKELY(!str->isStrictlyInteger(lval))) {
          ni->constImmPos = 1;
          ni->constImm.u_SA = str_id;
        }
      }
    }
  } break;

  default: ;
  }

  stack.processOpcode(ni->pc());
}

void
Translator::requestResetHighLevelTranslator() {
  if (dbgTranslateCoin) {
    dbgTranslateCoin->reset();
  }
}

bool DynLocation::canBeAliased() const {
  return isValue() &&
    ((Translator::liveFrameIsPseudoMain() && isLocal()) ||
     isVariant());
}

DynLocation* TraceletContext::recordRead(const Location& l) {
  DynLocation* dl;
  if (!mapGet(m_currentMap, l, &dl)) {
    // We should never try to read a location that has been deleted
    ASSERT(!mapContains(m_deletedSet, l));
    // If the given location was not in m_currentMap, then it shouldn't
    // be in m_changeSet either
    ASSERT(!mapContains(m_changeSet, l));
    // Get the live type; if it's a home, don't bother getting the
    // type of it's referent yet
    RuntimeType rtt = Translator::liveType(l, *curUnit());
    ASSERT(rtt.isIter() || rtt.isHome() || !rtt.isVagueValue());
    // Allocate a new DynLocation to represent this and store it in the
    // current map.
    dl = m_t->newDynLocation(l, rtt);
    if (m_aliasTaint && dl->canBeAliased()) {
      dl->rtt = rtt.setValueType(KindOfInvalid);
    }
    m_currentMap[l] = dl;
    // Record that we depend on the live type of the specified location
    // as well (and remember what the live type was)
    m_dependencies[l] = dl;
  }
  if (Trace::enabled) {
    string tstr = dl->pretty();
    TRACE(2, "diff: %s\n", tstr.c_str());
  }
  return dl;
}

void TraceletContext::recordWrite(DynLocation* dl,
                                  NormalizedInstruction* source) {
  dl->source = source;
  m_currentMap[dl->location] = dl;
  m_changeSet.insert(dl->location);
  m_deletedSet.erase(dl->location);
}

void TraceletContext::recordDelete(const Location& l) {
  // We should not be trying to delete the rtt of location that is
  // not in m_currentMap
  ASSERT(m_currentMap.find(l) != m_currentMap.end());
  ASSERT(m_deletedSet.find(l) == m_deletedSet.end());
  m_currentMap.erase(l);
  m_changeSet.erase(l);
  m_deletedSet.insert(l);
}

void TraceletContext::aliasTaint() {
  m_aliasTaint = true;
  for (ChangeMap::iterator it = m_currentMap.begin();
       it != m_currentMap.end(); ++it) {
    DynLocation* dl = it->second;
    if (dl->canBeAliased()) {
      TRACE(1, "(%s, %d) <- inner type invalidated\n",
            it->first.spaceName(), it->first.offset);
      RuntimeType newRtt = dl->rtt.setValueType(KindOfInvalid);
      it->second = m_t->newDynLocation(dl->location, newRtt);
    }
  }
}

/*
 * op --
 * pc --
 * unit --
 * offset --
 *
 *   Helpers for recovering context of this instruction.
 */
Opcode NormalizedInstruction::op() const {
  return *pc();
}

PC NormalizedInstruction::pc() const {
  return unit()->at(source.offset());
}

const Unit* NormalizedInstruction::unit() const {
  return m_unit;
}

Offset NormalizedInstruction::offset() const {
  return source.offset();
}

/*
 * analyze --
 *
 *   Given a sequence of bytecodes, return our tracelet IR.
 */
Tracelet
Translator::analyze(const SrcKey *csk) {
  Tracelet t;
  t.m_sk = *csk;

  TraceletContext tas(&t);
  ImmStack immStack;
  stackFrameOffset = 0;
  int oldStackFrameOffset = 0;

  // numOpcodes counts the original number of opcodes in a tracelet
  // before the translator does any optimization
  t.m_numOpcodes = 0;

  /**
   * The purposes of this analysis is to determine:
   *  1. Pre-conditions: What locations get read before they get written to:
   *     we will need typechecks for these and we will want to load them into
   *     registers. (m_dependencies)
   *  2. Post-conditions: the locations that have been written to and are
   *     still live at the end of the tracelet. We need to allocate registers
   *     of these and we need to spill them at the end of the tracelet.
   *     (m_changes)
   *  3. Determine the runtime types for each instruction's input locations
   *     and output locations.
   *
   * The analysis works by doing a single pass over the instructions. It
   * effectively simulates the execution of each instruction, updating its
   * knowledge about types as it goes.
   *
   * The TraceletContext class is used to keep track of the current state of
   * the world. Initially it is empty, and when the inputs for the first
   * instruction are analyzed we call recordRead(). The recordRead() function
   * in turn inspects the live types of the inputs and adds them to the type
   * map. This serves two purposes: (1) it figures out what typechecks this
   * tracelet needs; and (2) it guarantees that the code we generate will
   * satisfy the live types that are about to be passed in.
   *
   * Over time the TraceletContext's type map will change. However, we need to
   * record what the types _were_ right before and right after a given
   * instruction executes. This is where the NormalizedInstruction class comes
   * in. We store the RuntimeTypes from the TraceletContext right before an
   * instruction executes into the NormalizedInstruction's 'inputs' field, and
   * we store the RuntimeTypes from the TraceletContext right after the
   * instruction executes into the 'outputs' field.
   */

  SrcKey sk = *csk; // copy for local use
  const Unit *unit = curUnit();
  for (;; sk.advance(unit)) {
    NormalizedInstruction* ni = t.newNormalizedInstruction();
    ni->source = sk;
    ni->stackOff = stackFrameOffset;
    ni->funcd = t.m_arState.m_topFunc;
    ni->m_unit = unit;
    ni->preppedByRef = false;
    bool breakBB = opcodeBreaksBB(ni->op());
    ni->breaksBB = breakBB;

    if (t.m_analysisFailed) {
      t.m_instrStream.append(ni);
      ++t.m_numOpcodes;
      /*
       * If we were unable to analyze the instruction stream, continue
       * but only so we can frame the boundaries of the BB.
       */
      if (breakBB) {
        break;
      }
      continue;
    }
    oldStackFrameOffset = stackFrameOffset;
    for (int i = 0; i < numImmediates(ni->op()); i++) {
      ni->imm[i] = getImm(ni->pc(), i);
    }
    ni->immVecPtr = getImmVector(ni->pc());

    // Use the basic block analyzer to follow the flow of immediate values.
    findImmable(immStack, ni);

    SKTRACE(1, sk, "stack args: virtual sfo now %d\n", stackFrameOffset);

    /*
     * Note that there is some asymmetry with how RuntimeType is used for
     * inputs vs. how RuntimeType is used for outputs. For inputs, the
     * RuntimeType for a Home will always have the type information (if its
     * known) for the local that the Home refers to. Local locations will never
     * appear direct in the inputs, because they are effectively covered by the
     * corresponding Homes being consumed from the stack.
     *
     * For outputs, the RuntimeType for a Home will never have the type
     * information for the local that is refers to. This is because we don't
     * really care about what the local's type is when a Home is produced.
     * Also, if an instruction writes to a local, the local will directly
     * appear in the outputs.
     */

    // Translation could fail entirely (because of an unknown opcode), or
    // encounter an input that cannot be computed.
    try {
      vector<Location> inputLocs;
      getInputs(t, ni, stackFrameOffset, inputLocs);
      for (unsigned int i = 0; i < inputLocs.size(); i++) {
        SKTRACE(2, sk, "typing input %d\n", i);
        const Location& l = inputLocs[i];
        DynLocation* dl = tas.recordRead(l);
        DynLocation* homeDl = NULL;
        // If dl is a home, go lookup the type of its referent
        if (dl->rtt.isHome()) {
          homeDl = dl;
          dl = tas.recordRead(dl->rtt.homeLocation());
        }
        ASSERT(!dl->rtt.isHome());
        ASSERT(homeDl == NULL || homeDl->rtt.isHome());
        const RuntimeType& rtt = dl->rtt;
        if (rtt.isVagueValue()) {
          // Consumed a "poisoned" output: e.g., result of an array
          // deref.
          throwUnknownInput();
        }
        if (rtt.isValue() && rtt.isVariant() &&
            rtt.innerType() == KindOfInvalid &&
            ni->op() != OpBindH &&
            ni->op() != OpPopV) {
          throwUnknownInput();
        }
        ni->inputs.push_back(dl);
        if (homeDl != NULL) {
          ni->inputHomes.push_back(homeDl);
        }
      }
    } catch (TranslationFailedExc& tfe) {
      SKTRACE(1, sk, "Translator fail: %s:%d\n", tfe.m_file, tfe.m_line);
      t.m_analysisFailed = true;
      t.m_instrStream.append(ni);
      ++t.m_numOpcodes;
      if (breakBB) {
        break;
      }
      continue;
    } catch (UnknownInputExc& uie) {
      // Subtle: if this instruction consumes an unknown runtime type,
      // break the BB on the *previous* instruction. We know that a
      // previous instruction exists, because the KindOfInvalid must
      // have come from somewhere.
      assert(t.m_instrStream.last);
      TRACE(2, "Consumed unknown input (%s:%d); breaking BB at predecessor\n",
            uie.m_file, uie.m_line);
      goto breakBB;
    }

    SKTRACE(2, sk, "stack args: virtual sfo now %d\n", stackFrameOffset);

    bool writeAlias; // initialized by reference.

    try {
      getOutputs(t, ni, stackFrameOffset, writeAlias);
    } catch(TranslationFailedExc& tfe) {
      SKTRACE(1, sk, "Translator getOutputs fail: %s:%d\n",
              tfe.m_file, tfe.m_line);
      t.m_analysisFailed = true;
      t.m_instrStream.append(ni);
      ++t.m_numOpcodes;
      if (breakBB) {
        break;
      }
      continue;
    }

    // If the instruction could have potentially changed the inner
    // type of a var, then we invalidate all our assumptions about
    // inner types.
    // XXX We can do better than this. Here are some ideas:
    //   1) If the old inner type that was overwritten was type X,
    //      then we only need to invalidate other vars whose inner
    //      type was X.
    //   2) If the new inner type that was written was type Y, then
    //      we only need to invalidate other vars whose inner type
    //      was something different than Y.
    //   3) If the old inner type and the new inner type are the
    //      same, no invalidation is necessary.
    //   4) We could augment TraceletContext to do alias analysis on
    //      vars. When needed, we would add checks at the top of the
    //      tracelet to ensure that two incoming vars to not point to
    //      the same inner cell.
    if (writeAlias) {
      tas.aliasTaint();
    }

    DynLocation* outputs[] = { ni->outStack, ni->outLocal };
    for (ssize_t i = 0; i < 2; ++i) {
      if (outputs[i]) {
        DynLocation* o = outputs[i];
        if (o->rtt.isHome()) {
          SKTRACE(2, sk, "inserting output home #(%s, %d)\n",
                  o->location.spaceName(), o->location.offset);
        } else {
          SKTRACE(2, sk, "inserting output t(%d->%d) #(%s, %d)\n",
                  o->rtt.outerType(), o->rtt.innerType(),
                  o->location.spaceName(), o->location.offset);
        }
        tas.recordWrite(o, ni);
      }
    }

    SKTRACE(1, sk, "stack args: virtual sfo now %d\n", stackFrameOffset);

    // This assert failing means that your instruction has an
    // inconsistent row in the InstrInfo table; the stackDelta doesn't
    // agree with the inputs and outputs.
    ASSERT(getStackDelta(*ni) == (stackFrameOffset - oldStackFrameOffset));
    // If this instruction decreased the depth of the stack, mark the
    // appropriate stack locations as "dead"
    if (stackFrameOffset < oldStackFrameOffset) {
      for (int i = stackFrameOffset; i < oldStackFrameOffset; ++i) {
        Location loc(Location::Stack, i);
        tas.recordDelete(loc);
        ni->deadLocs.push_back(loc);
      }
    }

    t.m_stackChange += getStackDelta(*ni);

    t.m_instrStream.append(ni);
    ++t.m_numOpcodes;

    analyzeInstr(t, *ni);

    if (debug) {
      // The interpreter has lots of nice sanity assertions in debug mode
      // that the translator doesn't exercise. As a cross-check on the
      // translator's correctness, this is a debug-only facility for
      // sending a random selection of instructions through the
      // interpreter.
      //
      // Set stress_txInterpPct to a value between 1 and 100. If you want
      // to reproduce a failing case, look for the seed in the log and set
      // stress_txInterpSeed accordingly.
      if (!dbgTranslateCoin) {
        dbgTranslateCoin = new BiasedCoin(Trace::stress_txInterpPct,
                                          Trace::stress_txInterpSeed);
        TRACE(1, "BiasedCoin(stress_txInterpPct,stress_txInterpSeed): "
              "pct %f, seed %d\n",
              dbgTranslateCoin->getPercent(), dbgTranslateCoin->getSeed());
      }
      ASSERT(dbgTranslateCoin);
      if (dbgTranslateCoin->flip()) {
        SKTRACE(3, ni->source, "stress interp\n");
        ni->txSupported = false;
      }

      if ((ni->op()) > Trace::moduleLevel(Trace::tmp0) &&
          (ni->op()) < Trace::moduleLevel(Trace::tmp1)) {
        ni->txSupported = false;
      }
    }

    Op txOpBisectLowOp  = (Op)moduleLevel(Trace::txOpBisectLow),
       txOpBisectHighOp = (Op)moduleLevel(Trace::txOpBisectHigh);
    if (txOpBisectLowOp  > OpLowInvalid &&
        txOpBisectHighOp > OpLowInvalid &&
        txOpBisectHighOp < OpHighInvalid) {
      // If the user specified an operation bisection interval [Low, High]
      // that is strictly included in (OpLowInvalid, OpHighInvalid), then
      // only support the operations in that interval. Since the default
      // value of moduleLevel is 0 and OpLowInvalid is also 0, this ensures
      // that bisection is disabled by default.
      CT_ASSERT(OpLowInvalid >= 0);
      if (ni->op() < txOpBisectLowOp ||
          ni->op() > txOpBisectHighOp)
        ni->txSupported = false;
    }
    // If this instruction is supported by the translator and it consumes
    // homes from the stack, remove any corresponding Loc instructions from
    // the instruction stream
    if (ni->txSupported) {
      for (unsigned i = 0; i < ni->inputHomes.size(); ++i) {
        DynLocation* dl = ni->inputHomes[i];
        NormalizedInstruction* locInstr = dl->source;
        if (locInstr && locInstr->txSupported) {
          ASSERT(locInstr->deadLocs.size() == 0);
          t.m_instrStream.remove(locInstr);
        }
      }

      // If this is a Pop instruction the previous instruction is SetH, SetOpH,
      // or BindH, we mark the previous instruction so that it takes card of
      // discarding the right hand side
      if ((ni->op() == OpPopC || ni->op() == OpPopV) &&
          ni->prev != NULL) {
        NormalizedInstruction* prevInstr = ni->prev;
        Opcode prevOp = prevInstr->op();
        if (prevInstr->txSupported &&
            (prevOp == OpSetH || prevOp == OpBindH ||
             prevOp == OpIncDecH || prevOp == OpPrint)) {
          ASSERT(prevInstr->outStack);
          prevInstr->outStack = NULL;
          for (unsigned i = 0; i < ni->deadLocs.size(); ++i) {
            prevInstr->deadLocs.push_back(ni->deadLocs[i]);
          }
          ni->deadLocs.clear();
          t.m_instrStream.remove(ni);
        }
      }
    }

    if (breakBB) {
      SKTRACE(1, sk, "BB broken\n");
      sk.advance(unit);
      goto breakBB;
    }
  }
breakBB:
  // Mark the last instruction appropriately
  ASSERT(t.m_instrStream.last);
  t.m_instrStream.last->breaksBB = true;
  // Populate t.m_changes, t.intermediates, t.m_dependencies
  t.m_dependencies = tas.m_dependencies;
  t.m_changes.clear();
  LocationSet::iterator it = tas.m_changeSet.begin();
  for (; it != tas.m_changeSet.end(); ++it) {
    t.m_changes[*it] = tas.m_currentMap[*it];
  }

  TRACE(1, "Tracelet done: stack delta %d\n", t.m_stackChange);
  return t;
}

Translator::Translator() {
  initInstrInfo();
}

Translator*
Translator::Get() {
  return TranslatorX64::Get();
}

ActRecState::ActRecState() :
  m_state(GUESSABLE), m_topFunc(NULL),
  m_entryArDelta(InvalidEntryArDelta) {
}

void
ActRecState::pushFuncD(const Func* func) {
  TRACE(2, "ActRecState: pushStatic func %p(%s)\n", func, func->m_name->data());
  m_state = KNOWN;
  m_topFunc = func;
}

void
ActRecState::pushDynFunc(void) {
  TRACE(2, "ActRecState: pushDynFunc, preparing to give up on reffiness\n");
  m_state = UNKNOWABLE;
  m_topFunc = NULL;
}

/**
 * getReffiness() returns true if the parameter specified by argNum is pass
 * by reference, otherwise it returns false. This function may also throw an
 * UnknownInputException if the reffiness cannot be determined.
 *
 * Note that the 'entryArDelta' parameter specifies the delta between sp at
 * the beginning of the tracelet and ar.
 */
bool
ActRecState::getReffiness(int argNum, int entryArDelta, RefDeps* outRefDeps) {
  TRACE(2, "ActRecState: reffiness in state %d, reading arg %d\n",
        m_state, argNum);
  const Func* topFunc = m_topFunc;
  switch(m_state) {
    case KNOWN: break;
    case GUESSABLE: {
      ASSERT(topFunc == NULL);
      // In principle, this is burning assumptions derived from the Func into
      // the TC. However, we do not burn in the Func* itself, and we record our
      // assumptions about arg reffiness as tracelet guards (that's where the
      // information in outRefDeps will end up). Therefore, if these conditions
      // change in the future, we'll correctly end up retranslating.
      m_entryArDelta = entryArDelta;
      const ActRec* ar = arFromSpOffset((ActRec*)vmsp(), m_entryArDelta);
      topFunc = ar->m_func;
      TRACE(2, "ActRecState: guessing reffiness %p(%s)\n",
            ar->m_func, ar->m_func->m_name->data());
      break;
    };
    case UNKNOWABLE:
      TRACE(2, "ActRecState: unknowable, throwing in the towel\n");
      throwUnknownInput();
      not_reached();
  };
  ASSERT(topFunc);
  bool retval = topFunc->byRef(argNum);

  if (outRefDeps != NULL && m_state == GUESSABLE) {
    TRACE(2, "ActRecState: guessing arg%d -> %d\n", argNum, retval);
    outRefDeps->addDep(argNum, retval);
  }
  return retval;
}

} } }
