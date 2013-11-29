/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_INSIDE_HPHP_COMPLEX_TYPES_H_
#error Directly including 'type_variant.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#include <type_traits>

#include "hphp/util/trace.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/hphp-value.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/macros.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayIter;
class MutableArrayIter;

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

class Variant : private TypedValue {
 public:
  friend class Array;
  friend class VariantVectorBase;
  friend class c_Vector;
  friend class c_Map;
  friend class c_StableMap;

  /**
   * setUninitNull occurs frequently; use this version where possible.
   */
  inline void setUninitNull() {
    m_type = KindOfUninit;
    assert(!isInitialized());
  }

  Variant() {
    setUninitNull();
  }

  enum class NullInit {};
  explicit Variant(NullInit) { m_type = KindOfNull; }
  enum class NoInit {};
  explicit Variant(NoInit) {}
  enum NoInc { noInc = 0 };

  static ALWAYS_INLINE void destructData(RefData* num, DataType t) {
    tvDecRefHelper(t, uint64_t(num));
  }

  // D462768 showed no gain from inlining, even just with INLINE_VARIANT_HELPER.
  ~Variant();

  void reset() {
    // only for special memory sweeping!
    m_type = KindOfNull;
  }

  /**
   * Constructors. We can't really use template<T> here, since that will make
   * Variant being able to take many other external types, messing up those
   * operator overloads.
   */
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
  /* implicit */ Variant(CArrRef v);
  /* implicit */ Variant(CObjRef v);
  /* implicit */ Variant(CResRef v);
  /* implicit */ Variant(StringData *v);
  /* implicit */ Variant(ArrayData *v);
  /* implicit */ Variant(ObjectData *v);
  /* implicit */ Variant(ResourceData *v);
  /* implicit */ Variant(RefData *r);
  /* implicit */ Variant(RefData *r, NoInc);

  // for static strings only
  explicit Variant(const StringData *v);

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

  /*
   * Creation constructor from ArrayInit that avoids a null check.
   */
  enum class ArrayInitCtor { Tag };
  explicit Variant(ArrayData* ad, ArrayInitCtor) {
    m_type = KindOfArray;
    m_data.parr = ad;
    ad->incRefCount();
  }

#ifdef INLINE_VARIANT_HELPER
  ALWAYS_INLINE
  /* implicit */ Variant(CVarRef v) { constructValHelper(v); }
  ALWAYS_INLINE
  /* implicit */ Variant(CVarStrongBind v) { constructRefHelper(variant(v)); }
  ALWAYS_INLINE
  /* implicit */ Variant(CVarWithRefBind v) {
    constructWithRefHelper(variant(v));
  }
#else
  /* implicit */ Variant(CVarRef v);
  /* implicit */ Variant(CVarStrongBind v);
  /* implicit */ Variant(CVarWithRefBind v);
#endif

  /*
   * Move ctor
   *
   * Note: not semantically a move constructor.  Like our "copy
   * constructor", unboxes refs and turns uninits to null.
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
    v.reset();
  }

  /*
   * Move assign
   *
   * Note: not semantically a move assignment operator.  Like our
   * "copy asignment operator", unboxes refs and turns uninits to
   * null.
   */
  Variant& operator=(Variant &&rhs) {
    // a = std::move(a), ILLEGAL per C++11 17.6.4.9
    assert(this != &rhs);
    if (rhs.m_type == KindOfRef) return *this = *rhs.m_data.pref->var();

    Variant& lhs = m_type == KindOfRef ? *m_data.pref->var() : *this;

    Variant goner((NoInit()));
    goner.m_data = lhs.m_data;
    goner.m_type = lhs.m_type;

    lhs.m_data = rhs.m_data;
    lhs.m_type = rhs.m_type == KindOfUninit ? KindOfNull : rhs.m_type;

    rhs.reset();
    return *this;
  }

 private:
  friend class VarNR;

 public:
  /**
   * Break bindings and set to null.
   */
  void unset() {
    RefData* d = m_data.pref;
    DataType t = m_type;
    m_type = KindOfUninit;
    if (IS_REFCOUNTED_TYPE(t)) destructData(d, t);
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
  Variant &setWithRef(CVarRef v);

  /**
   * Fast accessors that can be used by generated code when type inference can
   * prove that m_type will have a certain value at a given point in time
   */

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

  ObjectData *objectForCall() const {
    if (m_type == KindOfObject) return m_data.pobj;
    if (m_type == KindOfRef) {
      Variant *t = m_data.pref->var();
      if (t->m_type == KindOfObject) return t->m_data.pobj;
    }
    throw_call_non_object();
    return nullptr;
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
  bool instanceof(const String& s) const;
  bool instanceof(Class* cls) const;

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
  Variant &assign(CVarRef v);
  Variant &assignVal(CVarRef v) { return assign(v); }
  Variant &assignRef(CVarRef v);

  Variant &operator=(CVarRef v) {
    return assign(v);
  }
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
  Variant &operator += (CVarRef v) = delete;
  Variant &operator += (int     n) = delete;
  Variant &operator += (int64_t   n) = delete;
  Variant &operator += (double  n)  = delete;

  Variant  operator -  () const = delete;
  Variant  operator -  (CVarRef v) const = delete;
  Variant &operator -= (CVarRef v) = delete;
  Variant &operator -= (int     n) = delete;
  Variant &operator -= (int64_t   n) = delete;
  Variant &operator -= (double  n) = delete;

  Variant  operator *  (CVarRef v) const = delete;
  Variant &operator *= (CVarRef v) = delete;
  Variant &operator *= (int     n) = delete;
  Variant &operator *= (int64_t   n) = delete;
  Variant &operator *= (double  n) = delete;

  Variant  operator /  (CVarRef v) const = delete;
  Variant &operator /= (CVarRef v) = delete;
  Variant &operator /= (int     n) = delete;
  Variant &operator /= (int64_t   n) = delete;
  Variant &operator /= (double  n) = delete;

  int64_t    operator %  (CVarRef v) const = delete;
  Variant &operator %= (CVarRef v) = delete;
  Variant &operator %= (int     n) = delete;
  Variant &operator %= (int64_t   n) = delete;
  Variant &operator %= (double  n) = delete;

  Variant  operator |  (CVarRef v) const = delete;
  Variant &operator |= (CVarRef v) = delete;
  Variant  operator &  (CVarRef v) const = delete;
  Variant &operator &= (CVarRef v) = delete;
  Variant  operator ^  (CVarRef v) const = delete;
  Variant &operator ^= (CVarRef v) = delete;
  Variant &operator <<=(int64_t n) = delete;
  Variant &operator >>=(int64_t n) = delete;

  Variant &operator ++ () = delete;
  Variant  operator ++ (int) = delete;
  Variant &operator -- () = delete;
  Variant  operator -- (int) = delete;

  /**
   * Iterator functions. See array-iterator.h for end() and next().
   */
  ArrayIter begin(const String& context = null_string) const;
  // used by generated code
  MutableArrayIter begin(Variant *key, Variant &val,
                         const String& context = null_string);

  // Called before iteration to give array a chance to escalate.
  void escalate();

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

  /**
   * Get the wrapped APCHandle, if any.
   */
  APCHandle *getAPCHandle() const;

  /*
   * Print information about a variant to stdout.  For debugging
   * purposes.
   */
  void dump() const;

  /**
   * Offset functions
   */
  Variant rvalAtHelper(int64_t offset, ACCESSPARAMS_DECL) const;
  Variant rvalAt(int offset, ACCESSPARAMS_DECL) const {
    return rvalAt((int64_t)offset, flags);
  }
  Variant rvalAt(int64_t offset, ACCESSPARAMS_DECL) const {
    if (m_type == KindOfArray) {
      return m_data.parr->get(offset, flags & AccessFlags::Error);
    }
    return rvalAtHelper(offset, flags);
  }
  Variant rvalAt(double offset, ACCESSPARAMS_DECL) const = delete;
  Variant rvalAt(litstr offset, ACCESSPARAMS_DECL) const = delete;
  Variant rvalAt(const String& offset, ACCESSPARAMS_DECL) const;
  Variant rvalAt(CVarRef offset, ACCESSPARAMS_DECL) const;

  template <typename T>
  CVarRef rvalRefHelper(T offset, CVarRef tmp, ACCESSPARAMS_DECL) const;
  CVarRef rvalRef(int offset, CVarRef tmp, ACCESSPARAMS_DECL) const {
    return rvalRef((int64_t)offset, tmp, flags);
  }

  CVarRef rvalRef(int64_t offset, CVarRef tmp, ACCESSPARAMS_DECL) const {
    if (m_type == KindOfArray) {
      return m_data.parr->get(offset, flags & AccessFlags::Error);
    }
    return rvalRefHelper(offset, tmp, flags);
  }
  CVarRef rvalRef(double offset, CVarRef tmp, ACCESSPARAMS_DECL) const = delete;
  CVarRef rvalRef(litstr offset, CVarRef tmp, ACCESSPARAMS_DECL) const = delete;
  CVarRef rvalRef(const String& offset, CVarRef tmp, ACCESSPARAMS_DECL) const;
  CVarRef rvalRef(CVarRef offset, CVarRef tmp, ACCESSPARAMS_DECL) const;

  // for when we know its an array or null
  template <typename T>
  CVarRef rvalAtRefHelper(T offset, ACCESSPARAMS_DECL) const;
  CVarRef rvalAtRef(int offset, ACCESSPARAMS_DECL) const {
    return rvalAtRefHelper((int64_t)offset, flags);
  }
  CVarRef rvalAtRef(double offset, ACCESSPARAMS_DECL) const = delete;
  CVarRef rvalAtRef(int64_t offset, ACCESSPARAMS_DECL) const {
    return rvalAtRefHelper(offset, flags);
  }
  CVarRef rvalAtRef(const String& offset, ACCESSPARAMS_DECL) const {
    return rvalAtRefHelper<const String&>(offset, flags);
  }
  CVarRef rvalAtRef(CVarRef offset, ACCESSPARAMS_DECL) const {
    return rvalAtRefHelper<CVarRef>(offset, flags);
  }
  const Variant operator[](int     key) const { return rvalAt(key);}
  const Variant operator[](int64_t   key) const { return rvalAt(key);}
  const Variant operator[](double  key) const = delete;
  const Variant operator[](const String& key) const { return rvalAt(key);}
  const Variant operator[](CArrRef key) const { return rvalAt(key);}
  const Variant operator[](CObjRef key) const { return rvalAt(key);}
  const Variant operator[](CVarRef key) const { return rvalAt(key);}
  const Variant operator[](const char*) const = delete;

  template<typename T>
  Variant &lval(const T &key) {
    if (m_type == KindOfRef) {
      return m_data.pref->var()->lval(key);
    }

    assert(m_type == KindOfArray);
    Variant *ret = nullptr;
    ArrayData *arr = m_data.parr;
    ArrayData *escalated = arr->lval(key, ret, arr->hasMultipleRefs());
    if (escalated != arr) set(escalated);
    assert(ret);
    return *ret;
  }

  Variant &lvalAt();

  static Variant &lvalInvalid();
  static Variant &lvalBlackHole();

  Variant &lvalAt(int     key, ACCESSPARAMS_DECL);
  Variant &lvalAt(int64_t   key, ACCESSPARAMS_DECL);
  Variant &lvalAt(double  key, ACCESSPARAMS_DECL) = delete;
  Variant &lvalAt(litstr  key, ACCESSPARAMS_DECL) = delete;
  Variant &lvalAt(const String& key, ACCESSPARAMS_DECL);
  Variant &lvalAt(CVarRef key, ACCESSPARAMS_DECL);

  Variant &lvalRef(int     key, Variant& tmp, ACCESSPARAMS_DECL);
  Variant &lvalRef(int64_t   key, Variant& tmp, ACCESSPARAMS_DECL);
  Variant &lvalRef(double  key, Variant& tmp, ACCESSPARAMS_DECL) = delete;
  Variant &lvalRef(litstr  key, Variant& tmp, ACCESSPARAMS_DECL) = delete;
  Variant &lvalRef(const String& key, Variant& tmp, ACCESSPARAMS_DECL);
  Variant &lvalRef(CVarRef key, Variant& tmp, ACCESSPARAMS_DECL);

  template <typename T>
  ALWAYS_INLINE static CVarRef SetImpl(Variant *self, T key, CVarRef v,
                                       bool isKey);

  template <typename T>
  ALWAYS_INLINE static CVarRef SetRefImpl(Variant *self, T key, CVarRef v,
                                          bool isKey);

  CVarRef set(int     key, CVarRef v) { return set((int64_t)key, v); }
  CVarRef set(int64_t   key, CVarRef v);
  CVarRef set(double  key, CVarRef v) = delete;
  CVarRef set(litstr  key, CVarRef v, bool isString = false) = delete;
  CVarRef set(const String& key, CVarRef v, bool isString = false);
  CVarRef set(CVarRef key, CVarRef v);

  CVarRef append(CVarRef v);

  CVarRef setRef(int     key, CVarRef v) { return setRef((int64_t)key, v); }
  CVarRef setRef(int64_t   key, CVarRef v);
  CVarRef setRef(double  key, CVarRef v) = delete;
  CVarRef setRef(litstr  key, CVarRef v, bool isString = false) = delete;
  CVarRef setRef(const String& key, CVarRef v, bool isString = false);
  CVarRef setRef(CVarRef key, CVarRef v);

  CVarRef set(int     key, RefResult v) { return setRef(key, variant(v)); }
  CVarRef set(int64_t   key, RefResult v) { return setRef(key, variant(v)); }
  CVarRef set(double  key, RefResult v) = delete;
  CVarRef set(litstr  key, RefResult v, bool isString = false) = delete;
  CVarRef set(const String& key, RefResult v, bool isString = false) {
    return setRef(key, variant(v), isString);
  }
  CVarRef set(CVarRef key, RefResult v) { return setRef(key, variant(v)); }

  CVarRef appendRef(CVarRef v);
  CVarRef append(RefResult v) { return appendRef(variant(v)); }

  void remove(int     key) { removeImpl((int64_t)key);}
  void remove(int64_t   key) { removeImpl(key);}
  void remove(double  key) = delete;
  void remove(litstr  key, bool isString = false) = delete;
  void remove(const String& key, bool isString = false) {
    removeImpl(key, isString);
  }
  void remove(CVarRef key);

  /**
   * More array operations.
   */
  Variant pop();
  Variant dequeue();
  void prepend(CVarRef v);

  /**
   * For C++ library users to write "var.cast<c_MyClass>()->mf_func()".
   */
  template<typename T>
  T *cast() const {
    return toObject().getTyped<T>();
  }

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
  void callOffsetUnset(CVarRef key);
  int64_t getNumData() const { return m_data.num; }
  void setEvalScalar();

  void setToDefaultObject();

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

  void removeImpl(double key) = delete;
  void removeImpl(int64_t key);
  void removeImpl(CVarRef key, bool isString = false);
  void removeImpl(const String& key, bool isString = false);

  CVarRef set(bool    v);
  CVarRef set(int     v);
  CVarRef set(int64_t   v);
  CVarRef set(double  v);
  CVarRef set(litstr  v) = delete;
  CVarRef set(const std::string & v) {
    return set(String(v));
  }
  CVarRef set(StringData  *v);
  CVarRef set(ArrayData   *v);
  CVarRef set(ObjectData  *v);
  CVarRef set(ResourceData  *v);
  CVarRef set(const StringData  *v) = delete;
  CVarRef set(const ArrayData   *v) = delete;
  CVarRef set(const ObjectData  *v) = delete;
  CVarRef set(const ResourceData  *v) = delete;

  CVarRef set(const String& v) { return set(v.get()); }
  CVarRef set(const StaticString & v);
  CVarRef set(CArrRef v) { return set(v.get()); }
  CVarRef set(CObjRef v) { return set(v.get()); }
  CVarRef set(CResRef v) { return set(v.get()); }

  template<typename T>
  CVarRef set(const SmartObject<T> &v) {
    return set(v.get());
  }

  template<typename T>
  CVarRef set(const SmartResource<T> &v) {
    return set(v.get());
  }

  // only called from constructor
  void init(ObjectData *v);

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

#ifdef INLINE_VARIANT_HELPER
public:
#endif
  static ALWAYS_INLINE void PromoteToRef(CVarRef v) {
    assert(&v != &null_variant);
    if (v.m_type != KindOfRef) {
      auto const ref = RefData::Make(*v.asTypedValue());
      const_cast<Variant&>(v).m_type = KindOfRef;
      const_cast<Variant&>(v).m_data.pref = ref;
    }
  }

  ALWAYS_INLINE void assignValHelper(CVarRef v) {
    AssignValHelper(this, &v);
  }

  ALWAYS_INLINE void assignRefHelper(CVarRef v) {
    assert(tvIsPlausible(*this) && tvIsPlausible(v));

    PromoteToRef(v);
    RefData* r = v.m_data.pref;
    r->incRefCount(); // in case destruct() triggers deletion of v
    RefData* d = m_data.pref;
    DataType t = m_type;
    m_type = KindOfRef;
    m_data.pref = r;
    if (IS_REFCOUNTED_TYPE(t)) destructData(d, t);
  }

public:
  ALWAYS_INLINE void constructRefHelper(CVarRef v) {
    assert(tvIsPlausible(v));
    PromoteToRef(v);
    v.m_data.pref->incRefCount();
    m_data.pref = v.m_data.pref;
    m_type = KindOfRef;
  }

  ALWAYS_INLINE void constructValHelper(CVarRef v) {
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
    v.reset();
  }

  ALWAYS_INLINE
  void setWithRefHelper(CVarRef v, bool destroy) {
    assert(tvIsPlausible(*this) && tvIsPlausible(v));

    assert(this != &v);

    CVarRef rhs = v.m_type == KindOfRef && !v.m_data.pref->isReferenced()
      ? *v.m_data.pref->var() : v;
    if (IS_REFCOUNTED_TYPE(rhs.m_type)) {
      assert(rhs.m_data.pstr);
      rhs.m_data.pstr->incRefCount();
    }

    RefData* d = m_data.pref;
    DataType t = m_type;
    m_type = rhs.m_type;
    if (m_type == KindOfUninit) m_type = KindOfNull; // drop uninit
    m_data.num = rhs.m_data.num;
    if (destroy) destructData(d, t);
  }

  ALWAYS_INLINE
  void constructWithRefHelper(CVarRef v) {
    setWithRefHelper(v, false);
  }

#ifdef INLINE_VARIANT_HELPER
private:
#endif

  template<typename T>
  static ALWAYS_INLINE Variant &LvalAtImpl0(Variant *self, T key, Variant *tmp,
                                            bool blackHole, ACCESSPARAMS_DECL);

  template<typename T>
  ALWAYS_INLINE Variant &lvalAtImpl(T key, ACCESSPARAMS_DECL);

 private:
  /**
   * Checks whether the LHS array needs to be copied for a *one-level*
   * array set, e.g., "$a[] = $v" or "$a['x'] = $v".
   *
   * Note:
   *  (1) The semantics is equivalent to having a temporary variable
   * holding to RHS value, i.e., "$tmp = $v; $a[] = $tmp". This is NOT
   * exactly the same as PHP 5.3, where "$a = &$b; $a = array(); $a = $b;"
   * creates a recursive array, although the final assignment is not
   * strong-binding.
   *  (2) It does NOT work with multi-level array set, i.e., "$a[][] = $v".
   * The compiler needs to generate a real temporary.
   */
  bool needCopyForSet(CVarRef v) {
    assert(m_type == KindOfArray);
    if (m_data.parr->hasMultipleRefs()) return true;
    if (v.m_type == KindOfArray) return m_data.parr == v.m_data.parr;
    if (v.m_type == KindOfRef) {
      return m_data.parr == v.m_data.pref->var()->m_data.parr;
    }
    return false;
  }

  bool needCopyForSetRef(CVarRef v) {
    assert(m_type == KindOfArray);
    return m_data.parr->hasMultipleRefs();
  }

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
  CVarRef get() const { return m_var; }
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

  CVarRef append(CVarRef v) const { return m_var.append(v); }

  Variant pop() const { return m_var.pop(); }
  Variant dequeue() const { return m_var.dequeue(); }
  void prepend(CVarRef v) const { m_var.prepend(v); }

  bool isArray() const { return m_var.isArray(); }
  ArrNR toArrNR() const { return m_var.toArrNR(); }

private:
  mutable Variant m_var;
};

inline VRefParamValue vref(CVarRef v) {
  return VRefParamValue(strongBind(v));
}

inline VRefParam directRef(CVarRef v) {
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
  explicit VarNR(CArrRef v);
  explicit VarNR(CObjRef v);
  explicit VarNR(StringData *v);
  explicit VarNR(const StringData *v) {
    assert(v && v->isStatic());
    init(KindOfStaticString);
    m_data.pstr = const_cast<StringData*>(v);
  }
  explicit VarNR(ArrayData *v);
  explicit VarNR(ObjectData *v);

  VarNR(const VarNR &v) : TypedValueAux(v) {}

  explicit VarNR() { asVariant()->setUninitNull(); }

  ~VarNR() { if (debug) checkRefCount(); }

  operator CVarRef() const { return *asVariant(); }

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

inline const Variant Array::operator[](CVarRef key) const {
  return rvalAt(key);
}

inline void Array::setWithRef(CVarRef k, CVarRef v,
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
    if (str->getCount() == 1) {
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

}

#endif // incl_HPHP_VARIANT_H_
