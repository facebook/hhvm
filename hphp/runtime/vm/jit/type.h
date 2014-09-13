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

#ifndef incl_HPHP_JIT_TYPE_H_
#define incl_HPHP_JIT_TYPE_H_

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-array.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/data-block.h"

#include "folly/Optional.h"

#include <cstdint>
#include <cstring>

namespace HPHP {
struct Func;

namespace jit {
struct DynLocation;

namespace constToBits_detail {
  template<class T>
  struct needs_promotion
    : std::integral_constant<
        bool,
        std::is_integral<T>::value ||
          std::is_same<T,bool>::value ||
          std::is_enum<T>::value
      >
  {};

  template<class T>
  typename std::enable_if<needs_promotion<T>::value,uint64_t>::type
  promoteIfNeeded(T t) { return static_cast<uint64_t>(t); }

  template<class T>
  typename std::enable_if<!needs_promotion<T>::value,T>::type
  promoteIfNeeded(T t) { return t; }
}

#define IRT_BOXES(name, bits)                                           \
  IRT(name,             (bits))                                         \
  IRT(Boxed##name,      (bits) << kBoxShift)                            \
  IRT(PtrTo##name,      (bits) << kPtrShift)                            \
  IRT(PtrToBoxed##name, (bits) << kPtrBoxShift)

#define IRT_BOXES_WITH_ANY(name, bits)                                  \
  IRT_BOXES(name, bits)                                                 \
  IRT(Any##name, k##name | kBoxed##name | kPtrTo##name |                \
                 kPtrToBoxed##name)


#define IRT_PHP(c)                                                      \
  c(Uninit,       1ULL << 0)                                            \
  c(InitNull,     1ULL << 1)                                            \
  c(Bool,         1ULL << 2)                                            \
  c(Int,          1ULL << 3)                                            \
  c(Dbl,          1ULL << 4)                                            \
  c(StaticStr,    1ULL << 5)                                            \
  c(CountedStr,   1ULL << 6)                                            \
  c(StaticArr,    1ULL << 7)                                            \
  c(CountedArr,   1ULL << 8)                                            \
  c(Obj,          1ULL << 9)                                            \
  c(Res,          1ULL << 10)
// Boxed*:       11-21
// PtrTo*:       22-32
// PtrToBoxed*:  33-43

// This list should be in non-decreasing order of specificity
#define IRT_PHP_UNIONS(c)                                               \
  c(Null,          kUninit|kInitNull)                                   \
  c(Str,           kStaticStr|kCountedStr)                              \
  c(Arr,           kStaticArr|kCountedArr)                              \
  c(NullableObj,   kObj|kInitNull|kUninit)                              \
  c(UncountedInit, kInitNull|kBool|kInt|kDbl|kStaticStr|kStaticArr)     \
  c(Uncounted,     kUncountedInit|kUninit)                              \
  c(InitCell,      kUncountedInit|kStr|kArr|kObj|kRes)                  \
  c(Cell,          kInitCell|kUninit)

#define IRT_RUNTIME                                                     \
  IRT(Cls,         1ULL << 44)                                          \
  IRT(Func,        1ULL << 45)                                          \
  IRT(VarEnv,      1ULL << 46)                                          \
  IRT(NamedEntity, 1ULL << 47)                                          \
  IRT(Cctx,        1ULL << 48) /* Class* with the lowest bit set,  */   \
                               /* as stored in ActRec.m_cls field  */   \
  IRT(RetAddr,     1ULL << 49) /* Return address */                     \
  IRT(StkPtr,      1ULL << 50) /* stack pointer */                      \
  IRT(FramePtr,    1ULL << 51) /* frame pointer */                      \
  IRT(TCA,         1ULL << 52)                                          \
  IRT(ActRec,      1ULL << 53)                                          \
  IRT(RDSHandle,   1ULL << 54) /* RDS::Handle */                        \
  IRT(Nullptr,     1ULL << 55)                                          \
  IRT(ABC,         1ULL << 56) /* AsioBlockableChain */

// The definitions for these are in ir.cpp
#define IRT_UNIONS                                                      \
  IRT(Ctx,         kObj|kCctx)                                          \

// Gen, Counted, PtrToGen, and PtrToCounted are here instead of
// IRT_PHP_UNIONS because boxing them (e.g., BoxedGen, PtrToBoxedGen)
// would yield nonsense types.
#define IRT_SPECIAL                                                 \
  IRT(Bottom,       0)                                              \
  IRT(Top,          0xffffffffffffffffULL)                          \
  IRT(Counted,      kCountedStr|kCountedArr|kObj|kRes|kBoxedCell)   \
  IRT(PtrToCounted, kCounted << kPtrShift)                          \
  IRT(Gen,          kCell|kBoxedCell)                               \
  IRT(StackElem,    kGen|kCls)                                      \
  IRT(Init,         kGen & ~kUninit)                                \
  IRT(PtrToGen,     kGen << kPtrShift)                              \
  IRT(PtrToInit,    kInit << kPtrShift)

// All types (including union types) that represent program values,
// except Gen (which is special). Boxed*, PtrTo*, and PtrToBoxed* only
// exist for these types.
#define IRT_USERLAND(c) IRT_PHP(c) IRT_PHP_UNIONS(c)

// All types with just a single bit set
#define IRT_PRIMITIVE IRT_PHP(IRT_BOXES) IRT_RUNTIME

// All types
#define IR_TYPES IRT_USERLAND(IRT_BOXES_WITH_ANY) IRT_RUNTIME IRT_UNIONS \
  IRT_SPECIAL

/*
 * Type is used to represent the types of values in the jit. Every Type
 * represents a set of types, with Type::Top being a superset of all Types and
 * Type::Bottom being a subset of all Types. The elements forming these sets of
 * types come from the types of PHP-visible values (Str, Obj, Int, ...) and
 * runtime-internal types (Func, TCA, ActRec, ...).
 *
 * Types can be constructed from the predefined constants or by composing
 * existing Types in various ways. Unions, intersections, and subtractions are
 * all supported, though for implementation-specific reasons certain
 * combinations of specialized types cannot be represented. A type is
 * considered specialized if it refers to a specific Class or a
 * ArrayData::ArrayKind. As an example, if A and B are unrelated Classes,
 * Obj<A> | Obj<B> is impossible to represent. However, if B is a subclass of
 * A, Obj<A> | Obj<B> == Obj<B>, which can be represented as a Type.
 */
class Type {
  typedef uint64_t bits_t;

  static const size_t kBoxShift = 11;
  static const size_t kPtrShift = kBoxShift * 2;
  static const size_t kPtrBoxShift = kBoxShift + kPtrShift;

  enum TypedBits {
#define IRT(name, bits) k##name = (bits),
  IR_TYPES
#undef IRT
  };

  // An ArrayKind in the top 16 bits, optional RepoAuthType::Array* in
  // the lower 48 bits, and the low bit that says whether the kind is
  // valid.
  enum class ArrayInfo : uintptr_t {};

  // Tag that tells us if we're exactly equal to, or a subtype of a Class*.
  enum class ClassTag : uint8_t { Sub, Exact };

  // A const Class* with the low bit set if this is an exact type,
  // otherwise a subtype.
  struct ClassInfo {
    ClassInfo(const Class* cls, ClassTag tag)
        : m_bits(reinterpret_cast<uintptr_t>(cls)) {
      assert((m_bits & 1) == 0);
      switch (tag) {
        case ClassTag::Sub:
          break;
        case ClassTag::Exact:
          m_bits |= 1;
          break;
      }
    }

    const Class* get() const {
      return reinterpret_cast<const Class*>(m_bits & ~1);
    }

    bool isExact() const { return m_bits & 1; }

    bool operator==(const ClassInfo& rhs) const { return m_bits == rhs.m_bits; }

  private:
    uintptr_t m_bits;
  };

  bits_t m_bits:63;
  bool m_hasConstVal:1;

  union {
    uintptr_t m_extra;

    // Constant values. Validity determined by m_hasConstVal and m_bits.
    bool m_boolVal;
    int64_t m_intVal;
    double m_dblVal;
    const StringData* m_strVal;
    const ArrayData* m_arrVal;
    const HPHP::Func* m_funcVal;
    const Class* m_clsVal;
    jit::TCA m_tcaVal;
    RDS::Handle m_rdsHandleVal;
    TypedValue* m_ptrVal;

    // Specialization for object classes and arrays.
    ClassInfo m_class;
    ArrayInfo m_arrayInfo;
  };

  static ArrayInfo makeArrayInfo(folly::Optional<ArrayData::ArrayKind> kind,
                                 const RepoAuthType::Array* arrTy) {
    auto ret = reinterpret_cast<uintptr_t>(arrTy);
    if (kind.hasValue()) {
      ret |= 0x1;
      ret |= uintptr_t{*kind} << 48;
    }
    return static_cast<ArrayInfo>(ret);
  }

  static bool arrayKindValid(ArrayInfo info) {
    return static_cast<uintptr_t>(info) & 0x1;
  }

  static ArrayData::ArrayKind kind(ArrayInfo info) {
    assert(arrayKindValid(info));
    return static_cast<ArrayData::ArrayKind>(
      static_cast<uintptr_t>(info) >> 48
    );
  }

  // May return nullptr if we have no specialized array type
  // information.
  static const RepoAuthType::Array* arrayType(ArrayInfo info) {
    return reinterpret_cast<const RepoAuthType::Array*>(
      static_cast<uintptr_t>(info) & (-1ull >> 16) & ~0x1
    );
  }

  bool checkValid() const;

  explicit Type(bits_t bits, uintptr_t extra = 0)
    : m_bits(bits)
    , m_hasConstVal(false)
    , m_extra(extra)
  {
    assert(checkValid());
  }

  explicit Type(bits_t bits, ClassInfo classInfo)
    : m_bits(bits)
    , m_hasConstVal(false)
    , m_class(classInfo)
  {
    assert(checkValid());
  }

  explicit Type(bits_t bits, ArrayInfo arrayInfo)
    : m_bits(bits)
    , m_hasConstVal(false)
    , m_arrayInfo(arrayInfo)
  {
    assert(checkValid());
  }

  explicit Type(bits_t bits, ArrayData::ArrayKind) = delete;

  static bits_t bitsFromDataType(DataType outer, DataType inner);

  // combine, Union, and Intersect are used for operator| and operator&. See
  // .cpp for details.
  template<typename Oper>
  static Type combine(bits_t newBits, Type a, Type b);

  struct Union;
  struct Intersect;
  struct ArrayOps;
  struct ClassOps;

public:
# define IRT(name, ...) static const Type name;
  IR_TYPES
# undef IRT

  Type()
    : m_bits(kBottom)
    , m_hasConstVal(false)
    , m_extra(0)
  {}

  explicit Type(DataType outerType, DataType innerType = KindOfInvalid)
    : m_bits(bitsFromDataType(outerType, innerType))
    , m_hasConstVal(false)
    , m_extra(0)
  {}

  explicit Type(const DynLocation* dl);

  size_t hash() const {
    return hash_int64_pair(m_bits | (bits_t(m_hasConstVal) << 63), m_extra);
  }

  Type& operator=(Type b) {
    m_bits = b.m_bits;
    m_hasConstVal = b.m_hasConstVal;
    m_extra = b.m_extra;
    return *this;
  }

  std::string constValString() const;
  std::string toString() const;
  static std::string debugString(Type t);
  static Type fromString(const std::string& str);

  ////////// Support for constants //////////

 private:
  // forConst returns the Type to use for a given C++ value. The only
  // interesting case is int/bool disambiguation.  Enums are treated as ints.
  template<class T>
  static typename std::enable_if<
    std::is_integral<T>::value || std::is_enum<T>::value,
    Type
    >::type forConst(T) {
    return std::is_same<T,bool>::value ? Type::Bool : Type::Int;
  }
  static Type forConst(const HPHP::Func*)        { return Func; }
  static Type forConst(const Class*)             { return Cls; }
  static Type forConst(jit::TCA)                 { return TCA; }
  static Type forConst(double)                   { return Dbl; }
  static Type forConst(const StringData* sd) {
    assert(sd->isStatic());
    return StaticStr;
  }
  static Type forConst(const ArrayData* ad) {
    assert(ad->isStatic());
    return StaticArr.specialize(ad->kind());
  }

 public:
  // Returns true iff this Type represents a known value.
  bool isConst() const {
    return m_hasConstVal || subtypeOfAny(Uninit, InitNull, Nullptr);
  }

  // Returns true iff this Type is a constant value of type t.
  bool isConst(Type t) const {
    return subtypeOf(t) && isConst();
  }

  // Returns true iff this Type represents the constant val, using the same C++
  // type -> Type mapping as Type::cns().
  template<typename T>
  bool isConst(T val) const {
    return subtypeOf(cns(val));
  }

  template<typename T>
  static Type cns(T val, Type ret) {
    assert(!ret.m_hasConstVal);
    ret.m_hasConstVal = true;

    static_assert(sizeof(T) <= sizeof ret,
                  "Constant data was larger than supported");
    static_assert(std::is_pod<T>::value,
                  "Constant data wasn't a pod");
    const auto toCopy = constToBits_detail::promoteIfNeeded(val);
    static_assert(sizeof toCopy == 8,
                  "Unexpected size for toCopy");
    std::memcpy(&ret.m_extra, &toCopy, sizeof toCopy);
    return ret;
  }

  template<typename T>
  static Type cns(T val) {
    return cns(val, forConst(val));
  }

  static Type cns(std::nullptr_t) {
    return Type::Nullptr;
  }

  static Type cns(const TypedValue tv) {
    if (tv.m_type == KindOfUninit) return Type::Uninit;
    if (tv.m_type == KindOfNull)   return Type::InitNull;

    auto ret = [&] {
      switch (tv.m_type) {
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

        default:
          always_assert(false && "Invalid KindOf for constant TypedValue");
      }
    }();
    ret.m_hasConstVal = true;
    ret.m_extra = tv.m_data.num;
    return ret;
  }

  // If this represents a constant value, return the most specific strict
  // supertype of this we can represent. In most cases this just erases the
  // constant value: Int<4> -> Int, Dbl<2.5> -> Dbl. Arrays are special since
  // they can be both constant and specialized, so keep the array's kind in the
  // resulting type.
  Type dropConstVal() const {
    if (!m_hasConstVal) return *this;
    assert(!isUnion());

    if (subtypeOf(StaticArr)) {
      return Type::StaticArr.specialize(arrVal()->kind());
    }
    return Type(m_bits);
  }

  // Relaxes the type to one that we can check in codegen
  Type relaxToGuardable() const;

  bool hasRawVal() const {
    return m_hasConstVal;
  }

  uint64_t rawVal() const {
    assert(m_hasConstVal);
    return m_intVal;
  }

  bool boolVal() const {
    assert(subtypeOf(Bool) && m_hasConstVal);
    assert(m_boolVal <= 1);
    return m_boolVal;
  }

  int64_t intVal() const {
    assert(subtypeOf(Int) && m_hasConstVal);
    return m_intVal;
  }

  double dblVal() const {
    assert(subtypeOf(Dbl) && m_hasConstVal);
    return m_dblVal;
  }

  const StringData* strVal() const {
    assert(subtypeOf(StaticStr) && m_hasConstVal);
    return m_strVal;
  }

  const ArrayData* arrVal() const {
    assert(subtypeOf(StaticArr) && m_hasConstVal);
    return m_arrVal;
  }

  const HPHP::Func* funcVal() const {
    assert(subtypeOf(Func) && m_hasConstVal);
    return m_funcVal;
  }

  const Class* clsVal() const {
    assert(subtypeOf(Cls) && m_hasConstVal);
    return m_clsVal;
  }

  RDS::Handle rdsHandleVal() const {
    assert(subtypeOf(RDSHandle) && m_hasConstVal);
    return m_rdsHandleVal;
  }

  jit::TCA tcaVal() const {
    assert(subtypeOf(TCA) && m_hasConstVal);
    return m_tcaVal;
  }

  ////////// Methods to query properties of the type //////////

  bool isBoxed() const {
    return subtypeOf(BoxedCell);
  }

  bool notBoxed() const {
    assert(subtypeOf(Gen));
    return subtypeOf(Cell);
  }

  bool maybeBoxed() const {
    return maybe(BoxedCell);
  }

  bool isPtr() const {
    return subtypeOf(PtrToGen);
  }

  bool notPtr() const {
    return not(PtrToGen);
  }

  bool isCounted() const {
    return subtypeOf(Counted);
  }

  bool maybeCounted() const {
    return maybe(Counted);
  }

  bool notCounted() const {
    return not(Counted);
  }

  /*
   * Returns true iff this is a union type. Note that this is the plain old set
   * definition of union, so Type::Str, Type::Arr, and Type::Null will all
   * return true.
   */
  bool isUnion() const {
    // This will return true iff more than 1 bit is set in m_bits.
    return (m_bits & (m_bits - 1)) != 0;
  }

  /*
   * Returns true iff there exists a DataType in the range [KindOfUninit,
   * KindOfRef] that represents a non-strict supertype of this type.
   *
   * Pre: subtypeOf(StackElem)
   */
  bool isKnownDataType() const {
    assert(subtypeOf(StackElem));

    // Some unions that correspond to single KindOfs.
    if (subtypeOfAny(Str, Arr, BoxedCell)) return true;

    return !isUnion();
  }

  /*
   * Similar to isKnownDataType, with the added restriction that the type not
   * be Boxed.
   */
  bool isKnownUnboxedDataType() const {
    return isKnownDataType() && notBoxed();
  }

  /*
   * Returns true if this requires a register to hold a DataType at runtime.
   */
  bool needsReg() const {
    return subtypeOf(StackElem) && !isKnownDataType();
  }

  bool needsValueReg() const {
    return !subtypeOfAny(Null, Nullptr);
  }

  bool needsStaticBitCheck() const {
    return maybe(StaticStr | StaticArr);
  }

  bool canRunDtor() const {
    return maybe(CountedArr | BoxedCountedArr | Obj | BoxedObj |
                 Res | BoxedRes);
  }

  // return true if this corresponds to a type that is passed by value in C++
  bool isSimpleType() const {
    return subtypeOfAny(Bool, Int, Dbl, Null);
  }

  // return true if this corresponds to a type that is passed by reference in
  // C++
  bool isReferenceType() const {
    return subtypeOfAny(Str, Arr, Obj, Res);
  }

  int nativeSize() const {
    if (subtypeOf(Type::Int | Type::Func)) return sz::qword;
    if (subtypeOf(Type::Bool))             return sz::byte;
    not_implemented();
  }

  ////////// Support for specialized types: Obj classes and Arr kinds //////////

  /*
   * True if type can have a specialized class.
   */
  bool canSpecializeClass() const {
    return (m_bits & kAnyObj) && !(m_bits & kAnyArr);
  }

  /*
   * True if type can have specialized array information.
   */
  bool canSpecializeArray() const {
    return (m_bits & kAnyArr) && !(m_bits & kAnyObj);
  }

  bool canSpecializeAny() const {
    return canSpecializeClass() || canSpecializeArray();
  }

  Type specialize(const Class* klass) const {
    assert(canSpecializeClass() && getClass() == nullptr);
    return Type(m_bits, ClassInfo(klass, ClassTag::Sub));
  }

  Type specializeExact(const Class* klass) const {
    assert(canSpecializeClass() && getClass() == nullptr);
    return Type(m_bits, ClassInfo(klass, ClassTag::Exact));
  }

  Type specialize(ArrayData::ArrayKind arrayKind) const {
    assert(canSpecializeArray());
    return Type(m_bits, makeArrayInfo(arrayKind, nullptr));
  }

  Type specialize(const RepoAuthType::Array* array) const {
    assert(canSpecializeArray());
    return Type(m_bits, makeArrayInfo(folly::none, array));
  }

  bool isSpecialized() const {
    return (canSpecializeClass() && getClass()) ||
      (canSpecializeArray() && (hasArrayKind() || getArrayType()));
  }

  Type unspecialize() const {
    return Type(m_bits);
  }

  const Class* getClass() const {
    assert(canSpecializeClass());
    return m_class.get();
  }

  const Class* getExactClass() const {
    assert(canSpecializeClass() || subtypeOf(Type::Cls));
    return (m_hasConstVal || m_class.isExact()) ? getClass() : nullptr;
  }

  bool hasArrayKind() const {
    assert(canSpecializeArray());
    return m_hasConstVal || arrayKindValid(m_arrayInfo);
  }

  ArrayData::ArrayKind getArrayKind() const {
    assert(hasArrayKind());
    return m_hasConstVal ? m_arrVal->kind() : kind(m_arrayInfo);
  }

  folly::Optional<ArrayData::ArrayKind> getOptArrayKind() const {
    if (hasArrayKind()) return getArrayKind();
    return folly::none;
  }

  const RepoAuthType::Array* getArrayType() const {
    assert(canSpecializeArray());
    return m_hasConstVal ? nullptr : arrayType(m_arrayInfo);
  }

  // Returns a subset of *this containing only the members relating to its
  // specialization.
  //
  // {Int|Str|Obj<C>|BoxedObj<C>}.specializedType() == {Obj<C>|BoxedObj<C>}
  Type specializedType() const {
    assert(isSpecialized());
    if (canSpecializeClass()) return *this & AnyObj;
    if (canSpecializeArray()) return *this & AnyArr;
    not_reached();
  }

  ////////// Methods for comparing types //////////

  /*
   * Returns true iff this represents a non-strict subset of t2.
   */
  bool subtypeOf(Type t2) const;

  template<typename... Types>
  bool subtypeOfAny(Type t2, Types... ts) const {
    return subtypeOf(t2) || subtypeOfAny(ts...);
  }

  bool subtypeOfAny() const {
    return false;
  }

  /*
   * Returns true if this is a strict subtype of t2.
   */
  bool strictSubtypeOf(Type t2) const {
    return *this != t2 && subtypeOf(t2);
  }

  /*
   * Returns true if any subtype of this is a subtype of t2.
   */
  bool maybe(Type t2) const {
    return (*this & t2) != Bottom;
  }

  /*
   * Returns true if no subtypes of this are subtypes of t2.
   */
  bool not(Type t2) const {
    return !maybe(t2);
  }

  /*
   * Returns true if this is exactly equal to t2. Be careful: you
   * probably mean subtypeOf.
   */
  bool equals(Type t2) const {
    return m_bits == t2.m_bits && m_hasConstVal == t2.m_hasConstVal &&
      m_extra == t2.m_extra;
  }

  bool operator==(Type t2) const { return equals(t2); }
  bool operator!=(Type t2) const { return !operator==(t2); }

  ////////// Methods for combining Types in various ways //////////

  /*
   * Standard set operations: union, intersection, and difference.
   */
  Type operator|(Type other) const;
  Type& operator|=(Type other) { return *this = *this | other; }

  Type operator&(Type other) const;
  Type& operator&=(Type other) { return *this = *this & other; }

  Type operator-(Type other) const;
  Type& operator-=(Type other) { return *this = *this - other; }

  /*
   * unionOf: return the least common predefined supertype of t1 and t2,
   * i.e.. the most refined type t3 such that t1 <= t3 and t2 <= t3. Note that
   * arbitrary union types are possible using operator| but this function
   * always returns one of the predefined types.
   */
  static Type unionOf(Type t1, Type t2);

  ////////// Support for inner types of boxed and pointer types //////////

  Type box() const {
    assert(subtypeOf(Cell));
    // Boxing Uninit returns InitNull but that logic doesn't belong
    // here.
    assert(not(Uninit) || equals(Cell));
    return Type(m_bits << kBoxShift,
                isSpecialized() && !m_hasConstVal ? m_extra : 0);
  }

  // This computes the type effects of the Unbox opcode.
  Type unbox() const {
    assert(subtypeOf(Gen));
    return (*this & Cell) | (*this & BoxedCell).innerType();
  }

  Type innerType() const {
    assert(isBoxed() || equals(Bottom));
    return Type(m_bits >> kBoxShift, m_extra);
  }

  Type deref() const {
    assert(isPtr());
    return Type(m_bits >> kPtrShift, isSpecialized() ? m_extra : 0);
  }

  Type derefIfPtr() const {
    assert(subtypeOf(Gen | PtrToGen));
    return isPtr() ? deref() : *this;
  }

  // Returns the "stripped" version of this: dereferenced and unboxed,
  // if applicable.
  Type strip() const {
    return derefIfPtr().unbox();
  }

  Type ptr() const {
    assert(!isPtr());
    assert(subtypeOf(Gen));
    return Type(m_bits << kPtrShift,
                isSpecialized() && !m_hasConstVal ? m_extra : 0);
  }

  ////////// Methods for talking to other type systems in the VM //////////

  /*
   * Returns the most specific DataType that is a supertype of this
   * type.
   *
   * pre: isKnownDataType()
   */
  DataType toDataType() const;
};

typedef folly::Optional<Type> OptType;

inline bool operator<(Type a, Type b) { return a.strictSubtypeOf(b); }
inline bool operator>(Type a, Type b) { return b.strictSubtypeOf(a); }
inline bool operator<=(Type a, Type b) { return a.subtypeOf(b); }
inline bool operator>=(Type a, Type b) { return b.subtypeOf(a); }

/*
 * jit::Type must be small enough for efficient pass-by-value.
 */
static_assert(sizeof(Type) <= 2 * sizeof(uint64_t),
              "jit::Type should fit in (2 * sizeof(uint64_t))");

/*
 * Return the most refined type that can be used to represent the type
 * in a live TypedValue.
 */
Type liveTVType(const TypedValue* tv);

/*
 * Return the boxed version of the input type, taking into account php
 * semantics and subtle implementation details.
 */
Type boxType(Type);

/*
 * Create a Type from a RepoAuthType.
 */
Type convertToType(RepoAuthType ty);

/*
 * Return the type resulting from refining oldType with the fact that
 * it also belongs to newType. This essentially intersects the two
 * types, except that it has special logic for boxed types.  This
 * function always_asserts that the resulting type isn't Bottom.
 */
Type refineType(Type oldType, Type newType);

/*
 * Similar to refineType above, but this one doesn't get angry if the
 * resulting type is Bottom.
 */
Type refineTypeNoCheck(Type oldType, Type newType);

/*
 * Return the dest type for a LdRef with the given typeParam.
 *
 * pre: srcType.notBoxed()
 */
Type ldRefReturn(Type typeParam);

//////////////////////////////////////////////////////////////////////

struct TypeConstraint {
  /* implicit */ TypeConstraint(DataTypeCategory cat = DataTypeGeneric,
                                DataTypeCategory inner = DataTypeGeneric)
    : category(cat)
    , innerCat(inner)
    , weak(false)
    , m_specialized(0)
  {}

  explicit TypeConstraint(const Class* cls)
    : TypeConstraint(DataTypeSpecialized)
  {
    setDesiredClass(cls);
  }

  void applyConstraint(TypeConstraint newTc);

  std::string toString() const;

  TypeConstraint& setWeak(bool w = true) {
    weak = w;
    return *this;
  }

  bool operator==(TypeConstraint tc2) const {
    return category == tc2.category && innerCat == tc2.innerCat &&
      weak == tc2.weak && m_specialized == tc2.m_specialized;
  }
  bool operator!=(TypeConstraint tc2) const { return !(*this == tc2); }

  bool empty() const {
    return category == DataTypeGeneric && innerCat == DataTypeGeneric && !weak;
  }

  static constexpr uint8_t kWantArrayKind = 0x1;

  bool isSpecialized() const {
    return category == DataTypeSpecialized || innerCat == DataTypeSpecialized;
  }

  TypeConstraint& setWantArrayKind() {
    assert(!wantClass());
    assert(isSpecialized());
    m_specialized |= kWantArrayKind;
    return *this;
  }

  bool wantArrayKind() const { return m_specialized & kWantArrayKind; }

  TypeConstraint& setDesiredClass(const Class* cls) {
    assert(m_specialized == 0 ||
           desiredClass()->classof(cls) || cls->classof(desiredClass()));
    assert(isSpecialized());
    m_specialized = reinterpret_cast<uintptr_t>(cls);
    assert(wantClass());
    return *this;
  }

  bool wantClass() const {
    return m_specialized != 0 && !wantArrayKind();
  }

  const Class* desiredClass() const {
    assert(wantClass());
    return reinterpret_cast<const Class*>(m_specialized);
  }

  // Get the inner constraint, preserving m_specialized if appropriate.
  TypeConstraint inner() const {
    auto tc = TypeConstraint{innerCat}.setWeak(weak);
    if (tc.category == DataTypeSpecialized) tc.m_specialized = m_specialized;
    return tc;
  }

  // category starts as DataTypeGeneric and is refined to more specific values
  // by consumers of the type.
  DataTypeCategory category;

  // When a value is boxed, innerCat is used to determine how we can relax the
  // inner type. innerCat is only meaningful when category is at least
  // DataTypeCountness, since a category of DataTypeGeneric relaxes types all
  // the way to Gen which has no meaningful inner type.
  DataTypeCategory innerCat;

  // If weak is true, the consumer of the value being constrained doesn't
  // actually want to constrain the guard (if found). Most often used to figure
  // out if a type can be used without further constraining guards.
  bool weak;

 private:
  // m_specialized either holds a Class* or a 1 in its low bit, indicating that
  // for a DataTypeSpecialized constraint, we require the specified class or an
  // array kind, respectively.
  uintptr_t m_specialized;
};

const int kTypeWordOffset = offsetof(TypedValue, m_type) % 8;
const int kTypeShiftBits = kTypeWordOffset * CHAR_BIT;

// left shift an immediate DataType, for type, to the correct position
// within one of the registers used to pass a TypedValue by value.
inline uint64_t toDataTypeForCall(Type type) {
  return uint64_t(type.toDataType()) << kTypeShiftBits;
}


}}

#endif
