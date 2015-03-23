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

#ifndef incl_HPHP_JIT_TYPE_INL_H_
#error "type-inl.h should only be included by type.h"
#endif

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/class.h"

#include "hphp/util/hash.h"

#include <cstring>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

namespace type_detail {
///////////////////////////////////////////////////////////////////////////////

/*
 * Widen a constant value if needed.
 */
template<class T>
using needs_promotion = std::integral_constant<
  bool,
  (std::is_integral<T>::value ||
   std::is_same<T,bool>::value ||
   std::is_enum<T>::value)
>;

template<class T>
typename std::enable_if<needs_promotion<T>::value,uint64_t>::type
promote_cns_if_needed(T t) { return static_cast<uint64_t>(t); }

template<class T>
typename std::enable_if<!needs_promotion<T>::value,T>::type
promote_cns_if_needed(T t) { return t; }

/*
 * Return the Type to use for a given C++ value.
 *
 * The only interesting case is int/bool disambiguation.  Enums are treated as
 * ints.
 */
template<class T>
typename std::enable_if<
  std::is_integral<T>::value || std::is_enum<T>::value,
  Type
>::type for_const(T) {
  return std::is_same<T,bool>::value ? Type::Bool : Type::Int;
}
inline Type for_const(const StringData* sd) {
  assert(sd->isStatic());
  return Type::StaticStr;
}
inline Type for_const(const ArrayData* ad) {
  assert(ad->isStatic());
  return Type::StaticArray(ad->kind());
}
inline Type for_const(double)        { return Type::Dbl; }
inline Type for_const(const Func*)   { return Type::Func; }
inline Type for_const(const Class*)  { return Type::Cls; }
inline Type for_const(ConstCctx)     { return Type::Cctx; }
inline Type for_const(TCA)           { return Type::TCA; }

///////////////////////////////////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////
// Ptr.

inline Ptr add_ref(Ptr p) {
  if (p == Ptr::Unk || p == Ptr::ClsInit || p == Ptr::ClsCns) {
    return p;
  }
  return static_cast<Ptr>(static_cast<uint32_t>(p) | kPtrRefBit);
}

inline Ptr remove_ref(Ptr p) {
  // If p is unknown, or Ptr::Ref, we'll get back unknown.
  return static_cast<Ptr>(static_cast<uint32_t>(p) & ~kPtrRefBit);
}

/*
 * For use in this file.
 */
bool ptr_subtype(Ptr, Ptr);

///////////////////////////////////////////////////////////////////////////////
// Type.

inline Type::Type()
  : m_bits(kBottom)
  , m_ptrKind(0)
  , m_hasConstVal(false)
  , m_extra(0)
{}

inline Type::Type(DataType outer, DataType inner)
  : m_bits(bitsFromDataType(outer, inner))
  , m_ptrKind(0)
  , m_hasConstVal(false)
  , m_extra(0)
{}

inline Type::Type(DataType outer, KindOfAny)
  : m_bits(outer == KindOfRef ? kBoxedCell
                              : bitsFromDataType(outer, KindOfUninit))
  , m_ptrKind(0)
  , m_hasConstVal(false)
  , m_extra(0)
{}

inline Type& Type::operator=(Type b) {
  m_bits = b.m_bits;
  m_hasConstVal = b.m_hasConstVal;
  m_ptrKind = b.m_ptrKind;
  m_extra = b.m_extra;
  return *this;
}

inline size_t Type::hash() const {
  return hash_int64_pair(m_rawInt, m_extra);
}

///////////////////////////////////////////////////////////////////////////////
// Comparison.

inline bool Type::operator==(Type rhs) const {
  return m_bits == rhs.m_bits &&
         m_ptrKind == rhs.m_ptrKind &&
         m_hasConstVal == rhs.m_hasConstVal &&
         m_extra == rhs.m_extra;
}

inline bool Type::operator!=(Type rhs) const {
  return !operator==(rhs);
}

inline bool Type::operator<=(Type rhs) const {
  auto lhs = *this;

  // Check for any members in lhs.m_bits that aren't in rhs.m_bits.
  if ((lhs.m_bits & rhs.m_bits) != lhs.m_bits) {
    return false;
  }

  // Check for Bottom; all the remaining cases assume `lhs' is not Bottom.
  if (lhs.m_bits == kBottom) return true;

  // If `rhs' is a constant, we must be the same constant.
  if (rhs.m_hasConstVal) {
    assert(!rhs.isUnion());
    return lhs.m_hasConstVal && lhs.m_extra == rhs.m_extra;
  }

  // If `rhs' could be a pointer, we must have a subtype relation in pointer
  // kinds or we're not a subtype.  (If `lhs' can't be a pointer, we found out
  // above when we intersected the bits.)  If neither can be a pointer, it's an
  // invariant that `m_ptrKind' will be Ptr::Unk so this will pass.
  if (lhs.ptrKind() != rhs.ptrKind() &&
      !ptr_subtype(lhs.ptrKind(), rhs.ptrKind())) {
    return false;
  }

  // Compare specializations only if `rhs' is specialized.
  return !rhs.isSpecialized() || lhs.spec() <= rhs.spec();
}

inline bool Type::operator>=(Type rhs) const {
  return rhs <= *this;
}

inline bool Type::operator<(Type rhs) const {
  return *this != rhs && *this <= rhs;
}

inline bool Type::operator>(Type rhs) const {
  return rhs < *this;
}

template<typename... Types>
bool Type::subtypeOfAny(Type t2, Types... ts) const {
  return *this <= t2 || subtypeOfAny(ts...);
}
inline bool Type::subtypeOfAny() const {
  return false;
}

inline bool Type::maybe(Type t2) const {
  return (*this & t2) != Bottom;
}

///////////////////////////////////////////////////////////////////////////////
// Is-a methods.

inline bool Type::isUnion() const {
  // This will return true iff more than 1 bit is set in m_bits.
  return (m_bits & (m_bits - 1)) != 0;
}

inline bool Type::isKnownDataType() const {
  assert(*this <= StkElem);

  // Some unions correspond to single KindOfs.
  return subtypeOfAny(Str, Arr, BoxedCell) || !isUnion();
}

inline bool Type::needsReg() const {
  return *this <= StkElem && !isKnownDataType();
}

inline bool Type::needsValueReg() const {
  return !subtypeOfAny(Null, Nullptr);
}

inline bool Type::isSimpleType() const {
  return subtypeOfAny(Bool, Int, Dbl, Null);
}

inline bool Type::isReferenceType() const {
  return subtypeOfAny(Str, Arr, Obj, Res);
}

///////////////////////////////////////////////////////////////////////////////
// Constant type creation.

template<typename T>
Type Type::cns(T val, Type ret) {
  assert(!ret.m_hasConstVal);
  ret.m_hasConstVal = true;

  static_assert(sizeof(T) <= sizeof(ret),
                "Constant data was larger than supported");
  static_assert(std::is_pod<T>::value,
                "Constant data wasn't a pod");

  const auto toCopy = type_detail::promote_cns_if_needed(val);
  static_assert(sizeof(toCopy) == 8,
                "Unexpected size for toCopy");

  std::memcpy(&ret.m_extra, &toCopy, sizeof(toCopy));
  return ret;
}

template<typename T>
Type Type::cns(T val) {
  return cns(val, type_detail::for_const(val));
}

inline Type Type::cns(std::nullptr_t) {
  return Type::Nullptr;
}

inline Type Type::cns(const TypedValue& tv) {
  if (tv.m_type == KindOfUninit) return Type::Uninit;
  if (tv.m_type == KindOfNull)   return Type::InitNull;

  auto ret = [&] {
    switch (tv.m_type) {
      case KindOfUninit:
      case KindOfNull:
        not_reached();

      case KindOfClass:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfDouble:
      case KindOfStaticString:
        return Type(tv.m_type);

      case KindOfString:
        return type_detail::for_const(tv.m_data.pstr);

      case KindOfArray:
        return type_detail::for_const(tv.m_data.parr);

      case KindOfObject:
      case KindOfResource:
      case KindOfRef:
        always_assert(false && "Invalid KindOf for constant TypedValue");
    }
    not_reached();
  }();
  ret.m_hasConstVal = true;
  ret.m_extra = tv.m_data.num;
  return ret;
}

inline Type Type::dropConstVal() const {
  if (!m_hasConstVal) return *this;
  assert(!isUnion());

  if (*this <= StaticArr) {
    return Type::StaticArray(arrVal()->kind());
  }
  return Type(m_bits, ptrKind());
}

///////////////////////////////////////////////////////////////////////////////
// Constant introspection.

inline bool Type::hasConstVal() const {
  return m_hasConstVal;
}

inline bool Type::hasConstVal(Type t) const {
  return hasConstVal() && *this <= t;
}

template<typename T>
bool Type::hasConstVal(T val) const {
  return hasConstVal(cns(val));
}

inline uint64_t Type::rawVal() const {
  assert(hasConstVal());
  return m_extra;
}

#define IMPLEMENT_CNS_VAL(TypeName, name, valtype)      \
  inline valtype Type::name##Val() const {              \
    assert(hasConstVal(TypeName));                      \
    return m_##name##Val;                               \
  }

IMPLEMENT_CNS_VAL(Bool,       bool, bool)
IMPLEMENT_CNS_VAL(Int,        int,  int64_t)
IMPLEMENT_CNS_VAL(Dbl,        dbl,  double)
IMPLEMENT_CNS_VAL(StaticStr,  str,  const StringData*)
IMPLEMENT_CNS_VAL(StaticArr,  arr,  const ArrayData*)
IMPLEMENT_CNS_VAL(Func,       func, const HPHP::Func*)
IMPLEMENT_CNS_VAL(Cls,        cls,  const Class*)
IMPLEMENT_CNS_VAL(Cctx,       cctx, ConstCctx)
IMPLEMENT_CNS_VAL(TCA,        tca,  jit::TCA)
IMPLEMENT_CNS_VAL(RDSHandle,  rdsHandle,  rds::Handle)

#undef IMPLEMENT_CNS_VAL

///////////////////////////////////////////////////////////////////////////////
// Specialized type creation.

inline Type Type::Array(ArrayData::ArrayKind kind) {
  return Type(Type::Arr, ArraySpec(kind));
}

inline Type Type::Array(const RepoAuthType::Array* rat) {
  return Type(Type::Arr, ArraySpec(rat));
}

inline Type Type::Array(const Shape* shape) {
  return Type(Type::Arr, ArraySpec(shape));
}

inline Type Type::StaticArray(ArrayData::ArrayKind kind) {
  return Type(Type::StaticArr, ArraySpec(kind));
}

inline Type Type::StaticArray(const RepoAuthType::Array* rat) {
  return Type(Type::StaticArr, ArraySpec(rat));
}

inline Type Type::StaticArray(const Shape* shape) {
  return Type(Type::StaticArr, ArraySpec(shape));
}

inline Type Type::SubObj(const Class* cls) {
  if (cls->attrs() & AttrNoOverride) return ExactObj(cls);
  return Type(Type::Obj, ClassSpec(cls, ClassSpec::SubTag{}));
}

inline Type Type::ExactObj(const Class* cls) {
  return Type(Type::Obj, ClassSpec(cls, ClassSpec::ExactTag{}));
}

inline Type Type::unspecialize() const {
  return Type(m_bits, ptrKind());
}

///////////////////////////////////////////////////////////////////////////////
// Specialization introspection.

inline bool Type::isSpecialized() const {
  return clsSpec() || arrSpec();
}

inline bool Type::supports(SpecKind kind) const {
  switch (kind) {
    case SpecKind::None:
      return true;
    case SpecKind::Array:
      return m_bits & kAnyArr;
    case SpecKind::Class:
      return m_bits & kAnyObj;
  }
  not_reached();
}

inline ArraySpec Type::arrSpec() const {
  if (!supports(SpecKind::Array)) return ArraySpec::Bottom;

  // Currently, a Type which supports multiple specializations is trivial along
  // all of them.
  if (supports(SpecKind::Class)) return ArraySpec::Top;

  if (m_hasConstVal) {
    return ArraySpec(arrVal()->kind());
  }
  assert(m_arrSpec != ArraySpec::Bottom);
  return m_arrSpec;
}

inline ClassSpec Type::clsSpec() const {
  if (!supports(SpecKind::Class)) return ClassSpec::Bottom;

  // Currently, a Type which supports multiple specializations is trivial along
  // all of them.
  if (supports(SpecKind::Array)) return ClassSpec::Top;

  if (m_hasConstVal) {
    return ClassSpec(clsVal(), ClassSpec::ExactTag{});
  }
  assert(m_clsSpec != ClassSpec::Bottom);
  return m_clsSpec;
}

inline TypeSpec Type::spec() const {
  return TypeSpec(arrSpec(), clsSpec());
}

///////////////////////////////////////////////////////////////////////////////
// Inner types.

inline Type Type::box() const {
  assert(*this <= Cell);
  // Boxing Uninit returns InitNull but that logic doesn't belong here.
  assert(!maybe(Uninit) || *this == Cell);
  return Type(m_bits << kBoxShift,
              ptrKind(),
              isSpecialized() && !m_hasConstVal ? m_extra : 0);
}

inline Type Type::inner() const {
  assert(*this <= BoxedCell);
  return Type(m_bits >> kBoxShift, Ptr::Unk, m_extra);
}

inline Type Type::unbox() const {
  assert(*this <= Gen);
  return (*this & Cell) | (*this & BoxedCell).inner();
}

inline Type Type::ptr(Ptr kind) const {
  assert(*this <= Gen);
  return Type(m_bits << kPtrShift,
              kind,
              isSpecialized() && !m_hasConstVal ? m_extra : 0);
}

inline Type Type::deref() const {
  assert(*this <= PtrToGen);
  return Type(m_bits >> kPtrShift,
              Ptr::Unk /* no longer a pointer */,
              isSpecialized() ? m_extra : 0);
}

inline Type Type::derefIfPtr() const {
  assert(*this <= (Gen | PtrToGen));
  return *this <= PtrToGen ? deref() : *this;
}

inline Type Type::strip() const {
  return derefIfPtr().unbox();
}

inline Ptr Type::ptrKind() const {
  return static_cast<Ptr>(m_ptrKind);
}

///////////////////////////////////////////////////////////////////////////////
// Private methods.

inline Type::Type(bits_t bits, Ptr kind, uintptr_t extra /* = 0 */)
  : m_bits(bits)
  , m_ptrKind(static_cast<std::underlying_type<Ptr>::type>(kind))
  , m_hasConstVal(false)
  , m_extra(extra)
{
  assert(checkValid());
}

inline Type::Type(Type t, ArraySpec arraySpec)
  : m_bits(t.m_bits)
  , m_ptrKind(t.m_ptrKind)
  , m_hasConstVal(false)
  , m_arrSpec(arraySpec)
{
  assert(checkValid());
  assert(m_arrSpec != ArraySpec::Bottom);
}

inline Type::Type(Type t, ClassSpec classSpec)
  : m_bits(t.m_bits)
  , m_ptrKind(t.m_ptrKind)
  , m_hasConstVal(false)
  , m_clsSpec(classSpec)
{
  assert(checkValid());
  assert(m_clsSpec != ClassSpec::Bottom);
}

///////////////////////////////////////////////////////////////////////////////
}}
