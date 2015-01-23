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
// Tags.

inline Type::ClassInfo::ClassInfo(const Class* cls, Type::ClassTag tag)
  : m_bits(reinterpret_cast<uintptr_t>(cls))
{
  assert((m_bits & 1) == 0);
  switch (tag) {
    case ClassTag::Sub:
      break;
    case ClassTag::Exact:
      m_bits |= 1;
      break;
  }
}

inline const Class* Type::ClassInfo::get() const {
  return reinterpret_cast<const Class*>(m_bits & ~1);
}

inline bool Type::ClassInfo::isExact() const {
  return m_bits & 1;
}

inline bool Type::ClassInfo::operator==(const Type::ClassInfo& rhs) const {
  return m_bits == rhs.m_bits;
}

///////////////////////////////////////////////////////////////////////////////
// Creation.

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

inline bool Type::subtypeOf(Type t2) const {
  // First, check for any members in m_bits that aren't in t2.m_bits.
  if ((m_bits & t2.m_bits) != m_bits) {
    return false;
  }
  // The rest of the checks want to avoid constantly checking bottom on the
  // left side.
  if (m_bits == kBottom) return true;
  // If t2 is a constant, we must be the same constant (or Bottom, checked
  // above).
  if (t2.m_hasConstVal) {
    assert(!t2.isUnion());
    return m_hasConstVal && m_extra == t2.m_extra;
  }
  // If t2 could be a pointer, we must have a subtype relation in pointer kinds
  // or it's not a subtype.  (If t1 can't be a pointer also we found out above
  // when we intersected the bits.)  If neither can be a pointer, it's an
  // invariant that m_ptrKind will be Ptr::Unk so this will pass.
  if (rawPtrKind() != t2.rawPtrKind() &&
      !ptr_subtype(rawPtrKind(), t2.rawPtrKind())) {
    return false;
  }
  if (!t2.isSpecialized()) {
    return true;
  }
  return subtypeOfSpecialized(t2);
}

template<typename... Types>
bool Type::subtypeOfAny(Type t2, Types... ts) const {
  return subtypeOf(t2) || subtypeOfAny(ts...);
}

inline bool Type::subtypeOfAny() const {
  return false;
}

inline bool Type::strictSubtypeOf(Type t2) const {
  return *this != t2 && subtypeOf(t2);
}

inline bool Type::maybe(Type t2) const {
  return (*this & t2) != Bottom;
}

inline bool Type::not(Type t2) const {
  return !maybe(t2);
}

inline bool Type::equals(Type t2) const {
  return m_bits == t2.m_bits &&
         m_ptrKind == t2.m_ptrKind &&
         m_hasConstVal == t2.m_hasConstVal &&
         m_extra == t2.m_extra;
}

inline bool Type::operator==(Type t2) const {
  return equals(t2);
}

inline bool Type::operator!=(Type t2) const {
  return !operator==(t2);
}

///////////////////////////////////////////////////////////////////////////////
// Is-a methods.

inline bool Type::isZeroValType() const {
  return subtypeOfAny(Uninit, InitNull, Nullptr);
}

inline bool Type::isBoxed() const {
  return subtypeOf(BoxedCell);
}

inline bool Type::maybeBoxed() const {
  return maybe(BoxedCell);
}

inline bool Type::notBoxed() const {
  assert(subtypeOf(Gen));
  return subtypeOf(Cell);
}

inline bool Type::isPtr() const {
  return subtypeOf(PtrToGen);
}

inline bool Type::notPtr() const {
  return not(PtrToGen);
}

inline bool Type::isCounted() const {
  return subtypeOf(Counted);
}

inline bool Type::maybeCounted() const {
  return maybe(Counted);
}

inline bool Type::notCounted() const {
  return not(Counted);
}

inline bool Type::isUnion() const {
  // This will return true iff more than 1 bit is set in m_bits.
  return (m_bits & (m_bits - 1)) != 0;
}

inline bool Type::isKnownDataType() const {
  assert(subtypeOf(StkElem));

  // Some unions correspond to single KindOfs.
  return subtypeOfAny(Str, Arr, BoxedCell) || !isUnion();
}

inline bool Type::isKnownUnboxedDataType() const {
  return isKnownDataType() && notBoxed();
}

inline bool Type::needsReg() const {
  return subtypeOf(StkElem) && !isKnownDataType();
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
  return StaticArr.specialize(ad->kind());
}

inline bool Type::isConst() const {
  return m_hasConstVal || isZeroValType();
}

inline bool Type::isConst(Type t) const {
  return isConst() && subtypeOf(t);
}

template<typename T>
bool Type::isConst(T val) const {
  return subtypeOf(cns(val));
}

inline uint64_t Type::rawVal() const {
  assert(isConst());
  return isZeroValType() ? 0 : m_intVal;
}

inline bool Type::boolVal() const {
  assert(subtypeOf(Bool) && m_hasConstVal);
  return m_boolVal;
}

inline int64_t Type::intVal() const {
  assert(subtypeOf(Int) && m_hasConstVal);
  return m_intVal;
}

inline double Type::dblVal() const {
  assert(subtypeOf(Dbl) && m_hasConstVal);
  return m_dblVal;
}

inline const StringData* Type::strVal() const {
  assert(subtypeOf(StaticStr) && m_hasConstVal);
  return m_strVal;
}

inline const ArrayData* Type::arrVal() const {
  assert(subtypeOf(StaticArr) && m_hasConstVal);
  return m_arrVal;
}

inline const HPHP::Func* Type::funcVal() const {
  assert(subtypeOf(Func) && m_hasConstVal);
  return m_funcVal;
}

inline const Class* Type::clsVal() const {
  assert(subtypeOf(Cls) && m_hasConstVal);
  return m_clsVal;
}

inline ConstCctx Type::cctxVal() const {
  assert(subtypeOf(Cctx) && m_hasConstVal);
  return m_cctxVal;
}

inline RDS::Handle Type::rdsHandleVal() const {
  assert(subtypeOf(RDSHandle) && m_hasConstVal);
  return m_rdsHandleVal;
}

inline jit::TCA Type::tcaVal() const {
  assert(subtypeOf(TCA) && m_hasConstVal);
  return m_tcaVal;
}

inline Type Type::dropConstVal() const {
  if (!m_hasConstVal) return *this;
  assert(!isUnion());

  if (subtypeOf(StaticArr)) {
    return Type::StaticArr.specialize(arrVal()->kind());
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
// Specialized types.

inline bool Type::canSpecializeClass() const {
  return (m_bits & kAnyObj) && !(m_bits & kAnyArr);
}

inline bool Type::canSpecializeArray() const {
  return (m_bits & kAnyArr) && !(m_bits & kAnyObj);
}

inline bool Type::canSpecializeAny() const {
  return canSpecializeClass() || canSpecializeArray();
}

inline Type Type::specialize(const Class* klass) const {
  assert(klass != nullptr);
  assert(canSpecializeClass() && getClass() == nullptr);
  if (klass->attrs() & AttrNoOverride) return specializeExact(klass);
  return Type(m_bits, rawPtrKind(), ClassInfo(klass, ClassTag::Sub));
}

inline Type Type::specializeExact(const Class* klass) const {
  assert(klass != nullptr);
  assert(canSpecializeClass() && getClass() == nullptr);
  return Type(m_bits, rawPtrKind(), ClassInfo(klass, ClassTag::Exact));
}

inline Type Type::specialize(ArrayData::ArrayKind arrayKind) const {
  assert(canSpecializeArray());
  return Type(m_bits, rawPtrKind(), makeArrayInfo(arrayKind, nullptr));
}

inline Type Type::specialize(const RepoAuthType::Array* array) const {
  assert(canSpecializeArray());
  return Type(m_bits, rawPtrKind(), makeArrayInfo(folly::none, array));
}

inline Type Type::specialize(const Shape* shape) const {
  assert(canSpecializeArray());
  return Type(m_bits, rawPtrKind(), makeArrayInfo(shape));
}

inline Type Type::unspecialize() const {
  return Type(m_bits, rawPtrKind());
}

inline bool Type::isSpecialized() const {
  return (canSpecializeClass() && getClass()) ||
         (canSpecializeArray() && (hasArrayKind() || getArrayType()));
}

inline const Class* Type::getClass() const {
  assert(canSpecializeClass());
  return m_class.get();
}

inline const Class* Type::getExactClass() const {
  assert(canSpecializeClass() || subtypeOf(Type::Cls));
  return (m_hasConstVal || m_class.isExact()) ? getClass() : nullptr;
}

inline bool Type::hasArrayKind() const {
  assert(canSpecializeArray());
  return m_hasConstVal || arrayKindValid(m_arrayInfo);
}

inline ArrayData::ArrayKind Type::getArrayKind() const {
  assert(hasArrayKind());
  return m_hasConstVal ? m_arrVal->kind() : arrayKind(m_arrayInfo);
}

inline folly::Optional<ArrayData::ArrayKind> Type::getOptArrayKind() const {
  if (hasArrayKind()) return getArrayKind();
  return folly::none;
}

inline const RepoAuthType::Array* Type::getArrayType() const {
  assert(canSpecializeArray());
  if (m_hasConstVal) return nullptr;
  if (!arrayKindValid(m_arrayInfo)) return arrayType(m_arrayInfo);
  if (arrayKind(m_arrayInfo) == ArrayData::kStructKind) return nullptr;
  return arrayType(m_arrayInfo);
}

inline const Shape* Type::getArrayShape() const {
  assert(canSpecializeArray());
  if (m_hasConstVal) return nullptr;
  if (!arrayKindValid(m_arrayInfo)) return nullptr;
  if (arrayKind(m_arrayInfo) != ArrayData::kStructKind) return nullptr;
  return arrayShape(m_arrayInfo);
}

inline Type Type::specializedType() const {
  assert(isSpecialized());
  if (canSpecializeClass()) return *this & AnyObj;
  if (canSpecializeArray()) return *this & AnyArr;
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
// Inner types.

inline Type Type::box() const {
  assert(subtypeOf(Cell));
  // Boxing Uninit returns InitNull but that logic doesn't belong here.
  assert(not(Uninit) || equals(Cell));
  return Type(m_bits << kBoxShift,
              rawPtrKind(),
              isSpecialized() && !m_hasConstVal ? m_extra : 0);
}

inline Type Type::unbox() const {
  assert(subtypeOf(Gen));
  return (*this & Cell) | (*this & BoxedCell).innerType();
}

inline Type Type::ptr(Ptr kind) const {
  assert(!isPtr());
  assert(subtypeOf(Gen));
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
  assert(subtypeOf(Gen | PtrToGen));
  return isPtr() ? deref() : *this;
}

// Returns the "stripped" version of this: dereferenced and unboxed,
// if applicable.
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

inline Type::Type(bits_t bits, Ptr kind, ClassInfo classInfo)
  : m_bits(bits)
  , m_ptrKind(static_cast<std::underlying_type<Ptr>::type>(kind))
  , m_hasConstVal(false)
  , m_class(classInfo)
{
  assert(checkValid());
}

inline Type::Type(bits_t bits, Ptr kind, ArrayInfo arrayInfo)
  : m_bits(bits)
  , m_ptrKind(static_cast<std::underlying_type<Ptr>::type>(kind))
  , m_hasConstVal(false)
  , m_arrayInfo(arrayInfo)
{
  assert(checkValid());
}

///////////////////////////////////////////////////////////////////////////////
// ArrayInfo helpers.

inline
Type::ArrayInfo Type::makeArrayInfo(folly::Optional<ArrayData::ArrayKind> kind,
                                    const RepoAuthType::Array* arrTy) {
  auto ret = reinterpret_cast<uintptr_t>(arrTy);
  if (kind.hasValue()) {
    ret |= 0x1;
    ret |= uintptr_t{*kind} << 48;
  }
  return static_cast<ArrayInfo>(ret);
}

inline
Type::ArrayInfo Type::makeArrayInfo(const Shape* shape) {
  auto ret = reinterpret_cast<uintptr_t>(shape);
  ret |= 0x1;
  ret |= uintptr_t{ArrayData::kStructKind} << 48;
  return static_cast<ArrayInfo>(ret);
}

inline bool Type::arrayKindValid(Type::ArrayInfo info) {
  return static_cast<uintptr_t>(info) & 0x1;
}

inline ArrayData::ArrayKind Type::arrayKind(Type::ArrayInfo info) {
  assert(arrayKindValid(info));
  return static_cast<ArrayData::ArrayKind>(
    static_cast<uintptr_t>(info) >> 48
  );
}

inline const RepoAuthType::Array* Type::arrayType(Type::ArrayInfo info) {
  return reinterpret_cast<const RepoAuthType::Array*>(
    static_cast<uintptr_t>(info) & (-1ull >> 16) & ~0x1
  );
}

inline const Shape* Type::arrayShape(Type::ArrayInfo info) {
  return reinterpret_cast<const Shape*>(
    static_cast<uintptr_t>(info) & (-1ull >> 16) & ~0x1
  );
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
