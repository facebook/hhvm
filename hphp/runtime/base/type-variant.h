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

#ifndef incl_HPHP_VARIANT_H_
#define incl_HPHP_VARIANT_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/type-string.h"

#include <algorithm>
#include <type_traits>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Forward declare these to avoid including tv-conversions.h which has a
// circular dependency with this file.
void tvCastToVecInPlace(TypedValue*);
void tvCastToDictInPlace(TypedValue*);
void tvCastToKeysetInPlace(TypedValue*);
void tvCastToVArrayInPlace(TypedValue*);
void tvCastToDArrayInPlace(TypedValue*);

/*
 * This class predates HHVM.
 *
 * In hphpc, when type inference failed to know type of a variable, we
 * would use Variant to represent the php variable in generated C++.
 *
 * Now Variant is only used in C++ extensions, and the API is mostly
 * legacy stuff.  If you're writing a C++ extension, try to avoid
 * Variant when you can (but you often can't, and we don't really have
 * a replacement yet, sorry).
 *
 * In C++ extensions, this class can be used as a generic handle to
 * one of our other data types (e.g. StringData, ArrayData), and it
 * may also be a handle to a RefData.
 *
 * Beware:
 *
 *    For historical reasons, this class does a lot of things you
 *    don't really expect in a well-behaved C++ class.
 *
 *    For example, the copy constructor is not a copy constructor (it
 *    unboxes refs and converts KindOfUninit to KindOfNull).  A
 *    similar story applies to the move constructor.  (And this means
 *    we may actually rely on whether copy elision (NRVO etc) is
 *    happening in some places for correctness.)
 *
 *    Use carefully.
 *
 */

struct Variant : private TypedValue {
  enum class NullInit {};
  enum class NoInit {};
  enum class CellCopy {};
  enum class CellDup {};
  enum class ArrayInitCtor {};
  enum class StrongBind {};
  enum class Attach {};
  enum class WithRefBind {};

  Variant() noexcept { m_type = KindOfUninit; }
  explicit Variant(NullInit) noexcept { m_type = KindOfNull; }
  explicit Variant(NoInit) noexcept {}

  /* implicit */ Variant(bool    v) noexcept {
    m_type = KindOfBoolean; m_data.num = v;
  }
  /* implicit */ Variant(int     v) noexcept {
    m_type = KindOfInt64; m_data.num = v;
  }
  // The following two overloads will accept int64_t whether it's
  // implemented as long or long long.
  /* implicit */ Variant(long   v) noexcept {
    m_type = KindOfInt64; m_data.num = v;
  }
  /* implicit */ Variant(long long v) noexcept {
    m_type = KindOfInt64; m_data.num = v;
  }
  /* implicit */ Variant(unsigned long v) noexcept {
    m_type = KindOfInt64; m_data.num = v;
  }
  /* implicit */ Variant(unsigned long long v) noexcept {
    m_type = KindOfInt64; m_data.num = v;
  }
  /* implicit */ Variant(double    v) noexcept {
    m_type = KindOfDouble; m_data.dbl = v;
  }

  /* implicit */ Variant(const char* v) {
    m_type = KindOfString;
    m_data.pstr = StringData::Make(v);
  }
  /* implicit */ Variant(const std::string &v) {
    m_type = KindOfString;
    StringData *s = StringData::Make(v.c_str(), v.size(), CopyString);
    assert(s);
    m_data.pstr = s;
  }
  /* implicit */ Variant(const StaticString &v) noexcept {
    assert(v.get() && !v.get()->isRefCounted());
    m_type = KindOfPersistentString;
    m_data.pstr = v.get();
  }

  Variant(const Variant& other, WithRefBind) {
    tvDupWithRef(*other.asTypedValue(), *asTypedValue());
    if (m_type == KindOfUninit) m_type = KindOfNull;
  }

  /* implicit */ Variant(const String& v) noexcept : Variant(v.get()) {}
  /* implicit */ Variant(const Array& v) noexcept : Variant(v.get()) { }
  /* implicit */ Variant(const Object& v) noexcept : Variant(v.get()) {}
  /* implicit */ Variant(const Resource& v) noexcept
  : Variant(v.hdr()) {}

  /*
   * Explicit conversion constructors. These all manipulate ref-counts of bare
   * pointers as a side-effect, so we want to be explicit when its happening.
   */
  explicit Variant(StringData* v) noexcept;
  explicit Variant(ArrayData* v) noexcept {
    if (v) {
      m_data.parr = v;
      if (v->isRefCounted()) {
        m_type = v->toDataType();
        v->rawIncRefCount();
      } else {
        m_type = v->toPersistentDataType();
      }
    } else {
      m_type = KindOfNull;
    }
  }
  explicit Variant(ObjectData* v) noexcept {
    if (v) {
      m_type = KindOfObject;
      m_data.pobj = v;
      v->incRefCount();
    } else {
      m_type = KindOfNull;
    }
  }
  explicit Variant(RefData* r) noexcept {
    if (r) {
      m_type = KindOfRef;
      m_data.pref = r;
      r->incRefCount();
    } else {
      m_type = KindOfNull;
    }
  }

  template <typename T>
  explicit Variant(const req::ptr<T>& ptr) : Variant(ptr.get()) { }
  template <typename T>
  explicit Variant(req::ptr<T>&& ptr) noexcept
    : Variant(ptr.detach(), Attach{}) { }

  /*
   * Creation constructor from ArrayInit that avoids a null check and an
   * inc-ref.
   */
  explicit Variant(ArrayData* ad, DataType dt, ArrayInitCtor) noexcept {
    assert(ad->toDataType() == dt);
    m_type = dt;
    m_data.parr = ad;
  }

  enum class PersistentArrInit {};
  Variant(const ArrayData* ad, DataType dt, PersistentArrInit) noexcept {
    assert(ad->toPersistentDataType() == dt);
    assert(!ad->isRefCounted());
    m_data.parr = const_cast<ArrayData*>(ad);
    m_type = dt;
  }

  enum class PersistentStrInit {};
  explicit Variant(const StringData *s, PersistentStrInit) noexcept {
    assert(!s->isRefCounted());
    m_data.pstr = const_cast<StringData*>(s);
    m_type = KindOfPersistentString;
  }

  // These are prohibited, but declared just to prevent accidentally
  // calling the bool constructor just because we had a pointer to
  // const.
  /* implicit */ Variant(const void*) = delete;
  template<typename Ret, typename... Args>
  /* implicit */ Variant(Ret (*)(Args...)) = delete;
  template<class Class, typename Ret, typename... Args>
  /* implicit */ Variant(Ret (Class::*)(Args...)) = delete;
  template<class Class, typename Ret, typename... Args>
  /* implicit */ Variant(Ret (Class::*)(Args...) const) = delete;

  //////////////////////////////////////////////////////////////////////

  /*
   * Copy constructor and copy assignment do not semantically make
   * copies: they unbox refs and turn uninits to null.
   */

  Variant(const Variant& v) noexcept;

  Variant(const Variant& v, CellCopy) noexcept {
    m_type = v.m_type;
    m_data = v.m_data;
  }

  Variant(const Variant& v, CellDup) noexcept {
    m_type = v.m_type;
    m_data = v.m_data;
    tvIncRefGen(asTypedValue());
  }

  Variant(StrongBind, Variant& v) {
    assert(tvIsPlausible(v));
    tvBoxIfNeeded(v.asTypedValue());
    refDup(*v.asTypedValue(), *asTypedValue());
  }

  Variant& operator=(const Variant& v) noexcept {
    return assign(v);
  }

  /*
   * Move ctors
   *
   * Note: not semantically moves.  Like our "copy constructor", these
   * unbox refs and turn uninits to null.
   */

  Variant(Variant&& v) noexcept {
    if (UNLIKELY(v.m_type == KindOfRef)) {
      // We can't avoid the refcounting when it's a ref.  Do basically
      // what a copy would have done.
      moveRefHelper(std::move(v));
      return;
    }

    assert(this != &v);
    if (v.m_type != KindOfUninit) {
      m_type = v.m_type;
      m_data = v.m_data;
      v.m_type = KindOfNull;
    } else {
      m_type = KindOfNull;
    }
  }

  // Move ctor for strings
  /* implicit */ Variant(String&& v) noexcept {
    StringData *s = v.get();
    if (LIKELY(s != nullptr)) {
      m_data.pstr = s;
      m_type = s->isRefCounted() ? KindOfString : KindOfPersistentString;
      v.detach();
    } else {
      m_type = KindOfNull;
    }
  }

  // Move ctor for arrays
  /* implicit */ Variant(Array&& v) noexcept {
    ArrayData *a = v.get();
    if (LIKELY(a != nullptr)) {
      m_data.parr = a;
      m_type = a->isRefCounted() ? a->toDataType() : a->toPersistentDataType();
      v.detach();
    } else {
      m_type = KindOfNull;
    }
  }

  // Move ctor for objects
  /* implicit */ Variant(Object&& v) noexcept {
    ObjectData *pobj = v.get();
    if (pobj) {
      m_type = KindOfObject;
      m_data.pobj = pobj;
      v.detach();
    } else {
      m_type = KindOfNull;
    }
  }

  // Move ctor for resources
  /* implicit */ Variant(Resource&& v) noexcept {
    auto hdr = v.hdr();
    if (hdr) {
      m_type = KindOfResource;
      m_data.pres = hdr;
      v.detachHdr();
    } else {
      m_type = KindOfNull;
    }
  }

  /*
   * Move assign
   *
   * Note: not semantically moves.  Like our "copies", these unbox
   * refs and turn uninits to null.
   */
  Variant& operator=(Variant &&rhs) noexcept {
    assert(this != &rhs); // we end up as null on a self move-assign.
    if (rhs.m_type == KindOfRef) return *this = *rhs.m_data.pref->var();

    Variant& lhs = m_type == KindOfRef ? *m_data.pref->var() : *this;

    Variant goner((NoInit()));
    goner.m_data = lhs.m_data;
    goner.m_type = lhs.m_type;

    if (rhs.m_type == KindOfUninit) {
      lhs.m_type = KindOfNull;
    } else {
      lhs.m_type = rhs.m_type;
      lhs.m_data = rhs.m_data;
      rhs.m_type = KindOfNull;
    }

    return *this;
  }

  ALWAYS_INLINE ~Variant() noexcept {
    tvDecRefGen(asTypedValue());
    if (debug) {
      memset(this, kTVTrashFill2, sizeof(*this));
    }
  }


  //////////////////////////////////////////////////////////////////////

  /*
   * During sweeping, request-allocated things are not allowed to be decref'd
   * or manipulated.  This function is used to cause a Variant to go
   * into a state where its destructor will have no effects on the
   * request local heap, in cases where sweepable objects can't
   * organize things to avoid running Variant destructors.
   */
  void releaseForSweep() { m_type = KindOfNull; }

  //////////////////////////////////////////////////////////////////////

  /**
   * Break bindings and set to uninit.
   */
  void unset() {
    auto const old = *asTypedValue();
    m_type = KindOfUninit;
    tvDecRefGen(old);
  }

  /**
   * set to null without breaking bindings (if any), faster than v_a = null;
   */
  void setNull() noexcept {
    tvSetNull(*asTypedValue());
  }

  /**
   * Clear the original data, and set it to be the same as in v, and if
   * v is referenced, keep the reference.
   */
  Variant& setWithRef(TypedValue v) noexcept {
    tvSetWithRef(v, *asTypedValue());
    if (m_type == KindOfUninit) m_type = KindOfNull;
    return *this;
  }
  Variant& setWithRef(const Variant& v) noexcept {
    return setWithRef(*v.asTypedValue());
  }

  static Variant attach(TypedValue tv) noexcept {
    return Variant{tv, Attach{}};
  }
  static Variant attach(StringData* var) noexcept {
    return Variant{var, Attach{}};
  }
  static Variant attach(ArrayData* var) noexcept {
    return Variant{var, Attach{}};
  }
  static Variant attach(ObjectData* var) noexcept {
    return Variant{var, Attach{}};
  }
  static Variant attach(ResourceData* var) noexcept {
    return Variant{var, Attach{}};
  }
  static Variant attach(ResourceHdr* var) noexcept {
    return Variant{var, Attach{}};
  }
  static Variant attach(RefData* var) noexcept {
    return Variant{var, Attach{}};
  }

///////////////////////////////////////////////////////////////////////////////
// int64

  ALWAYS_INLINE int64_t asInt64Val() const {
    assert(m_type == KindOfInt64);
    return m_data.num;
  }

  ALWAYS_INLINE int64_t toInt64Val() const {
    assert(is(KindOfInt64));
    return
        LIKELY(m_type == KindOfInt64) ?
        m_data.num : m_data.pref->var()->m_data.num;
  }

///////////////////////////////////////////////////////////////////////////////
// double

  ALWAYS_INLINE double asDoubleVal() const {
    assert(m_type == KindOfDouble);
    return m_data.dbl;
  }

  ALWAYS_INLINE double toDoubleVal() const {
    assert(is(KindOfDouble));
    return
        LIKELY(m_type == KindOfDouble) ?
        m_data.dbl : m_data.pref->var()->m_data.dbl;
  }

///////////////////////////////////////////////////////////////////////////////
// boolean

  ALWAYS_INLINE bool asBooleanVal() const {
    assert(m_type == KindOfBoolean);
    return m_data.num;
  }

  ALWAYS_INLINE bool toBooleanVal() const {
    assert(is(KindOfBoolean));
    return
        LIKELY(m_type == KindOfBoolean) ?
        m_data.num : m_data.pref->var()->m_data.num;
  }

///////////////////////////////////////////////////////////////////////////////
// string

  ALWAYS_INLINE const String& asCStrRef() const {
    assert(isStringType(m_type) && m_data.pstr);
    return *reinterpret_cast<const String*>(&m_data.pstr);
  }

  ALWAYS_INLINE const String& toCStrRef() const {
    assert(isString());
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.pstr : m_data.pstr);
    return *reinterpret_cast<const String*>(LIKELY(isStringType(m_type)) ?
        &m_data.pstr : &m_data.pref->tv()->m_data.pstr);
  }

  ALWAYS_INLINE String& asStrRef() {
    assert(isStringType(m_type) && m_data.pstr);
    // The caller is likely going to modify the string, so we have to eagerly
    // promote KindOfPersistentString -> KindOfString.
    m_type = KindOfString;
    return *reinterpret_cast<String*>(&m_data.pstr);
  }

  ALWAYS_INLINE String& toStrRef() {
    assert(isString());
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.pstr : m_data.pstr);
    // The caller is likely going to modify the string, so we have to eagerly
    // promote KindOfPersistentString -> KindOfString.
    auto tv = LIKELY(isStringType(m_type)) ? this : m_data.pref->tv();
    tv->m_type = KindOfString;
    return *reinterpret_cast<String*>(&tv->m_data.pstr);
  }

///////////////////////////////////////////////////////////////////////////////
// array

  ALWAYS_INLINE const Array& asCArrRef() const {
    assert(isArrayLikeType(m_type) && m_data.parr);
    return *reinterpret_cast<const Array*>(&m_data.parr);
  }

  ALWAYS_INLINE const Array& toCArrRef() const {
    assert(isArray());
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.parr : m_data.parr);
    return *reinterpret_cast<const Array*>(LIKELY(isArrayLikeType(m_type)) ?
        &m_data.parr : &m_data.pref->tv()->m_data.parr);
  }

  ALWAYS_INLINE Array& asArrRef() {
    assert(isArrayLikeType(m_type) && m_data.parr);
    m_type = m_data.parr->toDataType();
    return *reinterpret_cast<Array*>(&m_data.parr);
  }

  ALWAYS_INLINE Array& toArrRef() {
    assert(isArray());
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.parr : m_data.parr);
    auto tv = LIKELY(isArrayLikeType(m_type)) ? this : m_data.pref->tv();
    tv->m_type = tv->m_data.parr->toDataType();
    return *reinterpret_cast<Array*>(&tv->m_data.parr);
  }

///////////////////////////////////////////////////////////////////////////////
// object

  ALWAYS_INLINE const Object& asCObjRef() const {
    assert(m_type == KindOfObject && m_data.pobj);
    return *reinterpret_cast<const Object*>(&m_data.pobj);
  }

  ALWAYS_INLINE const Object& toCObjRef() const {
    assert(is(KindOfObject));
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.pobj : m_data.pobj);
    return *reinterpret_cast<const Object*>(LIKELY(m_type == KindOfObject) ?
        &m_data.pobj : &m_data.pref->tv()->m_data.pobj);
  }

  ALWAYS_INLINE Object & asObjRef() {
    assert(m_type == KindOfObject && m_data.pobj);
    return *reinterpret_cast<Object*>(&m_data.pobj);
  }

  ALWAYS_INLINE const Resource& asCResRef() const {
    assert(m_type == KindOfResource && m_data.pres);
    return *reinterpret_cast<const Resource*>(&m_data.pres);
  }

  ALWAYS_INLINE const Resource& toCResRef() const {
    assert(is(KindOfResource));
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.pres : m_data.pres);
    return *reinterpret_cast<const Resource*>(LIKELY(m_type == KindOfResource) ?
        &m_data.pres : &m_data.pref->tv()->m_data.pres);
  }

  ALWAYS_INLINE Resource & asResRef() {
    assert(m_type == KindOfResource && m_data.pres);
    return *reinterpret_cast<Resource*>(&m_data.pres);
  }

  ALWAYS_INLINE Object& toObjRef() {
    assert(is(KindOfObject));
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.pobj : m_data.pobj);
    return *reinterpret_cast<Object*>(LIKELY(m_type == KindOfObject) ?
        &m_data.pobj : &m_data.pref->tv()->m_data.pobj);
  }

  /**
   * Type testing functions
   */
  DataType getType() const {
    return m_type == KindOfRef ? m_data.pref->var()->m_type : m_type;
  }
  DataType getRawType() const {
    return m_type;
  }
  bool is(DataType type) const {
    return getType() == type;
  }
  bool isInitialized() const {
    return m_type != KindOfUninit;
  }
  bool isNull() const {
    return isNullType(getType());
  }
  bool isBoolean() const {
    return getType() == KindOfBoolean;
  }
  bool isInteger() const {
    return getType() == KindOfInt64;
  }
  bool isDouble() const {
    return getType() == KindOfDouble;
  }
  bool isString() const {
    return isStringType(getType());
  }
  bool isArray() const {
    return isArrayLikeType(getType());
  }
  bool isPHPArray() const {
    return isArrayType(getType());
  }
  bool isVecArray() const {
    return isVecType(getType());
  }
  bool isDict() const {
    return isDictType(getType());
  }
  bool isKeyset() const {
    return isKeysetType(getType());
  }
  bool isHackArray() const {
    return isHackArrayType(getType());
  }
  bool isObject() const {
    return getType() == KindOfObject;
  }
  bool isResource() const {
    return getType() == KindOfResource;
  }

  bool isNumeric(bool checkString = false) const noexcept;
  DataType toNumeric(int64_t &ival, double &dval, bool checkString = false)
    const;
  bool isScalar() const noexcept;
  bool isIntVal() const {
    switch (m_type) {
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfObject:
      case KindOfResource:
        return true;
      case KindOfDouble:
      case KindOfPersistentString:
      case KindOfString:
      case KindOfPersistentVec:
      case KindOfVec:
      case KindOfPersistentDict:
      case KindOfDict:
      case KindOfPersistentKeyset:
      case KindOfKeyset:
      case KindOfPersistentArray:
      case KindOfArray:
        return false;
      case KindOfRef:
        return m_data.pref->var()->isIntVal();
    }
    not_reached();
  }

  // Is "define('CONSTANT', <this value>)" legal?
  bool isAllowedAsConstantValue() const;

  /**
   * Whether or not there are at least two variables that are strongly bound.
   */
  bool isReferenced() const {
    return m_type == KindOfRef && m_data.pref->isReferenced();
  }

  /**
   * Get reference count of weak or strong binding. For debugging purpose.
   */
  int getRefCount() const noexcept;

  bool getBoolean() const {
    assert(getType() == KindOfBoolean);
    return m_type == KindOfRef ? m_data.pref->var()->m_data.num : m_data.num;
  }
  int64_t getInt64() const {
    assert(getType() == KindOfInt64);
    return m_type == KindOfRef ? m_data.pref->var()->m_data.num : m_data.num;
  }
  double getDouble() const {
    assert(getType() == KindOfDouble);
    return m_type == KindOfRef ? m_data.pref->var()->m_data.dbl : m_data.dbl;
  }

  /**
   * Operators
   */
  Variant& assign(const Variant& v) noexcept {
    tvSet(tvToInitCell(v.asTypedValue()), *asTypedValue());
    return *this;
  }
  Variant& assignRef(Variant& v) noexcept {
    assert(&v != &uninit_variant);
    tvBoxIfNeeded(v.asTypedValue());
    tvBind(v.asTypedValue(), asTypedValue());
    return *this;
  }
  Variant& assignRef(VRefParam v) = delete;

  // Generic assignment operator. Forward argument (preserving rvalue-ness and
  // lvalue-ness) to the appropriate set function, as long as its not a Variant.
  template <typename T>
  typename std::enable_if<
    !std::is_same<
      Variant,
      typename std::remove_reference<typename std::remove_cv<T>::type>::type
    >::value
    &&
    !std::is_same<
      VarNR,
      typename std::remove_reference<typename std::remove_cv<T>::type>::type
    >::value,
    Variant&
  >::type operator=(T&& v) {
    set(std::forward<T>(v));
    return *this;
  }

  Variant  operator +  () const = delete;
  Variant &operator += (const Variant& v) = delete;
  Variant &operator += (int     n) = delete;
  Variant &operator += (int64_t   n) = delete;
  Variant &operator += (double  n)  = delete;

  Variant  operator -  () const = delete;
  Variant  operator -  (const Variant& v) const = delete;
  Variant &operator -= (const Variant& v) = delete;
  Variant &operator -= (int     n) = delete;
  Variant &operator -= (int64_t   n) = delete;
  Variant &operator -= (double  n) = delete;

  Variant  operator *  (const Variant& v) const = delete;
  Variant &operator *= (const Variant& v) = delete;
  Variant &operator *= (int     n) = delete;
  Variant &operator *= (int64_t   n) = delete;
  Variant &operator *= (double  n) = delete;

  Variant  operator /  (const Variant& v) const = delete;
  Variant &operator /= (const Variant& v) = delete;
  Variant &operator /= (int     n) = delete;
  Variant &operator /= (int64_t   n) = delete;
  Variant &operator /= (double  n) = delete;

  int64_t    operator %  (const Variant& v) const = delete;
  Variant &operator %= (const Variant& v) = delete;
  Variant &operator %= (int     n) = delete;
  Variant &operator %= (int64_t   n) = delete;
  Variant &operator %= (double  n) = delete;

  Variant  operator |  (const Variant& v) const = delete;
  Variant &operator |= (const Variant& v) = delete;
  Variant  operator &  (const Variant& v) const = delete;
  Variant &operator &= (const Variant& v) = delete;
  Variant  operator ^  (const Variant& v) const = delete;
  Variant &operator ^= (const Variant& v) = delete;
  Variant &operator <<=(int64_t n) = delete;
  Variant &operator >>=(int64_t n) = delete;

  Variant &operator ++ () = delete;
  Variant  operator ++ (int) = delete;
  Variant &operator -- () = delete;
  Variant  operator -- (int) = delete;

  /*
   * Variant used to implicitly convert to all these types.  (It still
   * implicitly converts *from* most of them.)
   *
   * We're leaving these functions deleted for now because we fear the
   * possibility of changes to overload resolution by not declaring
   * them.  Eventually when fewer of these types have implicit
   * conversions we'll remove them.
   */
  /* implicit */ operator bool   () const = delete;
  /* implicit */ operator char   () const = delete;
  /* implicit */ operator short  () const = delete;
  /* implicit */ operator int    () const = delete;
  /* implicit */ operator int64_t  () const = delete;
  /* implicit */ operator double () const = delete;
  /* implicit */ operator String () const = delete;
  /* implicit */ operator Array  () const = delete;
  /* implicit */ operator Object () const = delete;

  /**
   * Explicit type conversions
   */
  bool toBoolean() const {
    if (isNullType(m_type)) return false;
    if (m_type <= KindOfInt64) return m_data.num;
    return toBooleanHelper();
  }
  char toByte() const { return (char)toInt64();}
  short toInt16(int base = 10) const { return (short)toInt64(base);}
  int toInt32(int base = 10) const { return (int)toInt64(base);}
  int64_t toInt64() const {
    if (isNullType(m_type)) return 0;
    if (m_type <= KindOfInt64) return m_data.num;
    return toInt64Helper(10);
  }
  int64_t toInt64(int base) const {
    if (isNullType(m_type)) return 0;
    if (m_type <= KindOfInt64) return m_data.num;
    return toInt64Helper(base);
  }
  double toDouble() const {
    if (m_type == KindOfDouble) return m_data.dbl;
    return toDoubleHelper();
  }

  String toString() const& {
    if (isStringType(m_type)) return String{m_data.pstr};
    return toStringHelper();
  }

  String toString() && {
    if (isStringType(m_type)) {
      m_type = KindOfNull;
      return String::attach(m_data.pstr);
    }
    return toStringHelper();
  }

  // Convert a non-array-like type to a PHP array, leaving PHP arrays and Hack
  // arrays unchanged. Use toPHPArray() if you want the result to always be a
  // PHP array.
  Array toArray() const {
    if (isArrayLikeType(m_type)) return Array(m_data.parr);
    return toArrayHelper();
  }
  Array toPHPArray() const {
    if (isArrayType(m_type)) return Array(m_data.parr);
    return toPHPArrayHelper();
  }
  Object toObject() const {
    if (m_type == KindOfObject) return Object{m_data.pobj};
    return toObjectHelper();
  }
  Resource toResource() const {
    if (m_type == KindOfResource) return Resource{m_data.pres};
    return toResourceHelper();
  }

  Array toVecArray() const {
    if (isVecType(m_type)) return Array{m_data.parr};
    auto copy = *this;
    tvCastToVecInPlace(copy.asTypedValue());
    assertx(copy.isVecArray());
    return Array::attach(copy.detach().m_data.parr);
  }

  Array toDict() const {
    if (isDictType(m_type)) return Array{m_data.parr};
    auto copy = *this;
    tvCastToDictInPlace(copy.asTypedValue());
    assertx(copy.isDict());
    return Array::attach(copy.detach().m_data.parr);
  }

  Array toKeyset() const {
    if (isKeysetType(m_type)) return Array{m_data.parr};
    auto copy = *this;
    tvCastToKeysetInPlace(copy.asTypedValue());
    assertx(copy.isKeyset());
    return Array::attach(copy.detach().m_data.parr);
  }

  Array toVArray() const {
    if (isArrayType(m_type)) return asCArrRef().toVArray();
    auto copy = *this;
    tvCastToVArrayInPlace(copy.asTypedValue());
    assertx(copy.isPHPArray() && copy.asCArrRef().isVArray());
    return Array::attach(copy.detach().m_data.parr);
  }

  Array toDArray() const {
    if (isArrayType(m_type)) return Array{m_data.parr};
    auto copy = *this;
    tvCastToDArrayInPlace(copy.asTypedValue());
    assertx(copy.isPHPArray());
    return Array::attach(copy.detach().m_data.parr);
  }

  template <typename T>
  typename std::enable_if<std::is_base_of<ResourceData,T>::value, bool>::type
  isa() const {
    if (m_type == KindOfResource) {
      return m_data.pres->data()->instanceof<T>();
    }
    if (m_type == KindOfRef && m_data.pref->var()->m_type == KindOfResource) {
      return m_data.pref->var()->m_data.pres->data()->instanceof<T>();
    }
    return false;
  }

  template <typename T>
  typename std::enable_if<std::is_base_of<ObjectData,T>::value, bool>::type
  isa() const {
    if (m_type == KindOfObject) {
      return m_data.pobj->instanceof<T>();
    }
    if (m_type == KindOfRef &&
               m_data.pref->var()->m_type == KindOfObject) {
      return m_data.pref->var()->m_data.pobj->instanceof<T>();
    }
    return false;
  }

  /**
   * Whether or not calling toKey() will throw a bad type exception
   */
  bool canBeValidKey() const {
    return !isArrayType(getType()) && getType() != KindOfObject;
  }

  /*
   * Convert to a valid key or throw an exception. If convertStrKeys is true
   * int-like string keys will be converted to int keys.
   */
  VarNR toKey(const ArrayData*) const;

  /* Creating a temporary Array, String, or Object with no ref-counting and
   * no type checking, use it only when we have checked the variant type and
   * we are sure the internal data will have a reference until the temporary
   * one gets out-of-scope.
   */
  StrNR toStrNR() const {
    return StrNR(getStringData());
  }
  ArrNR toArrNR() const {
    return ArrNR(getArrayData());
  }
  ObjNR toObjNR() const {
    return ObjNR(getObjectData());
  }

  /*
   * Low level access that should be restricted to internal use.
   */
  int64_t *getInt64Data() const {
    assert(getType() == KindOfInt64);
    return m_type == KindOfRef ? &m_data.pref->var()->m_data.num :
                     const_cast<int64_t*>(&m_data.num);
  }
  double *getDoubleData() const {
    assert(getType() == KindOfDouble);
    return m_type == KindOfRef ? &m_data.pref->var()->m_data.dbl :
                     const_cast<double*>(&m_data.dbl);
  }
  StringData *getStringData() const {
    assert(isStringType(getType()));
    return m_type == KindOfRef ? m_data.pref->var()->m_data.pstr : m_data.pstr;
  }
  StringData *getStringDataOrNull() const {
    // This is a necessary evil because getStringData() returns
    // an undefined result if this is a null variant
    assert(isNull() || isString());
    return m_type == KindOfRef ?
      (m_data.pref->var()->m_type <= KindOfNull ? nullptr :
        m_data.pref->var()->m_data.pstr) :
      (m_type <= KindOfNull ? nullptr : m_data.pstr);
  }
  ArrayData *getArrayData() const {
    assert(isArray());
    return m_type == KindOfRef ? m_data.pref->var()->m_data.parr : m_data.parr;
  }
  ArrayData *getArrayDataOrNull() const {
    // This is a necessary evil because getArrayData() returns
    // an undefined result if this is a null variant
    assert(isNull() || isArray());
    return m_type == KindOfRef ?
      (m_data.pref->var()->m_type <= KindOfNull ? nullptr :
        m_data.pref->var()->m_data.parr) :
      (m_type <= KindOfNull ? nullptr : m_data.parr);
  }
  ObjectData* getObjectData() const {
    assert(is(KindOfObject));
    return m_type == KindOfRef ? m_data.pref->var()->m_data.pobj : m_data.pobj;
  }
  ObjectData *getObjectDataOrNull() const {
    // This is a necessary evil because getObjectData() returns
    // an undefined result if this is a null variant
    assert(isNull() || is(KindOfObject));
    return m_type == KindOfRef ?
      (m_data.pref->var()->m_type <= KindOfNull ? nullptr :
        m_data.pref->var()->m_data.pobj) :
      (m_type <= KindOfNull ? nullptr : m_data.pobj);
  }
  Variant *getRefData() const {
    assert(m_type == KindOfRef);
    return m_data.pref->var();
  }

  int64_t getNumData() const { return m_data.num; }

  /*
   * Make any Variant strings and arrays not ref counted (e.g., static).
   * Use it, for example, if you need a long-lived Variant before the Memory
   * Manager has been initialized.
   * You will still get an assertion if the Variant is an object, resource, etc.
   */
  void setEvalScalar();

  /*
   * Access this Variant as a TypedValue. Does not unbox refs, etc.
   */
  const TypedValue* asTypedValue() const { return this; }
        TypedValue* asTypedValue()       { return this; }

  /*
   * Access this Variant as a Cell.  I.e. unboxes it if it was a
   * KindOfRef.
   */
  const Cell* asCell() const { return tvToCell(asTypedValue()); }
        Cell* asCell()       { return tvToCell(asTypedValue()); }

  /*
   * Read this Variant as an InitCell, without incrementing the
   * reference count.  I.e. unbox if it is boxed, and turn
   * KindOfUninit into KindOfNull.
   */
  Cell asInitCellTmp() const {
    if (UNLIKELY(m_type == KindOfRef)) {
      return *m_data.pref->tv();
    }
    if (m_type == KindOfUninit) return make_tv<KindOfNull>();
    return *this;
  }

  /*
   * Access this Variant as a Ref, converting it to a Ref it isn't
   * one.
   */
  Ref* asRef() { tvBoxIfNeeded(asTypedValue()); return this; }

  TypedValue detach() noexcept {
    auto tv = *asTypedValue();
    m_type = KindOfNull;
    return tv;
  }

 private:
  ResourceData* getResourceData() const {
    assert(is(KindOfResource));
    return m_type == KindOfRef ? m_data.pref->var()->m_data.pres->data() :
                                 m_data.pres->data();
  }

  ResourceData* detachResourceData() {
    assert(is(KindOfResource));
    if (LIKELY(m_type == KindOfResource)) {
      m_type = KindOfNull;
      return m_data.pres->data();
    }
    m_type = KindOfNull;
    return m_data.pref->tv()->m_data.pres->data();
  }

  ObjectData* detachObjectData() {
    assert(is(KindOfObject));
    if (LIKELY(m_type == KindOfObject)) {
      m_type = KindOfNull;
      return m_data.pobj;
    } else {
      m_type = KindOfNull;
      return m_data.pref->tv()->m_data.pobj;
    }
  }

  template <typename T>
  friend typename std::enable_if<
    std::is_base_of<ResourceData,T>::value,
    ResourceData*
  >::type deref(const Variant& v) { return v.getResourceData(); }

  template <typename T>
  friend typename std::enable_if<
    std::is_base_of<ObjectData,T>::value,
    ObjectData*
  >::type deref(const Variant& v) { return v.getObjectData(); }

  template <typename T>
  friend typename std::enable_if<
    std::is_base_of<ResourceData,T>::value,
    ResourceData*
  >::type detach(Variant&& v) { return v.detachResourceData(); }

  template <typename T>
  friend typename std::enable_if<
    std::is_base_of<ObjectData,T>::value,
    ObjectData*
  >::type detach(Variant&& v) { return v.detachObjectData(); }

  explicit Variant(ResourceData* v) noexcept {
    if (v) {
      m_type = KindOfResource;
      m_data.pres = v->hdr();
      v->incRefCount();
    } else {
      m_type = KindOfNull;
    }
  }
  explicit Variant(ResourceHdr* v) noexcept {
    if (v) {
      m_type = KindOfResource;
      m_data.pres = v;
      v->incRefCount();
    } else {
      m_type = KindOfNull;
    }
  }

  /*
   * This set of constructors act like the normal constructors for the
   * given types except that they do not increment the reference count
   * of the passed value.  They are used for the req::ptr move constructor.
   */
  Variant(StringData* var, Attach) noexcept {
    if (var) {
      m_type = var->isRefCounted() ? KindOfString : KindOfPersistentString;
      m_data.pstr = var;
    } else {
      m_type = KindOfNull;
    }
  }
  Variant(ArrayData* var, Attach) noexcept {
    if (var) {
      m_type =
        var->isRefCounted() ? var->toDataType() : var->toPersistentDataType();
      m_data.parr = var;
    } else {
      m_type = KindOfNull;
    }
  }
  Variant(ObjectData* var, Attach) noexcept {
    if (var) {
      m_type = KindOfObject;
      m_data.pobj = var;
    } else {
      m_type = KindOfNull;
    }
  }
  Variant(ResourceData* var, Attach) noexcept {
    if (var) {
      m_type = KindOfResource;
      m_data.pres = var->hdr();
    } else {
      m_type = KindOfNull;
    }
  }
  Variant(ResourceHdr* var, Attach) noexcept {
    if (var) {
      m_type = KindOfResource;
      m_data.pres = var;
    } else {
      m_type = KindOfNull;
    }
  }
  Variant(RefData* var, Attach) noexcept {
    if (var) {
      m_type = KindOfRef;
      m_data.pref = var;
    } else {
      m_type = KindOfNull;
    }
  }
  Variant(TypedValue tv, Attach) noexcept : TypedValue(tv) {}

  bool isPrimitive() const { return !isRefcountedType(m_type); }
  bool isObjectConvertable() {
    assert(m_type != KindOfRef);
    return m_type <= KindOfNull ||
      (m_type == KindOfBoolean && !m_data.num) ||
      (isStringType(m_type) && m_data.pstr->empty());
  }

  void set(bool    v) noexcept;
  void set(int     v) noexcept;
  void set(int64_t   v) noexcept;
  void set(double  v) noexcept;
  void set(const char* v) = delete;
  void set(const std::string & v) {
    return set(String(v));
  }
  void set(StringData  *v) noexcept;
  void set(ArrayData   *v) noexcept;
  void set(ObjectData  *v) noexcept;
  void set(ResourceHdr *v) noexcept;
  void set(ResourceData *v) noexcept { set(v->hdr()); }
  void set(const StringData *v) = delete;
  void set(const ArrayData *v) = delete;
  void set(const ObjectData *v) = delete;
  void set(const ResourceData *v) = delete;
  void set(const ResourceHdr *v) = delete;

  void set(const String& v) noexcept { set(v.get()); }
  void set(const StaticString & v) noexcept;
  void set(const Array& v) noexcept { set(v.get()); }
  void set(const Object& v) noexcept { set(v.get()); }
  void set(const Resource& v) noexcept { set(v.hdr()); }

  void set(String&& v) noexcept { steal(v.detach()); }
  void set(Array&& v) noexcept { steal(v.detach()); }
  void set(Object&& v) noexcept { steal(v.detach()); }
  void set(Resource&& v) noexcept { steal(v.detachHdr()); }

  template<typename T>
  void set(const req::ptr<T> &v) noexcept {
    return set(v.get());
  }

  template <typename T>
  void set(req::ptr<T>&& v) noexcept {
    return steal(v.detach());
  }

  void steal(StringData* v) noexcept;
  void steal(ArrayData* v) noexcept;
  void steal(ObjectData* v) noexcept;
  void steal(ResourceHdr* v) noexcept;
  void steal(ResourceData* v) noexcept { steal(v->hdr()); }

private:
  void moveRefHelper(Variant&& v) {
    assert(tvIsPlausible(v));

    assert(v.m_type == KindOfRef);
    m_type = v.m_data.pref->tv()->m_type; // Can't be KindOfUninit.
    m_data = v.m_data.pref->tv()->m_data;
    tvIncRefGen(asTypedValue());
    decRefRef(v.m_data.pref);
    v.m_type = KindOfNull;
  }

  bool   toBooleanHelper() const;
  int64_t  toInt64Helper(int base = 10) const;
  double toDoubleHelper() const;
  String toStringHelper() const;
  Array  toArrayHelper() const;
  Array  toPHPArrayHelper() const;
  Object toObjectHelper() const;
  Resource toResourceHelper() const;

  DataType convertToNumeric(int64_t *lval, double *dval) const;
};

Variant operator+(const Variant & lhs, const Variant & rhs) = delete;

struct RefResultValue {
  const Variant& get() const { return m_var; }
private:
  Variant m_var;
};

struct VRefParamValue {
  template <class T> /* implicit */ VRefParamValue(const T &v) : m_var(v) {}

  /* implicit */ VRefParamValue() : m_var(Variant::NullInit()) {}
  /* implicit */ VRefParamValue(RefResult v)
    : m_var(Variant::StrongBind{}, const_cast<Variant&>(variant(v))) {} // XXX
  template <typename T>
  Variant &operator=(const T &v) const = delete;
  operator const Variant&() const { return m_var; }
  const Variant *operator&() const { return &m_var; } // FIXME
  const Variant *operator->() const { return &m_var; }

  const Variant& wrapped() const { return m_var; }

  explicit operator bool   () const { return m_var.toBoolean();}
  operator int    () const { return m_var.toInt32();}
  operator int64_t() const { return m_var.toInt64();}
  operator double () const { return m_var.toDouble();}
  operator String () const { return m_var.toString();}
  operator Array  () const { return m_var.toArray();}
  operator Object () const { return m_var.toObject();}
  explicit operator Resource () const { return m_var.toResource();}

  bool is(DataType type) const { return m_var.is(type); }
  bool isString() const { return m_var.isString(); }
  bool isObject() const { return m_var.isObject(); }
  bool isReferenced() const { return m_var.isReferenced(); }
  bool isNull() const { return m_var.isNull(); }
  bool isRefData() const { return m_var.asTypedValue()->m_type == KindOfRef; }

  bool toBoolean() const { return m_var.toBoolean(); }
  int64_t toInt64() const { return m_var.toInt64(); }
  double toDouble() const { return m_var.toDouble(); }
  String toString() const { return m_var.toString(); }
  StringData *getStringData() const { return m_var.getStringData(); }
  Array toArray() const { return m_var.toArray(); }
  Object toObject() const { return m_var.toObject(); }
  Resource toResource() const { return m_var.toResource(); }
  ObjectData *getObjectData() const { return m_var.getObjectData(); }

  bool isArray() const { return m_var.isArray(); }
  bool isHackArray() const { return m_var.isHackArray(); }
  bool isPHPArray() const { return m_var.isPHPArray(); }
  ArrNR toArrNR() const { return m_var.toArrNR(); }

  RefData* getRefData() const {
    assert(isRefData());
    return m_var.asTypedValue()->m_data.pref;
  }
  RefData* getRefDataOrNull() const {
    return isRefData() ? m_var.asTypedValue()->m_data.pref : nullptr;
  }
  Variant* getVariantOrNull() const {
    return isRefData() ? m_var.asTypedValue()->m_data.pref->var() : nullptr;
  }
  void assignIfRef(const Variant& other) const {
    if (auto ref = getVariantOrNull()) *ref = other;
  }
  void assignIfRef(Variant&& other) const {
    if (auto ref = getVariantOrNull()) *ref = std::move(other);
  }
private:
  Variant m_var;
};

///////////////////////////////////////////////////////////////////////////////
// VarNR

struct VarNR : private TypedValueAux {
  static VarNR MakeKey(const String& s) {
    if (s.empty()) return VarNR(staticEmptyString());
    int64_t n;
    if (UNLIKELY(s.get()->isStrictlyInteger(n))) {
      if (RuntimeOption::EvalHackArrCompatNotices) {
        raise_intish_index_cast();
      }
      return VarNR(n);
    }
    return VarNR(s);
  }

  // Use to hold variant that do not need ref-counting
  explicit VarNR(bool    v) { init(KindOfBoolean); m_data.num = (v?1:0);}
  explicit VarNR(int     v) { init(KindOfInt64  ); m_data.num = v;}
  // The following two overloads will accept int64_t whether it's
  // implemented as long or long long.
  explicit VarNR(long    v) { init(KindOfInt64  ); m_data.num = v;}
  explicit VarNR(long long v) { init(KindOfInt64  ); m_data.num = v;}
  explicit VarNR(uint64_t  v) { init(KindOfInt64  ); m_data.num = v;}
  explicit VarNR(double  v) { init(KindOfDouble ); m_data.dbl = v;}

  explicit VarNR(const StaticString &v) {
    assert(v.get() && !v.get()->isRefCounted());
    init(KindOfPersistentString);
    m_data.pstr = v.get();
  }

  explicit VarNR(const String& v);
  explicit VarNR(const Array& v);
  explicit VarNR(const Object& v);
  explicit VarNR(StringData *v);
  explicit VarNR(const StringData *v) {
    assert(v && !v->isRefCounted());
    init(KindOfPersistentString);
    m_data.pstr = const_cast<StringData*>(v);
  }
  explicit VarNR(ArrayData *v);
  explicit VarNR(const ArrayData*) = delete;
  explicit VarNR(ObjectData *v);
  explicit VarNR(const ObjectData*) = delete;

  explicit VarNR(TypedValue tv) { init(tv.m_type); m_data = tv.m_data; }

  VarNR(const VarNR &v) : TypedValueAux(v) {}

  explicit VarNR() { asVariant()->asTypedValue()->m_type = KindOfUninit; }

  ~VarNR() {
    if (debug) {
      checkRefCount();
      memset(this, kTVTrashFill2, sizeof(*this));
    }
  }

  TypedValue tv() const { return *this; }

  operator const Variant&() const { return *asVariant(); }

  bool isNull() const {
    return asVariant()->isNull();
  }
private:
  /* implicit */ VarNR(const char* v) = delete;
  /* implicit */ VarNR(const std::string & v) = delete;

  void init(DataType dt) {
    m_type = dt;
    if (debug) varNrFlag() = NR_FLAG;
  }
  const Variant *asVariant() const {
    return &tvAsCVarRef(static_cast<const TypedValue*>(this));
  }
  Variant* asVariant() {
    return &tvAsVariant(static_cast<TypedValue*>(this));
  }
  void checkRefCount() {
    assert(isRefcountedType(m_type) ? varNrFlag() == NR_FLAG : true);

    switch (m_type) {
      DT_UNCOUNTED_CASE:
        return;
      case KindOfString:
        assert(m_data.pstr->checkCount());
        return;
      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfArray:
        assert(m_data.parr->checkCount());
        return;
      case KindOfObject:
        assert(m_data.pobj->checkCount());
        return;
      case KindOfResource:
        assert(m_data.pres->checkCount());
        return;
      case KindOfRef:
        assert(m_data.pref->checkCount());
        return;
    }
    not_reached();
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * The lvalBlackHole is used in array operations when a NewElem can't
 * create a new slot.  (Basically if the next integer key in an array
 * is already at the maximum integer.)
 */
Variant& lvalBlackHole();

/*
 * The lvalBlackHole has request lifetime.
 */
void initBlackHole();
void clearBlackHole();

///////////////////////////////////////////////////////////////////////////////
// breaking circular dependencies

inline Variant Array::operator[](Cell key) const {
  return rvalAt(key);
}
inline Variant Array::operator[](int key) const {
  return rvalAt(key);
}
inline Variant Array::operator[](int64_t key) const {
  return rvalAt(key);
}
inline Variant Array::operator[](const String& key) const {
  return rvalAt(key);
}
inline Variant Array::operator[](const Variant& key) const {
  return rvalAt(key);
}

inline void Array::append(const Variant& v) {
  append(*v.asTypedValue());
}
inline void Array::appendWithRef(const Variant& v) {
  appendWithRef(*v.asTypedValue());
}
inline void Array::prepend(const Variant& v) {
  prepend(*v.asTypedValue());
}

ALWAYS_INLINE Variant uninit_null() {
  return Variant();
}

ALWAYS_INLINE Variant init_null() {
  return Variant(Variant::NullInit());
}

inline Variant &concat_assign(Variant &v1, const char* s2) = delete;

inline Variant &concat_assign(Variant &v1, const String& s2) {
  if (v1.getType() == KindOfString) {
    auto& str = v1.asStrRef();
    if (!str.get()->cowCheck()) {
      str += s2.slice();
      return v1;
    }
  }

  auto s1 = v1.toString();
  s1 += s2;
  v1 = s1;
  return v1;
}

//////////////////////////////////////////////////////////////////////

// Defined here for include order reasons.
inline RefData::~RefData() {
  assert(m_kind == HeaderKind::Ref);
  tvAsVariant(&m_tv).~Variant();
}

//////////////////////////////////////////////////////////////////////

inline Array& forceToArray(Variant& var) {
  if (!var.isArray()) var = Variant(Array::Create());
  return var.toArrRef();
}

inline Array& forceToDict(Variant& var) {
  if (!var.isDict()) var = Variant(Array::CreateDict());
  return var.toArrRef();
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE Variant empty_string_variant() {
  return Variant(staticEmptyString(), Variant::PersistentStrInit{});
}

template <typename T>
inline Variant toVariant(const req::ptr<T>& p) {
  return p ? Variant(p) : Variant(false);
}

template <typename T>
inline Variant toVariant(req::ptr<T>&& p) {
  return p ? Variant(std::move(p)) : Variant(false);
}

template <>
inline bool is_null(const Variant& v) {
  return v.isNull();
}

template <typename T>
inline bool isa_non_null(const Variant& v) {
  return v.isa<T>();
}

// Defined here to avoid introducing a dependency cycle between type-variant
// and type-array
ALWAYS_INLINE Cell Array::convertKey(Cell k) const {
  return cellToKey(k, m_arr ? m_arr.get() : staticEmptyArray());
}
ALWAYS_INLINE Cell Array::convertKey(const Variant& k) const {
  return convertKey(*k.asCell());
}

inline VarNR Variant::toKey(const ArrayData* ad) const {
  return VarNR(tvToKey(*this, ad));
}

//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_VARIANT_H_
