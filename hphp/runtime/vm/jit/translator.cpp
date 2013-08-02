/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/translator.h"

// Translator front-end: parse instruction stream into basic blocks, decode
// and normalize instructions. Propagate run-time type info to instructions
// to annotate their inputs and outputs with types.
#include <cinttypes>
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>

#include <vector>
#include <string>

#include "folly/Conv.h"

#include "hphp/util/trace.h"
#include "hphp/util/biased_coin.h"
#include "hphp/util/map_walker.h"
#include "hphp/runtime/base/file_repository.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir-factory.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/tracelet.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/pendq.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {
namespace Transl {

using namespace HPHP;
using HPHP::JIT::Type;
using HPHP::JIT::HhbcTranslator;

TRACE_SET_MOD(trans)

static __thread BiasedCoin *dbgTranslateCoin;
Translator* transl;
Lease Translator::s_writeLease;

struct TraceletContext {
  TraceletContext() = delete;

  TraceletContext(Tracelet* t, const TypeMap& initialTypes)
    : m_t(t)
    , m_numJmps(0)
    , m_aliasTaint(false)
    , m_varEnvTaint(false)
  {
    for (auto& kv : initialTypes) {
      TRACE(1, "%s\n",
            Trace::prettyNode("InitialType", kv.first, kv.second).c_str());
      m_currentMap[kv.first] = t->newDynLocation(kv.first, kv.second);
    }
  }

  Tracelet*   m_t;
  ChangeMap   m_currentMap;
  DepMap      m_dependencies;
  DepMap      m_resolvedDeps; // dependencies resolved by static analysis
  LocationSet m_changeSet;
  LocationSet m_deletedSet;
  int         m_numJmps;
  bool        m_aliasTaint;
  bool        m_varEnvTaint;

  RuntimeType currentType(const Location& l) const;
  DynLocation* recordRead(const InputInfo& l, bool useHHIR,
                          DataType staticType = KindOfInvalid);
  void recordWrite(DynLocation* dl);
  void recordDelete(const Location& l);
  void recordJmp();
  void aliasTaint();
  void varEnvTaint();
};

void InstrStream::append(NormalizedInstruction* ni) {
  if (last) {
    assert(first);
    last->next = ni;
    ni->prev = last;
    ni->next = nullptr;
    last = ni;
    return;
  }
  assert(!first);
  first = ni;
  last = ni;
  ni->prev = nullptr;
  ni->next = nullptr;
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
  ni->prev = nullptr;
  ni->next = nullptr;
}

/*
 * locPhysicalOffset --
 *
 *   Return offset, in cells, of this location from its base
 *   pointer. It needs a function descriptor to see how many locals
 *   to skip for iterators; if the current frame pointer is not the context
 *   you're looking for, be sure to pass in a non-default f.
 */
int locPhysicalOffset(Location l, const Func* f) {
  f = f ? f : liveFunc();
  assert_not_implemented(l.space == Location::Stack ||
                         l.space == Location::Local ||
                         l.space == Location::Iter);
  int localsToSkip = l.space == Location::Iter ? f->numLocals() : 0;
  int iterInflator = l.space == Location::Iter ? kNumIterCells : 1;
  return -((l.offset + 1) * iterInflator + localsToSkip);
}

RuntimeType Translator::liveType(Location l,
                                 const Unit& u,
                                 bool specialize) {
  Cell *outer;
  switch (l.space) {
    case Location::Stack:
      // Stack accesses must be to addresses pushed before
      // translation time; if they are to addresses pushed after,
      // they should be hitting in the changemap.
      assert(locPhysicalOffset(l) >= 0);
      // fallthru
    case Location::Local: {
      Cell *base;
      int offset = locPhysicalOffset(l);
      base    = l.space == Location::Stack ? vmsp() : vmfp();
      outer = &base[offset];
    } break;
    case Location::Iter: {
      const Iter *it = frame_iter(liveFrame(), l.offset);
      TRACE(1, "Iter input: fp %p, iter %p, offset %" PRId64 "\n", vmfp(),
            it, l.offset);
      return RuntimeType(it);
    } break;
    case Location::Litstr: {
      return RuntimeType(u.lookupLitstrId(l.offset));
    } break;
    case Location::Litint: {
      return RuntimeType(l.offset);
    } break;
    case Location::This: {
      return outThisObjectType();
    } break;
    default: {
      not_reached();
    }
  }
  assert(IS_REAL_TYPE(outer->m_type));
  return liveType(outer, l, specialize);
}

RuntimeType
Translator::liveType(const Cell* outer, const Location& l, bool specialize) {
  always_assert(analysisDepth() == 0);

  if (!outer) {
    // An undefined global; starts out as a variant null
    return RuntimeType(KindOfRef, KindOfNull);
  }
  DataType outerType = (DataType)outer->m_type;
  assert(IS_REAL_TYPE(outerType));
  DataType valueType = outerType;
  DataType innerType = KindOfInvalid;
  const Cell* valCell = outer;
  if (outerType == KindOfRef) {
    // Variant. Pick up the inner type, too.
    valCell = outer->m_data.pref->tv();
    innerType = valCell->m_type;
    assert(IS_REAL_TYPE(innerType));
    valueType = innerType;
    assert(innerType != KindOfRef);
    FTRACE(2, "liveType {}: Var -> {}\n", l.pretty(), tname(innerType));
  } else {
    FTRACE(2, "liveType {}: {}\n", l.pretty(), tname(outerType));
  }
  const Class *klass = nullptr;
  if (valueType == KindOfObject) {
    // Only infer the class if specialization requested
    if (specialize) {
      klass = valCell->m_data.pobj->getVMClass();
    }
  }
  RuntimeType retval = RuntimeType(outerType, innerType);
  if (klass != nullptr) {
    retval = retval.setKnownClass(klass);
  }
  return retval;
}

RuntimeType Translator::outThisObjectType() {
  /*
   * Use the current method's context class (ctx) as a constraint.
   * For instance methods, if $this is non-null, we are guaranteed
   * that $this is an instance of ctx or a class derived from
   * ctx. Zend allows this assumption to be violated but we have
   * deliberately chosen to diverge from them here.
   *
   * Note that if analysisDepth() != 0 we'll have !hasThis() here,
   * because our fake ActRec has no $this, but we'll still return the
   * correct object type because arGetContextClass() looks at
   * ar->m_func's class for methods.
   */
  const Class *ctx = liveFunc()->isMethod() ?
    arGetContextClass(liveFrame()) : nullptr;
  if (ctx) {
    assert(!liveFrame()->hasThis() ||
           liveFrame()->getThis()->getVMClass()->classof(ctx));
    TRACE(2, "OutThisObject: derived from Class \"%s\"\n",
          ctx->name()->data());
    return RuntimeType(KindOfObject, KindOfInvalid, ctx);
  }
  return RuntimeType(KindOfObject, KindOfInvalid);
}

bool Translator::liveFrameIsPseudoMain() {
  ActRec* ar = (ActRec*)vmfp();
  return ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope();
}

Location
Translator::tvToLocation(const TypedValue* tv, const TypedValue* frame) {
  const Cell *arg0 = frame + locPhysicalOffset(Location(Location::Local, 0));
  // Physical stack offsets grow downwards from the frame pointer. See
  // locPhysicalOffset.
  int offset = -(tv - arg0);
  assert(offset >= 0);
  assert(offset < ((ActRec*)frame)->m_func->numLocals());
  TRACE(2, "tvToLocation: %p -> L:%d\n", tv, offset);
  return Location(Location::Local, offset);
}

static int64_t typeToMask(DataType t) {
  return (t == KindOfInvalid) ? 1 : (1 << (1 + getDataTypeIndex(t)));
}

struct InferenceRule {
  int64_t mask;
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

#define TYPE_MASK(name) \
  static const int64_t name ## Mask = typeToMask(KindOf ## name);
TYPE_MASK(Invalid);
TYPE_MASK(Uninit);
TYPE_MASK(Null);
TYPE_MASK(Boolean);
static const int64_t IntMask = typeToMask(KindOfInt64);
TYPE_MASK(Double);
static const int64_t StringMask = typeToMask(KindOfString) |
                                typeToMask(KindOfStaticString);
TYPE_MASK(Array);
TYPE_MASK(Object);

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
 * Returns the type of the output of a bitwise operator on the two
 * DynLocs. The only case that doesn't result in KindOfInt64 is String
 * op String.
 */
static const InferenceRule BitOpRules[] = {
  { UninitMask | NullMask | BooleanMask |
    IntMask | DoubleMask | ArrayMask | ObjectMask,
    KindOfInt64 },
  { StringMask, KindOfString },
  { 0, KindOfInvalid },
};

static RuntimeType bitOpType(DynLocation* a, DynLocation* b) {
  vector<DynLocation*> ins;
  ins.push_back(a);
  if (b) ins.push_back(b);
  return RuntimeType(inferType(BitOpRules, ins));
}

static uint32_t m_w = 1;    /* must not be zero */
static uint32_t m_z = 1;    /* must not be zero */

static uint32_t get_random()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

static const int kTooPolyPred = 2;
static const int kTooPolyRet = 6;

bool
isNormalPropertyAccess(const NormalizedInstruction& i,
                       int propInput,
                       int objInput) {
  const LocationCode lcode = i.immVec.locationCode();
  return
    i.immVecM.size() == 1 &&
    (lcode == LC || lcode == LL || lcode == LR || lcode == LH) &&
    mcodeMaybePropName(i.immVecM[0]) &&
    i.inputs[propInput]->isString() &&
    i.inputs[objInput]->valueType() == KindOfObject;
}

bool
mInstrHasUnknownOffsets(const NormalizedInstruction& ni, Class* context) {
  const MInstrInfo& mii = getMInstrInfo(ni.mInstrOp());
  unsigned mi = 0;
  unsigned ii = mii.valCount() + 1;
  for (; mi < ni.immVecM.size(); ++mi) {
    MemberCode mc = ni.immVecM[mi];
    if (mcodeMaybePropName(mc)) {
      const Class* cls = nullptr;
      if (getPropertyOffset(ni, context, cls, mii, mi, ii).offset == -1) {
        return true;
      }
      ++ii;
    } else {
      return true;
    }
  }

  return false;
}

PropInfo getPropertyOffset(const NormalizedInstruction& ni,
                           Class* ctx,
                           const Class*& baseClass,
                           const MInstrInfo& mii,
                           unsigned mInd, unsigned iInd) {
  if (mInd == 0) {
    auto const baseIndex = mii.valCount();
    baseClass = ni.inputs[baseIndex]->rtt.isObject()
      ? ni.inputs[baseIndex]->rtt.valueClass()
      : nullptr;
  } else {
    baseClass = ni.immVecClasses[mInd - 1];
  }
  if (!baseClass) return PropInfo();

  if (!ni.inputs[iInd]->rtt.isString()) {
    return PropInfo();
  }
  auto* const name = ni.inputs[iInd]->rtt.valueString();
  if (!name) return PropInfo();

  bool accessible;
  // If we are not in repo-authoriative mode, we need to check that
  // baseClass cannot change in between requests
  if (!RuntimeOption::RepoAuthoritative ||
      !(baseClass->preClass()->attrs() & AttrUnique)) {
    if (!ctx) return PropInfo();
    if (!ctx->classof(baseClass)) {
      if (baseClass->classof(ctx)) {
        // baseClass can change on us in between requests, but since
        // ctx is an ancestor of baseClass we can make the weaker
        // assumption that the object is an instance of ctx
        baseClass = ctx;
      } else {
        // baseClass can change on us in between requests and it is
        // not related to ctx, so bail out
        return PropInfo();
      }
    }
  }
  // Lookup the index of the property based on ctx and baseClass
  Slot idx = baseClass->getDeclPropIndex(ctx, name, accessible);
  // If we couldn't find a property that is accessible in the current
  // context, bail out
  if (idx == kInvalidSlot || !accessible) {
    return PropInfo();
  }
  // If it's a declared property we're good to go: even if a subclass
  // redefines an accessible property with the same name it's guaranteed
  // to be at the same offset
  return PropInfo(
    baseClass->declPropOffset(idx),
    baseClass->declPropHphpcType(idx)
  );
}

PropInfo getFinalPropertyOffset(const NormalizedInstruction& ni,
                                Class* context,
                                const MInstrInfo& mii) {
  unsigned mInd = ni.immVecM.size() - 1;
  unsigned iInd = mii.valCount() + 1 + mInd;

  const Class* cls = nullptr;
  return getPropertyOffset(ni, context, cls, mii, mInd, iInd);
}

static std::pair<DataType,double>
predictMVec(const NormalizedInstruction* ni) {
  auto info = getFinalPropertyOffset(*ni,
                                     ni->func()->cls(),
                                     getMInstrInfo(ni->mInstrOp()));
  if (info.offset != -1 && info.hphpcType != KindOfInvalid) {
    FTRACE(1, "prediction for CGetM prop: {}, hphpc\n",
           int(info.hphpcType));
    return std::make_pair(info.hphpcType, 1.0);
  }

  auto& immVec = ni->immVec;
  StringData* name;
  MemberCode mc;
  if (immVec.decodeLastMember(ni->m_unit, name, mc)) {
    auto pred = predictType(TypeProfileKey(mc, name));
    TRACE(1, "prediction for CGetM %s named %s: %d, %f\n",
          mc == MET ? "elt" : "prop",
          name->data(),
          pred.first,
          pred.second);
    return pred;
  }

  return std::make_pair(KindOfInvalid, 0.0);
}

/*
 * predictOutputs --
 *
 *   Provide a best guess for the output type of this instruction.
 */
static DataType
predictOutputs(SrcKey startSk,
               const NormalizedInstruction* ni) {
  if (!RuntimeOption::EvalJitTypePrediction) return KindOfInvalid;

  // In JitPGO mode, disable type prediction to avoid side exits
  if (RuntimeOption::EvalJitPGO) return KindOfInvalid;

  if (RuntimeOption::EvalJitStressTypePredPercent &&
      RuntimeOption::EvalJitStressTypePredPercent > int(get_random() % 100)) {
    int dt;
    while (true) {
      dt = get_random() % (KindOfRef + 1);
      switch (dt) {
        case KindOfNull:
        case KindOfBoolean:
        case KindOfInt64:
        case KindOfDouble:
        case KindOfString:
        case KindOfArray:
        case KindOfObject:
        case KindOfResource:
          break;
        // KindOfRef and KindOfUninit can't happen for lots of predicted
        // types.
        case KindOfRef:
        case KindOfUninit:
        default:
          continue;
      }
      break;
    }
    return DataType(dt);
  }

  if (ni->op() == OpMod) {
    // x % 0 returns boolean false, so we don't know for certain, but it's
    // probably an int.
    return KindOfInt64;
  }

  if (ni->op() == OpSqrt) {
    // sqrt returns a double, unless you pass something nasty to it.
    return KindOfDouble;
  }

  if (ni->op() == OpDiv) {
    // Integers can produce integers if there's no residue, but $i / $j in
    // general produces a double. $i / 0 produces boolean false, so we have
    // actually check the result.
    auto lhs = ni->inputs[0];
    auto rhs = ni->inputs[1];

    if (lhs->valueType() == KindOfDouble || rhs->valueType() == KindOfDouble) {
      return KindOfDouble;
    }

    if (rhs->isLiteral()) {
      if (ni->imm[1].u_I64A == 0) return KindOfBoolean;
      if (ni->imm[1].u_I64A == 1) return lhs->valueType();

      if (rhs->isLiteral()) {
        return ni->imm[0].u_I64A % ni->imm[1].u_I64A ? KindOfDouble
                                                     : KindOfInt64;
      }
    }

    return KindOfDouble;
  }

  if (ni->op() == OpClsCnsD) {
    const NamedEntityPair& cne =
      ni->unit()->lookupNamedEntityPairId(ni->imm[1].u_SA);
    StringData* cnsName = ni->m_unit->lookupLitstrId(ni->imm[0].u_SA);
    Class* cls = cne.second->getCachedClass();
    if (cls) {
      DataType dt = cls->clsCnsType(cnsName);
      if (dt != KindOfUninit) {
        TRACE(1, "clscnsd: %s:%s prediction type %d\n",
              cne.first->data(), cnsName->data(), dt);
        return dt;
      }
    }
  }

  if (ni->op() == OpSetM) {
    /*
     * SetM pushes null for certain rare combinations of input types, a string
     * if the base was a string, or (most commonly) its first stack input. We
     * mark the output as predicted here and do a very rough approximation of
     * what really happens; most of the time the prediction will be a noop
     * since VectorTranslator side exits in all uncommon cases.
     */

    // If the base is a string, the output is probably a string.
    Type baseType;
    switch (ni->immVec.locationCode()) {
      case LGL: case LGC:
      case LNL: case LNC:
      case LSL: case LSC:
        baseType = Type::Gen;
        break;

      default:
        baseType = Type::fromRuntimeType(ni->inputs[1]->rtt);
    }
    if (baseType.isString()) return KindOfString;

    // Otherwise, it's probably the input type.
    return ni->inputs[0]->rtt.valueType();
  }

  static const double kAccept = 1.0;
  std::pair<DataType, double> pred = std::make_pair(KindOfInvalid, 0.0);
  // Type predictions grow tracelets, and can have a side effect of making
  // them combinatorially explode if they bring in precondtions that vary a
  // lot. Get more conservative as evidence mounts that this is a
  // polymorphic tracelet.
  if (tx64->numTranslations(startSk) >= kTooPolyPred) return KindOfInvalid;
  if (ni->op() == OpCGetS) {
    const StringData* propName = ni->inputs[1]->rtt.valueStringOrNull();
    if (propName) {
      pred = predictType(TypeProfileKey(TypeProfileKey::StaticPropName,
                                        propName));
      TRACE(1, "prediction for static fields named %s: %d, %f\n",
            propName->data(),
            pred.first,
            pred.second);
    }
  } else if (hasImmVector(ni->op())) {
    pred = predictMVec(ni);
  }
  if (debug && pred.second < kAccept) {
    if (const StringData* invName = fcallToFuncName(ni)) {
      pred = predictType(TypeProfileKey(TypeProfileKey::MethodName, invName));
      TRACE(1, "prediction for methods named %s: %d, %f\n",
            invName->data(),
            pred.first,
            pred.second);
    }
  }
  if (pred.second >= kAccept) {
    TRACE(1, "accepting prediction of type %d\n", pred.first);
    assert(pred.first != KindOfUninit);
    return pred.first;
  }
  return KindOfInvalid;
}

/**
 * Returns the type of the value a SetOpL will store into the local.
 */
static RuntimeType setOpOutputType(NormalizedInstruction* ni,
                                   const vector<DynLocation*>& inputs) {
  assert(inputs.size() == 2);
  const int kValIdx = 0;
  const int kLocIdx = 1;
  unsigned char op = ni->imm[1].u_OA;
  DynLocation locLocation(inputs[kLocIdx]->location,
                          inputs[kLocIdx]->rtt.unbox());
  assert(inputs[kLocIdx]->location.isLocal());
  switch (op) {
    case SetOpPlusEqual:
    case SetOpMinusEqual:
    case SetOpMulEqual: {
      // Same as OutArith, except we have to fiddle with inputs a bit.
      vector<DynLocation*> arithInputs;
      arithInputs.push_back(&locLocation);
      arithInputs.push_back(inputs[kValIdx]);
      return RuntimeType(inferType(ArithRules, arithInputs));
    }
    case SetOpConcatEqual: return RuntimeType(KindOfString);
    case SetOpDivEqual:
    case SetOpModEqual:    return RuntimeType(KindOfInvalid);
    case SetOpAndEqual:
    case SetOpOrEqual:
    case SetOpXorEqual:    return bitOpType(&locLocation, inputs[kValIdx]);
    case SetOpSlEqual:
    case SetOpSrEqual:     return RuntimeType(KindOfInt64);
    default:
      not_reached();
  }
}

static RuntimeType
getDynLocType(const SrcKey startSk,
              NormalizedInstruction* ni,
              InstrFlags::OutTypeConstraints constraint) {
  using namespace InstrFlags;
  auto const& inputs = ni->inputs;
  assert(constraint != OutFInputL);

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
    CS(OutResource,    KindOfResource);
#undef CS
    case OutPred: {
      auto dt = predictOutputs(startSk, ni);
      if (dt != KindOfInvalid) ni->outputPredicted = true;
      return RuntimeType(dt);
    }

    case OutClassRef: {
      Op op = Op(ni->op());
      if ((op == OpAGetC && inputs[0]->isString())) {
        const StringData *sd = inputs[0]->rtt.valueString();
        if (sd) {
          Class *klass = Unit::lookupUniqueClass(sd);
          TRACE(3, "KindOfClass: derived class \"%s\" from string literal\n",
                klass ? klass->preClass()->name()->data() : "NULL");
          return RuntimeType(klass);
        }
      } else if (op == OpSelf) {
        return RuntimeType(liveClass());
      } else if (op == OpParent) {
        Class* clss = liveClass();
        if (clss != nullptr)
          return RuntimeType(clss->parent());
      }
      return RuntimeType(KindOfClass);
    }

    case OutCns: {
      // If it's a system constant, burn in its type. Otherwise we have
      // to accept prediction; use the translation-time value, or fall back
      // to the targetcache if none exists.
      StringData *sd = ni->m_unit->lookupLitstrId(ni->imm[0].u_SA);
      assert(sd);
      const TypedValue* tv = Unit::lookupPersistentCns(sd);
      if (tv) {
        return RuntimeType(tv->m_type);
      }
      tv = Unit::lookupCns(sd);
      // In JitPGO mode, we disable type predictions to avoid side exits
      if (tv && !RuntimeOption::EvalJitPGO) {
        ni->outputPredicted = true;
        TRACE(1, "CNS %s: guessing runtime type %d\n", sd->data(), tv->m_type);
        return RuntimeType(tv->m_type);
      }
      return RuntimeType(KindOfInvalid);
    }

    case OutNullUninit: {
      assert(ni->op() == OpNullUninit);
      return RuntimeType(KindOfUninit);
    }

    case OutStringImm: {
      assert(ni->op() == OpString);
      StringData *sd = ni->m_unit->lookupLitstrId(ni->imm[0].u_SA);
      assert(sd);
      return RuntimeType(sd);
    }

    case OutArrayImm: {
      assert(ni->op() == OpArray);
      ArrayData *ad = ni->m_unit->lookupArrayId(ni->imm[0].u_AA);
      assert(ad);
      return RuntimeType(ad);
    }

    case OutBooleanImm: {
      assert(ni->op() == OpTrue || ni->op() == OpFalse);
      return RuntimeType(ni->op() == OpTrue);
    }

    case OutThisObject: {
      return Translator::outThisObjectType();
    }

    case OutVUnknown: {
      return RuntimeType(KindOfRef, KindOfInvalid);
    }

    case OutArith: {
      return RuntimeType(inferType(ArithRules, inputs));
    }

    case OutSameAsInput: {
      /*
       * Relies closely on the order that inputs are pushed in
       * getInputs().  (Pushing top of stack first for multi-stack
       * consumers, stack elements before M-vectors and locals, etc.)
       */
      assert(inputs.size() >= 1);
      auto op = ni->op();
      ASSERT_NOT_IMPLEMENTED(
        // Sets and binds that take multiple arguments have the rhs
        // pushed first.  In the case of the M-vector versions, the
        // rhs comes before the M-vector elements.
        op == OpSetL  || op == OpSetN  || op == OpSetG  || op == OpSetS  ||
        op == OpBindL || op == OpBindG || op == OpBindS || op == OpBindN ||
        op == OpBindM ||
        // Dup takes a single element.
        op == OpDup
      );

      const int idx = 0; // all currently supported cases.

      if (debug) {
        if (!inputs[idx]->rtt.isVagueValue()) {
          if (op == OpBindG || op == OpBindN || op == OpBindS ||
              op == OpBindM || op == OpBindL) {
            assert(inputs[idx]->rtt.isRef() && !inputs[idx]->isLocal());
          } else {
            assert(inputs[idx]->rtt.valueType() ==
                   inputs[idx]->rtt.outerType());
          }
        }
      }
      return inputs[idx]->rtt;
    }

    case OutCInputL: {
      assert(inputs.size() >= 1);
      const DynLocation* in = inputs[inputs.size() - 1];
      RuntimeType retval;
      if (in->rtt.outerType() == KindOfUninit) {
        // Locals can be KindOfUninit, so we need to convert
        // this to KindOfNull
        retval = RuntimeType(KindOfNull);
      } else {
        retval = in->rtt.unbox();
      }
      TRACE(2, "Input (%d, %d) -> (%d, %d)\n",
            in->rtt.outerType(), in->rtt.innerType(),
            retval.outerType(), retval.innerType());
      return retval;
    }

    case OutIncDec: {
      const RuntimeType &inRtt = ni->inputs[0]->rtt;
      // TODO: instead of KindOfInvalid this should track the actual
      // type we will get from interping a non-int IncDec.
      return RuntimeType(IS_INT_TYPE(inRtt.valueType()) ?
                         KindOfInt64 : KindOfInvalid);
    }

    case OutStrlen: {
      auto const& rtt = ni->inputs[0]->rtt;
      return RuntimeType(rtt.isString() ? KindOfInt64 : KindOfInvalid);
    }

    case OutCInput: {
      assert(inputs.size() >= 1);
      const DynLocation* in = inputs[inputs.size() - 1];
      if (in->rtt.outerType() == KindOfRef) {
        return in->rtt.unbox();
      }
      return in->rtt;
    }

    case OutBitOp: {
      assert(inputs.size() == 2 ||
             (inputs.size() == 1 && ni->op() == OpBitNot));
      if (inputs.size() == 2) {
        return bitOpType(inputs[0], inputs[1]);
      } else {
        return bitOpType(inputs[0], nullptr);
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
using namespace InstrFlags;
static const struct {
  Op op;
  InstrInfo info;
} instrInfoSparse [] = {

  // Op             Inputs            Outputs       OutputTypes    Stack delta
  // --             ------            -------       -----------    -----------

  /*** 1. Basic instructions ***/

  { OpNop,         {None,             None,         OutNone,           0 }},
  { OpPopC,        {Stack1|
                    DontGuardStack1,  None,         OutNone,          -1 }},
  { OpPopV,        {Stack1|
                    DontGuardStack1|
                    IgnoreInnerType,  None,         OutNone,          -1 }},
  { OpPopR,        {Stack1|
                    DontGuardStack1|
                    IgnoreInnerType,  None,         OutNone,          -1 }},
  { OpDup,         {Stack1,           StackTop2,    OutSameAsInput,    1 }},
  { OpBox,         {Stack1,           Stack1,       OutVInput,         0 }},
  { OpUnbox,       {Stack1,           Stack1,       OutCInput,         0 }},
  { OpBoxR,        {Stack1,           Stack1,       OutVInput,         0 }},
  { OpUnboxR,      {Stack1,           Stack1,       OutCInput,         0 }},

  /*** 2. Literal and constant instructions ***/

  { OpNull,        {None,             Stack1,       OutNull,           1 }},
  { OpNullUninit,  {None,             Stack1,       OutNullUninit,     1 }},
  { OpTrue,        {None,             Stack1,       OutBooleanImm,     1 }},
  { OpFalse,       {None,             Stack1,       OutBooleanImm,     1 }},
  { OpInt,         {None,             Stack1,       OutInt64,          1 }},
  { OpDouble,      {None,             Stack1,       OutDouble,         1 }},
  { OpString,      {None,             Stack1,       OutStringImm,      1 }},
  { OpArray,       {None,             Stack1,       OutArrayImm,       1 }},
  { OpNewArray,    {None,             Stack1,       OutArray,          1 }},
  { OpNewTuple,    {StackN,           Stack1,       OutArray,          0 }},
  { OpAddElemC,    {StackTop3,        Stack1,       OutArray,         -2 }},
  { OpAddElemV,    {StackTop3,        Stack1,       OutArray,         -2 }},
  { OpAddNewElemC, {StackTop2,        Stack1,       OutArray,         -1 }},
  { OpAddNewElemV, {StackTop2,        Stack1,       OutArray,         -1 }},
  { OpNewCol,      {None,             Stack1,       OutObject,         1 }},
  { OpColAddElemC, {StackTop3,        Stack1,       OutObject,        -2 }},
  { OpColAddNewElemC, {StackTop2,     Stack1,       OutObject,        -1 }},
  { OpCns,         {None,             Stack1,       OutCns,            1 }},
  { OpCnsE,        {None,             Stack1,       OutCns,            1 }},
  { OpCnsU,        {None,             Stack1,       OutCns,            1 }},
  { OpClsCns,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpClsCnsD,     {None,             Stack1,       OutPred,           1 }},
  { OpFile,        {None,             Stack1,       OutString,         1 }},
  { OpDir,         {None,             Stack1,       OutString,         1 }},

  /*** 3. Operator instructions ***/

  /* Binary string */
  { OpConcat,      {StackTop2,        Stack1,       OutString,        -1 }},
  /* Arithmetic ops */
  { OpAdd,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpSub,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpMul,         {StackTop2,        Stack1,       OutArith,         -1 }},
  /* Div and mod might return boolean false. Sigh. */
  { OpDiv,         {StackTop2,        Stack1,       OutPred,          -1 }},
  { OpMod,         {StackTop2,        Stack1,       OutPred,          -1 }},
  { OpSqrt,        {Stack1,           Stack1,       OutPred,           0 }},
  /* Logical ops */
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
  { OpFatal,       {Stack1,           None,         OutNone,          -1 }},

  /*** 4. Control flow instructions ***/

  { OpJmp,         {None,             None,         OutNone,           0 }},
  { OpJmpZ,        {Stack1,           None,         OutNone,          -1 }},
  { OpJmpNZ,       {Stack1,           None,         OutNone,          -1 }},
  { OpSwitch,      {Stack1,           None,         OutNone,          -1 }},
  { OpSSwitch,     {Stack1,           None,         OutNone,          -1 }},
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
  { OpUnwind,      {None,             None,         OutNone,           0 }},

  /*** 5. Get instructions ***/

  { OpCGetL,       {Local,            Stack1,       OutCInputL,        1 }},
  { OpCGetL2,      {Stack1|Local,     StackIns1,    OutCInputL,        1 }},
  { OpCGetL3,      {StackTop2|Local,  StackIns2,    OutCInputL,        1 }},
  { OpCGetN,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetG,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetS,       {StackTop2,        Stack1,       OutPred,          -1 }},
  { OpCGetM,       {MVector,          Stack1,       OutPred,           1 }},
  { OpVGetL,       {Local,            Stack1,       OutVInputL,        1 }},
  { OpVGetN,       {Stack1,           Stack1,       OutVUnknown,       0 }},
  // TODO: In pseudo-main, the VGetG instruction invalidates what we know
  // about the types of the locals because it could cause any one of the
  // local variables to become "boxed". We need to add logic to tracelet
  // analysis to deal with this properly.
  { OpVGetG,       {Stack1,           Stack1,       OutVUnknown,       0 }},
  { OpVGetS,       {StackTop2,        Stack1,       OutVUnknown,      -1 }},
  { OpVGetM,       {MVector,          Stack1|Local, OutVUnknown,       1 }},
  { OpAGetC,       {Stack1,           Stack1,       OutClassRef,       0 }},
  { OpAGetL,       {Local,            Stack1,       OutClassRef,       1 }},

  /*** 6. Isset, Empty, and type querying instructions ***/

  { OpAKExists,    {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpIssetL,      {Local,            Stack1,       OutBoolean,        1 }},
  { OpIssetN,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIssetG,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIssetS,      {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpIssetM,      {MVector,          Stack1,       OutBoolean,        1 }},
  { OpEmptyL,      {Local,            Stack1,       OutBoolean,        1 }},
  { OpEmptyN,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpEmptyG,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpEmptyS,      {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpEmptyM,      {MVector,          Stack1,       OutBoolean,        1 }},
  { OpIsNullC,     {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsBoolC,     {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsIntC,      {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsDoubleC,   {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsStringC,   {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsArrayC,    {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsObjectC,   {Stack1,           Stack1,       OutBoolean,        0 }},
  { OpIsNullL,     {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsBoolL,     {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsIntL,      {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsDoubleL,   {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsStringL,   {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsArrayL,    {Local,            Stack1,       OutBoolean,        1 }},
  { OpIsObjectL,   {Local,            Stack1,       OutBoolean,        1 }},

  /*** 7. Mutator instructions ***/

  { OpSetL,        {Stack1|Local,     Stack1|Local, OutSameAsInput,    0 }},
  { OpSetN,        {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpSetG,        {StackTop2,        Stack1,       OutSameAsInput,   -1 }},
  { OpSetS,        {StackTop3,        Stack1,       OutSameAsInput,   -2 }},
  { OpSetM,        {MVector|Stack1,   Stack1|Local, OutPred,           0 }},
  { OpSetWithRefLM,{MVector|Local ,   Local,        OutNone,           0 }},
  { OpSetWithRefRM,{MVector|Stack1,   Local,        OutNone,          -1 }},
  { OpSetOpL,      {Stack1|Local,     Stack1|Local, OutSetOp,          0 }},
  { OpSetOpN,      {StackTop2,        Stack1|Local, OutUnknown,       -1 }},
  { OpSetOpG,      {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpSetOpS,      {StackTop3,        Stack1,       OutUnknown,       -2 }},
  { OpSetOpM,      {MVector|Stack1,   Stack1|Local, OutUnknown,        0 }},
  { OpIncDecL,     {Local,            Stack1|Local, OutIncDec,         1 }},
  { OpIncDecN,     {Stack1,           Stack1|Local, OutUnknown,        0 }},
  { OpIncDecG,     {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpIncDecS,     {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpIncDecM,     {MVector,          Stack1,       OutUnknown,        1 }},
  { OpBindL,       {Stack1|Local|
                    IgnoreInnerType,  Stack1|Local, OutSameAsInput,    0 }},
  { OpBindN,       {StackTop2,        Stack1|Local, OutSameAsInput,   -1 }},
  { OpBindG,       {StackTop2,        Stack1,       OutSameAsInput,   -1 }},
  { OpBindS,       {StackTop3,        Stack1,       OutSameAsInput,   -2 }},
  { OpBindM,       {MVector|Stack1,   Stack1|Local, OutSameAsInput,    0 }},
  { OpUnsetL,      {Local,            Local,        OutNone,           0 }},
  { OpUnsetN,      {Stack1,           Local,        OutNone,          -1 }},
  { OpUnsetG,      {Stack1,           None,         OutNone,          -1 }},
  { OpUnsetM,      {MVector,          Local,        OutNone,           0 }},

  /*** 8. Call instructions ***/

  { OpFPushFunc,   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushFuncD,  {None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushFuncU,  {None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushObjMethod,
                   {StackTop2,        FStack,       OutFDesc,
                                                     kNumActRecCells - 2 }},
  { OpFPushObjMethodD,
                   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushClsMethod,
                   {StackTop2,        FStack,       OutFDesc,
                                                     kNumActRecCells - 2 }},
  { OpFPushClsMethodF,
                   {StackTop2,        FStack,       OutFDesc,
                                                     kNumActRecCells - 2 }},
  { OpFPushClsMethodD,
                   {None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushCtor,   {Stack1,           Stack1|FStack,OutObject,
                                                         kNumActRecCells }},
  { OpFPushCtorD,  {None,             Stack1|FStack,OutObject,
                                                     kNumActRecCells + 1 }},
  { OpFPushCufIter,{None,             FStack,       OutFDesc,
                                                         kNumActRecCells }},
  { OpFPushCuf,    {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushCufF,   {Stack1,           FStack,       OutFDesc,
                                                     kNumActRecCells - 1 }},
  { OpFPushCufSafe,{StackTop2|DontGuardAny,
                                      StackTop2|FStack, OutFPushCufSafe,
                                                         kNumActRecCells }},
  { OpFPassC,      {FuncdRef,         None,         OutSameAsInput,    0 }},
  { OpFPassCW,     {FuncdRef,         None,         OutSameAsInput,    0 }},
  { OpFPassCE,     {FuncdRef,         None,         OutSameAsInput,    0 }},
  { OpFPassV,      {Stack1|FuncdRef,  Stack1,       OutUnknown,        0 }},
  { OpFPassR,      {Stack1|FuncdRef,  Stack1,       OutFInputR,        0 }},
  { OpFPassL,      {Local|FuncdRef,   Stack1,       OutFInputL,        1 }},
  { OpFPassN,      {Stack1|FuncdRef,  Stack1,       OutUnknown,        0 }},
  { OpFPassG,      {Stack1|FuncdRef,  Stack1,       OutFInputR,        0 }},
  { OpFPassS,      {StackTop2|FuncdRef,
                                      Stack1,       OutUnknown,       -1 }},
  { OpFPassM,      {MVector|FuncdRef, Stack1|Local, OutUnknown,        1 }},
  /*
   * FCall is special. Like the Ret* instructions, its manipulation of the
   * runtime stack are outside the boundaries of the tracelet abstraction.
   */
  { OpFCall,       {FStack,           Stack1,       OutPred,           0 }},
  { OpFCallArray,  {FStack,           Stack1,       OutPred,
                                                   -(int)kNumActRecCells }},
  // TODO: output type is known
  { OpFCallBuiltin,{BStackN,          Stack1,       OutPred,          0 }},
  { OpCufSafeArray,{StackTop3|DontGuardAny,
                                      Stack1,       OutArray,         -2 }},
  { OpCufSafeReturn,{StackTop3|DontGuardAny,
                                      Stack1,       OutUnknown,       -2 }},
  { OpDecodeCufIter,{Stack1,          None,         OutNone,          -1 }},

  /*** 11. Iterator instructions ***/

  { OpIterInit,    {Stack1,           Local,        OutUnknown,       -1 }},
  { OpMIterInit,   {Stack1,           Local,        OutUnknown,       -1 }},
  { OpWIterInit,   {Stack1,           Local,        OutUnknown,       -1 }},
  { OpIterInitK,   {Stack1,           Local,        OutUnknown,       -1 }},
  { OpMIterInitK,  {Stack1,           Local,        OutUnknown,       -1 }},
  { OpWIterInitK,  {Stack1,           Local,        OutUnknown,       -1 }},
  { OpIterNext,    {None,             Local,        OutUnknown,        0 }},
  { OpMIterNext,   {None,             Local,        OutUnknown,        0 }},
  { OpWIterNext,   {None,             Local,        OutUnknown,        0 }},
  { OpIterNextK,   {None,             Local,        OutUnknown,        0 }},
  { OpMIterNextK,  {None,             Local,        OutUnknown,        0 }},
  { OpWIterNextK,  {None,             Local,        OutUnknown,        0 }},
  { OpIterFree,    {None,             None,         OutNone,           0 }},
  { OpMIterFree,   {None,             None,         OutNone,           0 }},
  { OpCIterFree,   {None,             None,         OutNone,           0 }},
  { OpIterBreak,   {None,             None,         OutNone,           0 }},

  /*** 12. Include, eval, and define instructions ***/

  { OpIncl,        {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpInclOnce,    {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReq,         {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqOnce,     {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpReqDoc,      {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpEval,        {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpDefFunc,     {None,             None,         OutNone,           0 }},
  { OpDefTypedef,  {None,             None,         OutNone,           0 }},
  { OpDefCls,      {None,             None,         OutNone,           0 }},
  { OpDefCns,      {Stack1,           Stack1,       OutBoolean,        0 }},

  /*** 13. Miscellaneous instructions ***/

  { OpThis,        {None,             Stack1,       OutThisObject,     1 }},
  { OpBareThis,    {None,             Stack1,       OutUnknown,        1 }},
  { OpCheckThis,   {This,             None,         OutNone,           0 }},
  { OpInitThisLoc,
                   {None,             Local,        OutUnknown,        0 }},
  { OpStaticLoc,
                   {None,             Stack1,       OutBoolean,        1 }},
  { OpStaticLocInit,
                   {Stack1,           Local,        OutVUnknown,      -1 }},
  { OpCatch,       {None,             Stack1,       OutObject,         1 }},
  { OpVerifyParamType,
                   {Local,            None,         OutNone,           0 }},
  { OpClassExists, {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpInterfaceExists,
                   {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpTraitExists, {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpSelf,        {None,             Stack1,       OutClassRef,       1 }},
  { OpParent,      {None,             Stack1,       OutClassRef,       1 }},
  { OpLateBoundCls,{None,             Stack1,       OutClassRef,       1 }},
  { OpNativeImpl,  {None,             None,         OutNone,           0 }},
  { OpCreateCl,    {BStackN,          Stack1,       OutObject,         1 }},
  { OpStrlen,      {Stack1,           Stack1,       OutStrlen,         0 }},
  { OpIncStat,     {None,             None,         OutNone,           0 }},
  { OpArrayIdx,    {StackTop3,        Stack1,       OutUnknown,       -2 }},
  { OpFloor,       {Stack1,           Stack1,       OutDouble,         0 }},
  { OpCeil,        {Stack1,           Stack1,       OutDouble,         0 }},

  /*** 14. Continuation instructions ***/

  { OpCreateCont,  {None,             Stack1|Local, OutObject,         1 }},
  { OpContEnter,   {Stack1,           None,         OutNone,          -1 }},
  { OpUnpackCont,  {None,             StackTop2,    OutInt64,          2 }},
  { OpContSuspend, {Stack1,           None,         OutNone,          -1 }},
  { OpContSuspendK,{StackTop2,        None,         OutNone,          -2 }},
  { OpContRetC,    {Stack1,           None,         OutNone,          -1 }},
  { OpContCheck,   {None,             None,         OutNone,           0 }},
  { OpContRaise,   {None,             None,         OutNone,           0 }},
  { OpContValid,   {None,             Stack1,       OutBoolean,        1 }},
  { OpContKey,     {None,             Stack1,       OutUnknown,        1 }},
  { OpContCurrent, {None,             Stack1,       OutUnknown,        1 }},
  { OpContStopped, {None,             None,         OutNone,           0 }},
  { OpContHandle,  {Stack1,           None,         OutNone,          -1 }},
};

static hphp_hash_map<Op, InstrInfo> instrInfo;
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

const InstrInfo& getInstrInfo(Op op) {
  assert(instrInfoInited);
  return instrInfo[op];
}

static int numHiddenStackInputs(const NormalizedInstruction& ni) {
  assert(ni.immVec.isValid());
  return ni.immVec.numStackValues();
}

namespace {
int64_t countOperands(uint64_t mask) {
  const uint64_t ignore = FuncdRef | Local | Iter | AllLocals |
    DontGuardLocal | DontGuardStack1 | DontBreakLocal | DontBreakStack1 |
    IgnoreInnerType | DontGuardAny | This;
  mask &= ~ignore;

  static const uint64_t counts[][2] = {
    {Stack3,       1},
    {Stack2,       1},
    {Stack1,       1},
    {StackIns1,    2},
    {StackIns2,    3},
    {FStack,       kNumActRecCells},
  };

  int64_t count = 0;
  for (auto const& pair : counts) {
    if (mask & pair[0]) {
      count += pair[1];
      mask &= ~pair[0];
    }
  }
  assert(mask == 0);
  return count;
}
}

int64_t getStackPopped(const NormalizedInstruction& ni) {
  switch (ni.op()) {
    case OpFCall:        return ni.imm[0].u_IVA + kNumActRecCells;
    case OpFCallArray:   return kNumActRecCells + 1;

    case OpFCallBuiltin:
    case OpNewTuple:
    case OpCreateCl:     return ni.imm[0].u_IVA;

    default:             break;
  }

  uint64_t mask = getInstrInfo(ni.op()).in;
  int64_t count = 0;

  if (mask & MVector) {
    count += ni.immVec.numStackValues();
    mask &= ~MVector;
  }
  if (mask & (StackN | BStackN)) {
    count += ni.imm[0].u_IVA;
    mask &= ~(StackN | BStackN);
  }

  return count + countOperands(mask);
}

int64_t getStackPushed(const NormalizedInstruction& ni) {
  return countOperands(getInstrInfo(ni.op()).out);
}

int getStackDelta(const NormalizedInstruction& ni) {
  int hiddenStackInputs = 0;
  initInstrInfo();
  auto op = ni.op();
  switch (op) {
    case OpFCall: {
      int numArgs = ni.imm[0].u_IVA;
      return 1 - numArgs - kNumActRecCells;
    }

    case OpFCallBuiltin:
    case OpNewTuple:
    case OpCreateCl:
      return 1 - ni.imm[0].u_IVA;

    default:
      break;
  }
  const InstrInfo& info = instrInfo[op];
  if (info.in & MVector) {
    hiddenStackInputs = numHiddenStackInputs(ni);
    SKTRACE(2, ni.source, "Has %d hidden stack inputs\n", hiddenStackInputs);
  }
  int delta = instrInfo[op].numPushed - hiddenStackInputs;
  return delta;
}

static NormalizedInstruction* findInputSrc(NormalizedInstruction* ni,
                                           DynLocation* dl) {
  while (ni != nullptr) {
    if (ni->outStack == dl ||
        ni->outLocal == dl ||
        ni->outLocal2 == dl ||
        ni->outStack2 == dl ||
        ni->outStack3 == dl) {
      break;
    }
    ni = ni->prev;
  }
  return ni;
}

bool outputIsPredicted(SrcKey startSk,
                       NormalizedInstruction& inst) {
  auto const& iInfo = getInstrInfo(inst.op());
  auto doPrediction = iInfo.type == OutPred && !inst.breaksTracelet;
  if (doPrediction) {
    // All OutPred ops except for SetM have a single stack output for now.
    assert(iInfo.out == Stack1 || inst.op() == OpSetM);
    auto dt = predictOutputs(startSk, &inst);
    if (dt != KindOfInvalid) {
      inst.outPred = Type::fromDataType(dt);
    } else {
      doPrediction = false;
    }
  }

  return doPrediction;
}

/*
 * For MetaData information that affects whether we want to even put a
 * value in the ni->inputs, we need to look at it before we call
 * getInputs(), so this is separate from applyInputMetaData.
 *
 * We also check GuardedThis here, since RetC is short-circuited in
 * applyInputMetaData.
 */
void preInputApplyMetaData(Unit::MetaHandle metaHand,
                           NormalizedInstruction* ni) {
  if (!metaHand.findMeta(ni->unit(), ni->offset())) return;

  Unit::MetaInfo info;
  while (metaHand.nextArg(info)) {
    switch (info.m_kind) {
    case Unit::MetaInfo::Kind::NonRefCounted:
      ni->nonRefCountedLocals.resize(ni->func()->numLocals());
      ni->nonRefCountedLocals[info.m_data] = 1;
      break;
    case Unit::MetaInfo::Kind::GuardedThis:
      ni->guardedThis = true;
      break;
    default:
      break;
    }
  }
}

bool Translator::applyInputMetaData(Unit::MetaHandle& metaHand,
                                    NormalizedInstruction* ni,
                                    TraceletContext& tas,
                                    InputInfos &inputInfos) {
  if (!metaHand.findMeta(ni->unit(), ni->offset())) return false;

  Unit::MetaInfo info;
  if (!metaHand.nextArg(info)) return false;
  if (info.m_kind == Unit::MetaInfo::Kind::NopOut) {
    ni->noOp = true;
    return true;
  }

  /*
   * We need to adjust the indexes in MetaInfo::m_arg if this
   * instruction takes other stack arguments than those related to the
   * MVector.  (For example, the rhs of an assignment.)
   */
  const InstrInfo& iInfo = instrInfo[ni->op()];
  if (iInfo.in & AllLocals) {
    /*
     * RetC/RetV dont care about their stack input, but it may have
     * been annotated. Skip it (because RetC/RetV pretend they dont
     * have a stack input).
     */
    return false;
  }
  if (iInfo.in == FuncdRef) {
    /*
     * FPassC* pretend to have no inputs
     */
    return false;
  }
  const int base = !(iInfo.in & MVector) ? 0 :
                   !(iInfo.in & Stack1) ? 0 :
                   !(iInfo.in & Stack2) ? 1 :
                   !(iInfo.in & Stack3) ? 2 : 3;

  do {
    SKTRACE(3, ni->source, "considering MetaInfo of kind %d\n", info.m_kind);

    int arg = info.m_arg & Unit::MetaInfo::VectorArg ?
      base + (info.m_arg & ~Unit::MetaInfo::VectorArg) : info.m_arg;

    switch (info.m_kind) {
      case Unit::MetaInfo::Kind::NoSurprise:
        ni->noSurprise = true;
        break;
      case Unit::MetaInfo::Kind::GuardedCls:
        ni->guardedCls = true;
        break;
      case Unit::MetaInfo::Kind::ArrayCapacity:
        ni->imm[0].u_IVA = info.m_data;
        break;
      case Unit::MetaInfo::Kind::DataTypePredicted: {
        // In JitPGO, disable type predictions to avoid side exits
        if (RuntimeOption::EvalJitPGO) break;

        // If the original type was invalid or predicted, then use the
        // prediction in the meta-data.
        assert((unsigned) arg < inputInfos.size());

        SKTRACE(1, ni->source, "MetaInfo DataTypePredicted for input %d; "
                "newType = %d\n", arg, DataType(info.m_data));
        InputInfo& ii = inputInfos[arg];
        DynLocation* dl = tas.recordRead(ii, false, KindOfInvalid);
        NormalizedInstruction* src = findInputSrc(tas.m_t->m_instrStream.last,
                                                  dl);
        if (src) {
          // Update the rtt and mark src's output as predicted if either:
          //  a) we don't have type information yet (ie, it's KindOfInvalid), or
          //  b) src's output was predicted. This is assuming that the
          //     front-end's prediction is more accurate.
          if (dl->rtt.outerType() == KindOfInvalid || src->outputPredicted) {
            SKTRACE(1, ni->source, "MetaInfo DataTypePredicted for input %d; "
                    "replacing oldType = %d with newType = %d\n", arg,
                    dl->rtt.outerType(), DataType(info.m_data));
            dl->rtt = RuntimeType((DataType)info.m_data);
            src->outputPredicted = true;
            src->outputPredictionStatic = true;
          }
        }
        break;
      }
      case Unit::MetaInfo::Kind::DataTypeInferred: {
        assert((unsigned)arg < inputInfos.size());
        SKTRACE(1, ni->source, "MetaInfo DataTypeInferred for input %d; "
                   "newType = %d\n", arg, DataType(info.m_data));
        InputInfo& ii = inputInfos[arg];
        ii.dontGuard = true;
        DynLocation* dl = tas.recordRead(ii, true, (DataType)info.m_data);
        if (dl->rtt.outerType() != info.m_data &&
            (!dl->isString() || info.m_data != KindOfString)) {
          if (dl->rtt.outerType() != KindOfInvalid) {
            // Either static analysis is wrong, or
            // this was mis-predicted by the type
            // profiler, or this code is unreachable,
            // and there's an earlier bytecode in the tracelet
            // thats going to fatal
            NormalizedInstruction *src = nullptr;
            if (mapContains(tas.m_changeSet, dl->location)) {
              src = findInputSrc(tas.m_t->m_instrStream.last, dl);
              if (src && src->outputPredicted) {
                src->outputPredicted = false;
              } else {
                src = nullptr;
              }
            }
            if (!src) {
              // Not a type-profiler mis-predict
              if (tas.m_t->m_instrStream.first) {
                // We're not the first instruction, so punt
                // If this bytecode /is/ reachable, we'll
                // get here again, and that time, we will
                // be the first instruction
                punt();
              }
              not_reached();
            }
          }
          dl->rtt = RuntimeType((DataType)info.m_data);
          ni->markInputInferred(arg);
        } else {
          /*
           * Static inference confirmed the expected type
           * but if the expected type was provided by the type
           * profiler we want to clear outputPredicted to
           * avoid unneeded guards
           */
          if (mapContains(tas.m_changeSet, dl->location)) {
            NormalizedInstruction *src =
              findInputSrc(tas.m_t->m_instrStream.last, dl);
            if (src->outputPredicted) {
              src->outputPredicted = false;
              ni->markInputInferred(arg);
            }
          }
        }
        break;
      }

      case Unit::MetaInfo::Kind::String: {
        const StringData* sd = ni->unit()->lookupLitstrId(info.m_data);
        assert((unsigned)arg < inputInfos.size());
        InputInfo& ii = inputInfos[arg];
        ii.dontGuard = true;
        DynLocation* dl = tas.recordRead(ii, true, KindOfString);
        assert(!dl->rtt.isString() || !dl->rtt.valueString() ||
               dl->rtt.valueString() == sd);
        SKTRACE(1, ni->source, "MetaInfo on input %d; old type = %s\n",
                arg, dl->pretty().c_str());
        dl->rtt = RuntimeType(sd);
        break;
      }

      case Unit::MetaInfo::Kind::Class: {
        assert((unsigned)arg < inputInfos.size());
        InputInfo& ii = inputInfos[arg];
        DynLocation* dl = tas.recordRead(ii, true);
        if (dl->rtt.valueType() != KindOfObject) {
          continue;
        }

        const StringData* metaName = ni->unit()->lookupLitstrId(info.m_data);
        const StringData* rttName =
          dl->rtt.valueClass() ? dl->rtt.valueClass()->name() : nullptr;
        // The two classes might not be exactly the same, which is ok
        // as long as metaCls is more derived than rttCls.
        Class* metaCls = Unit::lookupUniqueClass(metaName);
        Class* rttCls = rttName ? Unit::lookupUniqueClass(rttName) : nullptr;
        if (metaCls && rttCls && metaCls != rttCls &&
            !metaCls->classof(rttCls)) {
          // Runtime type is more derived
          metaCls = 0;
        }
        if (metaCls && metaCls != rttCls) {
          SKTRACE(1, ni->source, "replacing input %d with a MetaInfo-supplied "
                  "class of %s; old type = %s\n",
                  arg, metaName->data(), dl->pretty().c_str());
          if (dl->rtt.isRef()) {
            dl->rtt = RuntimeType(KindOfRef, KindOfObject, metaCls);
          } else {
            dl->rtt = RuntimeType(KindOfObject, KindOfInvalid, metaCls);
          }
        }
        break;
      }

      case Unit::MetaInfo::Kind::MVecPropClass: {
        const StringData* metaName = ni->unit()->lookupLitstrId(info.m_data);
        Class* metaCls = Unit::lookupUniqueClass(metaName);
        if (metaCls) {
          ni->immVecClasses[arg] = metaCls;
        }
        break;
      }

      case Unit::MetaInfo::Kind::NopOut:
        // NopOut should always be the first and only annotation
        // and was handled above.
        not_reached();

      case Unit::MetaInfo::Kind::GuardedThis:
      case Unit::MetaInfo::Kind::NonRefCounted:
        // fallthrough; these are handled in preInputApplyMetaData.
      case Unit::MetaInfo::Kind::None:
        break;
    }
  } while (metaHand.nextArg(info));

  return false;
}

static void addMVectorInputs(NormalizedInstruction& ni,
                             int& currentStackOffset,
                             std::vector<InputInfo>& inputs) {
  assert(ni.immVec.isValid());
  ni.immVecM.reserve(ni.immVec.size());

  int UNUSED stackCount = 0;
  int UNUSED localCount = 0;

  currentStackOffset -= ni.immVec.numStackValues();
  int localStackOffset = currentStackOffset;

  auto push_stack = [&] {
    ++stackCount;
    inputs.emplace_back(Location(Location::Stack, localStackOffset++));
  };
  auto push_local = [&] (int imm) {
    ++localCount;
    inputs.emplace_back(Location(Location::Local, imm));
  };

  /*
   * Note that we have to push as we go so that the arguments come in
   * the order expected for the M-vector.
   *
   * Indexes into these argument lists must also be in the same order
   * as the information in Unit::MetaInfo, because the analysis phase
   * may replace some of them with literals.
   */

  /*
   * Also note: if we eventually have immediates that are not local
   * ids (i.e. string ids), this analysis step is going to have to be
   * a bit wiser.
   */
  const uint8_t* vec = ni.immVec.vec();
  const LocationCode lcode = LocationCode(*vec++);

  const bool trailingClassRef = lcode == LSL || lcode == LSC;

  switch (numLocationCodeStackVals(lcode)) {
  case 0: {
    if (lcode == LH) {
      inputs.emplace_back(Location(Location::This));
    } else {
      assert(lcode == LL || lcode == LGL || lcode == LNL);
      int numImms = numLocationCodeImms(lcode);
      for (int i = 0; i < numImms; ++i) {
        push_local(decodeVariableSizeImm(&vec));
      }
    }
  } break;
  case 1:
    if (lcode == LSL) {
      // We'll get the trailing stack value after pushing all the
      // member vector elements.
      push_local(decodeVariableSizeImm(&vec));
    } else {
      push_stack();
    }
    break;
  case 2:
    push_stack();
    if (!trailingClassRef) {
      // This one is actually at the back.
      push_stack();
    }
    break;
  default: not_reached();
  }

  // Now push all the members in the correct order.
  while (vec - ni.immVec.vec() < ni.immVec.size()) {
    const MemberCode mcode = MemberCode(*vec++);
    ni.immVecM.push_back(mcode);

    if (mcode == MW) {
      // No stack and no locals.
    } else if (memberCodeHasImm(mcode)) {
      int64_t imm = decodeMemberCodeImm(&vec, mcode);
      if (memberCodeImmIsLoc(mcode)) {
        push_local(imm);
      } else if (memberCodeImmIsString(mcode)) {
        inputs.emplace_back(Location(Location::Litstr, imm));
      } else {
        assert(memberCodeImmIsInt(mcode));
        inputs.emplace_back(Location(Location::Litint, imm));
      }
    } else {
      push_stack();
    }
    inputs.back().dontGuardInner = true;
  }

  if (trailingClassRef) {
    push_stack();
  }

  ni.immVecClasses.resize(ni.immVecM.size());

  assert(vec - ni.immVec.vec() == ni.immVec.size());
  assert(stackCount == ni.immVec.numStackValues());

  SKTRACE(2, ni.source, "M-vector using %d hidden stack "
                        "inputs, %d locals\n", stackCount, localCount);
}

void getInputs(SrcKey startSk, NormalizedInstruction& inst, InputInfos& infos,
               const Func* func, const LocalTypeFn& localType) {
  // TranslatorX64 expected top of stack to be index -1, with indexes growing
  // down from there. hhir defines top of stack to be index 0, with indexes
  // growing up from there. To compensate we start with a stack offset of 1 and
  // negate the index of any stack input after the call to getInputs.
  int stackOff = 1;
  getInputsImpl(startSk, &inst, stackOff, infos, func, localType);
  for (auto& info : infos) {
    if (info.loc.isStack()) info.loc.offset = -info.loc.offset;
  }
}

/*
 * getInputsImpl --
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
void getInputsImpl(SrcKey startSk,
                   NormalizedInstruction* ni,
                   int& currentStackOffset,
                   InputInfos& inputs,
                   const Func* func,
                   const LocalTypeFn& localType) {
#ifdef USE_TRACE
  const SrcKey& sk = ni->source;
#endif
  assert(inputs.empty());
  if (debug && !mapContains(instrInfo, ni->op())) {
    fprintf(stderr, "Translator does not understand "
      "instruction %s\n", opcodeToName(ni->op()));
    assert(false);
  }
  const InstrInfo& info = instrInfo[ni->op()];
  Operands input = info.in;
  if (input & FuncdRef) {
    inputs.needsRefCheck = true;
  }
  if (input & Iter) {
    inputs.emplace_back(Location(Location::Iter, ni->imm[0].u_IVA));
  }
  if (input & FStack) {
    currentStackOffset -= ni->imm[0].u_IVA; // arguments consumed
    currentStackOffset -= kNumActRecCells; // ActRec is torn down as well
  }
  if (input & IgnoreInnerType) ni->ignoreInnerType = true;
  if (input & Stack1) {
    SKTRACE(1, sk, "getInputs: stack1 %d\n", currentStackOffset - 1);
    inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
    if (input & DontGuardStack1) inputs.back().dontGuard = true;
    if (input & DontBreakStack1) inputs.back().dontBreak = true;
    if (input & Stack2) {
      SKTRACE(1, sk, "getInputs: stack2 %d\n", currentStackOffset - 1);
      inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
      if (input & Stack3) {
        SKTRACE(1, sk, "getInputs: stack3 %d\n", currentStackOffset - 1);
        inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
      }
    }
  }
  if (input & StackN) {
    int numArgs = ni->imm[0].u_IVA;
    SKTRACE(1, sk, "getInputs: stackN %d %d\n", currentStackOffset - 1,
            numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
      inputs.back().dontGuard = true;
      inputs.back().dontBreak = true;
    }
  }
  if (input & BStackN) {
    int numArgs = ni->imm[0].u_IVA;
    SKTRACE(1, sk, "getInputs: BStackN %d %d\n", currentStackOffset - 1,
            numArgs);
    for (int i = 0; i < numArgs; i++) {
      inputs.emplace_back(Location(Location::Stack, --currentStackOffset));
    }
  }
  if (input & MVector) {
    addMVectorInputs(*ni, currentStackOffset, inputs);
  }
  if (input & Local) {
    // All instructions that take a Local have its index at their first
    // immediate.
    int loc;
    auto insertAt = inputs.end();
    switch (ni->op()) {
      case OpSetWithRefLM:
        insertAt = inputs.begin();
        // fallthrough
      case OpFPassL:
        loc = ni->imm[1].u_IVA;
        break;

      default:
        loc = ni->imm[0].u_IVA;
        break;
    }
    SKTRACE(1, sk, "getInputs: local %d\n", loc);
    inputs.emplace(insertAt, Location(Location::Local, loc));
    if (input & DontGuardLocal) inputs.back().dontGuard = true;
    if (input & DontBreakLocal) inputs.back().dontBreak = true;
  }

  auto wantInlineReturn = [&] {
    const int localCount = ni->func()->numLocals();
    // Inline return causes us to guard this tracelet more precisely. If
    // we're already chaining to get here, just do a generic return in the
    // hopes of avoiding further specialization. The localCount constraint
    // is an unfortunate consequence of the current generic machinery not
    // working for 0 locals.
    if (tx64->numTranslations(startSk) >= kTooPolyRet && localCount > 0) {
      return false;
    }
    ni->nonRefCountedLocals.resize(localCount);
    int numRefCounted = 0;
    for (int i = 0; i < localCount; ++i) {
      auto curType = localType(i);
      if (ni->nonRefCountedLocals[i]) {
        assert(curType.notCounted() && "Static analysis was wrong");
      }
      numRefCounted += curType.maybeCounted();
    }
    return numRefCounted <= Translator::kMaxInlineReturnDecRefs;
  };

  if ((input & AllLocals) && wantInlineReturn()) {
    ni->inlineReturn = true;
    ni->ignoreInnerType = true;
    int n = ni->func()->numLocals();
    for (int i = 0; i < n; ++i) {
      if (!ni->nonRefCountedLocals[i]) {
        inputs.emplace_back(Location(Location::Local, i));
      }
    }
  }

  SKTRACE(1, sk, "stack args: virtual sfo now %d\n", currentStackOffset);
  TRACE(1, "%s\n", Trace::prettyNode("Inputs", inputs).c_str());

  if (inputs.size() &&
      ((input & DontGuardAny) || dontGuardAnyInputs(ni->op()))) {
    for (int i = inputs.size(); i--; ) {
      inputs[i].dontGuard = true;
    }
  }
  if (input & This) {
    inputs.emplace_back(Location(Location::This));
  }
}

bool dontGuardAnyInputs(Op op) {
  switch (op) {
#define CASE(iNm) case Op ## iNm:
#define NOOP(...)
  INSTRS
  PSEUDOINSTR_DISPATCH(NOOP)
    return false;

  default:
    return true;
  }
#undef NOOP
#undef CASE
}

bool outputDependsOnInput(const Op instr) {
  switch (instrInfo[instr].type) {
    case OutNull:
    case OutNullUninit:
    case OutString:
    case OutStringImm:
    case OutDouble:
    case OutBoolean:
    case OutBooleanImm:
    case OutInt64:
    case OutArray:
    case OutArrayImm:
    case OutObject:
    case OutResource:
    case OutThisObject:
    case OutUnknown:
    case OutVUnknown:
    case OutClassRef:
    case OutPred:
    case OutCns:
    case OutStrlen:
    case OutNone:
      return false;

    case OutFDesc:
    case OutSameAsInput:
    case OutCInput:
    case OutVInput:
    case OutCInputL:
    case OutVInputL:
    case OutFInputL:
    case OutFInputR:
    case OutArith:
    case OutBitOp:
    case OutSetOp:
    case OutIncDec:
    case OutFPushCufSafe:
      return true;
  }
  not_reached();
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
                            /*out*/   bool& varEnvTaint) {
  varEnvTaint = false;

  const vector<DynLocation*>& inputs = ni->inputs;
  const Op op = ni->op();

  initInstrInfo();
  assert_not_implemented(instrInfo.find(op) != instrInfo.end());
  const Operands outLocs = instrInfo[op].out;
  const OutTypeConstraints typeInfo = instrInfo[op].type;

  SKTRACE(1, ni->source, "output flavor %d\n", typeInfo);
  if (typeInfo == OutFInputL || typeInfo == OutFInputR ||
      typeInfo == OutVInputL) {
    // Variable number of outputs. If we box the loc we're reading,
    // we need to write out its boxed-ness.
    assert(inputs.size() >= 1);
    const DynLocation* in = inputs[inputs.size() - 1];
    DynLocation* outDynLoc = t.newDynLocation(in->location, in->rtt);
    outDynLoc->location = Location(Location::Stack, currentStackOffset++);
    bool isRef;
    if (typeInfo == OutVInputL) {
      isRef = true;
    } else {
      assert(typeInfo == OutFInputL || typeInfo == OutFInputR);
      isRef = ni->preppedByRef;
    }
    if (isRef) {
      // Locals can be KindOfUninit, so we need to convert
      // this to KindOfNull
      if (in->rtt.outerType() == KindOfUninit) {
        outDynLoc->rtt = RuntimeType(KindOfRef, KindOfNull);
      } else {
        outDynLoc->rtt = in->rtt.box();
      }
      SKTRACE(1, ni->source, "boxed type: %d -> %d\n",
              outDynLoc->rtt.outerType(), outDynLoc->rtt.innerType());
    } else {
      if (outDynLoc->rtt.outerType() == KindOfUninit) {
        outDynLoc->rtt = RuntimeType(KindOfNull);
      } else {
        outDynLoc->rtt = outDynLoc->rtt.unbox();
      }
      SKTRACE(1, ni->source, "unboxed type: %d\n",
              outDynLoc->rtt.outerType());
    }
    assert(outDynLoc->location.isStack());
    ni->outStack = outDynLoc;

    if (isRef && in->rtt.outerType() != KindOfRef &&
        typeInfo != OutFInputR &&
        in->location.isLocal()) {
      // VGetH or FPassH boxing a local
      DynLocation* smashedLocal =
          t.newDynLocation(in->location, outDynLoc->rtt);
      assert(smashedLocal->location.isLocal());
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
    assert(opnd != None && opnd != Stack3);  // no instr produces 3 values
    assert(opnd != FuncdRef);                // reffiness is immutable
    Location loc;
    switch (opnd) {
      // Pseudo-outputs that affect translator state
      case FStack: {
        currentStackOffset += kNumActRecCells;
        t.m_arState.pushFunc(*ni);
      } continue; // no instr-associated output

      case Local: {
        if (op == OpSetN || op == OpSetOpN || op == OpIncDecN ||
            op == OpBindN || op == OpUnsetN) {
          varEnvTaint = true;
          continue;
        }
        if (op == OpCreateCont) {
          // CreateCont stores Uninit to all locals but NormalizedInstruction
          // doesn't have enough output fields, so we special case it in
          // analyze().
          continue;
        }

        ASSERT_NOT_IMPLEMENTED(op == OpSetOpL ||
                               op == OpSetM || op == OpSetOpM ||
                               op == OpBindM ||
                               op == OpSetWithRefLM || op == OpSetWithRefRM ||
                               op == OpUnsetM ||
                               op == OpIncDecL ||
                               op == OpVGetM || op == OpFPassM ||
                               op == OpStaticLocInit || op == OpInitThisLoc ||
                               op == OpSetL || op == OpBindL ||
                               op == OpUnsetL ||
                               op == OpIterInit || op == OpIterInitK ||
                               op == OpMIterInit || op == OpMIterInitK ||
                               op == OpWIterInit || op == OpWIterInitK ||
                               op == OpIterNext || op == OpIterNextK ||
                               op == OpMIterNext || op == OpMIterNextK ||
                               op == OpWIterNext || op == OpWIterNextK);
        if (op == OpFPassM && !ni->preppedByRef) {
          // Equivalent to CGetM. Won't mutate the base.
          continue;
        }
        if (op == OpIncDecL) {
          assert(ni->inputs.size() == 1);
          const RuntimeType &inRtt = ni->inputs[0]->rtt;
          RuntimeType rtt = IS_INT_TYPE(inRtt.valueType()) ? inRtt :
            RuntimeType(KindOfInvalid);
          DynLocation* incDecLoc =
            t.newDynLocation(ni->inputs[0]->location, rtt);
          assert(incDecLoc->location.isLocal());
          ni->outLocal = incDecLoc;
          continue; // Doesn't mutate a loc's types for int. Carry on.
        }
        if (op == OpUnsetL) {
          assert(ni->inputs.size() == 1);
          DynLocation* inLoc = ni->inputs[0];
          assert(inLoc->location.isLocal());
          RuntimeType newLhsRtt = RuntimeType(KindOfUninit);
          Location locLocation = inLoc->location;
          SKTRACE(2, ni->source, "(%s, %" PRId64 ") <- type %d\n",
                  locLocation.spaceName(), locLocation.offset,
                  newLhsRtt.valueType());
          DynLocation* unsetLoc = t.newDynLocation(locLocation, newLhsRtt);
          assert(unsetLoc->location.isLocal());
          ni->outLocal = unsetLoc;
          continue;
        }
        if (op == OpStaticLocInit || op == OpInitThisLoc) {
          ni->outLocal = t.newDynLocation(Location(Location::Local,
                                                   ni->imm[0].u_OA),
                                          KindOfInvalid);
          continue;
        }
        if (op == OpSetM || op == OpSetOpM ||
            op == OpVGetM || op == OpBindM ||
            op == OpSetWithRefLM || op == OpSetWithRefRM ||
            op == OpUnsetM || op == OpFPassM) {
          switch (ni->immVec.locationCode()) {
            case LL: {
              const int kVecStart = (op == OpSetM ||
                                     op == OpSetOpM ||
                                     op == OpBindM ||
                                     op == OpSetWithRefLM ||
                                     op == OpSetWithRefRM) ?
                1 : 0; // 0 is rhs for SetM/SetOpM
              DynLocation* inLoc = ni->inputs[kVecStart];
              assert(inLoc->location.isLocal());
              Location locLoc = inLoc->location;
              if (op == OpUnsetM) {
                // UnsetM can change the value of its base local when it's an
                // array. Output a new DynLocation with a the same type to
                // reflect the new value.
                ni->outLocal = t.newDynLocation(locLoc, inLoc->rtt);
              } else if (inLoc->rtt.isString() ||
                  inLoc->rtt.valueType() == KindOfBoolean) {
                // Strings and bools produce value-dependent results; "" and
                // false upgrade to an array successfully, while other values
                // fail and leave the lhs unmodified.
                DynLocation* baseLoc = t.newDynLocation(locLoc, KindOfInvalid);
                assert(baseLoc->isLocal());
                ni->outLocal = baseLoc;
              } else if (inLoc->rtt.valueType() == KindOfUninit ||
                         inLoc->rtt.valueType() == KindOfNull) {
                RuntimeType newLhsRtt = inLoc->rtt.setValueType(
                  mcodeMaybePropName(ni->immVecM[0]) ?
                  KindOfObject : KindOfArray);
                SKTRACE(2, ni->source, "(%s, %" PRId64 ") <- type %d\n",
                        locLoc.spaceName(), locLoc.offset,
                        newLhsRtt.valueType());
                DynLocation* baseLoc = t.newDynLocation(locLoc, newLhsRtt);
                assert(baseLoc->location.isLocal());
                ni->outLocal = baseLoc;
              }
              // Note (if we start translating pseudo-mains):
              //
              // A SetM in pseudo-main might alias a local whose type we're
              // remembering:
              //
              //   $GLOBALS['a'] = 123; // $a :: Int
              //
              // and more deviously:
              //
              //   $loc['b'][17] = $GLOBALS; $x = 'b'; $y = 17;
              //   $loc[$x][$y]['a'] = 123; // $a :: Int
              break;
            }
            case LNL:
            case LNC:
              varEnvTaint = true;
              break;
            case LGL:
            case LGC:
              break;
            default:
              break;
          }
          continue;
        }
        if (op == OpSetOpL) {
          const int kLocIdx = 1;
          DynLocation* inLoc = ni->inputs[kLocIdx];
          assert(inLoc->location.isLocal());
          DynLocation* dl = t.newDynLocation();
          dl->location = inLoc->location;
          dl->rtt = setOpOutputType(ni, ni->inputs);
          if (inLoc->isRef()) {
            dl->rtt = dl->rtt.box();
          }
          SKTRACE(2, ni->source, "(%s, %" PRId64 ") <- type %d\n",
                  inLoc->location.spaceName(), inLoc->location.offset,
                  dl->rtt.valueType());
          assert(dl->location.isLocal());
          ni->outLocal = dl;
          continue;
        }
        if (op >= OpIterInit && op <= OpWIterNextK) {
          assert(op == OpIterInit || op == OpIterInitK ||
                 op == OpMIterInit || op == OpMIterInitK ||
                 op == OpWIterInit || op == OpWIterInitK ||
                 op == OpIterNext || op == OpIterNextK ||
                 op == OpMIterNext || op == OpMIterNextK ||
                 op == OpWIterNext || op == OpWIterNextK);
          const int kValImmIdx = 2;
          const int kKeyImmIdx = 3;
          DynLocation* outVal = t.newDynLocation();
          int off = ni->imm[kValImmIdx].u_IVA;
          outVal->location = Location(Location::Local, off);
          if (op == OpMIterInit || op == OpMIterInitK ||
              op == OpMIterNext || op == OpMIterNextK) {
            outVal->rtt = RuntimeType(KindOfRef, KindOfInvalid);
          } else {
            outVal->rtt = RuntimeType(KindOfInvalid);
          }
          ni->outLocal = outVal;
          if (op == OpIterInitK || op == OpIterNextK ||
              op == OpWIterInitK || op == OpWIterNextK ||
              op == OpMIterInitK || op == OpMIterNextK) {
            DynLocation* outKey = t.newDynLocation();
            int keyOff = getImm((Op*)ni->pc(), kKeyImmIdx).u_IVA;
            outKey->location = Location(Location::Local, keyOff);
            outKey->rtt = RuntimeType(KindOfInvalid);
            ni->outLocal2 = outKey;
          }
          continue;
        }
        assert(ni->inputs.size() == 2);
        const int kValIdx  = 0;
        const int kLocIdx = 1;
        DynLocation* inLoc = ni->inputs[kLocIdx];
        DynLocation* inVal  = ni->inputs[kValIdx];
        Location locLocation = inLoc->location;
        // Variant RHS possible only when binding.
        assert(inVal->rtt.isVagueValue() ||
               (op == OpBindL) ==
               (inVal->rtt.outerType() == KindOfRef));
        assert(!inVal->location.isLocal());
        assert(inLoc->location.isLocal());
        RuntimeType newLhsRtt = inVal->rtt.isVagueValue() || op == OpBindL ?
          inVal->rtt :
          inLoc->rtt.setValueType(inVal->rtt.outerType());
        if (inLoc->rtt.outerType() == KindOfRef) {
          assert(newLhsRtt.outerType() == KindOfRef);
        } else {
          assert(op == OpBindL ||
                 newLhsRtt.outerType() != KindOfRef);
        }
        SKTRACE(2, ni->source, "(%s, %" PRId64 ") <- type %d\n",
                locLocation.spaceName(), locLocation.offset,
                inVal->rtt.valueType());
        DynLocation* outLhsLoc = t.newDynLocation(locLocation, newLhsRtt);
        assert(outLhsLoc->location.isLocal());
        ni->outLocal = outLhsLoc;
      } continue; // already pushed an output for the local

      case Stack1:
      case Stack2: {
        loc = Location(Location::Stack, currentStackOffset++);
        if (ni->op() == OpFPushCufSafe) {
          // FPushCufSafe pushes its first stack input, then a bool.
          if (opnd == Stack2) {
            assert(ni->outStack == nullptr);
            auto* dl = t.newDynLocation(loc, ni->inputs[0]->rtt);
            ni->outStack = dl;
          } else {
            assert(ni->outStack2 == nullptr);
            auto* dl = t.newDynLocation(loc, KindOfBoolean);
            ni->outStack2 = dl;
          }
          continue;
        }
      } break;
      case StackIns1: {
        // First stack output is where the inserted element will go.
        // The output code for the instruction will affect what we
        // think about this location.
        loc = Location(Location::Stack, currentStackOffset++);

        // The existing top is just being moved up a notch.  This one
        // always functions as if it were OutSameAsInput.
        assert(ni->inputs.size() >= 1);
        ni->outStack2 = t.newDynLocation(
          Location(Location::Stack, currentStackOffset++),
          ni->inputs[0]->rtt
        );
      } break;
      case StackIns2: {
        // Similar to StackIns1.
        loc = Location(Location::Stack, currentStackOffset++);

        // Move the top two locations up a slot.
        assert(ni->inputs.size() >= 2);
        ni->outStack2 = t.newDynLocation(
          Location(Location::Stack, currentStackOffset++),
          ni->inputs[1]->rtt
        );
        ni->outStack3 = t.newDynLocation(
          Location(Location::Stack, currentStackOffset++),
          ni->inputs[0]->rtt
        );
      } break;
      default:
        not_reached();
    }
    DynLocation* dl = t.newDynLocation();
    dl->location = loc;
    dl->rtt = getDynLocType(t.m_sk, ni, typeInfo);
    SKTRACE(2, ni->source, "recording output t(%d->%d) #(%s, %" PRId64 ")\n",
            dl->rtt.outerType(), dl->rtt.innerType(),
            dl->location.spaceName(), dl->location.offset);
    assert(dl->location.isStack());
    ni->outStack = dl;
  }
}

void
Translator::requestResetHighLevelTranslator() {
  if (dbgTranslateCoin) {
    dbgTranslateCoin->reset();
  }
}

bool DynLocation::canBeAliased() const {
  return isValue() &&
    ((Translator::liveFrameIsPseudoMain() && isLocal()) || isRef());
}

// Test the type of a location without recording it as a read yet.
RuntimeType TraceletContext::currentType(const Location& l) const {
  DynLocation* dl;
  if (!mapGet(m_currentMap, l, &dl)) {
    assert(!mapContains(m_deletedSet, l));
    assert(!mapContains(m_changeSet, l));
    return tx64->liveType(l, *liveUnit());
  }
  return dl->rtt;
}

DynLocation* TraceletContext::recordRead(const InputInfo& ii,
                                         bool useHHIR,
                                         DataType staticType) {
  DynLocation* dl;
  const Location& l = ii.loc;
  if (!mapGet(m_currentMap, l, &dl)) {
    // We should never try to read a location that has been deleted
    assert(!mapContains(m_deletedSet, l));
    // If the given location was not in m_currentMap, then it shouldn't
    // be in m_changeSet either
    assert(!mapContains(m_changeSet, l));
    if (ii.dontGuard && !l.isLiteral()) {
      assert(!useHHIR || staticType != KindOfRef);
      dl = m_t->newDynLocation(l, RuntimeType(staticType));
      if (useHHIR && staticType != KindOfInvalid) {
        m_resolvedDeps[l] = dl;
      }
    } else {
      // TODO: Once the region translator supports guard relaxation
      //       (task #2598894), we can enable specialization for all modes.
      const bool specialize = tx64->mode() == TransLive;
      RuntimeType rtt = tx64->liveType(l, *liveUnit(), specialize);
      assert(rtt.isIter() || !rtt.isVagueValue());
      // Allocate a new DynLocation to represent this and store it in the
      // current map.
      dl = m_t->newDynLocation(l, rtt);

      if (!l.isLiteral()) {
        if (m_varEnvTaint && dl->isValue() && dl->isLocal()) {
          dl->rtt = RuntimeType(KindOfInvalid);
        } else if ((m_aliasTaint && dl->canBeAliased()) ||
                   (rtt.isValue() && rtt.isRef() && ii.dontGuardInner)) {
          dl->rtt = rtt.setValueType(KindOfInvalid);
        }
        // Record that we depend on the live type of the specified location
        // as well (and remember what the live type was)
        m_dependencies[l] = dl;
      }
    }
    m_currentMap[l] = dl;
  }
  TRACE(2, "recordRead: %s : %s\n", l.pretty().c_str(),
        dl->rtt.pretty().c_str());
  return dl;
}

void TraceletContext::recordWrite(DynLocation* dl) {
  TRACE(2, "recordWrite: %s : %s\n", dl->location.pretty().c_str(),
                                     dl->rtt.pretty().c_str());
  m_currentMap[dl->location] = dl;
  m_changeSet.insert(dl->location);
  m_deletedSet.erase(dl->location);
}

void TraceletContext::recordDelete(const Location& l) {
  // We should not be trying to delete the rtt of location that is
  // not in m_currentMap
  TRACE(2, "recordDelete: %s\n", l.pretty().c_str());
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
      TRACE(1, "(%s, %" PRId64 ") <- inner type invalidated\n",
            it->first.spaceName(), it->first.offset);
      RuntimeType newRtt = dl->rtt.setValueType(KindOfInvalid);
      it->second = m_t->newDynLocation(dl->location, newRtt);
    }
  }
}

void TraceletContext::varEnvTaint() {
  m_varEnvTaint = true;
  for (ChangeMap::iterator it = m_currentMap.begin();
       it != m_currentMap.end(); ++it) {
    DynLocation* dl = it->second;
    if (dl->isValue() && dl->isLocal()) {
      TRACE(1, "(%s, %" PRId64 ") <- type invalidated\n",
            it->first.spaceName(), it->first.offset);
      it->second = m_t->newDynLocation(dl->location,
                                       RuntimeType(KindOfInvalid));
    }
  }
}

void TraceletContext::recordJmp() {
  m_numJmps++;
}

void Translator::postAnalyze(NormalizedInstruction* ni, SrcKey& sk,
                             Tracelet& t, TraceletContext& tas) {
  if (ni->op() == OpBareThis &&
      ni->outStack->rtt.isVagueValue()) {
    SrcKey src = sk;
    const Unit* unit = ni->m_unit;
    src.advance(unit);
    Op next = toOp(*unit->at(src.offset()));
    if (next == OpInstanceOfD || next == OpIsNullC) {
      ni->outStack->rtt = RuntimeType(KindOfObject);
    }
    return;
  }
}

static bool isPop(const NormalizedInstruction* instr) {
  auto opc = instr->op();
  return (opc == OpPopC ||
          opc == OpPopV ||
          opc == OpPopR);
}

GuardType::GuardType(DataType outer, DataType inner)
  : outerType(outer), innerType(inner), klass(nullptr) {
}

GuardType::GuardType(const RuntimeType& rtt) {
  assert(rtt.isValue());
  outerType = rtt.outerType();
  innerType = rtt.innerType();
  klass = rtt.hasKnownType() ? rtt.knownClass() : nullptr;
}

GuardType::GuardType(const GuardType& other) {
  *this = other;
}


const DataType GuardType::getOuterType() const {
  return outerType;
}

const DataType GuardType::getInnerType() const {
  return innerType;
}

const Class* GuardType::getSpecializedClass() const {
  return klass;
}

bool GuardType::isSpecific() const {
  return outerType > KindOfInvalid;
}

bool GuardType::isSpecialized() const {
  return outerType == KindOfObject && klass != nullptr;
}

bool GuardType::isRelaxed() const {
  switch (outerType) {
    case KindOfAny:
    case KindOfUncounted:
    case KindOfUncountedInit:
      return true;
    default:
      return false;
  }
}

bool GuardType::isGeneric() const {
  return outerType == KindOfAny;
}

bool GuardType::isCounted() const {
  switch (outerType) {
    case KindOfAny:
    case KindOfStaticString:
    case KindOfString:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      return true;
    default:
      return false;
  }
}

bool GuardType::isMoreRefinedThan(const GuardType& other) const {
  return getCategory() > other.getCategory();
}

DataTypeCategory GuardType::getCategory() const {
  switch (outerType) {
    case KindOfAny:           return DataTypeGeneric;
    case KindOfUncounted:     return DataTypeCountness;
    case KindOfUncountedInit: return DataTypeCountnessInit;
    default:                  return klass != nullptr ?
                                                DataTypeSpecialized :
                                                DataTypeSpecific;
  }
}

bool GuardType::mayBeUninit() const {
  switch (outerType) {
    case KindOfAny:
    case KindOfUncounted:
    case KindOfUninit:
      return true;
    default:
      return false;
  }
}

GuardType GuardType::getCountness() const {
  // Note that translations need to be able to handle KindOfString and
  // KindOfStaticString interchangeably.  This implies that KindOfStaticString
  // needs to be treated as KindOfString, i.e. as possibly counted.
  assert(isSpecific());
  switch (outerType) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:  return GuardType(KindOfUncounted);
    default:            return GuardType(outerType, innerType);
  }
}

GuardType GuardType::dropSpecialization() const {
  return GuardType(outerType, innerType);
}

RuntimeType GuardType::getRuntimeType() const {
  if (klass != nullptr) {
    return RuntimeType(outerType, innerType).setKnownClass(klass);
  }
  return RuntimeType(outerType, innerType);
}

bool GuardType::isEqual(GuardType other) const {
  return outerType == other.outerType &&
         innerType == other.innerType &&
         klass == other.klass;
}

GuardType GuardType::getCountnessInit() const {
  assert(isSpecific());
  switch (outerType) {
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:  return GuardType(KindOfUncountedInit);
    default:            return GuardType(outerType, innerType);
  }
}


/**
 * Returns true iff loc is consumed by a Pop* instruction in the sequence
 * starting at instr.
 */
bool isPopped(DynLocation* loc, NormalizedInstruction* instr) {
  for (; instr ; instr = instr->next) {
    for (size_t i = 0; i < instr->inputs.size(); i++) {
      if (instr->inputs[i] == loc) {
        return isPop(instr);
      }
    }
  }
  return false;
}

DataTypeCategory
Translator::getOperandConstraintCategory(NormalizedInstruction* instr,
                                         size_t opndIdx,
                                         const GuardType& specType) {
  auto opc = instr->op();

  switch (opc) {
    case OpSetS:
    case OpSetG:
    case OpSetL: {
      if (opndIdx == 0) { // stack value
        // If the output on the stack is simply popped, then we don't
        // even care whether the type is ref-counted or not because
        // the ref-count is transfered to the target location.
        if (!instr->outStack || isPopped(instr->outStack, instr->next)) {
          return DataTypeGeneric;
        }
        return DataTypeCountness;
      }
      if (opc == OpSetL) {
        // old local value is dec-refed
        assert(opndIdx == 1);
        return DataTypeCountness;
      }
      return DataTypeSpecific;
    }

    case OpCGetL:
      return DataTypeCountnessInit;

    case OpRetC:
    case OpRetV:
      return DataTypeCountness;

    case OpFCall:
      // Note: instead of pessimizing calls that may be inlined with
      // DataTypeSpecific, we could apply the operand constraints of
      // the callee in constrainDep.
      return (instr->calleeTrace && !instr->calleeTrace->m_inliningFailed)
               ? DataTypeSpecific
               : DataTypeGeneric;

    case OpFCallArray:
      return DataTypeGeneric;

    case OpPopC:
    case OpPopV:
    case OpPopR:
      return DataTypeCountness;

    case OpContSuspend:
    case OpContSuspendK:
    case OpContRetC:
      // The stack input is teleported to the continuation's m_value field
      return DataTypeGeneric;

    case OpContHandle:
      // This always calls the interpreter
      return DataTypeGeneric;

    case OpAddElemC:
      // The stack input is teleported to the array
      return opndIdx == 0 ? DataTypeGeneric : DataTypeSpecific;

    case OpArrayIdx:
      // The default value (w/ opndIdx 0) is simply passed to a helper,
      // which takes care of dec-refing it if needed
      return opndIdx == 0 ? DataTypeGeneric : DataTypeSpecific;

    //
    // Collections and Iterator related specializations
    //
    case OpCGetM:
    case OpIssetM:
    case OpFPassM:
      if (instr->inputs.size() == 2 && opndIdx == 0) {
        const Class* klass = specType.getSpecializedClass();
        if (klass != nullptr && isOptimizableCollectionClass(klass)) {
          return DataTypeSpecialized;
        }
      }
      return DataTypeSpecific;
    case OpSetM:
      if (instr->inputs.size() == 3 && opndIdx == 1) {
        const Class* klass = specType.getSpecializedClass();
        if (klass != nullptr && isOptimizableCollectionClass(klass)) {
          return DataTypeSpecialized;
        }
      }
      return DataTypeSpecific;

    default:
      return DataTypeSpecific;
  }
}

GuardType
Translator::getOperandConstraintType(NormalizedInstruction* instr,
                                     size_t                 opndIdx,
                                     const GuardType&       specType) {
  DataTypeCategory dtCategory = getOperandConstraintCategory(instr,
                                                             opndIdx,
                                                             specType);
  switch (dtCategory) {
    case DataTypeGeneric:       return GuardType(KindOfAny);
    case DataTypeCountness:     return specType.getCountness();
    case DataTypeCountnessInit: return specType.getCountnessInit();
    case DataTypeSpecific:      return specType.dropSpecialization();
    case DataTypeSpecialized:
                                return specType;
  }
  return specType;
}

void Translator::constrainOperandType(GuardType&             relxType,
                                      NormalizedInstruction* instr,
                                      size_t                 opndIdx,
                                      const GuardType&       specType) {
  if (relxType.isEqual(specType)) return; // Can't constrain any further

  GuardType consType = getOperandConstraintType(instr, opndIdx, specType);
  if (consType.isMoreRefinedThan(relxType)) {
    relxType = consType;
  }
}

/*
 * This method looks at every use of loc in the stream of instructions
 * starting at firstInstr and constrains the relxType towards specType
 * according to each use.  Note that this method not only looks at
 * direct uses of loc, but it also recursively looks at any other
 * DynLocs whose type depends on loc's type.
 */
void Translator::constrainDep(const DynLocation* loc,
                              NormalizedInstruction* firstInstr,
                              GuardType specType,
                              GuardType& relxType) {
  if (relxType.isEqual(specType)) return; // can't contrain it any further

  for (NormalizedInstruction* instr = firstInstr; instr; instr = instr->next) {
    if (instr->noOp) continue;
    auto opc = instr->op();
    size_t nInputs = instr->inputs.size();
    for (size_t i = 0; i < nInputs; i++) {
      DynLocation* usedLoc = instr->inputs[i];
      if (usedLoc == loc) {
        constrainOperandType(relxType, instr, i, specType);

        // If the instruction's input doesn't propagate to its output,
        // then we're done.  Otherwise, we need to constrain relxType
        // based on the uses of the output.
        if (!outputDependsOnInput(opc)) continue;

        bool outputIsStackInput = false;
        const DynLocation* outStack = instr->outStack;
        const DynLocation* outLocal = instr->outLocal;

        switch (instrInfo[opc].type) {
          case OutSameAsInput:
            outputIsStackInput = true;
            break;

          case OutCInput:
            outputIsStackInput = true;
            // fall-through
          case OutCInputL:
            if (specType.getOuterType() == KindOfRef &&
                instr->isAnyOutputUsed()) {
              // Value gets unboxed along the way. Pessimize it for now.
              if (!relxType.isSpecialized()) {
                relxType = specType.dropSpecialization();
              }
              return;
            }
            break;

          default:
            if (!relxType.isSpecialized()) {
              relxType = specType.dropSpecialization();
            }
            return;
        }

        // The instruction input's type propagates to the outputs.
        // So constrain the dependence further based on uses of outputs.
        if ((i == 0           &&  outputIsStackInput) || // stack input @ [0]
            (i == nInputs - 1 && !outputIsStackInput)) { // local input is last
          if (outStack && !outStack->rtt.isVagueValue()) {
            // For SetL, getOperandConstraintCategory() generates
            // DataTypeGeneric if the stack output is popped.  In this
            // case, don't further constrain the stack output,
            // otherwise the Pop* would make it a DataTypeCountness.
            if (opc != OpSetL || !relxType.isGeneric()) {
              constrainDep(outStack, instr->next, specType, relxType);
            }
          }
          if (outLocal && !outLocal->rtt.isVagueValue()) {
            constrainDep(outLocal, instr->next, specType, relxType);
          }
        }
      }
    }
  }
}

/*
 * This method looks at all the uses of the tracelet dependencies in the
 * instruction stream and tries to relax the type associated with each location.
 */
void Translator::relaxDeps(Tracelet& tclet, TraceletContext& tctxt) {
  DynLocTypeMap locRelxTypeMap;

  // Initialize type maps.  Relaxed types start off very relaxed, and then
  // they may get more specific depending on how the instructions use them.
  DepMap& deps = tctxt.m_dependencies;
  for (auto depIt = deps.begin(); depIt != deps.end(); depIt++) {
    DynLocation*       loc = depIt->second;
    const RuntimeType& rtt = depIt->second->rtt;
    if (rtt.isValue() && !rtt.isVagueValue() && !rtt.isClass() &&
        !loc->location.isThis()) {
      GuardType relxType = GuardType(KindOfAny);
      GuardType specType = GuardType(rtt);
      constrainDep(loc, tclet.m_instrStream.first, specType, relxType);
      if (!specType.isEqual(relxType)) {
        locRelxTypeMap[loc] = relxType;
      }
    }
  }

  // For each dependency, if we found a more relaxed type for it, use
  // such type.
  for (auto& kv : locRelxTypeMap) {
    DynLocation* loc = kv.first;
    const GuardType& relxType = kv.second;
    TRACE(1, "relaxDeps: Loc: %s   oldType: %s   =>   newType: %s\n",
          loc->location.pretty().c_str(),
          deps[loc->location]->rtt.pretty().c_str(),
          RuntimeType(relxType.getOuterType(),
                      relxType.getInnerType(),
                      relxType.getSpecializedClass()).pretty().c_str());
    assert(deps[loc->location] == loc);
    assert(relxType.getOuterType() != KindOfInvalid);
    deps[loc->location]->rtt = relxType.getRuntimeType();
  }
}

bool callDestroysLocals(const NormalizedInstruction& inst,
                        const Func* caller) {
  auto* unit = caller->unit();
  auto checkTaintId = [&](Id id) {
    static const StringData* s_extract = StringData::GetStaticString("extract");
    return unit->lookupLitstrId(id)->isame(s_extract);
  };

  if (inst.op() == OpFCallBuiltin) return checkTaintId(inst.imm[2].u_SA);
  if (!isFCallStar(inst.op()))     return false;

  const FPIEnt *fpi = caller->findFPI(inst.source.offset());
  assert(fpi);
  Op* fpushPc = (Op*)unit->at(fpi->m_fpushOff);
  auto const op = *fpushPc;

  if (op == OpFPushFunc)  return true;
  if (op == OpFPushFuncD) return checkTaintId(getImm(fpushPc, 1).u_SA);
  if (op == OpFPushFuncU) {
    return checkTaintId(getImm(fpushPc, 1).u_SA) ||
           checkTaintId(getImm(fpushPc, 2).u_SA);
  }

  return false;
}

/*
 * Check whether the a given FCall should be analyzed for possible
 * inlining or not.
 */
bool shouldAnalyzeCallee(const NormalizedInstruction* fcall,
                         const FPIEnt* fpi,
                         const Op pushOp,
                         const int depth) {
  auto const numArgs = fcall->imm[0].u_IVA;
  auto const target  = fcall->funcd;

  if (!RuntimeOption::RepoAuthoritative) return false;

  if (pushOp != OpFPushFuncD && pushOp != OpFPushObjMethodD
      && pushOp != OpFPushCtorD && pushOp != OpFPushCtor
      && pushOp != OpFPushClsMethodD) {
    FTRACE(1, "analyzeCallee: push op ({}) was not supported\n",
           opcodeToName(pushOp));
    return false;
  }

  if (!target) {
    FTRACE(1, "analyzeCallee: target func not known\n");
    return false;
  }
  if (target->info()) {
    FTRACE(1, "analyzeCallee: target func is a builtin\n");
    return false;
  }

  constexpr int kMaxSubtraceAnalysisDepth = 2;
  if (depth + 1 >= kMaxSubtraceAnalysisDepth) {
    FTRACE(1, "analyzeCallee: max inlining depth reached\n");
    return false;
  }

  if (numArgs != target->numParams()) {
    FTRACE(1, "analyzeCallee: param count mismatch {} != {}\n",
           numArgs, target->numParams());
    return false;
  }
  if (target->numLocals() != target->numParams()) {
    FTRACE(1, "analyzeCallee: not inlining functions with more locals "
              "than params\n");
    return false;
  }

  if (pushOp == OpFPushClsMethodD && target->mayHaveThis()) {
    FTRACE(1, "analyzeCallee: not inlining static calls which may have a "
              "this pointer\n");
    return false;
  }

  // Find the fpush and ensure it's in this tracelet---refuse to
  // inline if there are any calls in order to prepare arguments.
  for (auto* ni = fcall->prev; ni; ni = ni->prev) {
    if (ni->source.offset() == fpi->m_fpushOff) {
      return true;
    }
    if (isFCallStar(ni->op()) || ni->op() == OpFCallBuiltin) {
      FTRACE(1, "analyzeCallee: fpi region contained other calls\n");
      return false;
    }
  }
  FTRACE(1, "analyzeCallee: push instruction was in a different "
            "tracelet\n");
  return false;
}

void Translator::analyzeCallee(TraceletContext& tas,
                               Tracelet& parent,
                               NormalizedInstruction* fcall) {
  auto const callerFunc  = fcall->func();
  auto const fpi         = callerFunc->findFPI(fcall->source.offset());
  auto const pushOp      = fcall->m_unit->getOpcode(fpi->m_fpushOff);

  if (!shouldAnalyzeCallee(fcall, fpi, pushOp, analysisDepth())) return;

  auto const numArgs     = fcall->imm[0].u_IVA;
  auto const target      = fcall->funcd;

  /*
   * Prepare a map for all the known information about the argument
   * types.
   *
   * Also, fill out KindOfUninit for any remaining locals.  The point
   * here is that the subtrace can't call liveType for a local or
   * stack location (since our ActRec is fake), so we need them all in
   * the TraceletContext.
   *
   * If any of the argument types are unknown (including inner-types
   * of KindOfRefs), we don't really try to analyze the callee.  It
   * might be possible to do this but we'll need to modify the
   * analyzer to support unknown input types before there are any
   * NormalizedInstructions in the Tracelet.
   */
  TypeMap initialMap;
  LocationSet callerArgLocs;
  for (int i = 0; i < numArgs; ++i) {
    auto callerLoc = Location(Location::Stack, fcall->stackOffset - i - 1);
    auto calleeLoc = Location(Location::Local, numArgs - i - 1);
    auto type      = tas.currentType(callerLoc);

    callerArgLocs.insert(callerLoc);

    if (type.isVagueValue()) {
      FTRACE(1, "analyzeCallee: {} has unknown type\n", callerLoc.pretty());
      return;
    }
    if (type.isValue() && type.isRef() &&
        type.innerType() == KindOfInvalid) {
      FTRACE(1, "analyzeCallee: {} has unknown inner-refdata type\n",
             callerLoc.pretty());
      return;
    }

    FTRACE(2, "mapping arg{} locs {} -> {} :: {}\n",
              numArgs - i - 1,
              callerLoc.pretty(),
              calleeLoc.pretty(),
              type.pretty());
    initialMap[calleeLoc] = type;
  }
  for (int i = numArgs; i < target->numLocals(); ++i) {
    initialMap[Location(Location::Local, i)] = RuntimeType(KindOfUninit);
  }

  /*
   * When reentering analyze to generate a Tracelet for a callee,
   * currently we handle this by creating a fake ActRec on the stack.
   *
   * This is mostly a compromise to deal with existing code during the
   * analysis phase which pretty liberally inspects live VM state.
   */
  ActRec fakeAR;
  fakeAR.m_savedRbp = reinterpret_cast<uintptr_t>(liveFrame());
  fakeAR.m_savedRip = 0xbaabaa;  // should never be inspected
  fakeAR.m_func = fcall->funcd;
  fakeAR.m_soff = 0xb00b00;      // should never be inspected
  fakeAR.m_numArgsAndCtorFlag = numArgs;
  fakeAR.m_varEnv = nullptr;

  /*
   * Even when inlining an object method, we can leave the m_this as
   * null.  See outThisObjectType().
   */
  fakeAR.m_this = nullptr;

  FTRACE(1, "analyzing sub trace =================================\n");
  auto const oldFP = vmfp();
  auto const oldSP = vmsp();
  auto const oldPC = vmpc();
  auto const oldAnalyzeCalleeDepth = m_analysisDepth++;
  vmpc() = nullptr; // should never be used
  vmsp() = nullptr; // should never be used
  vmfp() = reinterpret_cast<Cell*>(&fakeAR);
  auto restoreFrame = [&]{
    vmfp() = oldFP;
    vmsp() = oldSP;
    vmpc() = oldPC;
    m_analysisDepth = oldAnalyzeCalleeDepth;
  };
  SCOPE_EXIT {
    // It's ok to restoreFrame() twice---we have it in this scope
    // handler to ensure it still happens if we exit via an exception.
    restoreFrame();
    FTRACE(1, "finished sub trace ===================================\n");
  };

  auto subTrace = analyze(SrcKey(target, target->base()), initialMap);

  /*
   * Verify the target trace actually ended with a return, or we have
   * no business doing anything based on it right now.
   */
  if (!subTrace->m_instrStream.last ||
      (subTrace->m_instrStream.last->op() != OpRetC &&
       subTrace->m_instrStream.last->op() != OpRetV)) {
    FTRACE(1, "analyzeCallee: callee did not end in a return\n");
    return;
  }

  /*
   * If the IR can't inline this, give up now.  Below we're going to
   * start making changes to the traclet that is making the call
   * (potentially increasing the specificity of guards), and we don't
   * want to do that unnecessarily.
   */
  if (!JIT::shouldIRInline(callerFunc, target, *subTrace)) {
    if (UNLIKELY(Stats::enabledAny() && getenv("HHVM_STATS_FAILEDINL"))) {
      subTrace->m_inliningFailed = true;
      // Save the trace for stats purposes but don't waste time doing any
      // further processing since we know we won't inline it.
      fcall->calleeTrace = std::move(subTrace);
    }
    return;
  }

  /*
   * Disabled for now:
   *
   * Propagate the return type to our caller.  If the return type is
   * not vague, it will hold if we can inline the trace.
   *
   * This isn't really a sensible thing to do if we aren't also going
   * to inline the callee, however, because the return type may only
   * be what it is due to other output predictions (CGetMs or FCall)
   * inside the callee.  This means we would need to check the return
   * value in the caller still as if it were a predicted return type.
   */
  Location retVal(Location::Stack, 0);
  auto it = subTrace->m_changes.find(retVal);
  assert(it != subTrace->m_changes.end());
  FTRACE(1, "subtrace return: {}\n", it->second->pretty());
  if (false) {
    if (!it->second->rtt.isVagueValue() && !it->second->rtt.isRef()) {
      FTRACE(1, "changing callee's return type from {} to {}\n",
                fcall->outStack->rtt.pretty(),
                it->second->pretty());

      fcall->outputPredicted = true;
      fcall->outputPredictionStatic = false;
      fcall->outStack = parent.newDynLocation(fcall->outStack->location,
                                              it->second->rtt);
      tas.recordWrite(fcall->outStack);
    }
  }

  /*
   * In order for relaxDeps not to relax guards on some things we may
   * potentially have depended on here, we need to ensure that the
   * call instruction depends on all the inputs we've used.
   *
   * (We could do better by letting relaxDeps look through the
   * callee.)
   */
  restoreFrame();
  for (auto& loc : callerArgLocs) {
    fcall->inputs.push_back(tas.recordRead(InputInfo(loc), true));
  }

  FTRACE(1, "analyzeCallee: inline candidate\n");
  fcall->calleeTrace = std::move(subTrace);
}

static bool instrBreaksProfileBB(const NormalizedInstruction* instr) {
  return (instrIsNonCallControlFlow(instr->op()) ||
          instr->outputPredicted ||
          instr->op() == OpClsCnsD); // side exits if misses in the target cache
}

/*
 * analyze --
 *
 *   Given a sequence of bytecodes, return our tracelet IR.
 *
 * The purposes of this analysis is to determine:
 *
 *  1. Pre-conditions: What locations get read before they get written to:
 *     we will need typechecks for these and we will want to load them into
 *     registers. (m_dependencies)
 *
 *  2. Post-conditions: the locations that have been written to and are
 *     still live at the end of the tracelet. We need to allocate registers
 *     of these and we need to spill them at the end of the tracelet.
 *     (m_changes)
 *
 *  3. Determine the runtime types for each instruction's input locations
 *     and output locations.
 *
 * The main analysis works by doing a single pass over the instructions. It
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
 * instruction executes into the various output fields.
 */
std::unique_ptr<Tracelet> Translator::analyze(SrcKey sk,
                                              const TypeMap& initialTypes) {
  std::unique_ptr<Tracelet> retval(new Tracelet());
  auto func = sk.func();
  auto unit = sk.unit();
  auto& t = *retval;
  t.m_sk = sk;

  DEBUG_ONLY const char* file = unit->filepath()->data();
  DEBUG_ONLY const int lineNum = unit->getLineNumber(t.m_sk.offset());
  DEBUG_ONLY const char* funcName = func->fullName()->data();

  TRACE(1, "Translator::analyze %s:%d %s\n", file, lineNum, funcName);
  TraceletContext tas(&t, initialTypes);
  int stackFrameOffset = 0;
  int oldStackFrameOffset = 0;

  // numOpcodes counts the original number of opcodes in a tracelet
  // before the translator does any optimization
  t.m_numOpcodes = 0;
  Unit::MetaHandle metaHand;

  for (;; sk.advance(unit)) {
  head:
    NormalizedInstruction* ni = t.newNormalizedInstruction();
    ni->source = sk;
    ni->stackOffset = stackFrameOffset;
    ni->funcd = t.m_arState.knownFunc();
    ni->m_unit = unit;
    ni->breaksTracelet = false;
    ni->changesPC = opcodeChangesPC(ni->op());
    ni->fuseBranch = false;

    assert(!t.m_analysisFailed);
    oldStackFrameOffset = stackFrameOffset;
    populateImmediates(*ni);

    SKTRACE(1, sk, "stack args: virtual sfo now %d\n", stackFrameOffset);

    // Translation could fail entirely (because of an unknown opcode), or
    // encounter an input that cannot be computed.
    try {
      preInputApplyMetaData(metaHand, ni);
      InputInfos inputInfos;
      getInputsImpl(t.m_sk, ni, stackFrameOffset, inputInfos, sk.func(),
      [&](int i) {
        return Type::fromRuntimeType(
          tas.currentType(Location(Location::Local, i)));
      });
      bool noOp = applyInputMetaData(metaHand, ni, tas, inputInfos);
      if (noOp) {
        t.m_instrStream.append(ni);
        ++t.m_numOpcodes;
        stackFrameOffset = oldStackFrameOffset;
        continue;
      }
      if (inputInfos.needsRefCheck) {
        // Drive the arState machine; if it is going to throw an input
        // exception, do so here.
        int argNum = ni->imm[0].u_IVA;
        // instrSpToArDelta() returns the delta relative to the sp at the
        // beginning of the instruction, but checkByRef() wants the delta
        // relative to the sp at the beginning of the tracelet, so we adjust
        // by subtracting ni->stackOff
        int entryArDelta = instrSpToArDelta((Op*)ni->pc()) - ni->stackOffset;
        ni->preppedByRef = t.m_arState.checkByRef(argNum, entryArDelta,
                                                    &t.m_refDeps);
        SKTRACE(1, sk, "passing arg%d by %s\n", argNum,
                ni->preppedByRef ? "reference" : "value");
      }

      for (unsigned int i = 0; i < inputInfos.size(); i++) {
        SKTRACE(2, sk, "typing input %d\n", i);
        const InputInfo& ii = inputInfos[i];
        DynLocation* dl = tas.recordRead(ii, true);
        const RuntimeType& rtt = dl->rtt;
        // Some instructions are able to handle an input with an unknown type
        if (!ii.dontBreak && !ii.dontGuard) {
          if (rtt.isVagueValue()) {
            // Consumed a "poisoned" output: e.g., result of an array
            // deref.
            throwUnknownInput();
          }
          if (!ni->ignoreInnerType && !ii.dontGuardInner) {
            if (rtt.isValue() && rtt.isRef() &&
                rtt.innerType() == KindOfInvalid) {
              throwUnknownInput();
            }
          }
          if ((m_mode == TransProfile || m_mode == TransOptimize) &&
              t.m_numOpcodes > 0) {
            // We want to break blocks at every instrution that consumes a ref,
            // so that we avoid side exits.  Therefore, instructions consume ref
            // can only be the first in the tracelet/block.
            if (rtt.isValue() && rtt.isRef()) {
              throwUnknownInput();
            }
          }
        }
        ni->inputs.push_back(dl);
      }
    } catch (TranslationFailedExc& tfe) {
      SKTRACE(1, sk, "Translator fail: %s:%d\n", tfe.m_file, tfe.m_line);
      if (!t.m_numOpcodes) {
        t.m_analysisFailed = true;
        t.m_instrStream.append(ni);
        ++t.m_numOpcodes;
      }
      goto breakBB;
    } catch (UnknownInputExc& uie) {
      // Subtle: if this instruction consumes an unknown runtime type,
      // break the BB on the *previous* instruction. We know that a
      // previous instruction exists, because the KindOfInvalid must
      // have come from somewhere.
      always_assert(t.m_instrStream.last);
      SKTRACE(2, sk, "Consumed unknown input (%s:%d); breaking BB at "
        "predecessor\n", uie.m_file, uie.m_line);
      goto breakBB;
    }

    SKTRACE(2, sk, "stack args: virtual sfo now %d\n", stackFrameOffset);

    bool doVarEnvTaint; // initialized by reference.
    try {
      getOutputs(t, ni, stackFrameOffset, doVarEnvTaint);
    } catch (TranslationFailedExc& tfe) {
      SKTRACE(1, sk, "Translator getOutputs fail: %s:%d\n",
              tfe.m_file, tfe.m_line);
      if (!t.m_numOpcodes) {
        t.m_analysisFailed = true;
        t.m_instrStream.append(ni);
        ++t.m_numOpcodes;
      }
      goto breakBB;
    }

    if (isFCallStar(ni->op())) t.m_arState.pop();
    if (doVarEnvTaint || callDestroysLocals(*ni, func)) tas.varEnvTaint();

    DynLocation* outputs[] = { ni->outStack,
                               ni->outLocal, ni->outLocal2,
                               ni->outStack2, ni->outStack3 };
    for (size_t i = 0; i < sizeof(outputs) / sizeof(*outputs); ++i) {
      if (outputs[i]) {
        DynLocation* o = outputs[i];
        SKTRACE(2, sk, "inserting output t(%d->%d) #(%s, %" PRId64 ")\n",
                o->rtt.outerType(), o->rtt.innerType(),
                o->location.spaceName(), o->location.offset);
        tas.recordWrite(o);
      }
    }
    if (ni->op() == OpCreateCont) {
      // CreateCont stores Uninit to all locals but NormalizedInstruction
      // doesn't have enough output fields, so we special case it here.
      auto const numLocals = ni->func()->numLocals();
      for (unsigned i = 0; i < numLocals; ++i) {
        tas.recordWrite(t.newDynLocation(Location(Location::Local, i),
                                         KindOfUninit));
      }
    }

    SKTRACE(1, sk, "stack args: virtual sfo now %d\n", stackFrameOffset);

    // This assert failing means that your instruction has an
    // inconsistent row in the InstrInfo table; the stackDelta doesn't
    // agree with the inputs and outputs.
    assert(getStackDelta(*ni) == (stackFrameOffset - oldStackFrameOffset));
    // If this instruction decreased the depth of the stack, mark the
    // appropriate stack locations as "dead".  But we need to leave
    // them in the TraceletContext until after analyzeCallee (if this
    // is an FCall).
    if (stackFrameOffset < oldStackFrameOffset) {
      for (int i = stackFrameOffset; i < oldStackFrameOffset; ++i) {
        ni->deadLocs.push_back(Location(Location::Stack, i));
      }
    }

    if (ni->outputPredicted) {
      assert(ni->outStack);
      ni->outPred = Type::fromDynLocation(ni->outStack);
    }

    t.m_stackChange += getStackDelta(*ni);

    t.m_instrStream.append(ni);
    ++t.m_numOpcodes;

    /*
     * The annotation step attempts to track Func*'s associated with
     * given FCalls when the FPush is in a different tracelet.
     *
     * When we're analyzing a callee, we can't do this because we may
     * have class information in some of our RuntimeTypes that is only
     * true because of who the caller was.  (Normally it is only there
     * if it came from static analysis.)
     */
    if (analysisDepth() == 0) {
      annotate(ni);
    }

    if (ni->op() == OpFCall) {
      analyzeCallee(tas, t, ni);
    }

    for (auto& l : ni->deadLocs) {
      tas.recordDelete(l);
    }

    if (m_mode == TransProfile && instrBreaksProfileBB(ni)) {
      SKTRACE(1, sk, "BB broken\n");
      sk.advance(unit);
      goto breakBB;
    }

    // Check if we need to break the tracelet.
    //
    // If we've gotten this far, it mostly boils down to control-flow
    // instructions. However, we'll trace through a few unconditional jmps.
    if (ni->op() == OpJmp &&
        ni->imm[0].u_BA > 0 &&
        tas.m_numJmps < MaxJmpsTracedThrough) {
      // Continue tracing through jumps. To prevent pathologies, only trace
      // through a finite number of forward jumps.
      SKTRACE(1, sk, "greedily continuing through %dth jmp + %d\n",
              tas.m_numJmps, ni->imm[0].u_IA);
      tas.recordJmp();
      sk = SrcKey(func, sk.offset() + ni->imm[0].u_IA);
      goto head; // don't advance sk
    } else if (opcodeBreaksBB(ni->op()) ||
               (dontGuardAnyInputs(ni->op()) && opcodeChangesPC(ni->op()))) {
      SKTRACE(1, sk, "BB broken\n");
      sk.advance(unit);
      goto breakBB;
    }
    postAnalyze(ni, sk, t, tas);
  }
breakBB:
  NormalizedInstruction* ni = t.m_instrStream.last;
  while (ni) {
    // We dont want to end a tracelet with a literal; it will cause the literal
    // to be pushed on the stack, and the next tracelet will have to guard on
    // the type. Similarly, This, Self and Parent will lose type information
    // thats only useful in the following tracelet.
    if (isLiteral(ni->op()) || isThisSelfOrParent(ni->op())) {
      ni = ni->prev;
      continue;
    }
    break;
  }
  if (ni) {
    while (ni != t.m_instrStream.last) {
      t.m_stackChange -= getStackDelta(*t.m_instrStream.last);
      sk = t.m_instrStream.last->source;
      t.m_instrStream.remove(t.m_instrStream.last);
      --t.m_numOpcodes;
    }
  }

  // translateRegion doesn't support guard relaxation/specialization yet
  if (m_mode != TransProfile && m_mode != TransOptimize) {
    relaxDeps(t, tas);
  }

  // Mark the last instruction appropriately
  assert(t.m_instrStream.last);
  t.m_instrStream.last->breaksTracelet = true;
  // Populate t.m_changes, t.intermediates, t.m_dependencies
  t.m_dependencies = tas.m_dependencies;
  t.m_resolvedDeps = tas.m_resolvedDeps;
  t.m_changes.clear();
  LocationSet::iterator it = tas.m_changeSet.begin();
  for (; it != tas.m_changeSet.end(); ++it) {
    t.m_changes[*it] = tas.m_currentMap[*it];
  }

  TRACE(1, "Tracelet done: stack delta %d\n", t.m_stackChange);
  return retval;
}

Translator::Translator()
  : m_resumeHelper(nullptr)
  , m_createdTime(Timer::GetCurrentTimeMicros())
  , m_mode(TransInvalid)
  , m_profData(nullptr)
  , m_analysisDepth(0)
{
  initInstrInfo();
  if (RuntimeOption::EvalJitPGO) {
    m_profData = new ProfData();
  }
}

Translator::~Translator() {
  delete m_profData;
  m_profData = nullptr;
}

Translator*
Translator::Get() {
  return TranslatorX64::Get();
}

bool
Translator::isSrcKeyInBL(const SrcKey& sk) {
  auto unit = sk.unit();
  Lock l(m_dbgBlacklistLock);
  if (m_dbgBLSrcKey.find(sk) != m_dbgBLSrcKey.end()) {
    return true;
  }
  for (PC pc = unit->at(sk.offset()); !opcodeBreaksBB(toOp(*pc));
       pc += instrLen((Op*)pc)) {
    if (m_dbgBLPC.checkPC(pc)) {
      m_dbgBLSrcKey.insert(sk);
      return true;
    }
  }
  return false;
}

void
Translator::clearDbgBL() {
  Lock l(m_dbgBlacklistLock);
  m_dbgBLSrcKey.clear();
  m_dbgBLPC.clear();
}

bool
Translator::addDbgBLPC(PC pc) {
  Lock l(m_dbgBlacklistLock);
  if (m_dbgBLPC.checkPC(pc)) {
    // already there
    return false;
  }
  m_dbgBLPC.addPC(pc);
  return true;
}

void populateImmediates(NormalizedInstruction& inst) {
  for (int i = 0; i < numImmediates(inst.op()); i++) {
    inst.imm[i] = getImm((Op*)inst.pc(), i);
  }
  if (hasImmVector(toOp(*inst.pc()))) {
    inst.immVec = getImmVector((Op*)inst.pc());
  }
  if (inst.op() == OpFCallArray) {
    inst.imm[0].u_IVA = 1;
  }
}

const char* Translator::translateResultName(TranslateResult r) {
  static const char* const names[] = {
    "Failure",
    "Retry",
    "Success",
  };
  return names[r];
}

/*
 * Similar to applyInputMetaData, but designed to be used during ir
 * generation. Reads and writes types of values using hhbcTrans. This will
 * eventually replace applyInputMetaData.
 */
void readMetaData(Unit::MetaHandle& handle, NormalizedInstruction& inst,
                  HhbcTranslator& hhbcTrans, MetaMode metaMode /* = Normal */) {
  if (!handle.findMeta(inst.unit(), inst.offset())) return;

  Unit::MetaInfo info;
  if (!handle.nextArg(info)) return;
  if (info.m_kind == Unit::MetaInfo::Kind::NopOut) {
    inst.noOp = true;
    return;
  }

  /*
   * We need to adjust the indexes in MetaInfo::m_arg if this instruction takes
   * other stack arguments than those related to the MVector.  (For example,
   * the rhs of an assignment.)
   */
  auto const& iInfo = instrInfo[inst.op()];
  if (iInfo.in & AllLocals) {
    /*
     * RetC/RetV dont care about their stack input, but it may have been
     * annotated. Skip it (because RetC/RetV pretend they dont have a stack
     * input).
     */
    return;
  }
  if (iInfo.in == FuncdRef) {
    /*
     * FPassC* pretend to have no inputs
     */
    return;
  }
  const int base = !(iInfo.in & MVector) ? 0 :
                   !(iInfo.in & Stack1) ? 0 :
                   !(iInfo.in & Stack2) ? 1 :
                   !(iInfo.in & Stack3) ? 2 : 3;

  auto stackFilter = [metaMode, &inst](Location loc) {
    if (metaMode == MetaMode::Legacy && loc.space == Location::Stack) {
      loc.offset = -(loc.offset + 1) + inst.stackOffset;
    }
    return loc;
  };

  do {
    SKTRACE(3, inst.source, "considering MetaInfo of kind %d\n", info.m_kind);

    int arg = info.m_arg & Unit::MetaInfo::VectorArg ?
      base + (info.m_arg & ~Unit::MetaInfo::VectorArg) : info.m_arg;
    auto updateType = [&]{
      auto& input = *inst.inputs[arg];
      input.rtt = hhbcTrans.rttFromLocation(stackFilter(input.location));
    };

    switch (info.m_kind) {
      case Unit::MetaInfo::Kind::NoSurprise:
        inst.noSurprise = true;
        break;
      case Unit::MetaInfo::Kind::GuardedCls:
        inst.guardedCls = true;
        break;
      case Unit::MetaInfo::Kind::ArrayCapacity:
        inst.imm[0].u_IVA = info.m_data;
        break;
      case Unit::MetaInfo::Kind::DataTypePredicted: {
        if (metaMode == MetaMode::Legacy) break;
        auto const loc = stackFilter(inst.inputs[arg]->location).toLocation();
        auto const t = Type::fromDataType(DataType(info.m_data));
        auto const offset = inst.source.offset();

        // These 'predictions' mean the type is InitNull or the predicted type,
        // so we assert InitNull | t, then guard t. This allows certain
        // optimizations in the IR.
        hhbcTrans.assertTypeLocation(loc, Type::InitNull | t);
        hhbcTrans.checkTypeLocation(loc, t, offset);
        updateType();
        break;
      }
      case Unit::MetaInfo::Kind::DataTypeInferred: {
        hhbcTrans.assertTypeLocation(
          stackFilter(inst.inputs[arg]->location).toLocation(),
          Type::fromDataType(DataType(info.m_data)));
        updateType();
        break;
      }
      case Unit::MetaInfo::Kind::String: {
        hhbcTrans.assertString(
          stackFilter(inst.inputs[arg]->location).toLocation(),
          inst.unit()->lookupLitstrId(info.m_data));
        updateType();
        break;
      }
      case Unit::MetaInfo::Kind::Class: {
        RuntimeType& rtt = inst.inputs[arg]->rtt;
        if (rtt.valueType() != KindOfObject) {
          continue;
        }

        const StringData* metaName = inst.unit()->lookupLitstrId(info.m_data);
        const StringData* rttName =
          rtt.valueClass() ? rtt.valueClass()->name() : nullptr;
        // The two classes might not be exactly the same, which is ok
        // as long as metaCls is more derived than rttCls.
        Class* metaCls = Unit::lookupUniqueClass(metaName);
        Class* rttCls = rttName ? Unit::lookupUniqueClass(rttName) : nullptr;
        if (metaCls && rttCls && metaCls != rttCls &&
            !metaCls->classof(rttCls)) {
          // Runtime type is more derived
          metaCls = 0;
        }
        if (metaCls && metaCls != rttCls) {
          SKTRACE(1, inst.source, "replacing input %d with a MetaInfo-supplied "
                  "class of %s; old type = %s\n",
                  arg, metaName->data(), rtt.pretty().c_str());
          if (rtt.isRef()) {
            rtt = RuntimeType(KindOfRef, KindOfObject, metaCls);
          } else {
            rtt = RuntimeType(KindOfObject, KindOfInvalid, metaCls);
          }
        }
        break;
      }
      case Unit::MetaInfo::Kind::MVecPropClass: {
        const StringData* metaName = inst.unit()->lookupLitstrId(info.m_data);
        Class* metaCls = Unit::lookupUniqueClass(metaName);
        if (metaCls) {
          inst.immVecClasses[arg] = metaCls;
        }
        break;
      }
      case Unit::MetaInfo::Kind::NopOut:
        // NopOut should always be the first and only annotation
        // and was handled above.
        not_reached();

      case Unit::MetaInfo::Kind::GuardedThis:
      case Unit::MetaInfo::Kind::NonRefCounted:
        // fallthrough; these are handled in preInputApplyMetaData.
      case Unit::MetaInfo::Kind::None:
        break;
    }
  } while (handle.nextArg(info));
}

bool instrMustInterp(const NormalizedInstruction& inst) {
  if (RuntimeOption::EvalJitAlwaysInterpOne) return true;

  switch (inst.op()) {
    // Generate a case for each instruction we support at least partially.
# define CASE(name) case Op::name:
  INSTRS
# undef CASE
# define NOTHING(...) // PSEUDOINSTR_DISPATCH has the cases in it
  PSEUDOINSTR_DISPATCH(NOTHING)
# undef NOTHING
      return false;

    default:
      return true;
  }
}

void Translator::traceStart(Offset bcStartOffset) {
  assert(!m_irTrans);

  FTRACE(1, "{}{:-^40}{}\n",
         color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
         " HHIR during translation ",
         color(ANSI_COLOR_END));

  m_irTrans.reset(new JIT::IRTranslator(
    bcStartOffset, liveSpOff(), liveFunc()));
}

void Translator::traceEnd() {
  m_irTrans->hhbcTrans().end();
  FTRACE(1, "{}{:-^40}{}\n",
         color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
         "",
         color(ANSI_COLOR_END));
}

void Translator::traceFree() {
  FTRACE(1, "HHIR free: arena size: {}\n",
         m_irTrans->hhbcTrans().irFactory().arena().size());
  m_irTrans.reset();
}

Translator::TranslateResult
Translator::translateRegion(const RegionDesc& region,
                            RegionBlacklist& toInterp) {
  typedef JIT::RegionDesc::Block Block;
  FTRACE(1, "translateRegion starting with:\n{}\n", show(region));
  HhbcTranslator& ht = m_irTrans->hhbcTrans();
  assert(!region.blocks.empty());
  const SrcKey startSk = region.blocks.front()->start();

  for (auto b = 0; b < region.blocks.size(); b++) {
    auto const& block = region.blocks[b];
    Unit::MetaHandle metaHand;
    SrcKey sk = block->start();
    const Func* topFunc = nullptr;
    auto typePreds  = makeMapWalker(block->typePreds());
    auto byRefs     = makeMapWalker(block->paramByRefs());
    auto refPreds   = makeMapWalker(block->reffinessPreds());
    auto knownFuncs = makeMapWalker(block->knownFuncs());

    for (unsigned i = 0; i < block->length(); ++i, sk.advance(block->unit())) {
      // Update bcOff here so any guards or assertions from metadata are
      // attributed to this instruction.
      ht.setBcOff(sk.offset(), false);

      // Emit prediction guards. If this is the first instruction in the
      // region the guards will go to a retranslate request. Otherwise, they'll
      // go to a side exit.
      bool isFirstRegionInstr = block == region.blocks.front() && i == 0;
      while (typePreds.hasNext(sk)) {
        auto const& pred = typePreds.next();
        auto type = pred.type;
        auto loc  = pred.location;
        if (type.subtypeOf(Type::Cls)) {
          // Do not generate guards for class; instead assert the type
          assert(loc.tag() == JIT::RegionDesc::Location::Tag::Stack);
          ht.assertTypeLocation(loc, type);
        } else if (isFirstRegionInstr) {
          ht.guardTypeLocation(loc, type);
        } else {
          ht.checkTypeLocation(loc, type, sk.offset());
        }
      }

      // Emit reffiness guards. For now, we only support reffiness guards at
      // the beginning of the region.
      while (refPreds.hasNext(sk)) {
        assert(sk == startSk);
        auto const& pred = refPreds.next();
        ht.guardRefs(pred.arSpOffset, pred.mask, pred.vals);
      }

      if (RuntimeOption::EvalJitTransCounters && isFirstRegionInstr) {
        ht.emitIncTransCounter();
      }

      // Update the current funcd, if we have a new one.
      if (knownFuncs.hasNext(sk)) {
        topFunc = knownFuncs.next();
      }

      // Create and initialize the instruction.
      NormalizedInstruction inst;
      inst.source = sk;
      inst.m_unit = block->unit();
      inst.breaksTracelet =
        i == block->length() - 1 && block == region.blocks.back();
      inst.changesPC = opcodeChangesPC(inst.op());
      inst.funcd = topFunc;
      inst.nextOffset = kInvalidOffset;
      if (instrIsNonCallControlFlow(inst.op()) && !inst.breaksTracelet) {
        assert(b < region.blocks.size());
        inst.nextOffset = region.blocks[b+1]->start().offset();
      }
      inst.outputPredicted = false;
      populateImmediates(inst);

      // If this block ends with an inlined FCall, we don't emit anything for
      // the FCall and instead set up HhbcTranslator for inlining. Blocks from
      // the callee will be next in the region.
      if (i == block->length() - 1 &&
          inst.op() == OpFCall && block->inlinedCallee()) {
        auto const* callee = block->inlinedCallee();
        FTRACE(1, "\nstarting inlined call from {} to {} with {} args "
               "and stack:\n{}\n",
               block->func()->fullName()->data(),
               callee->fullName()->data(),
               inst.imm[0].u_IVA,
               ht.showStack());
        auto returnSk = inst.nextSk();
        auto returnFuncOff = returnSk.offset() - block->func()->base();
        ht.beginInlining(inst.imm[0].u_IVA, callee, returnFuncOff);
        continue;
      }

      // We can get a more precise output type for interpOne if we know all of
      // its inputs, so we still populate the rest of the instruction even if
      // this is true.
      inst.interp = toInterp.count(sk);

      // Apply the first round of metadata from the repo and get a list of
      // input locations.
      preInputApplyMetaData(metaHand, &inst);

      InputInfos inputInfos;
      getInputs(startSk, inst, inputInfos, block->func(), [&](int i) {
          return ht.traceBuilder()->getLocalType(i);
        });

      // Populate the NormalizedInstruction's input vector, using types from
      // HhbcTranslator.
      std::vector<DynLocation> dynLocs;
      dynLocs.reserve(inputInfos.size());
      auto newDynLoc = [&](const InputInfo& ii) {
        dynLocs.emplace_back(ii.loc, ht.rttFromLocation(ii.loc));
        FTRACE(2, "rttFromLocation: {} -> {}\n",
               ii.loc.pretty(), dynLocs.back().rtt.pretty());
        return &dynLocs.back();
      };
      FTRACE(2, "populating inputs for {}\n", inst.toString());
      for (auto const& ii : inputInfos) {
        inst.inputs.push_back(newDynLoc(ii));
      }

      // Apply the remaining metadata. This may change the types of some of
      // inst's inputs.
      readMetaData(metaHand, inst, ht);
      if (!inst.noOp && inputInfos.needsRefCheck) {
        assert(byRefs.hasNext(sk));
        inst.preppedByRef = byRefs.next();
      }

      // Check for a type prediction. Put it in the NormalizedInstruction so
      // the emit* method can use it if needed.
      auto const doPrediction = outputIsPredicted(startSk, inst);

      // Emit IR for the body of the instruction.
      try {
        m_irTrans->translateInstr(inst);
      } catch (const JIT::FailedIRGen& exn) {
        FTRACE(1, "ir generation for {} failed with {}\n",
               inst.toString(), exn.what());
        always_assert(!toInterp.count(sk));
        toInterp.insert(sk);
        return Retry;
      }

      // Check the prediction. If the predicted type is less specific than what
      // is currently on the eval stack, checkTypeLocation won't emit any code.
      if (doPrediction) {
        ht.checkTypeStack(0, inst.outPred,
                          sk.advanced(block->unit()).offset());
      }
    }

    assert(!typePreds.hasNext());
    assert(!byRefs.hasNext());
    assert(!refPreds.hasNext());
    assert(!knownFuncs.hasNext());
  }

  traceEnd();
  try {
    traceCodeGen();
  } catch (const JIT::FailedCodeGen& exn) {
    FTRACE(1, "code generation failed with {}\n", exn.what());
    SrcKey sk{exn.vmFunc, exn.bcOff};
    always_assert(!toInterp.count(sk));
    toInterp.insert(sk);
    return Retry;
  }

  return Success;
}

uint64_t* Translator::getTransCounterAddr() {
  if (!isTransDBEnabled()) return nullptr;

  TransID id = m_translations.size();

  // allocate a new chunk of counters if necessary
  if (id >= m_transCounters.size() * transCountersPerChunk) {
    uint32_t   size = sizeof(uint64_t) * transCountersPerChunk;
    auto *chunk = (uint64_t*)malloc(size);
    bzero(chunk, size);
    m_transCounters.push_back(chunk);
  }
  assert(id / transCountersPerChunk < m_transCounters.size());
  return &(m_transCounters[id / transCountersPerChunk]
           [id % transCountersPerChunk]);
}

void Translator::addTranslation(const TransRec& transRec) {
  if (Trace::moduleEnabledRelease(Trace::trans, 1)) {
    // Log the translation's size, creation time, SrcKey, and size
    Trace::traceRelease("New translation: %" PRId64 " %s %u %u %d\n",
                        Timer::GetCurrentTimeMicros() - m_createdTime,
                        folly::format("{}:{}:{}",
                          transRec.src.unit()->filepath()->data(),
                          transRec.src.getFuncId(),
                          transRec.src.offset()).str().c_str(),
                        transRec.aLen,
                        transRec.astubsLen,
                        transRec.kind);
  }

  if (!isTransDBEnabled()) return;
  uint32_t id = getCurrentTransID();
  m_translations.push_back(transRec);
  m_translations[id].setID(id);

  if (transRec.aLen > 0) {
    m_transDB[transRec.aStart] = id;
  }
  if (transRec.astubsLen > 0) {
    m_transDB[transRec.astubsStart] = id;
  }
}

uint64_t Translator::getTransCounter(TransID transId) const {
  if (!isTransDBEnabled()) return -1ul;
  assert(transId < m_translations.size());

  uint64_t counter;

  if (transId / transCountersPerChunk >= m_transCounters.size()) {
    counter = 0;
  } else {
    counter =  m_transCounters[transId / transCountersPerChunk]
                              [transId % transCountersPerChunk];
  }
  return counter;
}

void Translator::setTransCounter(TransID transId, uint64_t value) {
  assert(transId < m_translations.size());
  assert(transId / transCountersPerChunk < m_transCounters.size());

  m_transCounters[transId / transCountersPerChunk]
                 [transId % transCountersPerChunk] = value;
}

namespace {

struct DeferredPathInvalidate : public DeferredWorkItem {
  const std::string m_path;
  explicit DeferredPathInvalidate(const std::string& path) : m_path(path) {
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
    (void)g_vmContext->lookupPhpFile(spath.get(), "");
  }
};

}

void Translator::invalidateFile(Eval::PhpFile* f) {
  m_srcDB.invalidateCode(f);
}

static const char *transKindStr[] = {
#define DO(KIND) #KIND,
  TRANS_KINDS
#undef DO
};

const char *getTransKindName(TransKind kind) {
  assert(kind >= 0 && kind < TransInvalid);
  return transKindStr[kind];
}

TransRec::TransRec(SrcKey                   s,
                   MD5                      _md5,
                   TransKind                _kind,
                   const Tracelet&          t,
                   TCA                      _aStart,
                   uint32_t                 _aLen,
                   TCA                      _astubsStart,
                   uint32_t                 _astubsLen,
                   TCA                      _counterStart,
                   uint8_t                  _counterLen,
                   vector<TransBCMapping>   _bcMapping) :
    id(0), kind(_kind), src(s), md5(_md5),
    bcStopOffset(t.nextSk().offset()), aStart(_aStart), aLen(_aLen),
    astubsStart(_astubsStart), astubsLen(_astubsLen),
    counterStart(_counterStart), counterLen(_counterLen),
    bcMapping(_bcMapping) {
  for (DepMap::const_iterator dep = t.m_dependencies.begin();
       dep != t.m_dependencies.end();
       ++dep) {
    dependencies.push_back(*dep->second);
  }
}

string
TransRec::print(uint64_t profCount) const {
  const size_t kBufSize = 1000;
  static char formatBuf[kBufSize];

  snprintf(formatBuf, kBufSize,
           "Translation %d {\n"
           "  src.md5 = %s\n"
           "  src.funcId = %u\n"
           "  src.startOffset = 0x%x\n"
           "  src.stopOffset = 0x%x\n"
           "  kind = %u (%s)\n"
           "  aStart = %p\n"
           "  aLen = 0x%x\n"
           "  stubStart = %p\n"
           "  stubLen = 0x%x\n"
           "  profCount = %" PRIu64 "\n"
           "  bcMapping = %lu\n",
           id, md5.toString().c_str(), src.getFuncId(), src.offset(),
           bcStopOffset, kind, getTransKindName(kind), aStart, aLen,
           astubsStart, astubsLen, profCount, bcMapping.size());

  string ret(formatBuf);

  for (size_t i = 0; i < bcMapping.size(); i++) {
    snprintf(formatBuf, kBufSize, "    0x%x %p %p\n",
             bcMapping[i].bcStart,
             bcMapping[i].aStart,
             bcMapping[i].astubsStart);

    ret += string(formatBuf);
  }

  ret += "}\n\n";
  return ret;
}

void
ActRecState::pushFunc(const NormalizedInstruction& inst) {
  assert(isFPush(inst.op()));
  if (inst.op() == OpFPushFuncD || inst.op() == OpFPushFuncU) {
    const Unit& unit = *inst.unit();
    Id funcId = inst.imm[1].u_SA;
    auto const& nep = unit.lookupNamedEntityPairId(funcId);
    auto const func = Unit::lookupFunc(nep.second);
    if (func) func->validate();
    if (func && func->isNameBindingImmutable(&unit)) {
      pushFuncD(func);
      return;
    }
  }
  pushDynFunc();
}

void
ActRecState::pushFuncD(const Func* func) {
  TRACE(2, "ActRecState: pushStatic func %p(%s)\n", func, func->name()->data());
  func->validate();
  Record r;
  r.m_state = State::KNOWN;
  r.m_topFunc = func;
  r.m_entryArDelta = InvalidEntryArDelta;
  m_arStack.push_back(r);
}

void
ActRecState::pushDynFunc() {
  TRACE(2, "ActRecState: pushDynFunc\n");
  Record r;
  r.m_state = State::UNKNOWABLE;
  r.m_topFunc = nullptr;
  r.m_entryArDelta = InvalidEntryArDelta;
  m_arStack.push_back(r);
}

void
ActRecState::pop() {
  if (!m_arStack.empty()) {
    m_arStack.pop_back();
  }
}

/**
 * checkByRef() returns true if the parameter specified by argNum is pass
 * by reference, otherwise it returns false. This function may also throw an
 * UnknownInputException if the reffiness cannot be determined.
 *
 * Note that the 'entryArDelta' parameter specifies the delta between sp at
 * the beginning of the tracelet and ar.
 */
bool
ActRecState::checkByRef(int argNum, int entryArDelta, RefDeps* refDeps) {
  FTRACE(2, "ActRecState: getting reffiness for arg {}, arDelta {}\n",
         argNum, entryArDelta);
  if (m_arStack.empty()) {
    // The ActRec in question was pushed before the beginning of the
    // tracelet, so we can make a guess about parameter reffiness and
    // record our assumptions about parameter reffiness as tracelet
    // guards.
    const ActRec* ar = arFromSpOffset((ActRec*)vmsp(), entryArDelta);
    Record r;
    r.m_state = State::GUESSABLE;
    r.m_entryArDelta = entryArDelta;
    ar->m_func->validate();
    r.m_topFunc = ar->m_func;
    m_arStack.push_back(r);
  }
  Record& r = m_arStack.back();
  if (r.m_state == State::UNKNOWABLE) {
    TRACE(2, "ActRecState: unknowable, throwing in the towel\n");
    throwUnknownInput();
    not_reached();
  }
  assert(r.m_topFunc);
  bool retval = r.m_topFunc->byRef(argNum);
  if (r.m_state == State::GUESSABLE) {
    assert(r.m_entryArDelta != InvalidEntryArDelta);
    TRACE(2, "ActRecState: guessing arg%d -> %d\n", argNum, retval);
    refDeps->addDep(r.m_entryArDelta, argNum, retval);
  }
  return retval;
}

const Func*
ActRecState::knownFunc() {
  if (currentState() != State::KNOWN) return nullptr;
  assert(!m_arStack.empty());
  return m_arStack.back().m_topFunc;
}

ActRecState::State
ActRecState::currentState() {
  if (m_arStack.empty()) return State::GUESSABLE;
  return m_arStack.back().m_state;
}

const Func* lookupImmutableMethod(const Class* cls, const StringData* name,
                                  bool& magicCall, bool staticLookup) {
  if (!cls || RuntimeOption::EvalJitEnableRenameFunction) return nullptr;
  if (cls->attrs() & AttrInterface) return nullptr;
  bool privateOnly = false;
  if (!RuntimeOption::RepoAuthoritative ||
      !(cls->preClass()->attrs() & AttrUnique)) {
    Class* ctx = liveFunc()->cls();
    if (!ctx || !ctx->classof(cls)) {
      return nullptr;
    }
    if (!staticLookup) privateOnly = true;
  }

  const Func* func;
  MethodLookup::LookupResult res = staticLookup ?
    g_vmContext->lookupClsMethod(func, cls, name, 0,
                                 g_vmContext->getFP(), false) :
    g_vmContext->lookupObjMethod(func, cls, name, false);

  if (res == MethodLookup::LookupResult::MethodNotFound) return nullptr;

  assert(res == MethodLookup::LookupResult::MethodFoundWithThis ||
         res == MethodLookup::LookupResult::MethodFoundNoThis ||
         (staticLookup ?
          res == MethodLookup::LookupResult::MagicCallStaticFound :
          res == MethodLookup::LookupResult::MagicCallFound));

  magicCall =
    res == MethodLookup::LookupResult::MagicCallStaticFound ||
    res == MethodLookup::LookupResult::MagicCallFound;

  if ((privateOnly && (!(func->attrs() & AttrPrivate) || magicCall)) ||
      func->isAbstract() ||
      func->attrs() & AttrDynamicInvoke) {
    return nullptr;
  }

  if (staticLookup) {
    if (magicCall) {
      /*
       *  i) We cant tell if a magic call would go to __call or __callStatic
       *       - Could deal with this by checking for the existence of __call
       *
       * ii) hphp semantics is that in the case of an object call, we look
       *     for __call in the scope of the object (this is incompatible
       *     with zend) which means we would have to know that there is no
       *     __call higher up in the tree
       *       - Could deal with this by checking for AttrNoOverride on the
       *         class
       */
      func = nullptr;
    }
  } else if (!(func->attrs() & AttrPrivate)) {
    if (magicCall || func->attrs() & AttrStatic) {
      if (!(cls->preClass()->attrs() & AttrNoOverride)) {
        func = nullptr;
      }
    } else if (!(func->attrs() & AttrNoOverride && !func->hasStaticLocals()) &&
               !(cls->preClass()->attrs() & AttrNoOverride)) {
      func = nullptr;
    }
  }
  return func;
}

std::string traceletShape(const Tracelet& trace) {
  std::string ret;

  for (auto ni = trace.m_instrStream.first; ni; ni = ni->next) {
    using folly::toAppend;

    toAppend(opcodeToName(ni->op()), &ret);
    if (ni->immVec.isValid()) {
      toAppend(
        "<",
        locationCodeString(ni->immVec.locationCode()),
        &ret);
      for (auto& mc : ni->immVecM) {
        toAppend(" ", memberCodeString(mc), &ret);
      }
      toAppend(">", &ret);
    }
    toAppend(" ", &ret);
  }

  return ret;
}

} // HPHP::Transl

void invalidatePath(const std::string& path) {
  TRACE(1, "invalidatePath: abspath %s\n", path.c_str());
  PendQ::defer(new DeferredPathInvalidate(path));
}

} // HPHP
