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

#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator.h"

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
  if (*this <= TStaticVec) {
    if (m_vecVal->empty()) {
      return "vec()";
    }
    return folly::format("Vec({})", m_vecVal).str();
  }
  if (*this <= TStaticDict) {
    if (m_dictVal->empty()) {
      return "dict()";
    }
    return folly::format("Dict({})", m_dictVal).str();
  }
  if (*this <= TStaticKeyset) {
    if (m_keysetVal->empty()) {
      return "keyset()";
    }
    return folly::format("Keyset({})", m_keysetVal).str();
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
  if (*this <= TLvalToGen) {
    return folly::sformat("Lval: {}", m_ptrVal);
  }
  if (*this <= TMemToGen) {
    return folly::sformat("Mem: {}", m_ptrVal);
  }

  always_assert_flog(
    false,
    "Bad type in constValString(): {:#16x}:{}:{}:{}:{:#16x}",
    m_bits,
    static_cast<ptr_t>(m_ptr),
    static_cast<ptr_t>(m_mem),
    m_hasConstVal,
    m_extra
  );
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
#define PTRT(name, ...) \
  if (Ptr::name <= ptr) parts.emplace_back(#name);
  PTR_PRIMITIVE(PTRT, PTR_NO_R)
#undef PTRT
  return folly::sformat("{{{}}}", folly::join('|', parts));
}

static const jit::fast_map<Type, const char*> s_typeNames{
#define IRT(x, ...) {T##x, #x},
#define IRTP IRT
#define IRTL IRT
#define IRTM IRT
#define IRTX IRT
  IR_TYPES
#undef IRT
#undef IRTP
#undef IRTL
#undef IRTM
#undef IRTX
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

  if (t.maybe(TLvalToGen)) {
    assertx(!t.m_hasConstVal);
    auto ret = "LvalTo" +
      show(t.ptrKind() & Ptr::Ptr) +
      (t & TLvalToGen).deref().toString();

    t -= TLvalToGen;
    if (t != TBottom) ret += "|" + t.toString();
    return ret;
  }

  assertx(t.ptrKind() <= Ptr::NotPtr);
  assertx(t.memKind() <= Mem::NotMem);

  std::vector<std::string> parts;
  if (isSpecialized()) {
    if (auto clsSpec = t.clsSpec()) {
      auto const base = Type{m_bits & kClsSpecBits, t.ptrKind(), t.memKind()};
      auto const exact = clsSpec.exact() ? "=" : "<=";
      auto const name = clsSpec.cls()->name()->data();
      auto const partStr = folly::to<std::string>(base.toString(), exact, name);

      parts.push_back(partStr);
      t -= TAnyObj;
    } else if (auto arrSpec = t.arrSpec()) {
      auto str = Type{
        m_bits & kArrSpecBits,
        t.ptrKind(),
        t.memKind()
      }.toString();

      if (auto const kind = arrSpec.kind()) {
        str += "=";
        str += ArrayData::kindToString(*kind);
      }
      if (auto const ty = arrSpec.type()) {
        str += folly::to<std::string>(':', show(*ty));
      }
      parts.push_back(str);
      t -= TAnyArr;
    } else {
      not_reached();
    }
  }

  // Sort all types by decreasing number of bits in their representation. This
  // ensures that larger unions come first.
  static auto const sortedTypes = []{
    std::vector<std::pair<Type, const char*>> types{
#define IRT(x, ...) {T##x, #x},
#define IRTP IRT
#define IRTL IRT
#define IRTM IRT
#define IRTX IRT
      IR_TYPES
#undef IRT
#undef IRTP
#undef IRTL
#undef IRTM
#undef IRTX
    };
    std::sort(
      types.begin(), types.end(),
      [](const std::pair<Type, const char*>& a,
         const std::pair<Type, const char*>& b) {
        auto const pop1 = folly::popcount(a.first.m_bits);
        auto const pop2 = folly::popcount(b.first.m_bits);
        if (pop1 != pop2) return pop1 > pop2;
        return std::strcmp(a.second, b.second) < 0;
      }
    );
    // Remove Bottom
    while (!types.back().first.m_bits) types.pop_back();
    return types;
  }();

  // Decompose the type into a union of pre-defined types. Since we've sorted
  // the type list in decreasing size, this means the decomposition will be
  // minimal.
  for (auto const& t2 : sortedTypes) {
    if (t <= TBottom) break;
    if (t2.first <= t) {
      parts.push_back(t2.second);
      t -= t2.first;
    }
  }

  assertx(!parts.empty());
  if (parts.size() == 1) return parts.front();
  return folly::sformat("{{{}}}", folly::join('|', parts));
}

std::string Type::debugString(Type t) {
  return t.toString();
}

namespace {
enum TypeKey : uint8_t {
  None,
  Const,
  ClsSub,
  ClsExact,
  ArrSpec
};
}

void Type::serialize(ProfDataSerializer& ser) const {
  SCOPE_EXIT {
    ITRACE_MOD(Trace::hhbc, 2, "Type: {}\n", toString());
  };
  ITRACE_MOD(Trace::hhbc, 2, "Type>\n");
  Trace::Indent _;

  write_raw(ser, m_bits);
  write_raw(ser, m_ptr);
  write_raw(ser, m_mem);

  Type t = *this;
  if (t.maybe(TNullptr)) t = t - TNullptr;
  if (t <= TBoxedCell) t = inner();

  auto const key = m_hasConstVal ? TypeKey::Const :
    t.clsSpec() ? (t.clsSpec().exact() ? TypeKey::ClsExact : TypeKey::ClsSub) :
    t.arrSpec() ? TypeKey::ArrSpec : TypeKey::None;

  write_raw(ser, key);

  if (key == TypeKey::Const) {
    if (t <= TCls)       return write_class(ser, t.m_clsVal);
    if (t <= TFunc)      return write_func(ser, t.m_funcVal);
    if (t <= TStaticStr) return write_string(ser, t.m_strVal);
    if (t < TArrLike) {
      return write_array(ser, t.m_arrVal);
    }
    if (t <= TCctx)      return write_class(ser, t.m_cctxVal.cls());
    assertx(t.subtypeOfAny(TBool, TInt, TDbl));
    return write_raw(ser, t.m_extra);
  }

  if (key == TypeKey::ClsSub || key == TypeKey::ClsExact) {
    return write_class(ser, t.clsSpec().cls());
  }
  if (key == TypeKey::ArrSpec) {
    return write_raw(ser, t.m_extra);
  }
}

Type Type::deserialize(ProfDataDeserializer& ser) {
  ITRACE_MOD(Trace::hhbc, 2, "Type>\n");
  auto const ret = [&] {
    Trace::Indent _;
    Type t{};

    read_raw(ser, t.m_bits);
    read_raw(ser, t.m_ptr);
    read_raw(ser, t.m_mem);
    auto const key = read_raw<TypeKey>(ser);
    if (key == TypeKey::Const) {
      t.m_hasConstVal = true;
      if (t <= TCls) {
        t.m_clsVal = read_class(ser);
        return t;
      }
      if (t <= TFunc) {
        t.m_funcVal = read_func(ser);
        return t;
      }
      if (t <= TStaticStr) {
        t.m_strVal = read_string(ser);
        return t;
      }
      if (t < TArrLike) {
        t.m_arrVal = read_array(ser);
        return t;
      }
      if (t <= TCctx) {
        t.m_cctxVal = ConstCctx::cctx(read_class(ser));
        return t;
      }
      read_raw(ser, t.m_extra);
      return t;
    }

    t.m_hasConstVal = false;
    if (key == TypeKey::None) return t;
    if (key == TypeKey::ClsSub || key == TypeKey::ClsExact) {
      auto const cls = read_class(ser);
      if (key == TypeKey::ClsExact) {
        t.m_clsSpec = ClassSpec{cls, ClassSpec::ExactTag{}};
      } else {
        t.m_clsSpec = ClassSpec{cls, ClassSpec::SubTag{}};
      }
    } else {
      assertx(key == TypeKey::ArrSpec);
      read_raw(ser, t.m_extra);
      if (auto const arr = t.m_arrSpec.type()) {
        t.m_arrSpec.adjust(ser.remap(arr));
      }
    }
    return t;
  }();
  ITRACE_MOD(Trace::hhbc, 2, "Type: {}\n", ret.toString());
  return ret;
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
                "Bottom m_bits but nonzero others in {:#16x}:{}:{}:{:#16x}",
                m_bits, m_ptrVal, m_hasConstVal, m_extra);
  }

  // m_ptr and m_mem should be Bottom iff we have no kGen bits.
  assertx(((m_bits & kGen) == 0) == (m_ptr == Ptr::Bottom));
  assertx(((m_bits & kGen) == 0) == (m_mem == Mem::Bottom));

  // Ptr::NotPtr and Mem::NotMem should imply one another.
  assertx((m_ptr == Ptr::NotPtr) == (m_mem == Mem::NotMem));

  return true;
}

Type::bits_t Type::bitsFromDataType(DataType outer, DataType inner) {
  assertx(!isRefType(inner));
  assertx(inner == KindOfUninit || isRefType(outer));

  switch (outer) {
    case KindOfUninit           : return kUninit;
    case KindOfNull             : return kInitNull;
    case KindOfBoolean          : return kBool;
    case KindOfInt64            : return kInt;
    case KindOfDouble           : return kDbl;
    case KindOfPersistentString : return kPersistentStr;
    case KindOfString           : return kStr;
    case KindOfPersistentVec    : return kPersistentVec;
    case KindOfPersistentDict   : return kPersistentDict;
    case KindOfPersistentKeyset : return kPersistentKeyset;
    case KindOfPersistentArray  : return kPersistentArr;
    case KindOfVec              : return kVec;
    case KindOfDict             : return kDict;
    case KindOfKeyset           : return kKeyset;
    case KindOfArray            : return kArr;
    case KindOfResource         : return kRes;
    case KindOfObject           : return kObj;
    case KindOfFunc             : return kFunc;
    case KindOfClass            : return kCls;
    case KindOfRef:
      assertx(inner != KindOfUninit);
      return bitsFromDataType(inner, KindOfUninit) << kBoxShift;
  }
  not_reached();
}

DataType Type::toDataType() const {
  assertx(!maybe(TMemToGen) || m_bits == kBottom);
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
  if (*this <= TPersistentVec) return KindOfPersistentVec;
  if (*this <= TVec)         return KindOfVec;
  if (*this <= TPersistentDict) return KindOfPersistentDict;
  if (*this <= TDict)        return KindOfDict;
  if (*this <= TPersistentKeyset) return KindOfPersistentKeyset;
  if (*this <= TKeyset)      return KindOfKeyset;
  if (*this <= TObj)         return KindOfObject;
  if (*this <= TRes)         return KindOfResource;
  if (*this <= TFunc)        return KindOfFunc;
  if (*this <= TCls)         return KindOfClass;
  if (*this <= TBoxedCell)   return KindOfRef;
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

Type Type::modified() const {
  auto t = unspecialize();
  if (t.maybe(TArr))    t |= TArr;
  if (t.maybe(TDict))   t |= TDict;
  if (t.maybe(TVec))    t |= TVec;
  if (t.maybe(TKeyset)) t |= TKeyset;
  if (t.maybe(TStr))    t |= TStr;
  return t;
}

/*
 * Return true if the array satisfies requirement on the ArraySpec.
 */
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
            if (!tvMatchesRepoAuthType(arr->get(k).tv(), specElemType)) {
              break;
            }
          }
          if (k == arr->size()) return true;
          break;
        }
      }
    }
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

  // Make sure lhs's ptr and mem kinds are subtypes of rhs's.
  if (!(lhs.ptrKind() <= rhs.ptrKind()) ||
      !(lhs.memKind() <= rhs.memKind())) {
    return false;
  }

  // If `rhs' isn't specialized no further checking is needed.
  if (!rhs.isSpecialized()) {
    return true;
  }

  if (lhs.hasConstVal(TArr)) {
    // Arrays can be specialized in different ways.  Here, we check if the
    // constant array fits the kind()/type() of the specialization of `rhs', if
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
  auto const mem = lhs.memKind() | rhs.memKind();
  auto const spec = lhs.spec() | rhs.spec();

  return Type{bits, ptr, mem}.specialize(spec);
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
  auto mem = lhs.memKind() & rhs.memKind();
  auto arrSpec = lhs.arrSpec() & rhs.arrSpec();
  auto clsSpec = lhs.clsSpec() & rhs.clsSpec();

  // Certain component sublattices of Type are dependent on one another.  For
  // each set of such "interfering" components, if any component goes to
  // Bottom, we have to Bottom out the other components in the set as well.

  // Gen bits depend on both Ptr and Mem.
  if (ptr == Ptr::Bottom || mem == Mem::Bottom) bits &= ~kGen;

  // Arr/Cls bits and specs.
  if (arrSpec == ArraySpec::Bottom) bits &= ~kArrSpecBits;
  if (clsSpec == ClassSpec::Bottom) bits &= ~kClsSpecBits;
  if (!supports(bits, SpecKind::Array)) arrSpec = ArraySpec::Bottom;
  if (!supports(bits, SpecKind::Class)) clsSpec = ClassSpec::Bottom;

  // Ptr and Mem also depend on Gen bits. This must come after all possible
  // fixups of bits.
  if ((bits & kGen) == 0) {
    ptr = Ptr::Bottom;
    mem = Mem::Bottom;
  } else {
    if ((ptr & Ptr::Ptr) == Ptr::Bottom)    mem &= ~Mem::Mem;
    if ((mem & Mem::Mem) == Mem::Bottom)    ptr &= ~Ptr::Ptr;
    if ((ptr & Ptr::NotPtr) == Ptr::Bottom) mem &= ~Mem::NotMem;
    if ((mem & Mem::NotMem) == Mem::Bottom) ptr &= ~Ptr::NotPtr;

    static_assert(Ptr::Top == (Ptr::Ptr | Ptr::NotPtr), "");
    static_assert(Mem::Top == (Mem::Mem | Mem::NotMem), "");
  }

  return Type{bits, ptr, mem}.specialize({arrSpec, clsSpec});
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
  auto mem = lhs.memKind() - rhs.memKind();
  auto arrSpec = lhs.arrSpec() - rhs.arrSpec();
  auto clsSpec = lhs.clsSpec() - rhs.clsSpec();

  auto const have_gen_bits = (bits & kGen) != 0;

  auto const have_ptr     = (ptr & Ptr::Ptr) != Ptr::Bottom;
  auto const have_not_ptr = (ptr & Ptr::NotPtr) != Ptr::Bottom;
  auto const have_any_ptr = have_ptr || have_not_ptr;
  auto const have_mem     = (mem & Mem::Mem) != Mem::Bottom;
  auto const have_not_mem = (mem & Mem::NotMem) != Mem::Bottom;
  auto const have_any_mem = have_mem || have_not_mem;
  auto const have_memness = have_any_ptr || have_any_mem;

  auto const have_arr_bits = supports(bits, SpecKind::Array);
  auto const have_cls_bits = supports(bits, SpecKind::Class);
  auto const have_arr_spec = arrSpec != ArraySpec::Bottom;
  auto const have_cls_spec = clsSpec != ClassSpec::Bottom;

  // ptr and mem can only interact with clsSpec if lhs.m_bits has at least one
  // kGen member of kClsSpecBits.
  auto const have_ptr_cls = supports(lhs.m_bits & kGen, SpecKind::Class);

  // bits, ptr, and mem
  if (have_any_ptr) {
    bits |= lhs.m_bits & kGen;
    // The Not{Ptr,Mem} and {Ptr,Mem} components of Ptr and Mem don't interfere
    // with one another, so keep them separate.
    if (have_ptr)     mem |= (lhs.memKind() & Mem::Mem);
    if (have_not_ptr) mem |= (lhs.memKind() & Mem::NotMem);
  }
  if (have_any_mem) {
    bits |= lhs.m_bits & kGen;
    if (have_mem)     ptr |= (lhs.ptrKind() & Ptr::Ptr);
    if (have_not_mem) ptr |= (lhs.ptrKind() & Ptr::NotPtr);
  }
  if (have_arr_spec) bits |= lhs.m_bits & kArrSpecBits;
  if (have_cls_spec) bits |= lhs.m_bits & kClsSpecBits;

  // ptr and mem
  if (have_gen_bits || have_arr_spec || (have_cls_spec && have_ptr_cls)) {
    ptr = lhs.ptrKind();
    mem = lhs.memKind();
  }

  // specs
  if (have_memness || have_arr_bits) {
    arrSpec = lhs.arrSpec();
  }
  if ((have_memness && have_ptr_cls) || have_cls_bits) {
    clsSpec = lhs.clsSpec();
  }

  return Type{bits, ptr, mem}.specialize({arrSpec, clsSpec});
}

///////////////////////////////////////////////////////////////////////////////
// Conversions.

Type typeFromTV(tv_rval tv, const Class* ctx) {
  assertx(tvIsPlausible(*tv));

  if (type(tv) == KindOfObject) {
    auto const cls = val(tv).pobj->getVMClass();

    // We only allow specialization on classes that can't be overridden for
    // now.  If this changes, then this will need to specialize on sub object
    // types instead.
    if (!cls ||
        !(cls->attrs() & AttrNoOverride) ||
        (!(cls->attrs() & AttrUnique) && (!ctx || !ctx->classof(cls)))) {
      return TObj;
    }
    return Type::ExactObj(cls);
  }

  if (tvIsArray(tv)) return Type::Array(val(tv).parr->kind());

  auto outer = type(tv);
  auto inner = KindOfUninit;

  if (outer == KindOfPersistentString) outer = KindOfString;
  else if (outer == KindOfPersistentVec) outer = KindOfVec;
  else if (outer == KindOfPersistentDict) outer = KindOfDict;
  else if (outer == KindOfPersistentKeyset) outer = KindOfKeyset;

  if (isRefType(outer)) {
    inner = val(tv).pref->tv()->m_type;
    if (inner == KindOfPersistentString) inner = KindOfString;
    else if (inner == KindOfPersistentArray) inner = KindOfArray;
    else if (inner == KindOfPersistentVec) inner = KindOfVec;
    else if (inner == KindOfPersistentDict) inner = KindOfDict;
    else if (inner == KindOfPersistentKeyset) inner = KindOfKeyset;
  }
  return Type(outer, inner);
}

Type typeFromRAT(RepoAuthType ty, const Class* ctx) {
  using T = RepoAuthType::Tag;
  switch (ty.tag()) {
    case T::OptBool:        return TBool       | TInitNull;
    case T::OptInt:         return TInt        | TInitNull;
    case T::OptSStr:        return TStaticStr  | TInitNull;
    case T::OptStr:         return TStr        | TInitNull;
    case T::OptDbl:         return TDbl        | TInitNull;
    case T::OptRes:         return TRes        | TInitNull;
    case T::OptObj:         return TObj        | TInitNull;
    case T::OptArrKey:      return TInt | TStr | TInitNull;
    case T::OptUncArrKey:   return TInt | TPersistentStr | TInitNull;

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
    case T::UncArrKey:      return TInt | TPersistentStr;
    case T::ArrKey:         return TInt | TStr;
    case T::InitUnc:        return TUncountedInit;
    case T::Unc:            return TUncounted;
    case T::InitCell:       return TInitCell;
    case T::InitGen:        return TInitGen;
    case T::Gen:            return TGen;

#define X(A, B) \
      [&]{                                                              \
        if (auto const arr = ty.array()) return Type::B(arr);           \
        else return A;                                                  \
      }()

    case T::SArr:           return X(TStaticArr, StaticArray);
    case T::Arr:            return X(TArr, Array);
    case T::SVec:           return X(TStaticVec, StaticVec);
    case T::Vec:            return X(TVec, Vec);
    case T::SDict:          return X(TStaticDict, StaticDict);
    case T::Dict:           return X(TDict, Dict);
    case T::SKeyset:        return X(TStaticKeyset, StaticKeyset);
    case T::Keyset:         return X(TKeyset, Keyset);

    case T::OptSArr:        return X(TStaticArr, StaticArray) | TInitNull;
    case T::OptArr:         return X(TArr, Array)             | TInitNull;
    case T::OptSVec:        return X(TStaticVec, StaticVec)   | TInitNull;
    case T::OptVec:         return X(TVec, Vec)               | TInitNull;
    case T::OptSDict:       return X(TStaticDict, StaticDict) | TInitNull;
    case T::OptDict:        return X(TDict, Dict)             | TInitNull;
    case T::OptSKeyset:     return X(TStaticKeyset, StaticKeyset) | TInitNull;
    case T::OptKeyset:      return X(TKeyset, Keyset)         | TInitNull;
#undef X

#define X(A, B)                                                         \
      [&]{                                                              \
        if (auto const arr = ty.array()) return Type::B(A, arr);        \
        else return Type::B(A);                                         \
      }()

    case T::SVArr:          return X(ArrayData::kPackedKind, StaticArray);
    case T::VArr:           return X(ArrayData::kPackedKind, Array);

    case T::OptSVArr:       return X(ArrayData::kPackedKind, StaticArray)
                                   | TInitNull;
    case T::OptVArr:        return X(ArrayData::kPackedKind, Array)
                                   | TInitNull;

    case T::SDArr:          return X(ArrayData::kMixedKind, StaticArray);
    case T::DArr:           return X(ArrayData::kMixedKind, Array);

    case T::OptSDArr:       return X(ArrayData::kMixedKind, StaticArray)
                                   | TInitNull;
    case T::OptDArr:        return X(ArrayData::kMixedKind, Array)
                                   | TInitNull;
#undef X

    case T::SubObj:
    case T::ExactObj:
    case T::OptSubObj:
    case T::OptExactObj: {
      auto base = TObj;

      if (auto const cls = Unit::lookupUniqueClassInContext(ty.clsName(),
                                                            ctx)) {
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
    if (tmp.maybe(TCountedVec)) tmp |= TVec;
    if (tmp.maybe(TCountedDict)) tmp |= TDict;
    if (tmp.maybe(TCountedKeyset)) tmp |= TKeyset;
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
  } else if (t <= TVec) {
    t = TVec;
  } else if (t <= TDict) {
    t = TDict;
  } else if (t <= TKeyset) {
    t = TKeyset;
  } else if (t <= TArrLike) {
    t = TArrLike;
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
    case DataTypeBoxAndCountness:
      return !t.maybe(TCounted) ? TUncounted : t.unspecialize();

    case DataTypeBoxAndCountnessInit:
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
  always_assert_flog(t <= TGen, "t = {}", t);
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
  if (ty <= TVec) return TVec;
  if (ty <= TDict) return TDict;
  if (ty <= TKeyset) return TKeyset;

  if (ty <= TArrLike) return TArrLike;

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
