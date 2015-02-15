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

#include <type_traits>

#include "hphp/util/hash.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

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

bool ptr_subtype(Ptr, Ptr);

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
  // kinds or we're not a subtype.  (If lhs can't be a pointer, we found out
  // above when we intersected the bits.)  If neither can be a pointer, it's an
  // invariant that `m_ptrKind' will be Ptr::Unk so this will pass.
  if (lhs.rawPtrKind() != rhs.rawPtrKind() &&
      !ptr_subtype(lhs.rawPtrKind(), rhs.rawPtrKind())) {
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

inline bool Type::isZeroValType() const {
  return subtypeOfAny(Uninit, InitNull, Nullptr);
}

inline bool Type::isBoxed() const {
  return *this <= BoxedCell;
}

inline bool Type::maybeBoxed() const {
  return maybe(BoxedCell);
}

inline bool Type::notBoxed() const {
  assert(*this <= Gen);
  return *this <= Cell;
}

inline bool Type::isPtr() const {
  return *this <= PtrToGen;
}

inline bool Type::notPtr() const {
  return !maybe(PtrToGen);
}

inline bool Type::isCounted() const {
  return *this <= Counted;
}

inline bool Type::maybeCounted() const {
  return maybe(Counted);
}

inline bool Type::notCounted() const {
  return !maybe(Counted);
}

inline bool Type::isUnion() const {
  // This will return true iff more than 1 bit is set in m_bits.
  return (m_bits & (m_bits - 1)) != 0;
}

inline bool Type::isKnownDataType() const {
  assert(*this <= StkElem);

  // Some unions correspond to single KindOfs.
  return subtypeOfAny(Str, Arr, BoxedCell) || !isUnion();
}

inline bool Type::isKnownUnboxedDataType() const {
  return isKnownDataType() && notBoxed();
}

inline bool Type::needsReg() const {
  return *this <= StkElem && !isKnownDataType();
}

inline bool Type::needsValueReg() const {
  return !subtypeOfAny(Null, Nullptr);
}

inline bool Type::needsStaticBitCheck() const {
  return maybe(StaticStr | StaticArr);
}

inline bool Type::canRunDtor() const {
  return maybe(CountedArr | BoxedCountedArr | Obj | BoxedObj |
               Res | BoxedRes);
}

inline bool Type::isSimpleType() const {
  return subtypeOfAny(Bool, Int, Dbl, Null);
}

inline bool Type::isReferenceType() const {
  return subtypeOfAny(Str, Arr, Obj, Res);
}

///////////////////////////////////////////////////////////////////////////////
// Constant types.

inline Type Type::forConst(const StringData* sd) {
  assert(sd->isStatic());
  return StaticStr;
}

inline Type Type::forConst(const ArrayData* ad) {
  assert(ad->isStatic());
  return StaticArray(ad->kind());
}

inline bool Type::isConst() const {
  return m_hasConstVal || isZeroValType();
}

inline bool Type::isConst(Type t) const {
  return isConst() && *this <= t;
}

template<typename T>
bool Type::isConst(T val) const {
  return *this <= cns(val);
}

inline uint64_t Type::rawVal() const {
  assert(isConst());
  return isZeroValType() ? 0 : m_intVal;
}

inline bool Type::boolVal() const {
  assert(*this <= Bool && m_hasConstVal);
  return m_boolVal;
}

inline int64_t Type::intVal() const {
  assert(*this <= Int && m_hasConstVal);
  return m_intVal;
}

inline double Type::dblVal() const {
  assert(*this <= Dbl && m_hasConstVal);
  return m_dblVal;
}

inline const StringData* Type::strVal() const {
  assert(*this <= StaticStr && m_hasConstVal);
  return m_strVal;
}

inline const ArrayData* Type::arrVal() const {
  assert(*this <= StaticArr && m_hasConstVal);
  return m_arrVal;
}

inline const HPHP::Func* Type::funcVal() const {
  assert(*this <= Func && m_hasConstVal);
  return m_funcVal;
}

inline const Class* Type::clsVal() const {
  assert(*this <= Cls && m_hasConstVal);
  return m_clsVal;
}

inline ConstCctx Type::cctxVal() const {
  assert(*this <= Cctx && m_hasConstVal);
  return m_cctxVal;
}

inline rds::Handle Type::rdsHandleVal() const {
  assert(*this <= RDSHandle && m_hasConstVal);
  return m_rdsHandleVal;
}

inline jit::TCA Type::tcaVal() const {
  assert(*this <= TCA && m_hasConstVal);
  return m_tcaVal;
}

inline Type Type::dropConstVal() const {
  if (!m_hasConstVal) return *this;
  assert(!isUnion());

  if (*this <= StaticArr) {
    return Type::StaticArray(arrVal()->kind());
  }
  return Type(m_bits, rawPtrKind());
}

template<typename T>
Type Type::cns(T val, Type ret) {
  assert(!ret.m_hasConstVal);
  ret.m_hasConstVal = true;

  static_assert(sizeof(T) <= sizeof(ret),
                "Constant data was larger than supported");
  static_assert(std::is_pod<T>::value,
                "Constant data wasn't a pod");

  const auto toCopy = constToBits_detail::promoteIfNeeded(val);
  static_assert(sizeof(toCopy) == 8,
                "Unexpected size for toCopy");

  std::memcpy(&ret.m_extra, &toCopy, sizeof(toCopy));
  return ret;
}

template<typename T>
Type Type::cns(T val) {
  return cns(val, forConst(val));
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
        return forConst(tv.m_data.pstr);

      case KindOfArray:
        return forConst(tv.m_data.parr);

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
  return Type(m_bits, rawPtrKind());
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
              rawPtrKind(),
              isSpecialized() && !m_hasConstVal ? m_extra : 0);
}

inline Type Type::unbox() const {
  assert(*this <= Gen);
  return (*this & Cell) | (*this & BoxedCell).innerType();
}

inline Type Type::ptr(Ptr kind) const {
  assert(!isPtr());
  assert(*this <= Gen);
  return Type(m_bits << kPtrShift,
              kind,
              isSpecialized() && !m_hasConstVal ? m_extra : 0);
}

inline Type Type::deref() const {
  assert(isPtr());
  return Type(m_bits >> kPtrShift,
              Ptr::Unk /* no longer a pointer */,
              isSpecialized() ? m_extra : 0);
}

inline Type Type::derefIfPtr() const {
  assert(*this <= (Gen | PtrToGen));
  return isPtr() ? deref() : *this;
}

inline Type Type::strip() const {
  return derefIfPtr().unbox();
}

inline Ptr Type::ptrKind() const {
  assert(maybe(PtrToGen));
  return rawPtrKind();
}

inline Ptr Type::rawPtrKind() const {
  return static_cast<Ptr>(m_ptrKind);
}

inline Type Type::innerType() const {
  assert(isBoxed());
  assert(*this == Bottom || !isPtr());
  return Type(m_bits >> kBoxShift, Ptr::Unk, m_extra);
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
// TypeConstraint.

inline
TypeConstraint::TypeConstraint(DataTypeCategory cat /* = DataTypeGeneric */)
  : category(cat)
  , weak(false)
  , m_specialized(0)
{}

inline TypeConstraint::TypeConstraint(const Class* cls)
  : TypeConstraint(DataTypeSpecialized)
{
  setDesiredClass(cls);
}

inline TypeConstraint& TypeConstraint::setWeak(bool w /* = true */) {
  weak = w;
  return *this;
}

inline bool TypeConstraint::empty() const {
  return category == DataTypeGeneric && !weak;
}

inline bool TypeConstraint::operator==(TypeConstraint tc2) const {
  return category == tc2.category &&
         weak == tc2.weak &&
         m_specialized == tc2.m_specialized;
}

inline bool TypeConstraint::operator!=(TypeConstraint tc2) const {
  return !(*this == tc2);
}

///////////////////////////////////////////////////////////////////////////////
// TypeConstraint specialization.

inline bool TypeConstraint::isSpecialized() const {
  return category == DataTypeSpecialized;
}

inline TypeConstraint& TypeConstraint::setWantArrayKind() {
  assert(!wantClass());
  assert(isSpecialized());
  m_specialized |= kWantArrayKind;
  return *this;
}

inline bool TypeConstraint::wantArrayKind() const {
  return m_specialized & kWantArrayKind;
}

inline TypeConstraint& TypeConstraint::setWantArrayShape() {
  assert(!wantClass());
  assert(isSpecialized());
  setWantArrayKind();
  m_specialized |= kWantArrayShape;
  return *this;
}

inline bool TypeConstraint::wantArrayShape() const {
  return m_specialized & kWantArrayShape;
}

inline TypeConstraint& TypeConstraint::setDesiredClass(const Class* cls) {
  assert(m_specialized == 0 ||
         desiredClass()->classof(cls) || cls->classof(desiredClass()));
  assert(isSpecialized());
  m_specialized = reinterpret_cast<uintptr_t>(cls);
  assert(wantClass());
  return *this;
}

inline bool TypeConstraint::wantClass() const {
  return m_specialized && !wantArrayKind();
}

inline const Class* TypeConstraint::desiredClass() const {
  assert(wantClass());
  return reinterpret_cast<const Class*>(m_specialized);
}

///////////////////////////////////////////////////////////////////////////////
}}
