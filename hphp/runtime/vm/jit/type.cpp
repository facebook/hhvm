/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/shape.h"
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/base/struct-array-defs.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

#include <boost/algorithm/string/trim.hpp>

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/MapUtil.h>
#include <folly/gen/Base.h>

#include <vector>


namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

std::string Type::constValString() const {
  if (*this <= TBottom)   return "Bottom";
  if (*this <= TUninit)   return "Uninit";
  if (*this <= TInitNull) return "InitNull";
  if (*this <= TNullptr)  return "Nullptr";

  assertx(hasConstVal());

  if (*this <= TBool) {
    return m_boolVal ? "true" : "false";
  }
  if (*this <= TInt) {
    return folly::format("{}", m_intVal).str();
  }
  if (*this <= TDbl) {
    // don't format doubles as integers.
    auto s = folly::format("{}", m_dblVal).str();
    if (!strchr(s.c_str(), '.') && !strchr(s.c_str(), 'e')) {
      return folly::format("{:.1f}", m_dblVal).str();
    }
    return s;
  }
  if (*this <= TStaticStr) {
    auto str = m_strVal;
    return folly::format("\"{}\"", escapeStringForCPP(str->data(),
                                                      str->size())).str();
  }
  if (*this <= TStaticArr) {
    if (m_arrVal->empty()) {
      return "array()";
    }
    return folly::format("Array({})", m_arrVal).str();
  }
  if (*this <= TFunc) {
    return folly::format("Func({})", m_funcVal ? m_funcVal->fullName()->data()
                                               : "nullptr").str();
  }
  if (*this <= TCls) {
    return folly::format("Cls({})", m_clsVal ? m_clsVal->name()->data()
                                             : "nullptr").str();
  }
  if (*this <= TCctx) {
    if (!m_intVal) {
      return "Cctx(Cls(nullptr))";
    }
    auto const cls = m_cctxVal.cls();
    return folly::format("Cctx(Cls({}))", cls->name()).str();
  }
  if (*this <= TTCA) {
    auto name = getNativeFunctionName(m_tcaVal);
    const char* hphp = "HPHP::";

    if (!name.compare(0, strlen(hphp), hphp)) {
      name = name.substr(strlen(hphp));
    }
    auto pos = name.find_first_of('(');
    if (pos != std::string::npos) {
      name = name.substr(0, pos);
    }
    return folly::format("TCA: {}({})", m_tcaVal, boost::trim_copy(name)).str();
  }
  if (*this <= TRDSHandle) {
    return folly::format("rds::Handle({:#x})", m_rdsHandleVal).str();
  }
  if (*this <= TPtrToGen) {
    return folly::sformat("TV: {}", m_ptrVal);
  }

  always_assert_flog(false, "Bad type in constValString(): {:#16x}:{:#16x}",
                     m_raw, m_extra);
}

static std::string show(Ptr ptr) {
  always_assert(ptr <= Ptr::Ptr);

  switch (ptr) {
    case Ptr::Bottom:
    case Ptr::Top:
    case Ptr::NotPtr: not_reached();
    case Ptr::Ptr:    return "";

#define PTRT(name, ...) case Ptr::name: return #name;
    PTR_TYPES(PTRT, PTR_R)
#undef PTRT
  }

  std::vector<const char*> parts;
#define PTRT(name, ...) if (Ptr::name <= ptr) parts.emplace_back(#name);
  PTR_PRIMITIVE(PTRT, PTR_NO_R)
#undef PTRT
  return folly::sformat("{{{}}}", folly::join('|', parts));
}

static const std::unordered_map<Type, const char*> s_typeNames{
#define IRT(x, ...) {T##x, #x},
#define IRTP IRT
  IR_TYPES
#undef IRT
#undef IRTP
};

std::string Type::toString() const {
  // First, see if this is a predefined type.
  auto it = s_typeNames.find(*this);
  if (it != s_typeNames.end()) return it->second;

  if (maybe(TNullptr)) {
    return folly::to<std::string>(
      "Nullptr|",
      (*this - TNullptr).toString()
    );
  }

  if (*this <= TBoxedCell) {
    return folly::to<std::string>("Boxed", inner().toString());
  }

  if (m_hasConstVal) {
    if (*this <= TCls) {
      return folly::sformat("Cls={}", m_clsVal->name()->data());
    }
    return folly::sformat("{}<{}>",
                          dropConstVal().toString(), constValString());
  }

  auto t = *this;

  if (t.maybe(TPtrToGen)) {
    assertx(!t.m_hasConstVal);
    auto ret = "PtrTo" +
      show(t.ptrKind() & Ptr::Ptr) +
      (t & TPtrToGen).deref().toString();

    t -= TPtrToGen;
    if (t != TBottom) ret += "|" + t.toString();
    return ret;
  }

  assertx(t.ptrKind() <= Ptr::NotPtr);

  std::vector<std::string> parts;
  if (isSpecialized()) {
    if (auto clsSpec = t.clsSpec()) {
      auto const base = Type(m_bits & kClsSpecBits, t.ptrKind());
      auto const exact = clsSpec.exact() ? "=" : "<=";
      auto const name = clsSpec.cls()->name()->data();
      auto const partStr = folly::to<std::string>(base.toString(), exact, name);

      parts.push_back(partStr);
      t -= TAnyObj;
    } else if (auto arrSpec = t.arrSpec()) {
      auto str = Type(m_bits & kArrSpecBits, t.ptrKind()).toString();
      if (auto const kind = arrSpec.kind()) {
        str += "=";
        str += ArrayData::kindToString(*kind);
      }
      if (auto const ty = arrSpec.type()) {
        str += folly::to<std::string>(':', show(*ty));
      }
      if (auto const shape = arrSpec.shape()) {
        str += folly::to<std::string>(":", show(*shape));
      }
      parts.push_back(str);
      t -= TAnyArr;
    } else {
      not_reached();
    }
  }

  // Concat all of the primitive types in the custom union type
# define IRT(name, ...) if (T##name <= t) parts.push_back(#name);
# define IRTP(name, ...)
  IRT_PRIMITIVE
# undef IRT
# undef IRTP

  assertx(!parts.empty());
  if (parts.size() == 1) return parts.front();
  return folly::sformat("{{{}}}", folly::join('|', parts));
}

std::string Type::debugString(Type t) {
  return t.toString();
}

///////////////////////////////////////////////////////////////////////////////

bool Type::checkValid() const {
  // Note: be careful, the TFoo objects aren't all constructed yet in this
  // function.
  if (m_extra) {
    assertx(((m_bits & kClsSpecBits) == 0 || (m_bits & kArrSpecBits) == 0) &&
            "Conflicting specialization");
  }

  // We should have one canonical representation of Bottom.
  if (m_bits == kBottom) {
    assert_flog(*this == TBottom,
                "Bottom m_bits but nonzero others in {:#16x}:{:#16x}",
                m_raw, m_extra);
  }

  return true;
}

Type::bits_t Type::bitsFromDataType(DataType outer, DataType inner) {
  assertx(inner != KindOfRef);
  assertx(inner == KindOfUninit || outer == KindOfRef);

  switch (outer) {
    case KindOfUninit        : return kUninit;
    case KindOfNull          : return kInitNull;
    case KindOfBoolean       : return kBool;
    case KindOfInt64         : return kInt;
    case KindOfDouble        : return kDbl;
    case KindOfPersistentString : return kPersistentStr;
    case KindOfString        : return kStr;
    case KindOfPersistentArray : return kPersistentArr;
    case KindOfArray         : return kArr;
    case KindOfResource      : return kRes;
    case KindOfObject        : return kObj;
    case KindOfClass         : return kCls;
    case KindOfRef:
      assertx(inner != KindOfUninit);
      return bitsFromDataType(inner, KindOfUninit) << kBoxShift;
  }
  not_reached();
}

DataType Type::toDataType() const {
  assertx(!maybe(TPtrToGen) || m_bits == kBottom);
  assertx(isKnownDataType());

  // Order is important here: types must progress from more specific
  // to less specific to return the most specific DataType.
  if (*this <= TUninit)      return KindOfUninit;
  if (*this <= TInitNull)    return KindOfNull;
  if (*this <= TBool)        return KindOfBoolean;
  if (*this <= TInt)         return KindOfInt64;
  if (*this <= TDbl)         return KindOfDouble;
  if (*this <= TPersistentStr) return KindOfPersistentString;
  if (*this <= TStr)         return KindOfString;
  if (*this <= TPersistentArr) return KindOfPersistentArray;
  if (*this <= TArr)         return KindOfArray;
  if (*this <= TObj)         return KindOfObject;
  if (*this <= TRes)         return KindOfResource;
  if (*this <= TBoxedCell)   return KindOfRef;
  if (*this <= TCls)         return KindOfClass;
  always_assert_flog(false,
                     "Bad Type {} in Type::toDataType()", *this);
}

///////////////////////////////////////////////////////////////////////////////
// Combinators.

Type Type::specialize(TypeSpec spec) const {
  assertx(!spec.arrSpec() || supports(SpecKind::Array));
  assertx(!spec.clsSpec() || supports(SpecKind::Class));

  // If we don't have exactly one kind of specialization, or if our bits
  // support both kinds, don't specialize.
  if ((bool)spec.arrSpec() == (bool)spec.clsSpec() ||
      (supports(SpecKind::Array) && supports(SpecKind::Class))) {
    return *this;
  }

  if (spec.arrSpec() != ArraySpec::Bottom) {
    return Type{*this, spec.arrSpec()};
  }

  assertx(spec.clsSpec() != ClassSpec::Bottom);
  return Type{*this, spec.clsSpec()};
}

// Return true if the array satisfies requirement on the ArraySpec.
static bool arrayFitsSpec(const ArrayData* arr, const ArraySpec spec) {
  if (spec == ArraySpec::Top) return true;

  if (auto const spec_kind = spec.kind()) {
    if (arr->kind() == spec_kind) return true;
  }

  if (auto const rat_type = spec.type()) {
    using A = RepoAuthType::Array;
    if (arr->empty() && rat_type->emptiness() != A::Empty::No) return true;
    if (arr->isVectorData()) {
      switch (rat_type->tag()) {
        case A::Tag::Packed:
          if (arr->size() != rat_type->size()) break;
          // fall through
        case A::Tag::PackedN: {
          int64_t k = 0;
          for ( ; k < arr->size(); ++k) {
            auto const specElemType =
              rat_type->tag() == A::Tag::Packed ? rat_type->packedElem(k)
                                                : rat_type->elemType();
            if (!tvMatchesRepoAuthType(*(arr->get(k).asTypedValue()),
                                       specElemType)) {
              break;
            }
          }
          if (k == arr->size()) return true;
          break;
        }
      }
    }
  }

  if (arr->isStruct()) {
    if (StructArray::asStructArray(arr)->shape() == spec.shape()) return true;
  }

  return false;
}

bool Type::operator<=(Type rhs) const {
  auto const lhs = *this;

  // Check for any members in lhs.m_bits that aren't in rhs.m_bits.
  if ((lhs.m_bits & rhs.m_bits) != lhs.m_bits) {
    return false;
  }

  // Check for Bottom; all the remaining cases assume `lhs' is not Bottom.
  if (lhs.m_bits == kBottom) return true;

  // If `rhs' is a constant, we must be the same constant.
  if (rhs.m_hasConstVal) {
    assertx(!rhs.isUnion());
    return lhs.m_hasConstVal && lhs.m_extra == rhs.m_extra;
  }

  // Make sure lhs's ptr kind is a subtype of rhs's.
  if (!(lhs.ptrKind() <= rhs.ptrKind())) {
    return false;
  }

  // If rhs isn't specialized no further checking is needed.
  if (!rhs.isSpecialized()) {
    return true;
  }

  if (lhs.hasConstVal(TArr)) {
    // Arrays can be specialized in different ways, here we check if the
    // constant array fits the kind()/type() of the specialization of rhs, if
    // any.
    auto const lhs_arr = lhs.arrVal();
    auto const rhs_as = rhs.arrSpec();
    return arrayFitsSpec(lhs_arr, rhs_as);
  }

  // Compare specializations only if `rhs' is specialized.
  return lhs.spec() <= rhs.spec();
}

Type Type::operator|(Type rhs) const {
  auto lhs = *this;

  if (lhs == rhs || rhs == TBottom) return lhs;
  if (lhs == TBottom) return rhs;

  // Representing types like {Int<12>|Arr} could get messy and isn't useful in
  // practice, so unless we hit one of the trivial cases above, drop the
  // constant value(s).
  lhs = lhs.dropConstVal();
  rhs = rhs.dropConstVal();

  auto const bits = lhs.m_bits | rhs.m_bits;
  auto const ptr = lhs.ptrKind() | rhs.ptrKind();
  auto const spec = lhs.spec() | rhs.spec();

  return Type{bits, ptr}.specialize(spec);
}

Type Type::operator&(Type rhs) const {
  auto lhs = *this;

  // When intersecting a constant value with another type, the result will be
  // the constant value if the other value is a supertype of the constant, and
  // Bottom otherwise.
  if (lhs.m_hasConstVal) return lhs <= rhs ? lhs : TBottom;
  if (rhs.m_hasConstVal) return rhs <= lhs ? rhs : TBottom;

  auto bits = lhs.m_bits & rhs.m_bits;
  auto ptr = lhs.ptrKind() & rhs.ptrKind();
  auto arrSpec = lhs.arrSpec() & rhs.arrSpec();
  auto clsSpec = lhs.clsSpec() & rhs.clsSpec();

  // Filter out bits and pieces that no longer exist due to other components
  // going to Bottom, starting with bits.
  if (ptr == Ptr::Bottom) bits &= ~kGen;
  if (arrSpec == ArraySpec::Bottom) bits &= ~kArrSpecBits;
  if (clsSpec == ClassSpec::Bottom) bits &= ~kClsSpecBits;

  // ptr
  if ((bits & kGen) == 0) ptr = Ptr::Bottom;

  // specs
  if (!supports(bits, SpecKind::Array)) arrSpec = ArraySpec::Bottom;
  if (!supports(bits, SpecKind::Class)) clsSpec = ClassSpec::Bottom;

  return Type{bits, ptr}.specialize({arrSpec, clsSpec});
}

Type Type::operator-(Type rhs) const {
  auto lhs = *this;
  if (rhs == TBottom) return lhs;
  if (lhs <= rhs) return TBottom;
  if (lhs.hasConstVal()) return lhs;    // not covered by rhs.

  // If `rhs' has a constant value, but `lhs' doesn't, conservatively return
  // `lhs', rather than trying to represent things like "everything except
  // Int<24>". Boolean is a special case.
  if (rhs.m_hasConstVal) {
    if (rhs <= TBool && lhs <= TBool) {
      auto const res = !rhs.boolVal();
      if (lhs.hasConstVal() && lhs.boolVal() != res) return TBottom;
      return cns(res);
    }
    return lhs;
  }

  // For each component C, we should subtract C_rhs from C_lhs iff every other
  // component of lhs that can intersect with C is subsumed by the
  // corresponding component of rhs. This prevents us from removing members of
  // lhs that weren't present in rhs, but would be casualties of removing
  // certain bits in lhs.
  //
  // As an example, consider PtrToRMembInt - PtrToRefStr. Simple subtraction of
  // each component would yield PtrToMembInt, but that would mean we removed
  // PtrToRefInt from the lhs despite it not being in rhs. Checking if Int is a
  // subset of Str shows us that removing Ref from lhs would erase types not
  // present in rhs.
  //
  // In practice, it's more concise to eagerly do each subtraction, then check
  // for components that went to Bottom as a way of seeing which components of
  // lhs were subsets of the corresponding components in rhs. When we find a
  // component that we weren't supposed to subtract, just restore lhs's
  // original value.
  auto bits = lhs.m_bits & ~rhs.m_bits;
  auto ptr = lhs.ptrKind() - rhs.ptrKind();
  auto arrSpec = lhs.arrSpec() - rhs.arrSpec();
  auto clsSpec = lhs.clsSpec() - rhs.clsSpec();

  auto const have_gen_bits = (bits & kGen) != 0;
  auto const have_arr_bits = supports(bits, SpecKind::Array);
  auto const have_cls_bits = supports(bits, SpecKind::Class);
  auto const have_ptr      = ptr != Ptr::Bottom;
  auto const have_arr_spec = arrSpec != ArraySpec::Bottom;
  auto const have_cls_spec = clsSpec != ClassSpec::Bottom;

  // ptr can only interact with clsSpec if lhs.m_bits has at least one kGen
  // member of kClsSpecBits.
  auto const have_ptr_cls = supports(lhs.m_bits & kGen, SpecKind::Class);

  // bits
  if (have_ptr) bits |= lhs.m_bits & kGen;
  if (have_arr_spec) bits |= lhs.m_bits & kArrSpecBits;
  if (have_cls_spec) bits |= lhs.m_bits & kClsSpecBits;

  // ptr
  if (have_gen_bits || have_arr_spec || (have_cls_spec && have_ptr_cls)) {
    ptr = lhs.ptrKind();
  }

  // specs
  if (have_ptr || have_arr_bits) arrSpec = lhs.arrSpec();
  if ((have_ptr && have_ptr_cls) || have_cls_bits) clsSpec = lhs.clsSpec();

  return Type{bits, ptr}.specialize({arrSpec, clsSpec});
}

///////////////////////////////////////////////////////////////////////////////
// Conversions.

Type typeFromTV(const TypedValue* tv) {
  assertx(tv->m_type == KindOfClass || tvIsPlausible(*tv));

  if (tv->m_type == KindOfObject) {
    auto const cls = tv->m_data.pobj->getVMClass();

    // We only allow specialization on classes that can't be overridden for
    // now.  If this changes, then this will need to specialize on sub object
    // types instead.
    if (!cls || !(cls->attrs() & AttrNoOverride)) return TObj;
    return Type::ExactObj(cls);
  }

  if (isArrayType(tv->m_type)) {
    auto const ar = tv->m_data.parr;
    if (ar->kind() == ArrayData::kStructKind) {
      return Type::Array(StructArray::asStructArray(ar)->shape());
    }
    return Type::Array(tv->m_data.parr->kind());
  }

  auto outer = tv->m_type;
  auto inner = KindOfUninit;

  if (outer == KindOfPersistentString) outer = KindOfString;
  if (outer == KindOfRef) {
    inner = tv->m_data.pref->tv()->m_type;
    if (inner == KindOfPersistentString) inner = KindOfString;
    else if (inner == KindOfPersistentArray) inner = KindOfArray;
  }
  return Type(outer, inner);
}

Type typeFromRAT(RepoAuthType ty) {
  using T = RepoAuthType::Tag;
  switch (ty.tag()) {
    case T::OptBool:        return TBool      | TInitNull;
    case T::OptInt:         return TInt       | TInitNull;
    case T::OptSStr:        return TStaticStr | TInitNull;
    case T::OptStr:         return TStr       | TInitNull;
    case T::OptDbl:         return TDbl       | TInitNull;
    case T::OptRes:         return TRes       | TInitNull;
    case T::OptObj:         return TObj       | TInitNull;

    case T::Uninit:         return TUninit;
    case T::InitNull:       return TInitNull;
    case T::Null:           return TNull;
    case T::Bool:           return TBool;
    case T::Int:            return TInt;
    case T::Dbl:            return TDbl;
    case T::Res:            return TRes;
    case T::SStr:           return TStaticStr;
    case T::Str:            return TStr;
    case T::Obj:            return TObj;

    case T::Cell:           return TCell;
    case T::Ref:            return TBoxedInitCell;
    case T::InitUnc:        return TUncountedInit;
    case T::Unc:            return TUncounted;
    case T::InitCell:       return TInitCell;
    case T::InitGen:        return TInitGen;
    case T::Gen:            return TGen;

    // TODO(#4205897): option specialized array types
    case T::OptArr:         return TArr       | TInitNull;
    case T::OptSArr:        return TStaticArr | TInitNull;

    case T::SArr:
      if (auto const ar = ty.array()) return Type::StaticArray(ar);
      return TStaticArr;
    case T::Arr:
      if (auto const ar = ty.array()) return Type::Array(ar);
      return TArr;

    case T::SubObj:
    case T::ExactObj:
    case T::OptSubObj:
    case T::OptExactObj: {
      auto base = TObj;

      if (auto const cls = Unit::lookupClassOrUniqueClass(ty.clsName())) {
        if (ty.tag() == T::ExactObj || ty.tag() == T::OptExactObj) {
          base = Type::ExactObj(cls);
        } else {
          base = Type::SubObj(cls);
        }
      }
      if (ty.tag() == T::OptSubObj || ty.tag() == T::OptExactObj) {
        base |= TInitNull;
      }
      return base;
    }
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

Type ldRefReturn(Type typeParam) {
  // Guarding on specialized types and uncommon unions like {Int|Bool} is
  // expensive enough that we only want to do it in situations where we've
  // manually confirmed the benefit.
  typeParam = relaxToGuardable(typeParam);
  always_assert(typeParam <= TCell);

  // Refs can never contain Uninit, so this lets us return UncountedInit rather
  // than Uncounted, and InitCell rather than Cell.
  return typeParam - TUninit;
}

Type negativeCheckType(Type srcType, Type typeParam) {
  if (srcType <= typeParam)      return TBottom;
  if (!srcType.maybe(typeParam)) return srcType;
  // Checks relating to StaticStr and StaticArr are not, in general, precise.
  // They may reject some Statics in some situations, where we only guard using
  // the type tag and not by loading the count field.
  auto tmp = srcType - typeParam;
  if (typeParam.maybe(TPersistent)) {
    if (tmp.maybe(TCountedStr)) tmp |= TStr;
    if (tmp.maybe(TCountedArr)) tmp |= TArr;
  }
  return tmp;
}

Type boxType(Type t) {
  // If t contains Uninit, replace it with InitNull.
  t = t.maybe(TUninit) ? (t - TUninit) | TInitNull : t;
  // We don't try to track when a BoxedStaticStr might be converted to
  // a BoxedStr, and we never guard on staticness for strings, so
  // boxing a string needs to forget this detail.  Same thing for
  // arrays.
  if (t <= TStr) {
    t = TStr;
  } else if (t <= TArr) {
    t = TArr;
  }
  // When boxing an Object, if the inner class does not have AttrNoOverride,
  // drop the class specialization.
  if (t < TObj && t.clsSpec() &&
      !(t.clsSpec().cls()->attrs() & AttrNoOverride)) {
    t = t.unspecialize();
  }
  // Everything else is just a pure type-system boxing operation.
  return t.box();
}

//////////////////////////////////////////////////////////////////////

static Type relaxCell(Type t, DataTypeCategory cat) {
  assertx(t <= TCell);

  switch (cat) {
    case DataTypeGeneric:
      return TGen;

    case DataTypeCountness:
      return !t.maybe(TCounted) ? TUncounted : t.unspecialize();

    case DataTypeCountnessInit:
      if (t <= TUninit) return TUninit;
      return (!t.maybe(TCounted) && !t.maybe(TUninit))
        ? TUncountedInit : t.unspecialize();

    case DataTypeSpecific:
      return t.unspecialize();

    case DataTypeSpecialized:
      assertx(t.isSpecialized());
      return t;
  }

  not_reached();
}

Type relaxType(Type t, DataTypeCategory cat) {
  always_assert_flog(t <= TGen && t != TBottom, "t = {}", t);
  if (cat == DataTypeGeneric) return TGen;
  auto const relaxed =
    (t & TCell) <= TBottom ? TBottom : relaxCell(t & TCell, cat);
  return t <= TCell ? relaxed : relaxed | TBoxedInitCell;
}

Type relaxToGuardable(Type ty) {
  assertx(ty <= TGen);
  ty = ty.unspecialize();

  // ty is unspecialized and we don't support guarding on CountedArr or
  // StaticArr, so widen any subtypes of Arr to Arr.
  if (ty <= TArr) return TArr;

  // We can guard on StaticStr but not CountedStr.
  if (ty <= TCountedStr)     return TStr;

  if (ty <= TBoxedCell)      return TBoxedCell;
  if (ty.isKnownDataType())  return ty;
  if (ty <= TUncountedInit)  return TUncountedInit;
  if (ty <= TUncounted)      return TUncounted;
  if (ty <= TCell)           return TCell;
  if (ty <= TGen)            return TGen;
  not_reached();
}

}}
