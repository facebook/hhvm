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

#ifndef incl_HPHP_VARIANT_H_
#define incl_HPHP_VARIANT_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/type-string.h"

#include <algorithm>
#include <type_traits>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

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
  enum class ArrayInitCtor {};

  Variant() { m_type = KindOfUninit; }
  explicit Variant(NullInit) { m_type = KindOfNull; }
  explicit Variant(NoInit) {}

  /* implicit */ Variant(bool    v) { m_type = KindOfBoolean; m_data.num = v; }
  /* implicit */ Variant(int     v) { m_type = KindOfInt64; m_data.num = v; }
  // The following two overloads will accept int64_t whether it's
  // implemented as long or long long.
  /* implicit */ Variant(long   v) { m_type = KindOfInt64; m_data.num = v; }
  /* implicit */ Variant(long long v) { m_type = KindOfInt64; m_data.num = v; }
  /* implicit */ Variant(uint64_t  v) { m_type = KindOfInt64; m_data.num = v; }

  /* implicit */ Variant(double  v) { m_type = KindOfDouble; m_data.dbl = v; }

  /* implicit */ Variant(litstr  v);
  /* implicit */ Variant(const std::string &v);
  /* implicit */ Variant(const StaticString &v) {
    m_type = KindOfStaticString;
    StringData *s = v.get();
    assert(s);
    m_data.pstr = s;
  }

  /* implicit */ Variant(const String& v);
  /* implicit */ Variant(const Array& v);
  /* implicit */ Variant(const Object& v);
  /* implicit */ Variant(const Resource& v);
  /* implicit */ Variant(StringData *v);
  /* implicit */ Variant(ArrayData *v);
  /* implicit */ Variant(ObjectData *v);
  /* implicit */ Variant(ResourceData *v);
  /* implicit */ Variant(RefData *r);

  /* implicit */ Variant(CVarStrongBind v);
  /* implicit */ Variant(CVarWithRefBind v);

  /*
   * Creation constructor from ArrayInit that avoids a null check.
   */
  explicit Variant(ArrayData* ad, ArrayInitCtor) {
    m_type = KindOfArray;
    m_data.parr = ad;
    ad->incRefCount();
  }

  // for static strings only
  explicit Variant(const StringData *v);

  // These are prohibited, but declared just to prevent accidentally
  // calling the bool constructor just because we had a pointer to
  // const.
  /* implicit */ Variant(const ArrayData *v) = delete;
  /* implicit */ Variant(const ObjectData *v) = delete;
  /* implicit */ Variant(const ResourceData *v) = delete;
  /* implicit */ Variant(const RefData *v) = delete;
  /* implicit */ Variant(const TypedValue *v) = delete;
  /* implicit */ Variant(TypedValue *v) = delete;
  /* implicit */ Variant(const /* implicit */ Variant *v) = delete;
  /* implicit */ Variant(/* implicit */ Variant *v) = delete;

  //////////////////////////////////////////////////////////////////////

  /*
   * Copy constructor and copy assignment do not semantically make
   * copies: they unbox refs and turn uninits to null.
   */

  Variant(const Variant& v);

  Variant& operator=(const Variant& v) {
    return assign(v);
  }

  /*
   * Move ctors
   *
   * Note: not semantically moves.  Like our "copy constructor", these
   * unbox refs and turn uninits to null.
   */

  Variant(Variant&& v) {
    if (UNLIKELY(v.m_type == KindOfRef)) {
      // We can't avoid the refcounting when it's a ref.  Do basically
      // what a copy would have done.
      moveRefHelper(std::move(v));
      return;
    }

    assert(this != &v);
    m_type = v.m_type != KindOfUninit ? v.m_type : KindOfNull;
    m_data = v.m_data;
    v.m_type = KindOfNull;
  }

  // Move ctor for strings
  /* implicit */ Variant(String&& v) {
    StringData *s = v.get();
    if (LIKELY(s != nullptr)) {
      m_data.pstr = s;
      m_type = s->isStatic()
        ? KindOfStaticString
        : KindOfString;
      v.detach();
    } else {
      m_type = KindOfNull;
    }
  }

  // Move ctor for arrays
  /* implicit */ Variant(Array&& v) {
    m_type = KindOfArray;
    ArrayData *a = v.get();
    if (LIKELY(a != nullptr)) {
      m_data.parr = a;
      v.detach();
    } else {
      m_type = KindOfNull;
    }
  }

  // Move ctor for objects
  /* implicit */ Variant(Object&& v) {
    m_type = KindOfObject;
    ObjectData *pobj = v.get();
    if (pobj) {
      m_data.pobj = pobj;
      v.detach();
    } else {
      m_type = KindOfNull;
    }
  }

  // Move ctor for resources
  /* implicit */ Variant(Resource&& v) {
    m_type = KindOfResource;
    ResourceData *pres = v.get();
    if (pres) {
      m_data.pres = pres;
      v.detach();
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
  Variant& operator=(Variant &&rhs) {
    assert(this != &rhs); // TODO(#2484130): we end up as null on a
                          // self move-assign; decide if this is ok.
    if (rhs.m_type == KindOfRef) return *this = *rhs.m_data.pref->var();

    Variant& lhs = m_type == KindOfRef ? *m_data.pref->var() : *this;

    Variant goner((NoInit()));
    goner.m_data = lhs.m_data;
    goner.m_type = lhs.m_type;

    lhs.m_data = rhs.m_data;
    lhs.m_type = rhs.m_type == KindOfUninit ? KindOfNull : rhs.m_type;

    rhs.m_type = KindOfNull;
    return *this;
  }

  // D462768 showed no gain from inlining, even just into hphp-array.o
  ~Variant();

  //////////////////////////////////////////////////////////////////////

  /*
   * During sweeping, smart resources are not allowed to be decref'd
   * or manipulated.  This function is used to cause a Variant to go
   * into a state where its destructor will have no effects on the
   * request local heap, in cases where sweepable objects can't
   * organize things to avoid running Variant destructors.
   */
  void releaseForSweep() { m_type = KindOfNull; }

  //////////////////////////////////////////////////////////////////////

 public:
  /**
   * Break bindings and set to null.
   */
  void unset() {
    auto const d = m_data.num;
    auto const t = m_type;
    m_type = KindOfUninit;
    tvRefcountedDecRefHelper(t, d);
  }

  /**
   * set to null without breaking bindings (if any), faster than v_a = null;
   */
  void setNull();

  /**
   * Clear the original data, and set it to be the same as in v, and if
   * v is referenced, keep the reference.
   * In order to correctly copy circular arrays, even if v is the only
   * strong reference to arr, we still keep the reference.
   */
  Variant& setWithRef(const Variant& v);

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
    assert(IS_STRING_TYPE(m_type) && m_data.pstr);
    return *reinterpret_cast<const String*>(&m_data.pstr);
  }

  ALWAYS_INLINE const String& toCStrRef() const {
    assert(is(KindOfString) || is(KindOfStaticString));
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.pstr : m_data.pstr);
    return *reinterpret_cast<const String*>(LIKELY(IS_STRING_TYPE(m_type)) ?
        &m_data.pstr : &m_data.pref->tv()->m_data.pstr);
  }

  ALWAYS_INLINE String& asStrRef() {
    assert(IS_STRING_TYPE(m_type) && m_data.pstr);
    return *reinterpret_cast<String*>(&m_data.pstr);
  }

  ALWAYS_INLINE String& toStrRef() {
    assert(is(KindOfString) || is(KindOfStaticString));
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.pstr : m_data.pstr);
    return *reinterpret_cast<String*>(LIKELY(IS_STRING_TYPE(m_type)) ?
        &m_data.pstr : &m_data.pref->tv()->m_data.pstr);
  }

///////////////////////////////////////////////////////////////////////////////
// array

  ALWAYS_INLINE const Array& asCArrRef() const {
    assert(m_type == KindOfArray && m_data.parr);
    return *reinterpret_cast<const Array*>(&m_data.parr);
  }

  ALWAYS_INLINE const Array& toCArrRef() const {
    assert(is(KindOfArray));
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.parr : m_data.parr);
    return *reinterpret_cast<const Array*>(LIKELY(m_type == KindOfArray) ?
        &m_data.parr : &m_data.pref->tv()->m_data.parr);
  }

  ALWAYS_INLINE Array& asArrRef() {
    assert(m_type == KindOfArray && m_data.parr);
    return *reinterpret_cast<Array*>(&m_data.parr);
  }

  ALWAYS_INLINE Array& toArrRef() {
    assert(is(KindOfArray));
    assert(m_type == KindOfRef ? m_data.pref->var()->m_data.parr : m_data.parr);
    return *reinterpret_cast<Array*>(LIKELY(m_type == KindOfArray) ?
        &m_data.parr : &m_data.pref->tv()->m_data.parr);
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
    return IS_NULL_TYPE(getType());
  }
  bool isBoolean() const {
    return getType() == KindOfBoolean;
  }
  bool isDouble() const {
    return getType() == KindOfDouble;
  }
  bool isString() const {
    return IS_STRING_TYPE(getType());
  }
  bool isInteger() const;
  bool isNumeric(bool checkString = false) const;
  DataType toNumeric(int64_t &ival, double &dval, bool checkString = false)
    const;
  bool isScalar() const;
  bool isObject() const {
    return getType() == KindOfObject;
  }
  bool isIntVal() const {
    switch (m_type) {
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfObject:
      case KindOfResource:
        return true;
      case KindOfRef:
        return m_data.pref->var()->isIntVal();
      default:
        break;
    }
    return false;
  }
  bool isArray() const {
    return getType() == KindOfArray;
  }
  // Is "define('CONSTANT', <this value>)" legal?
  bool isAllowedAsConstantValue() const {
    return (m_type & kNotConstantValueTypeMask) == 0;
  }
  bool isResource() const;

  /**
   * Whether or not there are at least two variables that are strongly bound.
   */
  bool isReferenced() const {
    return m_type == KindOfRef && m_data.pref->isReferenced();
  }

  /**
   * Get reference count of weak or strong binding. For debugging purpose.
   */
  int getRefCount() const;

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
  Variant &assign(const Variant& v);
  Variant &assignVal(const Variant& v) { return assign(v); }
  Variant &assignRef(const Variant& v);

  Variant &operator=(RefResult v) { return assignRef(variant(v)); }
  Variant &operator=(CVarStrongBind v) { return assignRef(variant(v)); }
  Variant &operator=(CVarWithRefBind v) { return setWithRef(variant(v)); }

  Variant &operator=(const StaticString &v) {
    set(v);
    return *this;
  }
  template<typename T> Variant &operator=(const T &v) {
    set(v);
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
  template<typename T> /* implicit */ operator SmartObject<T>() const = delete;

  /**
   * Explicit type conversions
   */
  bool   toBoolean() const {
    if (IS_NULL_TYPE(m_type)) return false;
    if (m_type <= KindOfInt64) return m_data.num;
    return toBooleanHelper();
  }
  char   toByte   () const { return (char)toInt64();}
  short  toInt16  (int base = 10) const { return (short)toInt64(base);}
  int    toInt32  (int base = 10) const { return (int)toInt64(base);}
  int64_t  toInt64  () const {
    if (IS_NULL_TYPE(m_type)) return 0;
    if (m_type <= KindOfInt64) return m_data.num;
    return toInt64Helper(10);
  }
  int64_t  toInt64  (int base) const {
    if (IS_NULL_TYPE(m_type)) return 0;
    if (m_type <= KindOfInt64) return m_data.num;
    return toInt64Helper(base);
  }
  double toDouble () const {
    if (m_type == KindOfDouble) return m_data.dbl;
    return toDoubleHelper();
  }
  String toString () const {
    if (IS_STRING_TYPE(m_type)) {
      return m_data.pstr;
    }
    return toStringHelper();
  }
  Array  toArray  () const {
    if (m_type == KindOfArray) return m_data.parr;
    return toArrayHelper();
  }
  Object toObject () const {
    if (m_type == KindOfObject) return m_data.pobj;
    return toObjectHelper();
  }
  Resource toResource () const {
    if (m_type == KindOfResource) return m_data.pres;
    return toResourceHelper();
  }
  /**
   * Whether or not calling toKey() will throw a bad type exception
   */
  bool  canBeValidKey() const {
    switch (getType()) {
    case KindOfArray:  return false;
    case KindOfObject: return false;
    default:           return true;
    }
  }
  VarNR toKey   () const;
  /* Creating a temporary Array, String, or Object with no ref-counting and
   * no type checking, use it only when we have checked the variant type and
   * we are sure the internal data will have a reference until the temporary
   * one gets out-of-scope.
   */
  StrNR toStrNR () const {
    return StrNR(getStringData());
  }
  ArrNR toArrNR () const {
    return ArrNR(getArrayData());
  }
  ObjNR toObjNR() const {
    return ObjNR(getObjectData());
  }

  /**
   * Output functions
   */
  void serialize(VariableSerializer *serializer,
                 bool isArrayKey = false,
                 bool skipNestCheck = false) const;
  void unserialize(VariableUnserializer *unserializer,
                   Uns::Mode mode = Uns::Mode::Value);

  static Variant &lvalInvalid();
  static Variant &lvalBlackHole();

  /**
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
    assert(IS_STRING_TYPE(getType()));
    return m_type == KindOfRef ? m_data.pref->var()->m_data.pstr : m_data.pstr;
  }
  StringData *getStringDataOrNull() const {
    // This is a necessary evil because getStringData() returns
    // an undefined result if this is a null variant
    assert(isNull() || is(KindOfString) || is(KindOfStaticString));
    return m_type == KindOfRef ?
      (m_data.pref->var()->m_type <= KindOfNull ? nullptr :
        m_data.pref->var()->m_data.pstr) :
      (m_type <= KindOfNull ? nullptr : m_data.pstr);
  }
  ArrayData *getArrayData() const {
    assert(is(KindOfArray));
    return m_type == KindOfRef ? m_data.pref->var()->m_data.parr : m_data.parr;
  }
  ArrayData *getArrayDataOrNull() const {
    // This is a necessary evil because getArrayData() returns
    // an undefined result if this is a null variant
    assert(isNull() || is(KindOfArray));
    return m_type == KindOfRef ?
      (m_data.pref->var()->m_type <= KindOfNull ? nullptr :
        m_data.pref->var()->m_data.parr) :
      (m_type <= KindOfNull ? nullptr : m_data.parr);
  }
  ObjectData *getObjectData() const {
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
  ResourceData* getResourceData() const {
    assert(is(KindOfResource));
    return m_type == KindOfRef ? m_data.pref->var()->m_data.pres : m_data.pres;
  }
  Variant *getRefData() const {
    assert(m_type == KindOfRef);
    return m_data.pref->var();
  }

  ObjectData *getArrayAccess() const;
  int64_t getNumData() const { return m_data.num; }
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
   * Access this Variant as a Ref. Promotes this Variant to a ref
   * if it is not already a ref.
   */
  const Ref* asRef() const { PromoteToRef(*this); return this; }
        Ref* asRef()       { PromoteToRef(*this); return this; }

 private:
  bool isPrimitive() const { return !IS_REFCOUNTED_TYPE(m_type); }
  bool isObjectConvertable() {
    assert(m_type != KindOfRef);
    return m_type <= KindOfNull ||
      (m_type == KindOfBoolean && !m_data.num) ||
      (IS_STRING_TYPE(m_type) && m_data.pstr->empty());
  }

  const Variant& set(bool    v);
  const Variant& set(int     v);
  const Variant& set(int64_t   v);
  const Variant& set(double  v);
  const Variant& set(litstr  v) = delete;
  const Variant& set(const std::string & v) {
    return set(String(v));
  }
  const Variant& set(StringData  *v);
  const Variant& set(ArrayData   *v);
  const Variant& set(ObjectData  *v);
  const Variant& set(ResourceData  *v);
  const Variant& set(const StringData  *v) = delete;
  const Variant& set(const ArrayData   *v) = delete;
  const Variant& set(const ObjectData  *v) = delete;
  const Variant& set(const ResourceData  *v) = delete;

  const Variant& set(const String& v) { return set(v.get()); }
  const Variant& set(const StaticString & v);
  const Variant& set(const Array& v) { return set(v.get()); }
  const Variant& set(const Object& v) { return set(v.get()); }
  const Variant& set(const Resource& v) { return set(v.get()); }

  template<typename T>
  const Variant& set(const SmartObject<T> &v) {
    return set(v.get());
  }

  template<typename T>
  const Variant& set(const SmartResource<T> &v) {
    return set(v.get());
  }

  static ALWAYS_INLINE
  void AssignValHelper(Variant *self, const Variant *other) {
    assert(tvIsPlausible(*self) && tvIsPlausible(*other));

    if (UNLIKELY(self->m_type == KindOfRef)) self = self->m_data.pref->var();
    if (UNLIKELY(other->m_type == KindOfRef)) other = other->m_data.pref->var();
    // An early check for self == other here would be faster in that case, but
    // slows down the frequent case of self != other.
    // The following code is correct even if self == other.
    const DataType stype = self->m_type;
    const Value sdata = self->m_data;
    const DataType otype = other->m_type;
    if (UNLIKELY(otype == KindOfUninit)) {
      self->m_type = KindOfNull;
    } else {
      const Value odata = other->m_data;
      if (IS_REFCOUNTED_TYPE(otype)) {
        odata.pstr->incRefCount();
      }
      self->m_data = odata;
      self->m_type = otype;
    }
    tvRefcountedDecRefHelper(stype, sdata.num);
  }

  static ALWAYS_INLINE void PromoteToRef(const Variant& v) {
    assert(&v != &null_variant);
    if (v.m_type != KindOfRef) {
      auto const ref = RefData::Make(*v.asTypedValue());
      const_cast<Variant&>(v).m_type = KindOfRef;
      const_cast<Variant&>(v).m_data.pref = ref;
    }
  }

  ALWAYS_INLINE void assignValHelper(const Variant& v) {
    AssignValHelper(this, &v);
  }

  ALWAYS_INLINE void assignRefHelper(const Variant& v) {
    assert(tvIsPlausible(*this) && tvIsPlausible(v));

    PromoteToRef(v);
    RefData* r = v.m_data.pref;
    r->incRefCount(); // in case destruct() triggers deletion of v
    auto const d = m_data.num;
    auto const t = m_type;
    m_type = KindOfRef;
    m_data.pref = r;
    tvRefcountedDecRefHelper(t, d);
  }

public:
  ALWAYS_INLINE void constructRefHelper(const Variant& v) {
    assert(tvIsPlausible(v));
    PromoteToRef(v);
    v.m_data.pref->incRefCount();
    m_data.pref = v.m_data.pref;
    m_type = KindOfRef;
  }

  ALWAYS_INLINE void constructValHelper(const Variant& v) {
    assert(tvIsPlausible(v));

    const Variant *other =
      UNLIKELY(v.m_type == KindOfRef) ? v.m_data.pref->var() : &v;
    assert(this != other);
    if (IS_REFCOUNTED_TYPE(other->m_type)) {
      other->m_data.pstr->incRefCount();
    }
    m_type = other->m_type != KindOfUninit ? other->m_type : KindOfNull;
    m_data = other->m_data;
  }

  void moveRefHelper(Variant&& v) {
    assert(tvIsPlausible(v));

    assert(v.m_type == KindOfRef);
    m_type = v.m_data.pref->tv()->m_type; // Can't be KindOfUninit.
    m_data = v.m_data.pref->tv()->m_data;
    if (IS_REFCOUNTED_TYPE(m_type)) {
      m_data.pstr->incRefCount();
    }
    decRefRef(v.m_data.pref);
    v.m_type = KindOfNull;
  }

  ALWAYS_INLINE
  void setWithRefHelper(const Variant& v, bool destroy) {
    assert(tvIsPlausible(*this) && tvIsPlausible(v));

    assert(this != &v);

    const Variant& rhs =
      v.m_type == KindOfRef && !v.m_data.pref->isReferenced()
        ? *v.m_data.pref->var() : v;
    if (IS_REFCOUNTED_TYPE(rhs.m_type)) {
      assert(rhs.m_data.pstr);
      rhs.m_data.pstr->incRefCount();
    }

    auto const d = m_data.num;
    auto const t = m_type;
    m_type = rhs.m_type;
    if (m_type == KindOfUninit) m_type = KindOfNull; // drop uninit
    m_data.num = rhs.m_data.num;
    if (destroy) tvRefcountedDecRefHelper(t, d);
  }

  ALWAYS_INLINE
  void constructWithRefHelper(const Variant& v) {
    setWithRefHelper(v, false);
  }

private:
  bool   toBooleanHelper() const;
  int64_t  toInt64Helper(int base = 10) const;
  double toDoubleHelper() const;
  String toStringHelper() const;
  Array  toArrayHelper() const;
  Object toObjectHelper() const;
  Resource toResourceHelper() const;

  DataType convertToNumeric(int64_t *lval, double *dval) const;
};

Variant operator+(const Variant & lhs, const Variant & rhs) = delete;

class RefResultValue {
public:
  const Variant& get() const { return m_var; }
private:
  Variant m_var;
};

class VRefParamValue {
public:
  template <class T> /* implicit */ VRefParamValue(const T &v) : m_var(v) {}

  /* implicit */ VRefParamValue() : m_var(Variant::NullInit()) {}
  /* implicit */ VRefParamValue(RefResult v) : m_var(strongBind(v)) {}
  template <typename T>
  Variant &operator=(const T &v) const {
    m_var = v;
    return m_var;
  }
  VRefParamValue &operator=(const VRefParamValue &v) const {
    m_var = v.m_var;
    return *const_cast<VRefParamValue*>(this);
  }
  operator Variant&() const { return m_var; }
  Variant *operator&() const { return &m_var; } // FIXME
  Variant *operator->() const { return &m_var; }

  Variant& wrapped() const { return m_var; }

  explicit operator bool   () const { return m_var.toBoolean();}
  operator int    () const { return m_var.toInt32();}
  operator int64_t  () const { return m_var.toInt64();}
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
  ArrNR toArrNR() const { return m_var.toArrNR(); }

private:
  mutable Variant m_var;
};

inline VRefParamValue vref(const Variant& v) {
  return VRefParamValue(strongBind(v));
}

inline VRefParam directRef(const Variant& v) {
  return *(VRefParamValue*)&v;
}

/*
  these two classes are just to help choose the correct
  overload.
*/
class VariantStrongBind { private: Variant m_var; };
class VariantWithRefBind { private: Variant m_var; };

///////////////////////////////////////////////////////////////////////////////
// VarNR

class VarNR : private TypedValueAux {
public:
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
    init(KindOfStaticString);
    StringData *s = v.get();
    assert(s);
    m_data.pstr = s;
  }

  explicit VarNR(const String& v);
  explicit VarNR(const Array& v);
  explicit VarNR(const Object& v);
  explicit VarNR(StringData *v);
  explicit VarNR(const StringData *v) {
    assert(v && v->isStatic());
    init(KindOfStaticString);
    m_data.pstr = const_cast<StringData*>(v);
  }
  explicit VarNR(ArrayData *v);
  explicit VarNR(ObjectData *v);

  VarNR(const VarNR &v) : TypedValueAux(v) {}

  explicit VarNR() { asVariant()->asTypedValue()->m_type = KindOfUninit; }

  ~VarNR() { if (debug) checkRefCount(); }

  operator const Variant&() const { return *asVariant(); }

  bool isNull() const {
    return asVariant()->isNull();
  }
private:
  /* implicit */ VarNR(litstr  v) = delete;
  /* implicit */ VarNR(const std::string & v) = delete;

  void init(DataType dt) {
    m_type = dt;
    if (debug) varNrFlag() = NR_FLAG;
  }
  const Variant *asVariant() const {
    return (const Variant*)this;
  }
  Variant *asVariant() {
    return (Variant*)this;
  }
  void checkRefCount() {
    assert(m_type != KindOfRef);
    if (!IS_REFCOUNTED_TYPE(m_type)) return;
    assert(varNrFlag() == NR_FLAG);
    switch (m_type) {
    case KindOfArray:
      assert_refcount_realistic(m_data.parr->getCount());
      return;
    case KindOfString:
      assert_refcount_realistic(m_data.pstr->getCount());
      return;
    case KindOfObject:
      assert_refcount_realistic(m_data.pobj->getCount());
      return;
    case KindOfResource:
      assert_refcount_realistic(m_data.pres->getCount());
      return;
    default:
      break;
    }
    assert(false);
  }
};

///////////////////////////////////////////////////////////////////////////////
// breaking circular dependencies

inline const Variant Array::operator[](int     key) const {
  return rvalAt(key);
}

inline const Variant Array::operator[](int64_t   key) const {
  return rvalAt(key);
}

inline const Variant Array::operator[](const String& key) const {
  return rvalAt(key);
}

inline const Variant Array::operator[](const Variant& key) const {
  return rvalAt(key);
}

inline void Array::setWithRef(const Variant& k, const Variant& v,
                              bool isKey /* = false */) {
  lvalAt(k, isKey ? AccessFlags::Key : AccessFlags::None).setWithRef(v);
}

inline Variant uninit_null() {
  return Variant();
}

inline Variant init_null() {
  return Variant(Variant::NullInit());
}

// TODO(#2298051) litstr must die
inline Variant &concat_assign(Variant &v1, litstr s2) = delete;

inline Variant &concat_assign(Variant &v1, const String& s2) {
  if (v1.getType() == KindOfString) {
    auto& str = v1.asStrRef();
    if (str.get()->hasExactlyOneRef()) {
      str += StringSlice{s2.data(), static_cast<uint32_t>(s2.size())};
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
  assert(m_magic == Magic::kMagic);
  tvAsVariant(&m_tv).~Variant();
}

//////////////////////////////////////////////////////////////////////

inline Array& forceToArray(Variant& var) {
  if (!var.isArray()) var = Variant(Array::Create());
  return var.toArrRef();
}

//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_VARIANT_H_
