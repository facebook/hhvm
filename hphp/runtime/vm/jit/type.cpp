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

#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/jit/guard-constraint.h"
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

///////////////////////////////////////////////////////////////////////////////

// Static member definitions.
// This section can be safely deleted in C++17.
constexpr Type::bits_t Type::kBottom;
constexpr Type::bits_t Type::kTop;

#define IRT(name, bits)       constexpr Type::bits_t Type::k##name;
#define IRTP(name, ptr, bits)
#define IRTL(name, ptr, bits)
#define IRTM(name, ptr, bits)
#define IRTX(name, ptr, bits)
  IR_TYPES
#undef IRT
#undef IRTP
#undef IRTL
#undef IRTM
#undef IRTX

constexpr Type::bits_t Type::kArrSpecBits;
constexpr Type::bits_t Type::kClsSpecBits;

///////////////////////////////////////////////////////////////////////////////
// Vanilla array-spec manipulation.

Type Type::narrowToVanilla() const {
  return narrowToLayout(ArrayLayout::Vanilla());
}

Type Type::narrowToLayout(ArrayLayout layout) const {
  if (!supports(SpecKind::Array)) return *this;
  if (supports(SpecKind::Class) || supports(SpecKind::Record)) return *this;
  auto const spec = arrSpec().narrowToLayout(layout);
  if (spec == ArraySpec::Bottom()) return TBottom;
  return Type(*this, spec);
}

///////////////////////////////////////////////////////////////////////////////

const ArrayData* Type::arrLikeVal() const {
  assertx(hasConstVal(TArrLike));
  assertx(subtypeOfAny(TVec, TDict, TKeyset));
  return reinterpret_cast<const ArrayData*>(m_extra);
}

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
  if (*this <= TArrLike) {
    auto const type = getDataTypeString(m_arrVal->toDataType());
    auto const layout = arrSpec().layout().describe();
    return m_arrVal->empty()
      ? folly::sformat("{}[]={}", type, layout)
      : folly::sformat("{}({})={}", type, m_arrVal, layout);
  }
  if (*this <= TFunc) {
    return folly::format("Func({})", m_funcVal->fullName()->data()).str();
  }
  if (*this <= TCls) {
    return folly::format("Cls({})", m_clsVal->name()->data()).str();
  }
  if (*this <= TLazyCls) {
    return folly::format("LazyCls({})", m_lclsVal.name()->data()).str();
  }
  if (*this <= TClsMeth) {
    return folly::format("ClsMeth({},{})",
      m_clsmethVal->getCls() ?
      m_clsmethVal->getCls()->name()->data() : "nullptr",
      m_clsmethVal->getFunc() ?
      m_clsmethVal->getFunc()->fullName()->data() : "nullptr"
    ).str();
  }
  if (*this <= TRecDesc) {
    return folly::format("RecDesc({})", m_recVal->name()->data()).str();
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
  if (*this <= TPtrToCell) {
    return folly::sformat("TV: {}", m_ptrVal);
  }
  if (*this <= TLvalToCell) {
    return folly::sformat("Lval: {}", m_ptrVal);
  }
  if (*this <= TMemToCell) {
    return folly::sformat("Mem: {}", m_ptrVal);
  }

  always_assert_flog(
    false,
    "Bad type in constValString(): {}:{}:{}:{}:{:#16x}",
    m_bits.hexStr(),
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
    PTR_TYPES(PTRT)
#undef PTRT
  }

  std::vector<const char*> parts;
#define PTRT(name, ...) \
  if (Ptr::name <= ptr) parts.emplace_back(#name);
  PTR_PRIMITIVE(PTRT)
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

  if (m_hasConstVal) {
    if (*this <= TCls) {
      return folly::sformat("Cls={}", m_clsVal->name()->data());
    }
    if (*this <= TLazyCls) {
      return folly::sformat("LazyCls={}", m_lclsVal.name()->data());
    }
    if (*this <= TRecDesc) {
      return folly::sformat("RecDesc={}", m_recVal->name()->data());
    }
    return folly::sformat("{}<{}>",
                          dropConstVal().toString(), constValString());
  }

  auto t = *this;

  if (t.maybe(TPtrToCell)) {
    assertx(!t.m_hasConstVal);
    auto ret = "PtrTo" +
      show(t.ptrKind() & Ptr::Ptr) +
      (t & TPtrToCell).deref().toString();

    t -= TPtrToCell;
    if (t != TBottom) ret += "|" + t.toString();
    return ret;
  }

  if (t.maybe(TLvalToCell)) {
    assertx(!t.m_hasConstVal);
    auto ret = "LvalTo" +
      show(t.ptrKind() & Ptr::Ptr) +
      (t & TLvalToCell).deref().toString();

    t -= TLvalToCell;
    if (t != TBottom) ret += "|" + t.toString();
    return ret;
  }

  assertx(t.ptrKind() <= Ptr::NotPtr);
  assertx(t.memKind() <= Mem::NotMem);

  std::vector<std::string> parts;
  if (isSpecialized()) {
    if (auto const cls = t.clsSpec()) {
      auto const base = Type{m_bits & kClsSpecBits, t.ptrKind(), t.memKind()};
      parts.push_back(folly::to<std::string>(base.toString(), cls.toString()));
      t -= base;
    } else if (auto const rec = t.recSpec()) {
      auto const base = Type{m_bits & kRecSpecBits, t.ptrKind(), t.memKind()};
      parts.push_back(folly::to<std::string>(base.toString(), rec.toString()));
      t -= base;
    } else if (auto const arr = t.arrSpec()) {
      auto const base = Type{m_bits & kArrSpecBits, t.ptrKind(), t.memKind()};
      parts.push_back(folly::to<std::string>(base.toString(), arr.toString()));
      t -= base;
    } else {
      always_assert(false);
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
        auto const pop1 = a.first.m_bits.count();
        auto const pop2 = b.first.m_bits.count();
        if (pop1 != pop2) return pop1 > pop2;
        return std::strcmp(a.second, b.second) < 0;
      }
    );
    // Remove Bottom
    while (types.back().first.m_bits == kBottom) types.pop_back();
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
  ArrSpec,
  RecSub,
  RecExact,
};

TypeKey getTypeKey(const Type& t) {
  return t.hasConstVal() ? TypeKey::Const :
    t.clsSpec() ? (t.clsSpec().exact() ? TypeKey::ClsExact : TypeKey::ClsSub) :
    t.recSpec() ? (t.recSpec().exact() ? TypeKey::RecExact : TypeKey::RecSub) :
    t.arrSpec() ? TypeKey::ArrSpec : TypeKey::None;
}
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

  auto const key = getTypeKey(t);
  write_raw(ser, key);

  if (key == TypeKey::Const) {
    if (t <= TCls)       return write_class(ser, t.m_clsVal);
    if (t <= TLazyCls)   return write_lclass(ser, t.m_lclsVal);
    if (t <= TFunc)      return write_func(ser, t.m_funcVal);
    if (t <= TStaticStr) return write_string(ser, t.m_strVal);
    if (t < TArrLike) {
      return write_array(ser, t.m_arrVal);
    }
    if (use_lowptr && (t <= TClsMeth)) {
      return write_clsmeth(ser, t.m_clsmethVal);
    }
    assertx(t.subtypeOfAny(TBool, TInt, TDbl));
    return write_raw(ser, t.m_extra);
  }

  if (key == TypeKey::ClsSub || key == TypeKey::ClsExact) {
    return write_class(ser, t.clsSpec().cls());
  }
  if (key == TypeKey::RecSub || key == TypeKey::RecExact) {
    return write_record(ser, t.recSpec().rec());
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
      if (t <= TLazyCls) {
        t.m_lclsVal = read_lclass(ser);
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
      if (use_lowptr && (t <= TClsMeth)) {
        t.m_clsmethVal = read_clsmeth(ser);
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
    } else if (key == TypeKey::RecSub || key == TypeKey::RecExact) {
      auto const rec = read_record(ser);
      if (key == TypeKey::RecExact) {
        t.m_recSpec = RecordSpec{rec, RecordSpec::ExactTag{}};
      } else {
        t.m_recSpec = RecordSpec{rec, RecordSpec::SubTag{}};
      }
    } else {
      assertx(key == TypeKey::ArrSpec);
      read_raw(ser, t.m_extra);
      if (auto const arr = t.m_arrSpec.type()) {
        t.m_arrSpec.setType(ser.remap(arr));
      }
    }
    return t;
  }();
  ITRACE_MOD(Trace::hhbc, 2, "Type: {}\n", ret.toString());
  return ret;
}

size_t Type::stableHash() const {
  // Base hash
  auto const hash = hash_int64_pair(
    m_bits.hash(),
    ((static_cast<uint64_t>(m_ptr) << sizeof(m_mem) * CHAR_BIT) |
     static_cast<uint64_t>(m_mem)) ^ m_hasConstVal
  );

  // Specialization data
  Type t = *this;
  auto const key = getTypeKey(t);
  auto const extra = [&] () -> size_t {
    if (key == TypeKey::Const) {
      if (t <= TCls) return t.m_clsVal->stableHash();
      if (t <= TLazyCls) return t.m_lclsVal.name()->hashStatic();
      if (t <= TFunc) return t.m_funcVal->stableHash();
      if (t <= TStaticStr) return t.m_strVal->hashStatic();
      if (t < TArrLike) {
        return internal_serialize(
          VarNR(const_cast<ArrayData*>(t.m_arrVal))
        ).get()->hash();
      }
      if (use_lowptr && (t <= TClsMeth)) {
        auto const cls = t.m_clsmethVal->getCls();
        auto const func = t.m_clsmethVal->getFunc();
        return (cls ? cls->stableHash() : 0 ) ^ (func ? func->stableHash() : 0);
      }
      if (t.subtypeOfAny(TBool, TInt, TDbl)) return t.m_extra;
    }
    if (key == TypeKey::ClsSub || key == TypeKey::ClsExact) {
      auto const cls = t.clsSpec().cls();
      return cls ? cls->stableHash() : 0;
    }
    if (key == TypeKey::RecSub || key == TypeKey::RecExact) {
      auto const rec = t.recSpec().rec();
      return rec ? rec->stableHash() : 0;
    }
    if (key == TypeKey::ArrSpec) {
      auto const spec = t.arrSpec();
      return folly::hash::hash_combine(
        spec.layout().toUint16(),
        spec.type() ? spec.type()->id() : 0
      );
    }
    return 0;
  }();

  return folly::hash::hash_combine(
    hash,
    key,
    extra
  );
}

///////////////////////////////////////////////////////////////////////////////

bool Type::checkValid() const {
  // NOTE: Be careful: the TFoo objects aren't all constructed yet in this
  // function, and we can't call operator<=, etc. because they call checkValid.
  auto constexpr kNonNullConstVals = kArrLike | kCls | kLazyCls |
                                     kFunc | kRecDesc | kStr;
  if (m_hasConstVal && ((m_bits & kNonNullConstVals) == m_bits)) {
    assert_flog(m_extra, "Null constant type: {}", m_bits.hexStr());
  }
  if (m_extra) {
    assert_flog(((m_bits & kClsSpecBits) == kBottom ||
                 (m_bits & kRecSpecBits) == kBottom ||
                 (m_bits & kArrSpecBits) == kBottom) &&
                "Conflicting specialization: {}", m_bits.hexStr());
  }

  // We should have one canonical representation of Bottom.
  if (m_bits == kBottom) {
    assert_flog(*this == TBottom,
                "Bottom m_bits but nonzero others in {}:{}:{}:{:#16x}",
                m_bits.hexStr(), m_ptrVal, m_hasConstVal, m_extra);
  }

  // m_ptr and m_mem should be Bottom iff we have no kCell bits.
  assertx(((m_bits & kCell) == kBottom) == (m_ptr == Ptr::Bottom));
  assertx(((m_bits & kCell) == kBottom) == (m_mem == Mem::Bottom));

  // Ptr::NotPtr and Mem::NotMem should imply one another.
  assertx((m_ptr == Ptr::NotPtr) == (m_mem == Mem::NotMem));

  return true;
}

Type::bits_t Type::bitsFromDataType(DataType outer) {
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

    case KindOfVec              : return kVec;
    case KindOfDict             : return kDict;
    case KindOfKeyset           : return kKeyset;

    case KindOfResource         : return kRes;
    case KindOfObject           : return kObj;
    case KindOfRFunc            : return kRFunc;
    case KindOfFunc             : return kFunc;
    case KindOfClass            : return kCls;
    case KindOfLazyClass        : return kLazyCls;
    case KindOfClsMeth          : return kClsMeth;
    case KindOfRClsMeth         : return kRClsMeth;
    case KindOfRecord           : return kRecord;
  }
  not_reached();
}

DataType Type::toDataType() const {
  assertx(!maybe(TMemToCell) || m_bits == kBottom);
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
  if (*this <= TLazyCls)     return KindOfLazyClass;
  if (*this <= TClsMeth)     return KindOfClsMeth;
  if (*this <= TRecord)      return KindOfRecord;
  if (*this <= TRFunc)       return KindOfRFunc;
  if (*this <= TRClsMeth)    return KindOfRClsMeth;
  always_assert_flog(false, "Bad Type {} in Type::toDataType()", *this);
}

///////////////////////////////////////////////////////////////////////////////
// Combinators.

Type Type::specialize(TypeSpec spec) const {
  assertx(!spec.arrSpec() || supports(SpecKind::Array));
  assertx(!spec.clsSpec() || supports(SpecKind::Class));
  assertx(!spec.recSpec() || supports(SpecKind::Record));

  // If we don't have exactly one kind of specialization, or if our bits
  // support more than one kinds, don't specialize.
  if ((bool)spec.arrSpec() + (bool)spec.clsSpec() + (bool)spec.recSpec() != 1) {
    return *this;
  }
  if (supports(SpecKind::Array) +
      supports(SpecKind::Class) +
      supports(SpecKind::Record) > 1) return *this;

  if (spec.arrSpec() != ArraySpec::Bottom()) {
    return Type{*this, spec.arrSpec()};
  }

  if (spec.clsSpec() != ClassSpec::Bottom()) {
    return Type{*this, spec.clsSpec()};
  }

  assertx(spec.recSpec() != RecordSpec::Bottom());
  return Type{*this, spec.recSpec()};
}

Type Type::modified() const {
  auto t = unspecialize();
  if (t.maybe(TVec))    t |= TVec;
  if (t.maybe(TDict))   t |= TDict;
  if (t.maybe(TKeyset)) t |= TKeyset;
  if (t.maybe(TStr))    t |= TStr;
  auto const spec = ArraySpec(ArrayLayout::Vanilla());
  return arrSpec().vanilla() ? Type(t, spec) : t;
}

/*
 * Return true if the array satisfies requirement on the ArraySpec.
 * If the kind and RepoAuthType are both set, the array must match both.
 */
static bool arrayFitsSpec(const ArrayData* arr, ArraySpec spec) {
  if (spec == ArraySpec::Top()) return true;
  if (spec == ArraySpec::Bottom()) return false;

  if (!(ArrayLayout::FromArray(arr) <= spec.layout())) {
    return false;
  }

  if (!spec.type()) return true;

  using A = RepoAuthType::Array;
  auto const& type = *spec.type();
  if (arr->empty()) return type.emptiness() != A::Empty::No;

  // Right now, the only non-trivial RepoAuthType::Array we support is for
  // arrays with "vector" keys (0, 1, ... n - 1). They don't have to be packed.
  if (!arr->isVectorData()) return false;
  switch (type.tag()) {
    case A::Tag::Packed:
      if (arr->size() != type.size()) return false;
      // fall through
    case A::Tag::PackedN: {
      int64_t k = 0;
      auto const packed = type.tag() == A::Tag::Packed;
      for ( ; k < arr->size(); ++k) {
        auto const elemType = packed ? type.packedElem(k) : type.elemType();
        if (!tvMatchesRepoAuthType(arr->get(k), elemType)) {
          return false;
        }
      }
      return true;
    }
  }

  always_assert(false);
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
    assertx(rhs.ptrKind() == Ptr::Elem || !rhs.isUnion());
    return lhs.m_hasConstVal && lhs.m_extra == rhs.m_extra;
  }

  // Make sure lhs's ptr and mem kinds are subtypes of rhs's.
  if (!(lhs.ptrKind() <= rhs.ptrKind()) ||
      !(lhs.memKind() <= rhs.memKind())) {
    return false;
  }

  // Compare specializations only if `rhs' is specialized.
  if (!rhs.isSpecialized()) {
    return true;
  }
  if (lhs.hasConstVal(TArrLike)) {
    return arrayFitsSpec(lhs.m_arrVal, rhs.arrSpec());
  }
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

  // When intersecting a constant non-ptr type with another type, the result is
  // the constant type if the other type is a supertype of the constant, and
  // Bottom otherwise. However, for pointer types, we have to account for the
  // fact that we can't have a constant, specialized pointer.
  auto const handle_constant = [](const Type& constant, const Type& other) {
    auto const refines = constant.m_ptr == Ptr::NotPtr
      ? constant <= other
      : constant <= other.unspecialize();
    return refines ? constant : TBottom;
  };

  if (lhs.m_hasConstVal) return handle_constant(lhs, rhs);
  if (rhs.m_hasConstVal) return handle_constant(rhs, lhs);

  auto bits = lhs.m_bits & rhs.m_bits;
  auto ptr = lhs.ptrKind() & rhs.ptrKind();
  auto mem = lhs.memKind() & rhs.memKind();
  auto arrSpec = lhs.arrSpec() & rhs.arrSpec();
  auto clsSpec = lhs.clsSpec() & rhs.clsSpec();
  auto recSpec = lhs.recSpec() & rhs.recSpec();

  // Certain component sublattices of Type are dependent on one another.  For
  // each set of such "interfering" components, if any component goes to
  // Bottom, we have to Bottom out the other components in the set as well.

  // Cell bits depend on both Ptr and Mem.
  if (ptr == Ptr::Bottom || mem == Mem::Bottom) bits &= ~kCell;

  // Arr/Cls bits and specs.
  if (arrSpec == ArraySpec::Bottom()) bits &= ~kArrSpecBits;
  if (clsSpec == ClassSpec::Bottom()) bits &= ~kClsSpecBits;
  if (recSpec == RecordSpec::Bottom()) bits &= ~kRecSpecBits;
  if (!supports(bits, SpecKind::Array)) arrSpec = ArraySpec::Bottom();
  if (!supports(bits, SpecKind::Class)) clsSpec = ClassSpec::Bottom();
  if (!supports(bits, SpecKind::Record)) recSpec = RecordSpec::Bottom();

  // Ptr and Mem also depend on Cell bits. This must come after all possible
  // fixups of bits.
  if ((bits & kCell) == kBottom) {
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

  return Type{bits, ptr, mem}.specialize({arrSpec, clsSpec, recSpec});
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
  auto recSpec = lhs.recSpec() - rhs.recSpec();

  auto const have_gen_bits = (bits & kCell) != kBottom;

  auto const have_ptr     = (ptr & Ptr::Ptr) != Ptr::Bottom;
  auto const have_not_ptr = (ptr & Ptr::NotPtr) != Ptr::Bottom;
  auto const have_any_ptr = have_ptr || have_not_ptr;
  auto const have_mem     = (mem & Mem::Mem) != Mem::Bottom;
  auto const have_not_mem = (mem & Mem::NotMem) != Mem::Bottom;
  auto const have_any_mem = have_mem || have_not_mem;
  auto const have_memness = have_any_ptr || have_any_mem;

  auto const have_arr_bits = supports(bits, SpecKind::Array);
  auto const have_cls_bits = supports(bits, SpecKind::Class);
  auto const have_rec_bits = supports(bits, SpecKind::Record);
  auto const have_arr_spec = arrSpec != ArraySpec::Bottom();
  auto const have_cls_spec = clsSpec != ClassSpec::Bottom();
  auto const have_rec_spec = recSpec != RecordSpec::Bottom();

  // ptr and mem can only interact with clsSpec if lhs.m_bits has at least one
  // kCell member of kClsSpecBits.
  auto const have_ptr_cls = supports(lhs.m_bits & kCell, SpecKind::Class);

  // bits, ptr, and mem
  if (have_any_ptr) {
    bits |= lhs.m_bits & kCell;
    // The Not{Ptr,Mem} and {Ptr,Mem} components of Ptr and Mem don't interfere
    // with one another, so keep them separate.
    if (have_ptr)     mem |= (lhs.memKind() & Mem::Mem);
    if (have_not_ptr) mem |= (lhs.memKind() & Mem::NotMem);
  }
  if (have_any_mem) {
    bits |= lhs.m_bits & kCell;
    if (have_mem)     ptr |= (lhs.ptrKind() & Ptr::Ptr);
    if (have_not_mem) ptr |= (lhs.ptrKind() & Ptr::NotPtr);
  }
  if (have_arr_spec) bits |= lhs.m_bits & kArrSpecBits;
  if (have_cls_spec) bits |= lhs.m_bits & kClsSpecBits;
  if (have_rec_spec) bits |= lhs.m_bits & kRecSpecBits;

  // ptr and mem
  if (have_gen_bits || have_arr_spec ||
      (have_cls_spec && have_ptr_cls) || have_rec_spec) {
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
  if (have_memness || have_rec_bits) {
    recSpec = lhs.recSpec();
  }

  return Type{bits, ptr, mem}.specialize({arrSpec, clsSpec, recSpec});
}

///////////////////////////////////////////////////////////////////////////////
// Conversions.

Type typeFromTV(tv_rval tv, const Class* ctx) {
  assertx(tvIsPlausible(*tv));

  if (type(tv) == KindOfObject) {
    auto const cls = val(tv).pobj->getVMClass();
    assertx(cls);

    // We only allow specialization on classes that can't be overridden for
    // now.  If this changes, then this will need to specialize on sub object
    // types instead.
    if (!(cls->attrs() & AttrNoOverride) ||
        (!(cls->attrs() & AttrUnique) && (!ctx || !ctx->classof(cls)))) {
      return TObj;
    }
    return Type::ExactObj(cls);
  }
  if (isRecordType(type(tv))) {
    auto const rec = val(tv).prec->record();
    assertx(rec);

    if (!(rec->attrs() & AttrFinal) ||
        !(rec->attrs() & AttrUnique)) {
      return TRecord;
    }
    return Type::ExactRecord(rec);
  }

  auto const result = Type(dt_modulo_persistence(type(tv)));

  if (isArrayLikeType(type(tv))) {
    return allowBespokeArrayLikes() ? result : result.narrowToVanilla();
  }

  return result;
}

namespace {

bool ratArrIsCounted(const RepoAuthType::Array* arr, const Class* ctx) {
  using E = RepoAuthType::Array::Empty;
  using T = RepoAuthType::Array::Tag;

  if (arr->emptiness() == E::Maybe) return false;

  switch (arr->tag()) {
    case T::Packed: {
      auto const size = arr->size();
      for (size_t i = 0; i < size; ++i) {
        if (!(typeFromRAT(arr->packedElem(i), ctx) <= TCounted)) {
          return false;
        }
      }
      return true;
    }
    case T::PackedN:
      return typeFromRAT(arr->elemType(), ctx) <= TCounted;
  }

  return false;
}

}

//////////////////////////////////////////////////////////////////////

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
    case T::OptFunc:        return TFunc       | TInitNull;
    case T::OptCls:         return TCls        | TInitNull;
    case T::OptLazyCls:     return TLazyCls    | TInitNull;
    case T::OptClsMeth:     return TClsMeth    | TInitNull;
    case T::OptRecord:      return TRecord     | TInitNull;
    case T::OptArrKey:      return TInt | TStr | TInitNull;
    case T::OptUncArrKey:   return TInt | TPersistentStr | TInitNull;
    case T::OptUncStrLike:
      return TCls | TLazyCls | TPersistentStr | TInitNull;
    case T::OptStrLike:
      return TCls | TLazyCls | TStr | TInitNull;
    case T::OptArrKeyCompat:
      return TInt | TStr | TCls | TLazyCls | TInitNull;
    case T::OptUncArrKeyCompat:
      return TInt | TPersistentStr | TCls | TLazyCls | TInitNull;

    case T::UninitBool:     return TBool      | TUninit;
    case T::UninitInt:      return TInt       | TUninit;
    case T::UninitStr:      return TStr       | TUninit;
    case T::UninitSStr:     return TStaticStr | TUninit;
    case T::UninitObj:      return TObj       | TUninit;

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
    case T::Func:           return TFunc;
    case T::Cls:            return TCls;
    case T::LazyCls:        return TLazyCls;
    case T::ClsMeth:        return TClsMeth;
    case T::Record:         return TRecord;

    case T::Cell:           return TCell;
    case T::UncArrKey:      return TInt | TPersistentStr;
    case T::ArrKey:         return TInt | TStr;
    case T::UncStrLike:
      return TCls | TLazyCls | TPersistentStr;
    case T::StrLike:
      return TCls | TLazyCls | TStr;
    case T::UncArrKeyCompat:
      return TInt | TPersistentStr | TCls | TLazyCls;
    case T::ArrKeyCompat:
      return TInt | TStr | TCls | TLazyCls;
    case T::Num:            return TInt | TDbl;
    case T::OptNum:         return TInitNull | TInt | TDbl;
    case T::InitPrim:       return TInitNull | TBool | TInt | TDbl;
    case T::InitUnc:        return TUncountedInit;
    case T::Unc:            return TUncounted;
    case T::NonNull:        return TNonNull;
    case T::InitCell:       return TInitCell;

#define X(A, B, C)                                                      \
      [&]{                                                              \
        if (auto const arr = ty.array()) {                              \
          if (ratArrIsCounted(arr, ctx)) {                              \
            return Type::C(arr);                                        \
          } else {                                                      \
            return Type::B(arr);                                        \
          }                                                             \
        } else {                                                        \
          return A;                                                     \
        }                                                               \
      }()

    case T::SVec:           return X(TStaticVec, StaticVec, StaticVec);
    case T::Vec:            return X(TVec, Vec, CountedVec);
    case T::SDict:          return X(TStaticDict, StaticDict, StaticDict);
    case T::Dict:           return X(TDict, Dict, CountedDict);
    case T::SKeyset:        return X(TStaticKeyset, StaticKeyset, StaticKeyset);
    case T::Keyset:         return X(TKeyset, Keyset, Keyset);
    case T::VecCompat:      return X(TVec, Vec, CountedVec) | TClsMeth;

    case T::OptSVec:        return X(TStaticVec, StaticVec, StaticVec)
                                   | TInitNull;
    case T::OptVec:         return X(TVec, Vec, CountedVec)
                                   | TInitNull;
    case T::OptSDict:       return X(TStaticDict, StaticDict, StaticDict)
                                   | TInitNull;
    case T::OptDict:        return X(TDict, Dict, CountedDict)
                                   | TInitNull;
    case T::OptSKeyset:     return X(TStaticKeyset, StaticKeyset, StaticKeyset)
                                   | TInitNull;
    case T::OptKeyset:      return X(TKeyset, Keyset, Keyset)
                                   | TInitNull;
    case T::OptVecCompat:   return X(TVec, Vec, CountedVec)
                                   | TClsMeth | TInitNull;

    // If these have specializations, its not clear which type to
    // apply them to. Ignore them for now.
    case T::ArrLike:          return TArrLike;
    case T::OptArrLike:       return TArrLike | TInitNull;
    case T::SArrLike:         return TStaticArrLike;
    case T::OptSArrLike:      return TStaticArrLike | TInitNull;
    case T::ArrLikeCompat:    return TArrLike | TClsMeth;
    case T::OptArrLikeCompat: return TArrLike | TClsMeth | TInitNull;

#undef X

    case T::SubObj:
    case T::ExactObj:
    case T::OptSubObj:
    case T::OptExactObj:
    case T::UninitSubObj:
    case T::UninitExactObj: {
      auto base = TObj;

      if (auto const cls = Class::lookupUniqueInContext(ty.clsName(),
                                                        ctx, nullptr)) {
        if (ty.tag() == T::ExactObj ||
            ty.tag() == T::OptExactObj ||
            ty.tag() == T::UninitExactObj) {
          base = Type::ExactObj(cls);
        } else {
          base = Type::SubObj(cls);
        }
      }
      if (ty.tag() == T::OptSubObj || ty.tag() == T::OptExactObj) {
        base |= TInitNull;
      } else if (ty.tag() == T::UninitSubObj || ty.tag() == T::UninitExactObj) {
        base |= TUninit;
      }
      return base;
    }

    case T::SubCls:
    case T::ExactCls:
    case T::OptSubCls:
    case T::OptExactCls: {
      auto base = TCls;

      if (auto const cls = Class::lookupUniqueInContext(ty.clsName(),
                                                            ctx, nullptr)) {
        if (ty.tag() == T::ExactCls || ty.tag() == T::OptExactCls) {
          base = Type::ExactCls(cls);
        } else {
          base = Type::SubCls(cls);
        }
      }
      if (ty.tag() == T::OptSubCls || ty.tag() == T::OptExactCls) {
        base |= TInitNull;
      }
      return base;
    }

    case T::SubRecord:
    case T::ExactRecord:
    case T::OptSubRecord:
    case T::OptExactRecord: {
      auto base = TRecord;

      if (auto const rec = Unit::lookupUniqueRecDesc(ty.recordName())) {
        if (ty.tag() == T::ExactRecord || ty.tag() == T::OptExactRecord) {
          base = Type::ExactRecord(rec);
        } else {
          base = Type::SubRecord(rec);
        }
      }
      if (ty.tag() == T::OptSubRecord || ty.tag() == T::OptExactRecord) {
        base |= TInitNull;
      }
      return base;
    }
  }
  not_reached();
}

Type typeFromPropTC(const HPHP::TypeConstraint& tc,
                    const Class* propCls,
                    const Class* ctx,
                    bool isSProp) {
  assertx(tc.validForProp());

  if (!tc.isCheckable() || tc.isSoft()) return TCell;

  using A = AnnotType;
  auto const atToType = [&](AnnotType at) {
    switch (at) {
      case A::Null:       return TNull;
      case A::Bool:       return TBool;
      case A::Int:        return TInt;
      case A::Float:      return TDbl;
      case A::String:     return TStr;
      case A::Record:     return TRecord;
      // We only call this once we've attempted resolving the
      // type-constraint. If we successfully resolved it, we'll never get here,
      // So if we're here and we have AnnotType::Object, we don't know what the
      // type-hint is, so be conservative.
      case A::Object:
      case A::Mixed:      return TCell;
      case A::Resource:   return TRes;
      case A::Dict:       return TDict;
      case A::Vec:        return TVec;
      case A::Keyset:     return TKeyset;
      case A::Nonnull:    return TInitCell - TInitNull;
      case A::Number:     return TInt | TDbl;
      case A::ArrayKey:   return TInt | TStr;
      case A::VecOrDict:  return TVec | TDict;
      case A::ArrayLike:  return TArrLike;
      case A::Classname:  return TStr | TCls | TLazyCls;
      case A::This:
        always_assert(propCls != nullptr);
        return (isSProp && !tc.couldSeeMockObject())
          ? Type::ExactObj(propCls)
          : Type::SubObj(propCls);
      case A::Nothing:
      case A::NoReturn:
      case A::Self:
      case A::Parent:
      case A::Callable:
        break;
    }
    always_assert(false);
  };

  auto base = [&]{
    if (!tc.isObject()) return atToType(tc.type());

    auto const handleCls = [&] (const Class* cls) {
      // Don't try to be clever with magic interfaces
      if (interface_supports_non_objects(cls->name())) return TInitCell;

      if (isEnum(cls)) {
        if (auto const dt = cls->enumBaseTy()) return Type{*dt};
        return TInt | TStr;
      }
      return Type::SubObj(cls);
    };

    bool persistent = false;
    if (auto const alias = Unit::lookupTypeAlias(tc.typeName(), &persistent)) {
      if (persistent && !alias->invalid) {
        auto ty = [&]{
          if (alias->klass) return handleCls(alias->klass);
          return atToType(alias->type);
        }();
        if (alias->nullable) ty |= TInitNull;
        return ty;
      }
    }

    if (auto const cls =
          Class::lookupUniqueInContext(tc.typeName(), ctx, nullptr)) {
      return handleCls(cls);
    }

    // It could be an alias to mixed so we might have refs
    return TCell;
  }();
  if (tc.isNullable()) base |= TInitNull;
  return base;
}

//////////////////////////////////////////////////////////////////////

Type negativeCheckType(Type srcType, Type typeParam) {
  if (srcType <= typeParam)      return TBottom;
  if (!srcType.maybe(typeParam)) return srcType;
  // Checks relating to StaticStr and StaticArr are not, in general, precise.
  // They may reject some Statics in some situations, where we only guard using
  // the type tag and not by loading the count field.
  auto tmp = srcType - typeParam;
  if (typeParam.maybe(TPersistent)) {
    if (tmp.maybe(TCountedStr)) tmp |= TStr;
    if (tmp.maybe(TCountedVec)) tmp |= TVec;
    if (tmp.maybe(TCountedDict)) tmp |= TDict;
    if (tmp.maybe(TCountedKeyset)) tmp |= TKeyset;
  }
  return tmp & srcType;
}

//////////////////////////////////////////////////////////////////////

Type relaxType(Type t, DataTypeCategory cat) {
  assertx(t <= TCell);

  switch (cat) {
    case DataTypeGeneric:
      return TCell;

    case DataTypeIterBase:
      if (t <= TUninit) return TUninit;
      if (t <= TUncountedInit) return TUncountedInit;
      if (t <= TArrLike) return TArrLike;
      return t.unspecialize();

    case DataTypeCountnessInit:
      if (t <= TUninit) return TUninit;
      if (t <= TUncountedInit) return TUncountedInit;
      return t.unspecialize();

    case DataTypeSpecific:
      return t.unspecialize();

    case DataTypeSpecialized:
      assertx(t.isSpecialized());
      return t;
  }

  not_reached();
}

Type relaxToConstraint(Type t, const GuardConstraint& gc) {
  assertx(t <= TCell);
  if (!gc.isSpecialized()) return relaxType(t, gc.category);

  assertx(t.isSpecialized());
  assertx(gc.wantClass() + gc.isArrayLayoutSensitive() + gc.wantRecord() == 1);

  // NOTE: This second check here causes us to guard to specific classes where
  // we could guard to superclasses and unify multiple regions. Rethink it.
  if (gc.wantClass()) return t;
  assertx(gc.isArrayLayoutSensitive());
  assertx(allowBespokeArrayLikes());
  return t.arrSpec().vanilla() ? t.unspecialize().narrowToVanilla() : t;
}

Type relaxToGuardable(Type ty) {
  assertx(ty <= TCell);
  ty = ty.unspecialize();

  // We don't support guarding on counted-ness or static-ness, so widen
  // subtypes of any maybe-countable types to the full type.
  if (ty <= TVec) return TVec;
  if (ty <= TDict) return TDict;
  if (ty <= TKeyset) return TKeyset;
  if (ty <= TArrLike) return TArrLike;

  // We can guard on StaticStr but not CountedStr.
  if (ty <= TCountedStr)     return TStr;

  if (ty.isKnownDataType())  return ty;
  if (ty <= TUncountedInit)  return TUncountedInit;
  if (ty <= TUncounted)      return TUncounted;
  if (ty <= TCell)           return TCell;
  not_reached();
}

}}
