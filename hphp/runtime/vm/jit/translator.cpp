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
#include <algorithm>
#include <memory>
#include <utility>

#include "folly/Optional.h"
#include "folly/Conv.h"
#include "folly/MapUtil.h"

#include "hphp/util/trace.h"
#include "hphp/util/map-walker.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/file-repository.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/annotation.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/tracelet.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/pendq.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/runtime.h"

#define KindOfUnknown DontUseKindOfUnknownInThisFile
#define KindOfInvalid DontUseKindOfInvalidInThisFile

namespace {
TRACE_SET_MOD(trans);
}

namespace HPHP {
namespace JIT {

Lease Translator::s_writeLease;

struct TraceletContext {
  TraceletContext() = delete;

  TraceletContext(Tracelet* t, const TypeMap& initialTypes)
    : m_t(t)
    , m_numJmps(0)
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
  bool        m_varEnvTaint;

  RuntimeType currentType(const Location& l) const;
  DynLocation* recordRead(const InputInfo& l,
                          DataType staticType = KindOfAny);
  void recordWrite(DynLocation* dl);
  void recordDelete(const Location& l);
  void recordJmp();
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

// The global Translator object.
Translator* tx;

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
      base = l.space == Location::Stack ? vmsp()
                                        : reinterpret_cast<Cell*>(vmfp());
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
  DataType innerType = KindOfNone;
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
  RuntimeType retval = RuntimeType(outerType, innerType);
  const Class *klass = nullptr;
  if (specialize) {
    // Only record the class/array kind if specialization requested
    if (valueType == KindOfObject) {
      klass = valCell->m_data.pobj->getVMClass();
      if (klass != nullptr && (klass->attrs() & AttrFinal)) {
        retval = retval.setKnownClass(klass);
      }
    } else if (valueType == KindOfArray) {
      ArrayData::ArrayKind arrayKind = valCell->m_data.parr->kind();
      // We currently only benefit from array specialization for packed arrays.
      if (arrayKind == ArrayData::ArrayKind::kPackedKind) {
        retval = retval.setArrayKind(arrayKind);
      }
    }
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
    return RuntimeType(KindOfObject, KindOfNone, ctx);
  }
  return RuntimeType(KindOfObject, KindOfNone);
}

bool Translator::liveFrameIsPseudoMain() {
  ActRec* ar = (ActRec*)vmfp();
  return ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope();
}

static int64_t typeToMask(DataType t) {
  return (t == KindOfAny) ? 1 : (1 << (1 + getDataTypeIndex(t)));
}

struct InferenceRule {
  int64_t mask;
  DataType result;
};

static DataType inferType(const InferenceRule* rules,
                          const std::vector<DynLocation*>& inputs) {
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
  // We return KindOfAny by default if none of the rules applied.
  return KindOfAny;
}

/*
 * Inference rules used for OutArith. These are applied in order
 * row-by-row.
 */

#define TYPE_MASK(name) \
  static const int64_t name ## Mask = typeToMask(KindOf ## name);
TYPE_MASK(Any);
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
  { StringMask | AnyMask, KindOfAny },
  // Default to Int64
  { 0, KindOfInt64 },
};

static const InferenceRule ArithORules[] = {
  // Same rules as ArithRules, but default to KindOfAny
  { DoubleMask, KindOfDouble },
  { ArrayMask, KindOfArray },
  { StringMask | AnyMask, KindOfAny },
  { 0, KindOfAny },
};

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
  { 0, KindOfAny },
};

static RuntimeType bitOpType(DynLocation* a, DynLocation* b) {
  std::vector<DynLocation*> ins;
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

static const int kTooPolyRet = 6;

bool
isNormalPropertyAccess(const NormalizedInstruction& i,
                       int propInput,
                       int objInput) {
  const LocationCode lcode = i.immVec.locationCode();
  return
    i.immVecM.size() == 1 &&
    (lcode == LC || lcode == LL || lcode == LR || lcode == LH) &&
    mcodeIsProp(i.immVecM[0]) &&
    i.inputs[propInput]->isString() &&
    i.inputs[objInput]->valueType() == KindOfObject;
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
    baseClass->declPropRepoAuthType(idx)
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

static folly::Optional<DataType>
predictionForRepoAuthType(RepoAuthType repoTy) {
  using T = RepoAuthType::Tag;
  switch (repoTy.tag()) {
  case T::OptBool:  return KindOfBoolean;
  case T::OptInt:   return KindOfInt64;
  case T::OptDbl:   return KindOfDouble;
  case T::OptRes:   return KindOfResource;

  case T::OptSArr:
  case T::OptArr:
    return KindOfArray;

  case T::OptStr:
  case T::OptSStr:
    return KindOfString;

  case T::OptSubObj:
  case T::OptExactObj:
  case T::OptObj:
    return KindOfObject;

  case T::Bool:
  case T::Uninit:
  case T::InitNull:
  case T::Int:
  case T::Dbl:
  case T::Res:
  case T::Str:
  case T::Arr:
  case T::Obj:
  case T::Null:
  case T::SStr:
  case T::SArr:
  case T::SubObj:
  case T::ExactObj:
  case T::Cell:
  case T::Ref:
  case T::InitUnc:
  case T::Unc:
  case T::InitCell:
  case T::InitGen:
  case T::Gen:
    return folly::none;
  }
  not_reached();
}

static std::pair<DataType,double>
predictMVec(const NormalizedInstruction* ni) {
  auto info = getFinalPropertyOffset(*ni,
                                     ni->func()->cls(),
                                     getMInstrInfo(ni->mInstrOp()));
  if (info.offset != -1) {
    auto const predTy = predictionForRepoAuthType(info.repoAuthType);
    if (predTy) {
      FTRACE(1, "prediction for CGetM prop: {}, hphpc\n",
        static_cast<int>(*predTy));
      return std::make_pair(*predTy, 1.0);
    }
    // If the RepoAuthType converts to an exact data type, there's no
    // point in having a prediction because we know its type with 100%
    // accuracy.  Disable it in that case here.
    if (convertToDataType(info.repoAuthType)) {
      return std::make_pair(KindOfAny, 0.0);
    }
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

  return std::make_pair(KindOfAny, 0.0);
}

/*
 * predictOutputs --
 *
 *   Provide a best guess for the output type of this instruction.
 */
static DataType
predictOutputs(const NormalizedInstruction* ni) {
  if (!RuntimeOption::EvalJitTypePrediction) return KindOfAny;

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

  if (ni->op() == OpCns ||
      ni->op() == OpCnsE ||
      ni->op() == OpCnsU) {
    StringData* sd = ni->m_unit->lookupLitstrId(ni->imm[0].u_SA);
    auto const tv = Unit::lookupCns(sd);
    if (tv) return tv->m_type;
  }

  if (ni->op() == OpMod) {
    // x % 0 returns boolean false, so we don't know for certain, but it's
    // probably an int.
    return KindOfInt64;
  }

  if (ni->op() == OpPow) {
    // int ** int => int, unless result > 2 ** 52, then it's a double
    // anything ** double => double
    // double ** anything => double
    // anything ** anything => int
    auto lhs = ni->inputs[0];
    auto rhs = ni->inputs[1];

    if (lhs->valueType() == KindOfInt64 && rhs->valueType() == KindOfInt64) {
      // Best guess, since overflowing isn't common
      return KindOfInt64;
    }

    if (lhs->valueType() == KindOfDouble || rhs->valueType() == KindOfDouble) {
      return KindOfDouble;
    }

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

  if (ni->op() == OpAbs) {
    if (ni->inputs[0]->valueType() == KindOfDouble) {
      return KindOfDouble;
    }

    // some types can't be converted to integers and will return false here
    if (ni->inputs[0]->valueType() == KindOfArray) {
      return KindOfBoolean;
    }

    // If the type is not numeric we need to convert it to a numeric type,
    // a string can be converted to an Int64 or a Double but most other types
    // will end up being integral.
    return KindOfInt64;
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
     * since MInstrTranslator side exits in all uncommon cases.
     */

    auto const inDt = ni->inputs[0]->rtt.valueType();
    // If the base is a string, the output is probably a string. Unless the
    // member code is MW, then we're either going to fatal or promote the
    // string to an array.
    Type baseType;
    switch (ni->immVec.locationCode()) {
      case LGL: case LGC:
      case LNL: case LNC:
      case LSL: case LSC:
        baseType = Type::Gen;
        break;

      default:
        baseType = Type(ni->inputs[1]->rtt);
    }
    if (baseType <= Type::Str && ni->immVecM.size() == 1) {
      return ni->immVecM[0] == MW ? inDt : KindOfString;
    }

    // Otherwise, it's probably the input type.
    return inDt;
  }

  auto const op = ni->op();
  static const double kAccept = 1.0;
  std::pair<DataType, double> pred = std::make_pair(KindOfAny, 0.0);
  if (op == OpCGetS) {
    const StringData* propName = ni->inputs[1]->rtt.valueStringOrNull();
    if (propName) {
      pred = predictType(TypeProfileKey(TypeProfileKey::StaticPropName,
                                        propName));
      TRACE(1, "prediction for static fields named %s: %d, %f\n",
            propName->data(),
            pred.first,
            pred.second);
    }
  } else if (op == OpCGetM) {
    pred = predictMVec(ni);
  }
  if (pred.second < kAccept) {
    const StringData* const invName
      = ni->op() == Op::FCallD
        ? ni->m_unit->lookupLitstrId(ni->imm[2].u_SA)
        : nullptr;
    if (invName) {
      pred = predictType(TypeProfileKey(TypeProfileKey::MethodName, invName));
      FTRACE(1, "prediction for methods named {}: {}, {:.2}\n",
             invName->data(),
             pred.first,
             pred.second);
    }
  }
  if (pred.second >= kAccept) {
    FTRACE(1, "accepting prediction of type {}\n", pred.first);
    assert(pred.first != KindOfUninit);
    return pred.first;
  }
  return KindOfAny;
}

/**
 * Returns the type of the value a SetOpL will store into the local.
 */
static RuntimeType setOpOutputType(NormalizedInstruction* ni,
                                   const std::vector<DynLocation*>& inputs) {
  assert(inputs.size() == 2);
  const int kValIdx = 0;
  const int kLocIdx = 1;
  auto const op = static_cast<SetOpOp>(ni->imm[1].u_OA);
  DynLocation locLocation(inputs[kLocIdx]->location,
                          inputs[kLocIdx]->rtt.unbox());
  assert(inputs[kLocIdx]->location.isLocal());
  bool isOverflow = true;

  switch (op) {
  case SetOpOp::PlusEqual:
  case SetOpOp::MinusEqual:
  case SetOpOp::MulEqual: {
    isOverflow = false;
    // fallthrough
  }
  case SetOpOp::PlusEqualO:
  case SetOpOp::MinusEqualO:
  case SetOpOp::MulEqualO: {
    // Same as OutArith[O], except we have to fiddle with inputs a bit.
    std::vector<DynLocation*> arithInputs;
    arithInputs.push_back(&locLocation);
    arithInputs.push_back(inputs[kValIdx]);
    auto rules = isOverflow ? ArithORules : ArithRules;
    return RuntimeType(inferType(rules, arithInputs));
  }
  case SetOpOp::ConcatEqual: return RuntimeType(KindOfString);
  case SetOpOp::DivEqual:
  case SetOpOp::PowEqual:
  case SetOpOp::ModEqual:    return RuntimeType(KindOfAny);
  case SetOpOp::AndEqual:
  case SetOpOp::OrEqual:
  case SetOpOp::XorEqual:    return bitOpType(&locLocation, inputs[kValIdx]);
  case SetOpOp::SlEqual:
  case SetOpOp::SrEqual:     return RuntimeType(KindOfInt64);
  }
  not_reached();
}

const StaticString s_wait_handle("WaitHandle");

static RuntimeType
getDynLocType(const SrcKey startSk,
              NormalizedInstruction* ni,
              InstrFlags::OutTypeConstraints constraint,
              TransKind mode,
              int analysisDepth) {
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
    CS(OutUnknown,     KindOfAny); // Subtle interaction with BB-breaking.
    CS(OutFDesc,       KindOfAny); // Unclear if OutFDesc has a purpose.
    CS(OutArray,       KindOfArray);
    CS(OutObject,      KindOfObject);
    CS(OutResource,    KindOfResource);
#undef CS

    case OutIsTypeL: {
      assert(ni->op() == Op::IsTypeL);
      auto const rtt = ni->inputs[0]->rtt;
      if (rtt.isVagueValue() || !rtt.isValue() || analysisDepth == 0) {
        /*
         * Right now we do the same as KindOfBoolean when we're not
         * inlining, because we want to not change tracelet shapes in
         * that case.  (This is part of an optimization to inline
         * functions when can fold control flow.)
         */
        return RuntimeType(KindOfBoolean);
      }
      auto const retVal = [&]() -> folly::Optional<bool> {
        switch (static_cast<IsTypeOp>(ni->imm[1].u_OA)) {
        case IsTypeOp::Null:     return rtt.isNull();
        case IsTypeOp::Bool:     return rtt.isBoolean();
        case IsTypeOp::Int:      return rtt.isInt();
        case IsTypeOp::Dbl:      return rtt.isDouble();
        case IsTypeOp::Str:      return rtt.isString();
        case IsTypeOp::Arr:      return rtt.isArray();
        case IsTypeOp::Obj:      return rtt.isObject();
        case IsTypeOp::Scalar:   return folly::none;
        }
        not_reached();
      }();
      return !retVal ? RuntimeType(KindOfBoolean) : RuntimeType(*retVal);
    }

    case OutCns: {
      // If it's a system constant, burn in its type. Otherwise we have
      // to accept prediction; use the translation-time value, or fall back
      // to the targetcache if none exists.
      StringData* sd = ni->m_unit->lookupLitstrId(ni->imm[0].u_SA);
      assert(sd);
      const TypedValue* tv = Unit::lookupPersistentCns(sd);
      // KindOfUninit means this is a dynamic system constant so we don't know
      // the type. <img src="clowntown.jpg">
      if (tv && tv->m_type != KindOfUninit) {
        return RuntimeType(tv->m_type);
      }
    } // Fall through
    case OutPred: {
      // In TransProfile mode, disable type prediction to avoid side exits.
      auto dt = mode == TransKind::Profile ? KindOfAny : predictOutputs(ni);
      if (dt != KindOfAny) ni->outputPredicted = true;
      return RuntimeType(dt, dt == KindOfRef ? KindOfAny : KindOfNone);
    }

    case OutPredBool: {
      assert(ni->op() == OpInstanceOfD);
      StringData* name = ni->m_unit->lookupLitstrId(ni->imm[0].u_SA);
      assert(name);
      if (s_wait_handle.get()->isame(name)) {
        return RuntimeType(true);
      }
      return RuntimeType(KindOfBoolean);
    }

    case OutClassRef: {
      Op op = Op(ni->op());
      if ((op == OpAGetC && inputs[0]->isString())) {
        const StringData* sd = inputs[0]->rtt.valueString();
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

    case OutNullUninit: {
      assert(ni->op() == OpNullUninit);
      return RuntimeType(KindOfUninit);
    }

    case OutStringImm: {
      assert(ni->op() == OpString);
      StringData* sd = ni->m_unit->lookupLitstrId(ni->imm[0].u_SA);
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
      return RuntimeType(KindOfRef, KindOfAny);
    }

    case OutArith: {
      return RuntimeType(inferType(ArithRules, inputs));
    }
    case OutArithO: {
      return RuntimeType(inferType(ArithORules, inputs));
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
        op == OpBindM || op == OpVerifyRetTypeV || op == OpVerifyRetTypeC ||
        // Dup takes a single element.
        op == OpDup
      );

      const int idx = 0; // all currently supported cases.

      if (debug) {
        if (!inputs[idx]->rtt.isVagueValue()) {
          if (op == OpBindG || op == OpBindN || op == OpBindS ||
              op == OpBindM || op == OpBindL || op == OpVerifyRetTypeV) {
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
      // TODO: instead of KindOfAny this should track the actual
      // type we will get from interping a non-int IncDec.
      return RuntimeType(IS_INT_TYPE(inRtt.valueType()) ?
                         KindOfInt64 : KindOfAny);
    }

    case OutStrlen: {
      auto const& rtt = ni->inputs[0]->rtt;
      return RuntimeType(rtt.isString() ? KindOfInt64 : KindOfAny);
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

    case OutVInput:
    case OutVInputL:
    case OutFInputL:
    case OutFInputR:
      return RuntimeType(KindOfAny);

    case OutFPushCufSafe:
      not_reached();

    case OutNone: not_reached();
  }
  always_assert(false && "Invalid output type constraint");
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
  { OpPopA,        {Stack1,           None,         OutNone,          -1 }},
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
  { OpBoxRNop,     {None,             None,         OutNone,           0 }},
  { OpUnboxR,      {Stack1,           Stack1,       OutCInput,         0 }},
  { OpUnboxRNop,   {None,             None,         OutNone,           0 }},

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
  { OpNewMixedArray,  {None,          Stack1,       OutArray,          1 }},
  { OpNewLikeArrayL,  {None,          Stack1,       OutArray,          1 }},
  { OpNewPackedArray, {StackN,        Stack1,       OutArray,          0 }},
  { OpNewStructArray, {StackN,        Stack1,       OutArray,          0 }},
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
  { OpNameA,       {Stack1,           Stack1,       OutString,         0 }},

  /*** 3. Operator instructions ***/

  /* Binary string */
  { OpConcat,      {StackTop2,        Stack1,       OutString,        -1 }},
  { OpConcatN,     {StackN,           Stack1,       OutString,         0 }},
  /* Arithmetic ops */
  { OpAbs,         {Stack1,           Stack1,       OutPred,           0 }},
  { OpAdd,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpSub,         {StackTop2,        Stack1,       OutArith,         -1 }},
  { OpMul,         {StackTop2,        Stack1,       OutArith,         -1 }},
  /* Arithmetic ops that overflow ints to floats */
  { OpAddO,        {StackTop2,        Stack1,       OutArithO,        -1 }},
  { OpSubO,        {StackTop2,        Stack1,       OutArithO,        -1 }},
  { OpMulO,        {StackTop2,        Stack1,       OutArithO,        -1 }},
  /* Div and mod might return boolean false. Sigh. */
  { OpDiv,         {StackTop2,        Stack1,       OutPred,          -1 }},
  { OpMod,         {StackTop2,        Stack1,       OutPred,          -1 }},
  { OpPow,         {StackTop2,        Stack1,       OutPred,          -1 }},
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
  { OpInstanceOfD, {Stack1,           Stack1,       OutPredBool,       0 }},
  { OpPrint,       {Stack1,           Stack1,       OutInt64,          0 }},
  { OpClone,       {Stack1,           Stack1,       OutObject,         0 }},
  { OpExit,        {Stack1,           Stack1,       OutNull,           0 }},
  { OpFatal,       {Stack1,           None,         OutNone,          -1 }},

  /*** 4. Control flow instructions ***/

  { OpJmp,         {None,             None,         OutNone,           0 }},
  { OpJmpNS,       {None,             None,         OutNone,           0 }},
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
  { OpPushL,       {Local,            Stack1|Local, OutCInputL,        1 }},
  { OpCGetN,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetG,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpCGetS,       {StackTop2,        Stack1,       OutPred,          -1 }},
  { OpCGetM,       {MVector,          Stack1,       OutPred,           1 }},
  { OpVGetL,       {Local,            Stack1|Local, OutVInputL,        1 }},
  { OpVGetN,       {Stack1,           Stack1|Local, OutVUnknown,       0 }},
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
  { OpIsTypeC,     {Stack1|
                    DontGuardStack1,  Stack1,       OutBoolean,        0 }},
  { OpIsTypeL,     {Local,            Stack1,       OutIsTypeL,        1 }},

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
  { OpIncDecM,     {MVector,          Stack1|Local, OutUnknown,        1 }},
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
  { OpFPassVNop,   {None,             None,         OutNone,           0 }},
  { OpFPassV,      {Stack1|FuncdRef,  Stack1,       OutUnknown,        0 }},
  { OpFPassR,      {Stack1|FuncdRef,  Stack1,       OutFInputR,        0 }},
  { OpFPassL,      {Local|FuncdRef,   Stack1,       OutFInputL,        1 }},
  { OpFPassN,      {Stack1|FuncdRef,  Stack1,       OutUnknown,        0 }},
  { OpFPassG,      {Stack1|FuncdRef,  Stack1,       OutUnknown,        0 }},
  { OpFPassS,      {StackTop2|FuncdRef,
                                      Stack1,       OutUnknown,       -1 }},
  { OpFPassM,      {MVector|FuncdRef, Stack1|Local, OutUnknown,        1 }},
  /*
   * FCall is special. Like the Ret* instructions, its manipulation of the
   * runtime stack are outside the boundaries of the tracelet abstraction.
   */
  { OpFCall,       {FStack,           Stack1,       OutPred,           0 }},
  { OpFCallD,      {FStack,           Stack1,       OutPred,           0 }},
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
  { OpDefTypeAlias,{None,             None,         OutNone,           0 }},
  { OpDefCls,      {None,             None,         OutNone,           0 }},
  { OpNopDefCls,   {None,             None,         OutNone,           0 }},
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
                   {Local,            Local,        OutUnknown,        0 }},
  { OpVerifyRetTypeV,
                   {Stack1,           Stack1,       OutSameAsInput,    0 }},
  { OpVerifyRetTypeC,
                   {Stack1,           Stack1,       OutSameAsInput,    0 }},
  { OpOODeclExists,
                   {StackTop2,        Stack1,       OutBoolean,       -1 }},
  { OpSelf,        {None,             Stack1,       OutClassRef,       1 }},
  { OpParent,      {None,             Stack1,       OutClassRef,       1 }},
  { OpLateBoundCls,{None,             Stack1,       OutClassRef,       1 }},
  { OpNativeImpl,  {None,             None,         OutNone,           0 }},
  { OpCreateCl,    {BStackN,          Stack1,       OutObject,         1 }},
  { OpStrlen,      {Stack1,           Stack1,       OutStrlen,         0 }},
  { OpIncStat,     {None,             None,         OutNone,           0 }},
  { OpIdx,         {StackTop3,        Stack1,       OutUnknown,       -2 }},
  { OpArrayIdx,    {StackTop3,        Stack1,       OutUnknown,       -2 }},
  { OpFloor,       {Stack1,           Stack1,       OutDouble,         0 }},
  { OpCeil,        {Stack1,           Stack1,       OutDouble,         0 }},
  { OpCheckProp,   {None,             Stack1,       OutBoolean,        1 }},
  { OpInitProp,    {Stack1,           None,         OutNone,          -1 }},
  { OpSilence,     {Local|DontGuardAny,
                                      Local,        OutNone,           0 }},
  { OpAssertRATL,  {None,             None,         OutNone,           0 }},
  { OpAssertRATStk,{None,             None,         OutNone,           0 }},
  { OpBreakTraceHint,{None,           None,         OutNone,           0 }},

  /*** 14. Generator instructions ***/

  { OpCreateCont,  {None,             Stack1,       OutNull,           1 }},
  { OpContEnter,   {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpContRaise,   {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpYield,       {Stack1,           Stack1,       OutUnknown,        0 }},
  { OpYieldK,      {StackTop2,        Stack1,       OutUnknown,       -1 }},
  { OpContCheck,   {None,             None,         OutNone,           0 }},
  { OpContValid,   {None,             Stack1,       OutBoolean,        1 }},
  { OpContKey,     {None,             Stack1,       OutUnknown,        1 }},
  { OpContCurrent, {None,             Stack1,       OutUnknown,        1 }},

  /*** 15. Async functions instructions ***/

  { OpAwait,       {Stack1,           Stack1,       OutUnknown,        0 }},
};

static hphp_hash_map<Op, InstrInfo> instrInfo;
static bool instrInfoInited;
static void initInstrInfo() {
  if (!instrInfoInited) {
    for (size_t i = 0; i < sizeof(instrInfoSparse) / sizeof(instrInfoSparse[0]);
         i++) {
      instrInfo[instrInfoSparse[i].op] = instrInfoSparse[i].info;
    }
    if (!RuntimeOption::EvalCheckReturnTypeHints) {
      for (size_t j = 0; j < 2; ++j) {
        auto& ii = instrInfo[j == 0 ? OpVerifyRetTypeC : OpVerifyRetTypeV];
        ii.in = ii.out = None;
        ii.type = OutNone;
      }
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
    DontGuardStack1 | IgnoreInnerType | DontGuardAny | This;
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

int64_t getStackPopped(PC pc) {
  auto const op = *reinterpret_cast<const Op*>(pc);
  switch (op) {
    case Op::FCall:        return getImm((Op*)pc, 0).u_IVA + kNumActRecCells;
    case Op::FCallD:       return getImm((Op*)pc, 0).u_IVA + kNumActRecCells;
    case Op::FCallArray:   return kNumActRecCells + 1;

    case Op::NewPackedArray:
    case Op::ConcatN:
    case Op::FCallBuiltin:
    case Op::CreateCl:     return getImm((Op*)pc, 0).u_IVA;

    case Op::NewStructArray: return getImmVector((Op*)pc).size();

    default:             break;
  }

  uint64_t mask = getInstrInfo(op).in;
  int64_t count = 0;

  // All instructions with these properties are handled above
  assert((mask & (StackN | BStackN)) == 0);

  if (mask & MVector) {
    count += getImmVector((Op*)pc).numStackValues();
    mask &= ~MVector;
  }

  return count + countOperands(mask);
}

int64_t getStackPushed(PC pc) {
  return countOperands(getInstrInfo(*reinterpret_cast<const Op*>(pc)).out);
}

int getStackDelta(const NormalizedInstruction& ni) {
  int hiddenStackInputs = 0;
  initInstrInfo();
  auto op = ni.op();
  switch (op) {
    case Op::FCall:
    case Op::FCallD:
      {
        int numArgs = ni.imm[0].u_IVA;
        return 1 - numArgs - kNumActRecCells;
      }

    case Op::NewPackedArray:
    case Op::ConcatN:
    case Op::FCallBuiltin:
    case Op::CreateCl:
      return 1 - ni.imm[0].u_IVA;

    case Op::NewStructArray:
      return 1 - ni.immVec.numStackValues();

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

// Task #3449943: This returns true even if there's meta-data telling
// that the value was inferred.
bool outputIsPredicted(NormalizedInstruction& inst) {
  auto const& iInfo = getInstrInfo(inst.op());
  auto doPrediction =
    (iInfo.type == OutPred || iInfo.type == OutCns) && !inst.breaksTracelet;
  if (doPrediction) {
    // All OutPred ops except for SetM have a single stack output for now.
    assert(iInfo.out == Stack1 || inst.op() == OpSetM);
    auto dt = predictOutputs(&inst);
    if (dt != KindOfAny) {
      inst.outPred = Type(dt, dt == KindOfRef ? KindOfAny : KindOfNone);
      inst.outputPredicted = true;
    } else {
      doPrediction = false;
    }
  }

  return doPrediction;
}

static bool isNullableSpecializedObj(RepoAuthType rat) {
  switch (rat.tag()) {
  case RepoAuthType::Tag::OptExactObj:
  case RepoAuthType::Tag::OptSubObj: return true;
  default:                           return false;
  }
  not_reached();
}

bool isAlwaysNop(Op op) {
  if (isTypeAssert(op)) return true;
  switch (op) {
  case Op::UnboxRNop:
  case Op::BoxRNop:
  case Op::FPassVNop:
  case Op::FPassC:
    return true;
  case Op::VerifyRetTypeC:
  case Op::VerifyRetTypeV:
    return !RuntimeOption::EvalCheckReturnTypeHints;
  default:
    return false;
  }
}

void Translator::handleAssertionEffects(Tracelet& t,
                                        const NormalizedInstruction& ni,
                                        TraceletContext& tas,
                                        int currentStackOffset) {
  auto const loc = [&] {
    switch (ni.op()) {
    case Op::AssertRATL:
      return Location(Location::Local, ni.imm[0].u_LA);
    case Op::AssertRATStk:
      return Location(Location::Stack,
                      currentStackOffset - 1 - ni.imm[0].u_IVA);
    default:
      not_reached();
    }
  }();
  if (loc.isInvalid()) return;

  auto const rat = ni.imm[1].u_RATA;
  auto const assertTy = [&]() -> folly::Optional<RuntimeType> {
    using T = RepoAuthType::Tag;
    switch (rat.tag()) {
    case T::Uninit:   return RuntimeType{KindOfUninit};
    case T::InitNull: return RuntimeType{KindOfNull};
    case T::Int:      return RuntimeType{KindOfInt64};
    case T::Dbl:      return RuntimeType{KindOfDouble};
    case T::Res:      return RuntimeType{KindOfResource};
    case T::Bool:     return RuntimeType{KindOfBoolean};
    case T::SStr:     return RuntimeType{KindOfString};
    case T::Str:      return RuntimeType{KindOfString};
    case T::SArr:     return RuntimeType{KindOfArray};
    case T::Arr:      return RuntimeType{KindOfArray};
    case T::Obj:      return RuntimeType{KindOfObject};

    // We can turn these into information in hhbc-translator but can't
    // really remove guards, since it can be more than one DataType,
    // so don't do anything here.
    case T::OptInt:
    case T::OptDbl:
    case T::OptRes:
    case T::OptBool:
    case T::OptSStr:
    case T::OptStr:
    case T::OptSArr:
    case T::OptArr:
    case T::OptObj:
    case T::Null:    // could be KindOfUninit or KindOfNull
      return folly::none;

    case T::Ref:
      // We should be able to use this to avoid the outer-type guards
      // on KindOfRefs, but for now we don't because of complications
      // with communicating the predicted inner type to
      // hhbc-translator.
      return folly::none;

    // There's really not much we can do with a Cell assertion at
    // translation time, right now.
    case T::Cell:
      return folly::none;

    // Since these don't correspond to data types, there's not much we
    // can do in the current situation.
    case T::InitUnc:
    case T::Unc:
    case T::InitCell:
    case T::InitGen:
    case T::Gen:
      // These could also remove guards, but it's a little too hard to
      // get this information to hhbc-translator with this legacy
      // tracelet stuff since they don't map directly to a DataType.
      return folly::none;

    // Types where clsName() should be non-null
    case T::OptExactObj:
    case T::OptSubObj:
    case T::ExactObj:
    case T::SubObj:
      {
        /*
         * Even though the class must be defined at the point of the
         * AssertObj (unless it's optional), we might not have defined
         * it yet in this tracelet, or it might not be unique.  For
         * now just restrict this to unique classes (we could also
         * check parent of current context).
         *
         * There's nothing we can do with the 'exact' bit right now,
         * since neither analyze() nor the IR typesystem tracks that.
         */
        auto const cls = Unit::lookupUniqueClass(rat.clsName());
        // TODO: why is it returning none instead of KindOfObject if
        // it isn't unique?  (Leaving alone for now to match pre-RAT
        // behavior.)
        if (!cls || !(cls->attrs() & AttrUnique)) return folly::none;
        return RuntimeType{KindOfObject, KindOfNone, cls};
      }
    }
    not_reached();
  }();
  if (!assertTy) return;

  FTRACE(1, "examining assertion for {} :: {}\n", loc.pretty(), show(rat));

  /*
   * TODO(#4205897): handle nullable assert array types so we can turn
   * them on in hhbbc.
   */

  /*
   * For array types, we still want to be able to guard for on
   * specialized array kinds.  So, in these cases we do a normal read
   * (allowing a guard), and write down in resolvedDeps that we know
   * we don't need to guard if the guard is relaxed to KindOfArray.
   * If the guard on a specialized kind is relaxed, resolvedDeps is
   * used to tell the JIT it doesn't need the less-specific guard
   * because we know it statically.
   *
   * Note: currently the JIT will still guard the KindOfArray flag
   * when trying to guard it's packed (or whatever), since there's no
   * way to assert it's at least an array and still require a packed
   * guard.
   */
  if (assertTy->isArray()) {
    if (!tas.m_currentMap.count(loc)) {
      FTRACE(1, "Asserting array type; using resolvedDeps\n");
      tas.recordRead(InputInfo{loc});
      assert(tas.m_currentMap.count(loc));
      tas.m_resolvedDeps[loc] = t.newDynLocation(loc, *assertTy);
      return;
    }
  }

  /*
   * Nullable object assertions are slightly special: we have extra
   * information, but it's conditional on previously guarding that the
   * object wasn't null.  And if it is null in this tracelet, the
   * assertion should have no effects.
   *
   * We need to do a recordRead without setting dontGuard, so we'll
   * still leave a guard.  (Guard relaxation will remove it if nothing
   * ends up using it.)
   *
   * We also shouldn't try to remove predictions like the code below:
   * a prediction guard might be the reason we know it's not null
   * here.
   */
  if (isNullableSpecializedObj(rat)) {
    auto const dl = tas.recordRead(InputInfo{loc});
    if (dl->rtt.isNull()) {
      FTRACE(1, "assertion leaving live KindOfNull alone for ?Obj type\n");
      return;
    }
    if (!dl->isObject()) {
      // We're about to fail a VerifyParamType or something similar.
      FTRACE(1, "not adjusting {} since it's not an object\n",
             dl->pretty());
      return;
    }
    dl->rtt = *assertTy;
    return;
  }

  FTRACE(2, "using general assert effects path\n");
  InputInfo ii{loc};
  ii.dontGuard = true;
  auto const dl = tas.recordRead(ii, assertTy->outerType());

  if (dl->rtt.outerType() != assertTy->outerType() &&
      !(dl->rtt.isString() && assertTy->isString())) {
    if (!dl->rtt.isVagueValue()) {
      /*
       * The live or tracked type disagrees with ahead of time analysis.
       *
       * Either static analysis is wrong (bug), this was a mispredicted
       * type from warmup profiling, or the code is unreachable because
       * we're about to fatal earlier in this tracelet.  (e.g. a
       * VerifyParamType is about to throw.)
       */
      if (tas.m_changeSet.count(dl->location)) {
        auto const src = findInputSrc(tas.m_t->m_instrStream.last, dl);
        if (src && src->outputPredicted) {
          FTRACE(1, "correcting mispredicted type for {}\n", loc.pretty());
          src->outputPredicted = false;
          return;
        }
      }
      FTRACE(1, "punting for {}\n", loc.pretty());
      punt();
    }
  } else {
    /*
     * Static inference confirmed a type we already have, but if the
     * type we have came from profiling we want to clear outputPredicted
     * to avoid unnecessary guards.
     */
    if (tas.m_changeSet.count(dl->location)) {
      auto const src = findInputSrc(tas.m_t->m_instrStream.last, dl);
      if (src && src->outputPredicted) src->outputPredicted = false;
    }
  }

  /*
   * The valueClass fields we track on KindOfObject types basically
   * all come from static analysis, but they can also come from being
   * inside of an analyzeCallee situation and knowing more specific
   * argument types.  It's also possible in principle for a tracelet
   * to extend through a join point, and thereby propagate a better
   * type than one of the type assertions we get from static analysis.
   * A similar case can occur with specialized array types.
   *
   * So, sometimes we may know a more derived type here dynamically,
   * in which case we don't want to modify the RuntimeType and make it
   * worse.
   */
  auto const assertIsSpecializedObj =
    assertTy->isObject() && assertTy->valueClass() != nullptr;
  auto const liveIsSpecializedObj =
    dl->rtt.isObject() && dl->rtt.valueClass() != nullptr;
  auto const liveIsSpecializedArr =
    dl->rtt.isArray() && dl->rtt.hasArrayKind();
  if (assertIsSpecializedObj && liveIsSpecializedObj) {
    auto const assertCls = assertTy->valueClass();
    auto const liveCls = dl->rtt.valueClass();
    if (assertCls != liveCls && !assertCls->classof(liveCls)) {
      // liveCls must be more specialized than assertCls.
      FTRACE(1, "assertion leaving object curVal alone {}\n", loc.pretty());
      return;
    }
  }
  if (assertTy->isArray() && liveIsSpecializedArr) {
    FTRACE(1, "assertion leaving array curVal alone {}\n", loc.pretty());
    return;
  }

  FTRACE(1, "assertion effects {} -> {}\n", loc.pretty(), assertTy->pretty());
  dl->rtt = *assertTy;
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
   */

  /*
   * Also note: if we eventually have immediates that are not local
   * ids (i.e. string ids), this analysis step is going to have to be
   * a bit wiser.
   */
  auto opPtr = (const Op*)ni.source.pc();
  auto const location = getMLocation(opPtr);
  auto const lcode = location.lcode;

  const bool trailingClassRef = lcode == LSL || lcode == LSC;

  switch (numLocationCodeStackVals(lcode)) {
  case 0: {
    if (lcode == LH) {
      inputs.emplace_back(Location(Location::This));
    } else {
      assert(lcode == LL || lcode == LGL || lcode == LNL);
      if (location.hasImm()) {
        push_local(location.imm);
      }
    }
  } break;
  case 1:
    if (lcode == LSL) {
      // We'll get the trailing stack value after pushing all the
      // member vector elements.
      assert(location.hasImm());
      push_local(location.imm);
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
  for (auto const& member : getMVector(opPtr)) {
    auto const mcode = member.mcode;
    ni.immVecM.push_back(mcode);

    if (mcode == MW) {
      // No stack and no locals.
      continue;
    } else if (member.hasImm()) {
      int64_t imm = member.imm;
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

  assert(stackCount == ni.immVec.numStackValues());

  SKTRACE(2, ni.source, "M-vector using %d hidden stack "
                        "inputs, %d locals\n", stackCount, localCount);
}

void getInputs(SrcKey startSk, NormalizedInstruction& inst, InputInfos& infos,
               const Func* func, const LocalTypeFn& localType) {
  // MCGenerator expected top of stack to be index -1, with indexes growing
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
  always_assert_flog(
    instrInfo.count(ni->op()),
    "Invalid opcode in getInputsImpl: {}\n",
    opcodeToName(ni->op())
  );
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
    int numArgs = (ni->op() == Op::NewPackedArray ||
                   ni->op() == Op::ConcatN)
      ? ni->imm[0].u_IVA
      : ni->immVec.numStackValues();

    SKTRACE(1, sk, "getInputs: stackN %d %d\n",
            currentStackOffset - 1, numArgs);
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
    // (Almost) all instructions that take a Local have its index at
    // their first immediate.
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
  }

  auto wantInlineReturn = [&] {
    const int localCount = ni->func()->numLocals();
    // Inline return causes us to guard this tracelet more precisely. If
    // we're already chaining to get here, just do a generic return in the
    // hopes of avoiding further specialization. The localCount constraint
    // is an unfortunate consequence of the current generic machinery not
    // working for 0 locals.
    if (mcg->numTranslations(startSk) >= kTooPolyRet && localCount > 0) {
      return false;
    }
    int numRefCounted = 0;
    for (int i = 0; i < localCount; ++i) {
      if (localType(i).maybeCounted()) {
        numRefCounted++;
      }
    }
    return numRefCounted <= RuntimeOption::EvalHHIRInliningMaxReturnDecRefs;
  };

  if ((input & AllLocals) && wantInlineReturn()) {
    ni->inlineReturn = true;
    ni->ignoreInnerType = true;
    int n = ni->func()->numLocals();
    for (int i = 0; i < n; ++i) {
      inputs.emplace_back(Location(Location::Local, i));
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
    case OutPredBool:
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

    // NB: this sounds like it should have the output depend on the
    // input, but it behaves the same as OutBoolean unless we are
    // inlining, and in that case we don't relaxDeps.
    case OutIsTypeL:
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
    case OutArithO:
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

  const std::vector<DynLocation*>& inputs = ni->inputs;
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
            op == OpBindN || op == OpUnsetN || op == OpVGetN) {
          varEnvTaint = true;
          continue;
        }

        if (op == OpVerifyParamType) {
          assert(ni->inputs.size() == 1);
          auto func = ni->func();
          const auto& inRtt = ni->inputs[0]->rtt;
          auto paramId = ni->imm[0].u_IVA;
          const auto& tc = func->params()[paramId].typeConstraint;
          if (tc.isArray() && !tc.isSoft() && !func->mustBeRef(paramId) &&
              (inRtt.isVagueValue() || inRtt.isObject() || inRtt.isRef())) {
            auto* loc = t.newDynLocation(
              ni->inputs[0]->location,
              inRtt.isRef() ? RuntimeType(KindOfRef, KindOfAny)
                            : RuntimeType(KindOfAny));
            assert(loc->location.isLocal());
            ni->outLocal = loc;
          }
          continue;
        }

        ASSERT_NOT_IMPLEMENTED(op == OpSetOpL ||
                               op == OpSetM || op == OpSetOpM ||
                               op == OpBindM ||
                               op == OpSetWithRefLM || op == OpSetWithRefRM ||
                               op == OpUnsetM ||
                               op == OpIncDecL ||
                               op == OpVGetM || op == OpFPassM ||
                               op == OpIncDecM ||
                               op == OpStaticLocInit || op == OpInitThisLoc ||
                               op == OpSetL || op == OpBindL || op == OpVGetL ||
                               op == OpPushL || op == OpUnsetL ||
                               op == OpIterInit || op == OpIterInitK ||
                               op == OpMIterInit || op == OpMIterInitK ||
                               op == OpWIterInit || op == OpWIterInitK ||
                               op == OpIterNext || op == OpIterNextK ||
                               op == OpMIterNext || op == OpMIterNextK ||
                               op == OpWIterNext || op == OpWIterNextK ||
                               op == OpSilence);
        if (op == OpSilence) {
          switch (static_cast<SilenceOp>(ni->imm[0].u_OA)) {
            case SilenceOp::Start: {
              Location loc{Location::Local, ni->imm[0].u_LA};
              RuntimeType newType = RuntimeType(KindOfInt64);
              ni->outLocal = t.newDynLocation(loc, newType);
              break;
            }
            case SilenceOp::End:
              ni->outLocal = ni->inputs[0];
              break;
          }
          continue;
        }
        if (op == OpFPassM && !ni->preppedByRef) {
          // Equivalent to CGetM. Won't mutate the base.
          continue;
        }
        if (op == OpIncDecL) {
          assert(ni->inputs.size() == 1);
          const RuntimeType &inRtt = ni->inputs[0]->rtt;
          RuntimeType rtt =
            IS_INT_TYPE(inRtt.valueType()) ? inRtt : RuntimeType(KindOfAny);
          DynLocation* incDecLoc =
            t.newDynLocation(ni->inputs[0]->location, rtt);
          assert(incDecLoc->location.isLocal());
          ni->outLocal = incDecLoc;
          continue; // Doesn't mutate a loc's types for int. Carry on.
        }
        if (op == OpUnsetL || op == OpPushL) {
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
                                                   ni->imm[0].u_LA),
                                          KindOfAny);
          continue;
        }
        if (op == OpSetM || op == OpSetOpM ||
            op == OpVGetM || op == OpBindM ||
            op == OpSetWithRefLM || op == OpSetWithRefRM ||
            op == OpUnsetM || op == OpFPassM || op == OpIncDecM) {
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
                DynLocation* baseLoc = t.newDynLocation(locLoc, KindOfAny);
                assert(baseLoc->isLocal());
                ni->outLocal = baseLoc;
              } else if (inLoc->rtt.valueType() == KindOfUninit ||
                         inLoc->rtt.valueType() == KindOfNull) {
                RuntimeType newLhsRtt = inLoc->rtt.setValueType(
                  mcodeIsProp(ni->immVecM[0]) ?
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
            outVal->rtt = RuntimeType(KindOfRef, KindOfAny);
          } else {
            outVal->rtt = RuntimeType(KindOfAny);
          }
          ni->outLocal = outVal;
          if (op == OpIterInitK || op == OpIterNextK ||
              op == OpWIterInitK || op == OpWIterNextK ||
              op == OpMIterInitK || op == OpMIterNextK) {
            DynLocation* outKey = t.newDynLocation();
            int keyOff = getImm((Op*)ni->pc(), kKeyImmIdx).u_IVA;
            outKey->location = Location(Location::Local, keyOff);
            outKey->rtt = RuntimeType(KindOfAny);
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
    dl->rtt = getDynLocType(t.m_sk, ni, typeInfo, m_mode, analysisDepth());
    SKTRACE(2, ni->source, "recording output t(%d->%d) #(%s, %" PRId64 ")\n",
            dl->rtt.outerType(), dl->rtt.innerType(),
            dl->location.spaceName(), dl->location.offset);
    assert(dl->location.isStack());
    ni->outStack = dl;
  }
}

bool DynLocation::canBeAliased() const {
  return isValue() &&
    ((Translator::liveFrameIsPseudoMain() && isLocal()) || isRef());
}

// Test the type of a location without recording it as a read yet.
RuntimeType TraceletContext::currentType(const Location& l) const {
  auto const dl = folly::get_ptr(m_currentMap, l);
  if (!dl) {
    assert(!m_deletedSet.count(l));
    assert(!m_changeSet.count(l));
    return tx->liveType(l, *liveUnit());
  }
  return (*dl)->rtt;
}

DynLocation* TraceletContext::recordRead(const InputInfo& ii,
                                         DataType staticType) {
  if (staticType == KindOfNone) staticType = KindOfAny;

  const Location& l = ii.loc;
  auto dl = folly::get_default(m_currentMap, l, nullptr);
  if (!dl) {
    // We should never try to read a location that has been deleted
    assert(!m_deletedSet.count(l));
    // If the given location was not in m_currentMap, then it shouldn't
    // be in m_changeSet either
    assert(!m_changeSet.count(l));
    if (ii.dontGuard && !l.isLiteral()) {
      assert(staticType != KindOfRef);
      dl = m_t->newDynLocation(l, RuntimeType(staticType));
      if (staticType != KindOfAny) {
        m_resolvedDeps[l] = dl;
      }
    } else {
      const bool specialize = tx->mode() == TransKind::Live &&
        (RuntimeOption::EvalHHBCRelaxGuards ||
         RuntimeOption::EvalHHIRRelaxGuards);

      RuntimeType rtt = tx->liveType(l, *liveUnit(), specialize);
      assert(rtt.isIter() || !rtt.isVagueValue());
      // Allocate a new DynLocation to represent this and store it in the
      // current map.
      dl = m_t->newDynLocation(l, rtt);

      if (!l.isLiteral()) {
        if (m_varEnvTaint && dl->isValue() && dl->isLocal()) {
          dl->rtt = RuntimeType(KindOfAny);
        } else if (rtt.isValue() && rtt.isRef() && ii.dontGuardInner) {
          dl->rtt = rtt.setValueType(KindOfAny);
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

void TraceletContext::varEnvTaint() {
  m_varEnvTaint = true;
  for (ChangeMap::iterator it = m_currentMap.begin();
       it != m_currentMap.end(); ++it) {
    DynLocation* dl = it->second;
    if (dl->isValue() && dl->isLocal()) {
      TRACE(1, "(%s, %" PRId64 ") <- type invalidated\n",
            it->first.spaceName(), it->first.offset);
      it->second = m_t->newDynLocation(dl->location,
                                       RuntimeType(KindOfAny));
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
    Op next = *reinterpret_cast<const Op*>(unit->at(src.offset()));
    if (next == Op::InstanceOfD
          || (next == Op::IsTypeC &&
              ni->imm[0].u_OA == static_cast<uint8_t>(IsTypeOp::Null))) {
      ni->outStack->rtt = RuntimeType(KindOfObject);
    }
    return;
  }
}

static bool isPop(const NormalizedInstruction* instr) {
  auto opc = instr->op();
  return (opc == Op::PopC ||
          opc == Op::PopV ||
          opc == Op::PopR);
}

GuardType::GuardType(DataType outer, DataType inner)
  : outerType(outer), innerType(inner), klass(nullptr) {
}

GuardType::GuardType(const RuntimeType& rtt) {
  assert(rtt.isValue());
  outerType = rtt.outerType();
  innerType = rtt.innerType();
  if (rtt.hasKnownClass()) {
    klass = rtt.knownClass();
  } else if (rtt.hasArrayKind()) {
    arrayKindValid = true;
    arrayKind = rtt.arrayKind();
  } else {
    klass = nullptr;
  }
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
  return outerType > KindOfNone;
}

bool GuardType::isSpecialized() const {
  return (outerType == KindOfObject && klass != nullptr) ||
    (outerType == KindOfArray && arrayKindValid);
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
    default:                  return (klass != nullptr || arrayKindValid) ?
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
  if (outerType == KindOfObject && klass != nullptr) {
    return RuntimeType(outerType, innerType).setKnownClass(klass);
  }
  if (outerType == KindOfArray && arrayKindValid) {
    return RuntimeType(outerType, innerType).setArrayKind(arrayKind);
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

bool GuardType::hasArrayKind() const {
  return arrayKindValid;
}

ArrayData::ArrayKind GuardType::getArrayKind() const {
  return arrayKind;
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

    case OpUnsetL:
      return DataTypeCountness;

    case OpCGetL:
      return DataTypeCountnessInit;

    case OpPushL:
    case OpContEnter:
    case OpContRaise:
      return DataTypeGeneric;

    case OpRetC:
    case OpRetV:
      return DataTypeCountness;

    case OpFCall:
    case OpFCallD:
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

    case OpYield:
    case OpYieldK:
      // The stack input is teleported to the continuation's m_value field
      return DataTypeGeneric;

    case OpAddElemC:
      // The stack input is teleported to the array
      return opndIdx == 0 ? DataTypeGeneric : DataTypeSpecific;

    case OpIdx:
    case OpArrayIdx:
      // The default value (w/ opndIdx 0) is simply passed to a helper,
      // which takes care of dec-refing it if needed
      return opndIdx == 0 ? DataTypeGeneric : DataTypeSpecific;

    //
    // Collections and Iterator related specializations
    //
    case OpFPassM:
      if (instr->preppedByRef) {
        // This is equivalent to a VGetM.
        return DataTypeSpecific;
      }
      // fallthrough
    case OpCGetM:
    case OpIssetM:
      if (specType.getOuterType() == KindOfArray) {
        if (instr->inputs.size() == 2 && opndIdx == 0) {
          if (specType.hasArrayKind() &&
              specType.getArrayKind() == ArrayData::ArrayKind::kPackedKind &&
              instr->inputs[1]->isInt()) {
            return DataTypeSpecialized;
          }
        }
      } else if (specType.getOuterType() == KindOfObject) {
        if (instr->inputs.size() == 2 && opndIdx == 0) {
          const Class* klass = specType.getSpecializedClass();
          if (klass != nullptr && isOptimizableCollectionClass(klass)) {
            return DataTypeSpecialized;
          }
        }
      }
      return DataTypeSpecific;
    case OpSetM:
      if (specType.getOuterType() == KindOfObject) {
        if (instr->inputs.size() == 3 && opndIdx == 1) {
          const Class* klass = specType.getSpecializedClass();
          // Pairs are immutable so special case that here.
          if (klass != nullptr && isOptimizableCollectionClass(klass) &&
              klass != c_Pair::classof()) {
            return DataTypeSpecialized;
          }
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
  FTRACE(4, "got constraint {} for {}\n", dtCategory, *instr);
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
    FTRACE(3, "constraining from {}({}) to {}({})\n",
           relxType.getOuterType(), relxType.getInnerType(),
           consType.getOuterType(), consType.getInnerType());
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

  FTRACE(3, "\nconstraining dep {}\n", loc->pretty());
  for (auto instr = firstInstr; instr; instr = instr->next) {
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
        if (!outputDependsOnInput(opc)) {
          FTRACE(4, "output doesn't propagate to input; stopping\n");
          continue;
        }

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
              FTRACE(3, "constraining outStack dep {}\n", outStack->pretty());
              constrainDep(outStack, instr->next, specType, relxType);
            }
          }

          // PushL has a local output that doesn't depend on the input
          // type but its stack output does, so we special case it here.
          if (outLocal && !outLocal->rtt.isVagueValue() &&
              opc != OpPushL) {
            FTRACE(3, "constraining outLocal dep {}\n", outLocal->pretty());
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
  FTRACE(3, "starting relaxDeps\n");
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
  FTRACE(3, "applying relaxed deps\n");
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
    deps[loc->location]->rtt = relxType.getRuntimeType();
  }
  FTRACE(3, "relaxDeps finished\n");
}

const StaticString s_http_response_header("http_response_header");
const StaticString s_php_errormsg("php_errormsg");
const StaticString s_extract("extract");
const StaticString s_extractNative("__SystemLib\\extract");

bool callDestroysLocals(const NormalizedInstruction& inst,
                        const Func* caller) {
  auto locals = caller->localNames();
  for (int i = 0; i < caller->numNamedLocals(); ++i) {
    if (locals[i]->same(s_http_response_header.get()) ||
        locals[i]->same(s_php_errormsg.get())) {
      return true;
    }
  }

  auto* unit = caller->unit();
  auto checkTaintId = [&](Id id) {
    return unit->lookupLitstrId(id)->isame(s_extract.get())
    || unit->lookupLitstrId(id)->isame(s_extractNative.get());
  };

  if (inst.op() == OpFCallBuiltin) return checkTaintId(inst.imm[2].u_SA);
  if (!isFCallStar(inst.op()))     return false;

  const FPIEnt *fpi = caller->findFPI(inst.source.offset());
  assert(fpi);
  Op* fpushPc = (Op*)unit->at(fpi->m_fpushOff);
  auto const op = *fpushPc;

  if (op == OpFPushFunc) {
    // If the call has any arguments, the FPushFunc will be in a different
    // tracelet -- the tracelet will break on every FPass* because the reffiness
    // of the callee isn't knowable. So we have to say the call destroys locals,
    // to be conservative. If there aren't any arguments, then it can't destroy
    // locals -- even if the call is to extract(), there's no argument, so it
    // won't do anything.
    auto const numArgs = inst.imm[0].u_IVA;
    return (numArgs != 0);
  }
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
 *
 * TODO(2716400): support __call and friends?
 */
bool shouldAnalyzeCallee(const Tracelet& tlet,
                         const NormalizedInstruction* fcall,
                         const FPIEnt* fpi,
                         const Op pushOp,
                         const int depth) {
  auto const numArgs = fcall->imm[0].u_IVA;
  auto const target  = fcall->funcd;

  if (!RuntimeOption::RepoAuthoritative) return false;

  if (liveFrame()->resumed()) {
    // Do not inline from a resumed function
    return false;
  }

  if (pushOp != OpFPushFuncD &&
      pushOp != OpFPushObjMethodD &&
      pushOp != OpFPushCtorD &&
      pushOp != OpFPushCtor &&
      pushOp != OpFPushClsMethodD) {
    FTRACE(1, "analyzeCallee: push op ({}) was not supported\n",
           opcodeToName(pushOp));
    return false;
  }

  if (!target) {
    FTRACE(1, "analyzeCallee: target func not known\n");
    return false;
  }

  if (depth + 1 > RuntimeOption::EvalHHIRInliningMaxDepth) {
    FTRACE(1, "analyzeCallee: max inlining depth reached\n");
    return false;
  }

  if (tlet.m_stackSlackUsedForInlining +
      target->maxStackCells() > kStackCheckLeafPadding) {
    FTRACE(1, "analyzeCallee: could exceed leaf padding "
           "depth (currentUsed={}, needed={})\n",
           tlet.m_stackSlackUsedForInlining,
           target->maxStackCells());
    return false;
  }

  if (target->hasVariadicCaptureParam()) {
    FTRACE(1, "analyzeCallee: target func has variadic capture\n");
    return false;
  }

  if (pushOp == OpFPushClsMethodD && target->mayHaveThis()) {
    FTRACE(1, "analyzeCallee: not inlining static calls which may have a "
              "this pointer\n");
    return false;
  }

  if (numArgs > target->numParams()) {
    FTRACE(1, "analyzeCallee: excessive parameters passed "
              "(passed={}, expected={})\n",
              numArgs,
              target->numParams());
    return false;
  }

  // Find the fpush and ensure it's in this tracelet---refuse to
  // inline if there are any calls in order to prepare arguments.
  for (auto* ni = fcall->prev; ni; ni = ni->prev) {
    if (ni->source.offset() == fpi->m_fpushOff) {
      if (ni->op() == OpFPushObjMethodD ||
          ni->op() == OpFPushObjMethod) {
        if (!ni->inputs[ni->op() == OpFPushObjMethod]->isObject()) {
          /*
           * In this case, we're going to throw or fatal when we
           * execute the FPush*. But we have statically proven that
           * if we get to the FCall, then target is the Func that will
           * be called. So the FCall is unreachable - but unfortunately,
           * various assumptions by the jit will be violated if we try
           * to inline it. So just don't inline in that case.
           */
          return false;
        }
      }
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

  if (!shouldAnalyzeCallee(parent, fcall, fpi, pushOp, analysisDepth())) {
    return;
  }

  auto const numArgs     = fcall->imm[0].u_IVA;
  auto const target      = fcall->funcd;

  /*
   * If the argument count isn't exact, we need to figure out if we
   * can handle the DV init situation before proceeding.
   *
   * If we don't have a dv-init entry point for every argument that
   * isn't passed, we are supposed to raise a warning for the missing
   * arguments.  Since this will involve a catch trace in the IR that
   * keeps the frame alive, just give up if that's going to happen.
   */
  auto const entryOffset = [&]() -> Offset {
    auto const numParams = target->numParams();
    if (numArgs == numParams) return target->base();
    assert(numArgs < numParams);
    auto ret = Offset{kInvalidOffset};
    for (auto i = numArgs; i < numParams; ++i) {
      auto const& param = target->params()[i];
      if (param.hasDefaultValue()) {
        if (ret == kInvalidOffset) {
          ret = param.funcletOff;
        }
      } else {
        FTRACE(1, "analyzeCallee: refusing because we would "
                  "need to emit missing argument warnings");
        return kInvalidOffset;
      }
    }
    return ret;
  }();
  if (entryOffset == kInvalidOffset) return;
  if (entryOffset != target->base()) {
    FTRACE(1, "analyzeCallee: using dvInit at {}\n", entryOffset);
  }

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
    if (type.isValue() && type.isRef() && type.innerType() == KindOfAny) {
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
  fakeAR.m_sfp = liveFrame();
  fakeAR.m_savedRip = 0xbaabaa;  // should never be inspected
  fakeAR.m_func = fcall->funcd;
  fakeAR.m_soff = 0xb00b00;      // should never be inspected
  fakeAR.m_numArgsAndFlags = numArgs;
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
  vmfp() = &fakeAR;
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

  FTRACE(2, "analyzeCallee: slack used so far {}\n", target->maxStackCells());
  auto subTrace = analyze(
    SrcKey(target, entryOffset, false),
    initialMap,
    parent.m_stackSlackUsedForInlining + target->maxStackCells()
  );

  /*
   * Verify the target trace actually ended with something we
   * understand, or we have no business doing anything based on it
   * right now.
   */
  if (!subTrace->m_instrStream.last ||
      (subTrace->m_instrStream.last->op() != Op::RetC &&
       subTrace->m_instrStream.last->op() != Op::RetV &&
       subTrace->m_instrStream.last->op() != Op::CreateCont &&
       subTrace->m_instrStream.last->op() != Op::Await &&
       subTrace->m_instrStream.last->op() != Op::NativeImpl)) {
    FTRACE(1, "analyzeCallee: callee did not end in a supported way\n");
    return;
  }

  /*
   * If the IR can't inline this, give up now.  Below we're going to
   * start making changes to the tracelet that is making the call
   * (potentially increasing the specificity of guards), and we don't
   * want to do that unnecessarily.
   */
  if (!shouldIRInline(callerFunc, target, *subTrace)) {
    if (Stats::enabledAny() && getenv("HHVM_STATS_FAILEDINL")) {
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
  if (!target->isCPPBuiltin()) {
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
  }

  /*
   * In order for relaxDeps not to relax guards on some things we may
   * potentially have depended on here, we need to ensure that the
   * call instruction depends on all the inputs we've used.
   *
   * (We could do better by letting relaxDeps look through the callee.
   * But we won't, because eventually we'll handle it in IR guard
   * relaxation and/or PGO.)
   */
  restoreFrame();
  for (auto& loc : callerArgLocs) {
    fcall->inputs.push_back(tas.recordRead(InputInfo(loc)));
  }

  FTRACE(1, "analyzeCallee: inline candidate\n");
  fcall->calleeTrace = std::move(subTrace);
}

bool instrBreaksProfileBB(const NormalizedInstruction* instr) {
  if (instrIsNonCallControlFlow(instr->op()) ||
      instr->outputPredicted ||
      instr->op() == OpAwait || // may branch to scheduler and suspend execution
      instr->op() == OpClsCnsD) { // side exits if misses in the RDS
    return true;
  }
  // In profiling mode, don't trace through a control flow merge point
  if (instr->func()->anyBlockEndsAt(instr->offset())) {
    return true;
  }
  return false;
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
std::unique_ptr<Tracelet>
Translator::analyze(SrcKey sk,
                    const TypeMap& initialTypes,
                    int32_t const stackSlackUsedForInlining) {
  Timer _t(Timer::analyze);

  std::unique_ptr<Tracelet> retval(new Tracelet());
  retval->m_stackSlackUsedForInlining = stackSlackUsedForInlining;
  auto func = sk.func();
  auto unit = sk.unit();
  auto resumed = sk.resumed();
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

  for (;; sk.advance(unit)) {
  head:
    NormalizedInstruction* ni = t.newNormalizedInstruction();
    ni->source = sk;
    ni->stackOffset = stackFrameOffset;
    ni->funcd = t.m_arState.knownFunc();
    ni->m_unit = unit;
    ni->breaksTracelet = false;
    ni->changesPC = opcodeChangesPC(ni->op());

    assert(!t.m_analysisFailed);
    oldStackFrameOffset = stackFrameOffset;
    populateImmediates(*ni);

    SKTRACE(1, sk, "stack args: virtual sfo now %d\n", stackFrameOffset);

    // Translation could fail entirely (because of an unknown opcode), or
    // encounter an input that cannot be computed.
    try {
      if (isTypeAssert(ni->op())) {
        handleAssertionEffects(t, *ni, tas, stackFrameOffset);
      }
      InputInfos inputInfos;
      getInputsImpl(
        t.m_sk, ni, stackFrameOffset, inputInfos, sk.func(),
        [&](int i) {
          return Type(
            tas.currentType(Location(Location::Local, i)));
        }
      );

      if (isAlwaysNop(ni->op())) {
        ni->noOp = true;
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
        DynLocation* dl = tas.recordRead(ii);
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
                rtt.innerType() == KindOfAny) {
              throwUnknownInput();
            }
          }
          if ((m_mode == TransKind::Profile ||
               m_mode == TransKind::Optimize) &&
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
      SKTRACE(1, sk, "Translator fail: %s\n", tfe.what());
      if (!t.m_numOpcodes) {
        t.m_analysisFailed = true;
        t.m_instrStream.append(ni);
        ++t.m_numOpcodes;
      }
      goto breakBB;
    } catch (UnknownInputExc& uie) {
      // Subtle: if this instruction consumes an unknown runtime type,
      // break the BB on the *previous* instruction. We know that a
      // previous instruction exists, because the KindOfAny must
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
      SKTRACE(1, sk, "Translator getOutputs fail: %s\n", tfe.what());
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
      ni->outPred = Type(ni->outStack);
    }

    t.m_stackChange += getStackDelta(*ni);

    t.m_instrStream.append(ni);
    ++t.m_numOpcodes;

    /*
     * The historically named annotation step attempts to find direct Func*'s
     * for FCallD calls.
     */
    annotate(ni);

    if (ni->op() == Op::FCall || ni->op() == Op::FCallD) {
      analyzeCallee(tas, t, ni);
    }

    for (auto& l : ni->deadLocs) {
      tas.recordDelete(l);
    }

    if (analysisDepth() == 0 &&
        m_mode == TransKind::Profile &&
        instrBreaksProfileBB(ni)) {
      SKTRACE(1, sk, "Profiling BB broken\n");
      sk.advance(unit);
      goto breakBB;
    }

    // Check if we need to break the tracelet.  ///////////

    // If we've gotten this far, it mostly boils down to control-flow
    // instructions. However, we'll trace through a few unconditional jmps.
    if (tas.m_numJmps < MaxJmpsTracedThrough &&
        m_mode != TransKind::Profile) {
      // Continue tracing through jumps. To prevent pathologies, only trace
      // through a finite number of forward jumps.
      if (isUnconditionalJmp(ni->op()) &&
          ni->imm[0].u_BA > 0) {
        // Continue tracing through jumps. To prevent pathologies, only trace
        // through a finite number of forward jumps.
        SKTRACE(1, sk, "greedily continuing through %dth jmp + %d\n",
                tas.m_numJmps, ni->imm[0].u_IA);
        tas.recordJmp();
        ni->nextOffset = sk.offset() + ni->imm[0].u_IA;
        sk = SrcKey(func, ni->nextOffset, resumed);
        goto head; // don't advance sk
      }

      // If there is a Boolean value on stack with a predicted value,
      // trace through JmpZ/JmpNZ to take the likely branch.
      if (isConditionalJmp(ni->op()) && ni->imm[0].u_BA > 0) {
        auto const inputType = ni->inputs[0]->rtt;
        if (inputType.isBoolean() && inputType.valueBoolean() != -1) {
          bool const jmpnz = ni->op() == Op::JmpNZ;
          auto const taken = inputType.valueBoolean() == jmpnz;
          auto const offset = taken ? sk.offset() + ni->imm[0].u_BA
                                    : sk.advanced(unit).offset();

          SKTRACE(1, sk, "continuing through conditional jmp on branch "
                  "%s to %d\n",
                  taken ? "taken" : "next",
                  offset);
          ni->nextOffset = offset;
          tas.recordJmp();
          sk = SrcKey(func, offset, resumed);
          goto head; // don't advance sk
        }
      }
    }

    if (opcodeBreaksBB(ni->op()) ||
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
    if (isLiteral(ni->op()) ||
        isThisSelfOrParent(ni->op()) ||
        isTypeAssert(ni->op())) {
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

  if (RuntimeOption::EvalHHBCRelaxGuards && m_mode == TransKind::Live) {
    relaxDeps(t, tas);
  } else {
    FTRACE(3, "Not relaxing deps. HHBCRelax: {}, mode: {}\n",
           RuntimeOption::EvalHHBCRelaxGuards, show(m_mode));
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
  : uniqueStubs{}
  , m_createdTime(HPHP::Timer::GetCurrentTimeMicros())
  , m_mode(TransKind::Invalid)
  , m_profData(nullptr)
  , m_analysisDepth(0)
  , m_useAHot(RuntimeOption::RepoAuthoritative &&
              RuntimeOption::EvalJitAHotSize > 0)
{
  initInstrInfo();
  if (RuntimeOption::EvalJitPGO) {
    m_profData.reset(new ProfData());
  }
}

bool
Translator::isSrcKeyInBL(const SrcKey& sk) {
  auto unit = sk.unit();
  if (unit->isInterpretOnly()) return true;
  Lock l(m_dbgBlacklistLock);
  if (m_dbgBLSrcKey.find(sk) != m_dbgBLSrcKey.end()) {
    return true;
  }
  for (PC pc = unit->at(sk.offset());
      !opcodeBreaksBB(*reinterpret_cast<const Op*>(pc));
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
  auto offset = 1;
  for (int i = 0; i < numImmediates(inst.op()); ++i) {
    if (immType(inst.op(), i) == RATA) {
      auto rataPc = inst.pc() + offset;
      inst.imm[i].u_RATA = decodeRAT(inst.unit(), rataPc);
    } else {
      inst.imm[i] = getImm(reinterpret_cast<const Op*>(inst.pc()), i);
    }
    offset += immSize(reinterpret_cast<const Op*>(inst.pc()), i);
  }
  if (hasImmVector(*reinterpret_cast<const Op*>(inst.pc()))) {
    inst.immVec = getImmVector(reinterpret_cast<const Op*>(inst.pc()));
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

void Translator::traceStart(TransContext context) {
  assert(!m_irTrans);

  FTRACE(1, "{}{:-^40}{}\n",
         color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
         " HHIR during translation ",
         color(ANSI_COLOR_END));

  m_irTrans.reset(new IRTranslator(context));
}

void Translator::traceEnd() {
  assert(!m_irTrans->hhbcTrans().isInlining());
  m_irTrans->hhbcTrans().end();
  FTRACE(1, "{}{:-^40}{}\n",
         color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
         "",
         color(ANSI_COLOR_END));
}

void Translator::traceFree() {
  FTRACE(1, "HHIR free: arena size: {}\n",
         m_irTrans->hhbcTrans().unit().arena().size());
  m_irTrans.reset();
}

/*
 * Create two maps for all blocks in the region:
 *   - a map from RegionDesc::BlockId -> IR Block* for all region blocks
 *   - a map from RegionDesc::BlockId -> RegionDesc::Block
 */
void Translator::createBlockMaps(const RegionDesc&        region,
                                 BlockIdToIRBlockMap&     blockIdToIRBlock,
                                 BlockIdToRegionBlockMap& blockIdToRegionBlock)
{
  HhbcTranslator& ht = m_irTrans->hhbcTrans();
  IRBuilder& irb = ht.irBuilder();
  blockIdToIRBlock.clear();
  blockIdToRegionBlock.clear();
  for (auto regionBlock : region.blocks) {
    RegionDesc::Block* rBlock = regionBlock.get();
    auto id = rBlock->id();
    Offset bcOff = rBlock->start().offset();
    Block* iBlock = bcOff == irb.unit().bcOff() ? irb.unit().entry()
                                                : irb.unit().defBlock();
    blockIdToIRBlock[id]     = iBlock;
    blockIdToRegionBlock[id] = rBlock;
    FTRACE(1,
           "createBlockMaps: RegionBlock {} => IRBlock {} (BC offset = {})\n",
           id, iBlock->id(), bcOff);
  }
}

/*
 * Set IRBuilder's Block associated to blockId's block according to
 * the mapping in blockIdToIRBlock.
 */
void Translator::setIRBlock(RegionDesc::BlockId            blockId,
                            const BlockIdToIRBlockMap&     blockIdToIRBlock,
                            const BlockIdToRegionBlockMap& blockIdToRegionBlock)
{
  IRBuilder& irb = m_irTrans->hhbcTrans().irBuilder();
  auto rit = blockIdToRegionBlock.find(blockId);
  assert(rit != blockIdToRegionBlock.end());
  RegionDesc::Block* rBlock = rit->second;

  Offset bcOffset = rBlock->start().offset();

  auto iit = blockIdToIRBlock.find(blockId);
  assert(iit != blockIdToIRBlock.end());

  assert(!irb.hasBlock(bcOffset));
  FTRACE(3, "  setIRBlock: blockId {}, offset {} => IR Block {}\n",
         blockId, bcOffset, iit->second->id());
  irb.setBlock(bcOffset, iit->second);
}

/*
 * Set IRBuilder's Blocks for srcBlockId's successors' offsets within
 * the region.
 */
void Translator::setSuccIRBlocks(
  const RegionDesc&              region,
  RegionDesc::BlockId            srcBlockId,
  const BlockIdToIRBlockMap&     blockIdToIRBlock,
  const BlockIdToRegionBlockMap& blockIdToRegionBlock)
{
  FTRACE(3, "setSuccIRBlocks: srcBlockId = {}\n", srcBlockId);
  IRBuilder& irb = m_irTrans->hhbcTrans().irBuilder();
  irb.resetOffsetMapping();
  for (auto& arc : region.arcs) {
    if (arc.src == srcBlockId) {
      RegionDesc::BlockId dstBlockId = arc.dst;
      setIRBlock(dstBlockId, blockIdToIRBlock, blockIdToRegionBlock);
    }
  }
}

/*
 * Compute the set of bytecode offsets that may follow the execution
 * of srcBlockId in the region.
 */
static void findSuccOffsets(const RegionDesc&              region,
                            RegionDesc::BlockId            srcBlockId,
                            const BlockIdToRegionBlockMap& blockIdToRegionBlock,
                            OffsetSet&                     set) {
  set.clear();
  for (auto& arc : region.arcs) {
    if (arc.src == srcBlockId) {
      RegionDesc::BlockId dstBlockId = arc.dst;
      auto rit = blockIdToRegionBlock.find(dstBlockId);
      assert(rit != blockIdToRegionBlock.end());
      RegionDesc::Block* rDstBlock = rit->second;
      Offset bcOffset = rDstBlock->start().offset();
      set.insert(bcOffset);
    }
  }
}

/*
 * Returns whether or not succOffsets contains both successors of inst.
 */
static bool containsBothSuccs(const OffsetSet&             succOffsets,
                              const NormalizedInstruction& inst) {
  assert(inst.op() == OpJmpZ || inst.op() == OpJmpNZ);
  if (inst.breaksTracelet) return false;
  Offset takenOffset      = inst.offset() + inst.imm[0].u_BA;
  Offset fallthruOffset   = inst.offset() + instrLen((Op*)(inst.pc()));
  bool   takenIncluded    = succOffsets.count(takenOffset);
  bool   fallthruIncluded = succOffsets.count(fallthruOffset);
  return takenIncluded && fallthruIncluded;
}

/*
 * Returns whether offset is a control-flow merge within region.
 */
static bool isMergePoint(Offset offset, const RegionDesc& region) {
  for (auto block : region.blocks) {
    auto const bid = block->id();
    if (block->start().offset() == offset) {
      auto inCount = int{0};
      for (auto arc : region.arcs) {
        if (arc.dst == bid) inCount++;
      }
      if (inCount >= 2) return true;
    }
  }
  return false;
}

/*
 * Returns whether the next instruction following inst (whether by
 * fallthrough or branch target) is a merge in region.
 */
static bool nextIsMerge(const NormalizedInstruction& inst,
                        const RegionDesc& region) {
  Offset fallthruOffset   = inst.offset() + instrLen((Op*)(inst.pc()));
  if (instrIsNonCallControlFlow(inst.op())) {
    Offset takenOffset      = inst.offset() + inst.imm[0].u_BA;
    return isMergePoint(takenOffset, region)
        || isMergePoint(fallthruOffset, region);
  }
  return isMergePoint(fallthruOffset, region);
}

Translator::TranslateResult
Translator::translateRegion(const RegionDesc& region,
                            bool bcControlFlow,
                            RegionBlacklist& toInterp) {
  const Timer translateRegionTimer(Timer::translateRegion);

  FTRACE(1, "translateRegion starting with:\n{}\n", show(region));
  HhbcTranslator& ht = m_irTrans->hhbcTrans();
  assert(!region.blocks.empty());
  const SrcKey startSk = region.blocks.front()->start();
  auto profilingFunc = false;

  BlockIdToIRBlockMap     blockIdToIRBlock;
  BlockIdToRegionBlockMap blockIdToRegionBlock;

  if (bcControlFlow) {
    ht.setGenMode(IRGenMode::CFG);
    createBlockMaps(region, blockIdToIRBlock, blockIdToRegionBlock);
  }

  Timer irGenTimer(Timer::translateRegion_irGeneration);
  for (auto b = 0; b < region.blocks.size(); b++) {
    auto const& block  = region.blocks[b];
    auto const blockId = block->id();
    auto sk            = block->start();
    auto typePreds     = makeMapWalker(block->typePreds());
    auto byRefs        = makeMapWalker(block->paramByRefs());
    auto refPreds      = makeMapWalker(block->reffinessPreds());
    auto knownFuncs    = makeMapWalker(block->knownFuncs());

    const Func* topFunc = nullptr;
    TransID profTransId = getTransId(blockId);
    ht.setProfTransID(profTransId);

    OffsetSet succOffsets;
    if (ht.genMode() == IRGenMode::CFG) {
      ht.irBuilder().startBlock(blockIdToIRBlock[blockId]);
      findSuccOffsets(region, blockId, blockIdToRegionBlock, succOffsets);
      setSuccIRBlocks(region, blockId, blockIdToIRBlock, blockIdToRegionBlock);
    }
    ht.irBuilder().recordOffset(sk.offset());

    for (unsigned i = 0; i < block->length(); ++i, sk.advance(block->unit())) {
      // Update bcOff here so any guards or assertions from metadata are
      // attributed to this instruction.
      ht.setBcOff(sk.offset(), false);

      // Emit prediction guards. If this is the first instruction in the
      // region the guards will go to a retranslate request. Otherwise, they'll
      // go to a side exit.
      bool isFirstRegionInstr = block == region.blocks.front() && i == 0;
      if (isFirstRegionInstr) ht.emitRB(Trace::RBTypeTraceletGuards, sk);

      while (typePreds.hasNext(sk)) {
        auto const& pred = typePreds.next();
        auto type = pred.type;
        auto loc  = pred.location;
        if (type <= Type::Cls) {
          // Do not generate guards for class; instead assert the type
          assert(loc.tag() == RegionDesc::Location::Tag::Stack);
          ht.assertType(loc, type);
        } else if (isFirstRegionInstr) {
          bool checkOuterTypeOnly = m_mode != TransKind::Profile;
          ht.guardTypeLocation(loc, type, checkOuterTypeOnly);
        } else {
          ht.checkType(loc, type, sk.offset());
        }
      }

      // Emit reffiness guards. For now, we only support reffiness guards at
      // the beginning of the region.
      while (refPreds.hasNext(sk)) {
        assert(sk == startSk);
        auto const& pred = refPreds.next();
        ht.guardRefs(pred.arSpOffset, pred.mask, pred.vals);
      }

      if (isFirstRegionInstr) {
        ht.endGuards();
        if (RuntimeOption::EvalJitTransCounters) {
          ht.emitIncTransCounter();
        }

        if (m_mode == TransKind::Profile) {
          profilingFunc = true;
          if (block->func()->isEntry(block->start().offset())) {
            ht.emitCheckCold(m_profData->curTransID());
          } else {
            ht.emitIncProfCounter(m_profData->curTransID());
          }
        }

        ht.emitRB(Trace::RBTypeTraceletBody, sk);
      }

      // Update the current funcd, if we have a new one.
      if (knownFuncs.hasNext(sk)) {
        topFunc = knownFuncs.next();
      }

      // Create and initialize the instruction.
      NormalizedInstruction inst(sk, block->unit());
      inst.breaksTracelet =
        i == block->length() - 1 && block == region.blocks.back();
      inst.changesPC = opcodeChangesPC(inst.op());
      inst.funcd = topFunc;
      if (instrIsNonCallControlFlow(inst.op()) && !inst.breaksTracelet) {
        assert(b < region.blocks.size());
        inst.nextOffset = region.blocks[b+1]->start().offset();
      }
      populateImmediates(inst);
      inst.nextIsMerge = nextIsMerge(inst, region);
      if (inst.op() == OpJmpZ || inst.op() == OpJmpNZ) {
        // TODO(t3730617): Could extend this logic to other
        // conditional control flow ops, e.g., IterNext, etc.
        inst.includeBothPaths = containsBothSuccs(succOffsets, inst);
      }

      // We can get a more precise output type for interpOne if we know all of
      // its inputs, so we still populate the rest of the instruction even if
      // this is true.
      inst.interp = toInterp.count(ProfSrcKey{profTransId, sk});

      InputInfos inputInfos;
      getInputs(startSk, inst, inputInfos, block->func(), [&](int i) {
          return ht.irBuilder().localType(i, DataTypeGeneric);
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
      if (isAlwaysNop(inst.op())) inst.noOp = true;
      if (!inst.noOp && inputInfos.needsRefCheck) {
        assert(byRefs.hasNext(sk));
        inst.preppedByRef = byRefs.next();
      }

      /*
       * Check for a type prediction. Put it in the
       * NormalizedInstruction so the emit* method can use it if
       * needed.  In PGO mode, we don't really need the values coming
       * from the interpreter type profiler.  TransKind::Profile
       * translations end whenever there's a side-exit, and type
       * predictions incur side-exits.  And when we stitch multiple
       * TransKind::Profile translations together to form a larger
       * region (in TransKind::Optimize mode), the guard for the top
       * of the stack essentially does the role of type prediction.
       * And, if the value is also inferred, then the guard is
       * omitted.
       */
      auto const doPrediction = mode() == TransKind::Live &&
                                  outputIsPredicted(inst);

      // If this block ends with an inlined FCall, we don't emit anything for
      // the FCall and instead set up HhbcTranslator for inlining. Blocks from
      // the callee will be next in the region.
      if (i == block->length() - 1 &&
          (inst.op() == Op::FCall || inst.op() == Op::FCallD) &&
          block->inlinedCallee()) {
        auto const* callee = block->inlinedCallee();
        FTRACE(1, "\nstarting inlined call from {} to {} with {} args "
               "and stack:\n{}\n",
               block->func()->fullName()->data(),
               callee->fullName()->data(),
               inst.imm[0].u_IVA,
               ht.showStack());
        auto returnSk = inst.nextSk();
        auto returnFuncOff = returnSk.offset() - block->func()->base();
        ht.beginInlining(inst.imm[0].u_IVA, callee, returnFuncOff,
                         doPrediction ? inst.outPred : Type::Gen);
        continue;
      }

      // Emit IR for the body of the instruction.
      try {
        m_irTrans->translateInstr(inst);
      } catch (const FailedIRGen& exn) {
        ProfSrcKey psk{profTransId, sk};
        always_assert_log(
          !toInterp.count(psk),
          [&] {
            std::ostringstream oss;
            oss << folly::format("IR generation failed with {}\n", exn.what());
            print(oss, m_irTrans->hhbcTrans().unit());
            return oss.str();
          });
        toInterp.insert(psk);
        return Retry;
      }

      // Insert a fallthrough jump
      if (ht.genMode() == IRGenMode::CFG &&
          i == block->length() - 1 && block != region.blocks.back()) {
        if (instrAllowsFallThru(inst.op())) {
          auto nextOffset = inst.nextOffset != kInvalidOffset
            ? inst.nextOffset
            : inst.offset() + instrLen((Op*)(inst.pc()));
          // prepareForSideExit is done later in Trace mode, but it
          // needs to happen here or else we generate the SpillStack
          // after the fallthrough jump, which is just weird.
          if (b < region.blocks.size() - 1
              && region.isSideExitingBlock(blockId)) {
            ht.prepareForSideExit();
          }
          ht.endBlock(nextOffset, inst.nextIsMerge);
        }
      }

      // Check the prediction. If the predicted type is less specific than what
      // is currently on the eval stack, checkType won't emit any code.
      if (doPrediction && inst.outPred < ht.topType(0, DataTypeGeneric)) {
        ht.checkTypeStack(0, inst.outPred,
                          sk.advanced(block->unit()).offset());
      }
    }

    if (ht.genMode() == IRGenMode::Trace) {
      if (b < region.blocks.size() - 1 && region.isSideExitingBlock(blockId)) {
        ht.prepareForSideExit();
      }
    }

    assert(!typePreds.hasNext());
    assert(!byRefs.hasNext());
    assert(!refPreds.hasNext());
    assert(!knownFuncs.hasNext());
  }

  traceEnd();
  irGenTimer.end();

  try {
    translatorTraceCodeGen();
    if (profilingFunc) profData()->setProfiling(startSk.func()->getFuncId());
  } catch (const FailedCodeGen& exn) {
    SrcKey sk{exn.vmFunc, exn.bcOff, exn.resumed};
    ProfSrcKey psk{exn.profTransId, sk};
    always_assert_log(
      !toInterp.count(psk),
      [&] {
        std::ostringstream oss;
        oss << folly::format("code generation failed with {}\n", exn.what());
        print(oss, m_irTrans->hhbcTrans().unit());
        return oss.str();
      });
    toInterp.insert(psk);
    return Retry;
  } catch (const DataBlockFull& dbFull) {
    if (dbFull.name == "hot") {
      assert(m_useAHot);
      m_useAHot = false;
      // We can't return Retry here because the code block selection
      // will still say hot.
      return Translator::Failure;
    } else {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
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
                        HPHP::Timer::GetCurrentTimeMicros() - m_createdTime,
                        folly::format("{}:{}:{}",
                          transRec.src.unit()->filepath()->data(),
                          transRec.src.getFuncId(),
                          transRec.src.offset()).str().c_str(),
                        transRec.aLen,
                        transRec.acoldLen,
                        transRec.kind);
  }

  if (!isTransDBEnabled()) return;
  uint32_t id = getCurrentTransID();
  m_translations.emplace_back(transRec);
  m_translations[id].id = id;

  if (transRec.aLen > 0) {
    m_transDB[transRec.aStart] = id;
  }
  if (transRec.acoldLen > 0) {
    m_transDB[transRec.acoldStart] = id;
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
     * it will notice the underlying PhpFile* has changed.
     *
     * We don't actually need to *do* anything with the PhpFile* from
     * this lookup; since the path has changed, the file we'll get out is
     * going to be some new file, not the old file that needs invalidation.
     */
    (void)g_context->lookupPhpFile(spath.get(), "");
  }
};

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
                                  bool& magicCall, bool staticLookup,
                                  Class* ctx) {
  if (!cls || RuntimeOption::EvalJitEnableRenameFunction) return nullptr;
  if (cls->attrs() & AttrInterface) return nullptr;
  bool privateOnly = false;
  if (!RuntimeOption::RepoAuthoritative ||
      !(cls->preClass()->attrs() & AttrUnique)) {
    if (!ctx || !ctx->classof(cls)) {
      return nullptr;
    }
    if (!staticLookup) privateOnly = true;
  }

  const Func* func;
  LookupResult res = staticLookup ?
    g_context->lookupClsMethod(func, cls, name, nullptr, ctx, false) :
    g_context->lookupObjMethod(func, cls, name, ctx, false);

  if (res == LookupResult::MethodNotFound) return nullptr;

  assert(res == LookupResult::MethodFoundWithThis ||
         res == LookupResult::MethodFoundNoThis ||
         (staticLookup ?
          res == LookupResult::MagicCallStaticFound :
          res == LookupResult::MagicCallFound));

  magicCall =
    res == LookupResult::MagicCallStaticFound ||
    res == LookupResult::MagicCallFound;

  if ((privateOnly && (!(func->attrs() & AttrPrivate) || magicCall)) ||
      func->isAbstract() ||
      func->attrs() & AttrInterceptable) {
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
      // Even if a func has AttrNoOverride, if it has static locals it
      // is cloned into subclasses (to give them different copies of
      // the static locals), so we need to skip this.
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
    if (hasMVector(ni->op())) {
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

} // HPHP::JIT

void invalidatePath(const std::string& path) {
  TRACE(1, "invalidatePath: abspath %s\n", path.c_str());
  PendQ::defer(new JIT::DeferredPathInvalidate(path));
}

} // HPHP
