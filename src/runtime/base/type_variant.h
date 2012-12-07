/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __INSIDE_HPHP_COMPLEX_TYPES_H__
#error Directly including 'type_variant.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#ifndef __HPHP_VARIANT_H__
#define __HPHP_VARIANT_H__

#include <util/trace.h>
#include <runtime/base/types.h>
#include <runtime/base/hphp_value.h>
#include <runtime/base/type_string.h>
#include <runtime/base/type_object.h>
#include <runtime/base/type_array.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/macros.h>
#include <runtime/base/gc_roots.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayIter;
class MutableArrayIter;

/**
 * Perhaps the most important class in the entire runtime. When type inference
 * fails to know type of a variable, or when certain coding requires reference
 * or other dynamic-ness, we have to use Variant as a fallback of a specific
 * type. This normally means slower coding. Conceptually, Variant == zval,
 * in terms of tasks it has to perform. Therefore, this class is taking similar
 * switch(type) approach Zend takes, and this class is pretty much the entire
 * Zend re-implementation in a C++ way, whereas other classes in this library
 * represent type-specialized implementation of the language.
 *
 * Variant is also the only way to implement references. A reference is a
 * strong binding between two variables, meaning they both point to the same
 * underlying data.
 *
 * In this class, strong binding is done through "pref" member variable. All
 * others are for weak bindings. Primitive types can just make copies, but not
 * strings and arrays, which take a copy-on-write approach. This is done by
 * doing reference counting on pstr and parr members.
 *
 * In summary, we have really different approaches handling different types:
 *
 *           binding  copy-by-value copy-on-write  ref-counting
 *   num     weak      x (data)
 *   dbl     weak      x (data)
 *   str     weak      x (pointer)
 *   pstr    weak      x (pointer)        x             x
 *   parr    weak      x (pointer)        x             x
 *   pobj    weak      x (pointer)                      x
 *   pref    strong    x (pointer)                      x
 */

#define null ((Variant()))

#ifdef HHVM_GC
typedef GCRootTracker<Variant> VariantBase;
#else
struct VariantBase {};
#endif

class Variant : VariantBase {
 public:
  friend class Array;
  friend class VariantVectorBase;
  friend class c_Vector;
  friend class c_Map;
  friend class c_StableMap;

  /**
   * Variant does not formally derive from Countable, however it has a
   * _count field and implements all of the methods from Countable.
   */
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC

  /**
   * setUninitNull occurs frequently; use this version where possible.
   */
  inline void setUninitNull() {
    CT_ASSERT(offsetof(Variant, _count) == 8     &&
              offsetof(Variant, m_type) == 12    &&
              sizeof(_count) == sizeof(uint32_t) &&
              sizeof(m_type) == sizeof(uint32_t));
    /**
     * One qword store is faster than two dword stores, and gcc can't figure it
     * out. Note that there are no endianness assumptions: while m_typeAndCount
     * is a "split" integer field, we're writing all 0's to it so byte order
     * doesn't matter.
     *
     * The dance with the union is needed to explain to g++ 4.4 that the
     * store through m_countAndTypeUnion aliases _count and m_type.
     */
    m_countAndTypeUnion = 0;
    ASSERT(!isInitialized());
  }

  Variant() {
    setUninitNull();
  }

  enum NullInit { nullInit };
  Variant(NullInit) { _count = 0; m_type = KindOfNull; }
  enum NoInit { noInit };
  Variant(NoInit) {}

  void destruct();
  static void destructData(RefData* num, DataType t);

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
  Variant(bool    v) : _count(0), m_type(KindOfBoolean) { m_data.num = (v?1:0);}
  Variant(int     v) : _count(0), m_type(KindOfInt64  ) { m_data.num = v;}
  Variant(int64   v) : _count(0), m_type(KindOfInt64  ) { m_data.num = v;}
  Variant(uint64  v) : _count(0), m_type(KindOfInt64  ) { m_data.num = v;}
  Variant(long    v) : _count(0), m_type(KindOfInt64  ) { m_data.num = v;}
  Variant(double  v) : _count(0), m_type(KindOfDouble ) { m_data.dbl = v;}

  Variant(litstr  v);
  Variant(const std::string &v);
  Variant(const StaticString &v) : _count(0), m_type(KindOfStaticString) {
    StringData *s = v.get();
    ASSERT(s);
    m_data.pstr = s;
  }
  Variant(const StaticArray &v) : _count(0), m_type(KindOfArray) {
    ArrayData *a = v.get();
    ASSERT(a);
    m_data.parr = a;
  }

  Variant(CStrRef v);
  Variant(CArrRef v);
  Variant(CObjRef v);
  Variant(StringData *v);
  Variant(ArrayData *v);
  Variant(ObjectData *v);
  Variant(RefData *r);

  // These are prohibited, but defined just to prevent accidentally
  // calling the bool constructor just because we had a pointer to
  // const.
private:
  Variant(const StringData *v); // no definition
  Variant(const ArrayData *v);  // no definition
  Variant(const ObjectData *v); // no definition
  Variant(const RefData *v);    // no definition
  Variant(const Variant *v);    // no definition
  Variant(Variant *v);          // no definition
public:

#ifdef INLINE_VARIANT_HELPER
  inline ALWAYS_INLINE Variant(CVarRef v) { constructValHelper(v); }
  inline ALWAYS_INLINE
  Variant(CVarStrongBind v) { constructRefHelper(variant(v)); }
  inline ALWAYS_INLINE
  Variant(CVarWithRefBind v) {
    constructWithRefHelper(variant(v), 0);
  }
#else
  Variant(CVarRef v);
  Variant(CVarStrongBind v);
  Variant(CVarWithRefBind v);
#endif

 private:
  inline ALWAYS_INLINE void destructImpl();
  inline ALWAYS_INLINE static void destructDataImpl(RefData* d, DataType t);
  friend class VarNR;
  // This helper is only used to construct VarNR
  static const int NR_FLAG = 1 << 29;
  void initForVarNR(DataType dt) { _count = NR_FLAG; m_type = dt; }

 public:
  bool isVarNR() const { return _count == NR_FLAG; }
  void setVarNR() const { _count = NR_FLAG; }

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
  Variant &setWithRef(CVarRef v, const ArrayData *arr = NULL);

  /**
   * Fast accessors that can be used by generated code when type inference can
   * prove that m_type will have a certain value at a given point in time
   */

///////////////////////////////////////////////////////////////////////////////
// int64

  inline ALWAYS_INLINE int64 asInt64Val() const {
    ASSERT(m_type == KindOfInt64);
    return m_data.num;
  }

  inline ALWAYS_INLINE int64 toInt64Val() const {
    ASSERT(is(KindOfInt64));
    return
        LIKELY(m_type == KindOfInt64) ?
        m_data.num : m_data.pref->var()->m_data.num;
  }

///////////////////////////////////////////////////////////////////////////////
// double

  inline ALWAYS_INLINE double asDoubleVal() const {
    ASSERT(m_type == KindOfDouble);
    return m_data.dbl;
  }

  inline ALWAYS_INLINE double toDoubleVal() const {
    ASSERT(is(KindOfDouble));
    return
        LIKELY(m_type == KindOfDouble) ?
        m_data.dbl : m_data.pref->var()->m_data.dbl;
  }

///////////////////////////////////////////////////////////////////////////////
// boolean

  inline ALWAYS_INLINE bool asBooleanVal() const {
    ASSERT(m_type == KindOfBoolean);
    return m_data.num;
  }

  inline ALWAYS_INLINE bool toBooleanVal() const {
    ASSERT(is(KindOfBoolean));
    return
        LIKELY(m_type == KindOfBoolean) ?
        m_data.num : m_data.pref->var()->m_data.num;
  }

///////////////////////////////////////////////////////////////////////////////
// string

  inline ALWAYS_INLINE const String & asCStrRef() const {
    ASSERT(m_type == KindOfString || m_type == KindOfStaticString);
    ASSERT(m_data.pstr);
    return *(const String*)(this);
  }

  inline ALWAYS_INLINE const String & toCStrRef() const {
    ASSERT(is(KindOfString) || is(KindOfStaticString));
    ASSERT(m_type == KindOfRef ? m_data.pref->var()->m_data.pstr : m_data.pstr);
    return *(const String*)(
        LIKELY(m_type == KindOfString || m_type == KindOfStaticString) ?
        this : this->m_data.pref->var());
  }

  inline ALWAYS_INLINE String & asStrRef() {
    ASSERT(m_type == KindOfString || m_type == KindOfStaticString);
    ASSERT(m_data.pstr);
    return *(String*)(this);
  }

  inline ALWAYS_INLINE String & toStrRef() {
    ASSERT(is(KindOfString) || is(KindOfStaticString));
    ASSERT(m_type == KindOfRef ? m_data.pref->var()->m_data.pstr : m_data.pstr);
    return *(String*)(
        LIKELY(m_type == KindOfString || m_type == KindOfStaticString) ?
        this : this->m_data.pref->var());
  }

///////////////////////////////////////////////////////////////////////////////
// array

  inline ALWAYS_INLINE const Array & asCArrRef() const {
    ASSERT(m_type == KindOfArray);
    ASSERT(m_data.parr);
    return *(const Array*)(this);
  }

  inline ALWAYS_INLINE const Array & toCArrRef() const {
    ASSERT(is(KindOfArray));
    ASSERT(m_type == KindOfRef ? m_data.pref->var()->m_data.parr : m_data.parr);
    return *(const Array*)(
        LIKELY(m_type == KindOfArray) ?
        this : this->m_data.pref->var());
  }

  inline ALWAYS_INLINE Array & asArrRef() {
    ASSERT(m_type == KindOfArray);
    ASSERT(m_data.parr);
    return *(Array*)(this);
  }

  inline ALWAYS_INLINE Array & toArrRef() {
    ASSERT(is(KindOfArray));
    ASSERT(m_type == KindOfRef ? m_data.pref->var()->m_data.parr : m_data.parr);
    return *(Array*)(
        LIKELY(m_type == KindOfArray) ?
        this : this->m_data.pref->var());
  }

///////////////////////////////////////////////////////////////////////////////
// object

  inline ALWAYS_INLINE const Object & asCObjRef() const {
    ASSERT(m_type == KindOfObject);
    ASSERT(m_data.pobj);
    return *(const Object*)(this);
  }

  inline ALWAYS_INLINE const Object & toCObjRef() const {
    ASSERT(is(KindOfObject));
    ASSERT(m_type == KindOfRef ? m_data.pref->var()->m_data.pobj : m_data.pobj);
    return *(const Object*)(
        LIKELY(m_type == KindOfObject) ?
        this : this->m_data.pref->var());
  }

  inline ALWAYS_INLINE Object & asObjRef() {
    ASSERT(m_type == KindOfObject);
    ASSERT(m_data.pobj);
    return *(Object*)(this);
  }

  inline ALWAYS_INLINE Object & toObjRef() {
    ASSERT(is(KindOfObject));
    ASSERT(m_type == KindOfRef ? m_data.pref->var()->m_data.pobj : m_data.pobj);
    return *(Object*)(
        LIKELY(m_type == KindOfObject) ?
        this : this->m_data.pref->var());
  }

  ObjectData *objectForCall() const {
    if (m_type == KindOfObject) return m_data.pobj;
    if (m_type == KindOfRef) {
      Variant *t = m_data.pref->var();
      if (t->m_type == KindOfObject) return t->m_data.pobj;
    }
    throw_call_non_object();
    return 0;
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
    DataType type = getType();
    return type == KindOfStaticString || type == KindOfString;
  }
  bool isInteger() const;
  bool isNumeric(bool checkString = false) const;
  DataType toNumeric(int64 &ival, double &dval, bool checkString = false)
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
    return isNull() || isScalar();
  }
  bool isResource() const;
  bool instanceof(CStrRef s) const;

  /**
   * This method is for optimizing switch cases with all int literal
   * cases. firstNonZero refers to the case statement which is the
   * first non zero case in the statements. noMatch refers to a case
   * statement which does NOT exist in the statements.
   */
  int64 hashForIntSwitch(int64 firstNonZero, int64 noMatch) const;

  static int64 DoubleHashForIntSwitch(double dbl, int64 noMatch);

  int64 hashForStringSwitch(
      int64 firstTrueCaseHash,
      int64 firstNullCaseHash,
      int64 firstFalseCaseHash,
      int64 firstZeroCaseHash,
      int64 firstHash,
      int64 noMatchHash,
      bool &needsOrder) const;

  /**
   * Whether or not there are at least two variables that are strongly bound.
   */
  bool isReferenced() const {
    return m_type == KindOfRef && m_data.pref->getCount() > 1;
  }

  /**
   * Get reference count of weak or strong binding. For debugging purpose.
   */
  int getRefCount() const;

  bool getBoolean() const {
    ASSERT(getType() == KindOfBoolean);
    bool val = m_type == KindOfRef ? m_data.pref->var()->m_data.num : m_data.num;
    return val;
  }
  int64 getInt64() const {
    ASSERT(getType() == KindOfInt64);
    return m_type == KindOfRef ? m_data.pref->var()->m_data.num : m_data.num;
  }
  double getDouble() const {
    ASSERT(getType() == KindOfDouble);
    return m_type == KindOfRef ? m_data.pref->var()->m_data.dbl : m_data.dbl;
  }

  /**
   * Can just swap the data between variants sometimes to avoid inc and decref
   */
  void swap(Variant &other) {
    char tmp[sizeof(Variant)];
    memcpy(tmp, &other, sizeof(Variant));
    memcpy((char*)&other, (char*)this, sizeof(Variant));
    memcpy((char*)this, tmp, sizeof(Variant));
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

  Variant  operator +  () const;
  Variant unary_plus() const { return Variant(*this).operator+();}
  Variant  operator +  (CVarRef v) const;
  Variant &operator += (CVarRef v);
  Variant &operator += (int     n) { return operator+=((int64)n);}
  Variant &operator += (int64   n);
  Variant &operator += (double  n);

  Variant negate() const { return Variant(*this).operator-();}
  Variant  operator -  () const;
  Variant  operator -  (CVarRef v) const;
  Variant &operator -= (CVarRef v);
  Variant &operator -= (int     n) { return operator-=((int64)n);}
  Variant &operator -= (int64   n);
  Variant &operator -= (double  n);

  Variant  operator *  (CVarRef v) const;
  Variant &operator *= (CVarRef v);
  Variant &operator *= (int     n) { return operator*=((int64)n);}
  Variant &operator *= (int64   n);
  Variant &operator *= (double  n);

  Variant  operator /  (CVarRef v) const;
  Variant &operator /= (CVarRef v);
  Variant &operator /= (int     n) { return operator/=((int64)n);}
  Variant &operator /= (int64   n);
  Variant &operator /= (double  n);

  int64    operator %  (CVarRef v) const;
  Variant &operator %= (CVarRef v);
  Variant &operator %= (int     n) { return operator%=((int64)n);}
  Variant &operator %= (int64   n);
  Variant &operator %= (double  n);

  Variant  operator ~  () const;
  Variant  operator |  (CVarRef v) const;
  Variant &operator |= (CVarRef v);
  Variant  operator &  (CVarRef v) const;
  Variant &operator &= (CVarRef v);
  Variant  operator ^  (CVarRef v) const;
  Variant &operator ^= (CVarRef v);
  Variant &operator <<=(int64 n);
  Variant &operator >>=(int64 n);

  Variant &operator ++ ();
  Variant  operator ++ (int);
  Variant &operator -- ();
  Variant  operator -- (int);

  /**
   * These are convenient functions for writing extensions, since code
   * generation always uses explicit functions like same(), less() etc. that
   * are type specialized and unambiguous.
   */
  bool operator == (CVarRef v) const;
  bool operator != (CVarRef v) const;
  bool operator >= (CVarRef v) const;
  bool operator <= (CVarRef v) const;
  bool operator >  (CVarRef v) const;
  bool operator <  (CVarRef v) const;

  /**
   * Iterator functions. See array_iterator.h for end() and next().
   */
  ArrayIter begin(CStrRef context = null_string) const;
  // used by generated code
  MutableArrayIter begin(Variant *key, Variant &val,
                         CStrRef context = null_string);

  // Mutable iteration requires the most escalation.
  void escalate(bool mutableIteration = false);

  /**
   * Implicit type conversions. In general, we prefer explicit type conversion
   * functions. These are needed simply because Variant is a coerced type from
   * other types, and we need implicit type conversions to make our type
   * inference coding simpler (Expression::m_expectedType handling).
   */

  operator bool   () const { return toBoolean();}
  operator char   () const { return toByte();}
  operator short  () const { return toInt16();}
  operator int    () const { return toInt32();}
  operator int64  () const { return toInt64();}
  operator double () const { return toDouble();}
  operator String () const;
  operator Array  () const;
  operator Object () const;
  template<typename T> operator SmartObject<T>() const { return toObject();}

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
  int64  toInt64  () const {
    if (IS_NULL_TYPE(m_type)) return 0;
    if (m_type <= KindOfInt64) return m_data.num;
    return toInt64Helper(10);
  }
  int64  toInt64  (int base) const {
    if (IS_NULL_TYPE(m_type)) return 0;
    if (m_type <= KindOfInt64) return m_data.num;
    return toInt64Helper(base);
  }
  double toDouble () const {
    if (m_type == KindOfDouble) return m_data.dbl;
    return toDoubleHelper();
  }
  String toString () const {
    if (m_type == KindOfStaticString || m_type == KindOfString) {
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
  /**
   * Whether or not calling toKey() will throw a bad type exception
   */
  bool  canBeValidKey() const {
    switch (getType()) {
    case KindOfArray:  return false;
    case KindOfObject: return isResource();
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
   * Comparisons
   */
  bool same(bool    v2) const;
  bool same(int     v2) const;
  bool same(int64   v2) const;
  bool same(double  v2) const;
  bool same(litstr  v2) const;
  bool same(const StringData *v2) const;
  bool same(CStrRef v2) const;
  bool same(CArrRef v2) const;
  bool same(CObjRef v2) const;
  bool same(CVarRef v2) const;

  bool equal(bool    v2) const;
  bool equal(int     v2) const;
  bool equal(int64   v2) const;
  bool equal(double  v2) const;
  bool equal(litstr  v2) const;
  bool equal(const StringData *v2) const;
  bool equal(CStrRef v2) const;
  bool equal(CArrRef v2) const;
  bool equal(CObjRef v2) const;
  bool equal(CVarRef v2) const;

  bool equalAsStr(bool    v2) const;
  bool equalAsStr(int     v2) const;
  bool equalAsStr(int64   v2) const;
  bool equalAsStr(double  v2) const;
  bool equalAsStr(litstr v2) const;
  bool equalAsStr(const StringData *v2) const;
  bool equalAsStr(CStrRef v2) const;
  bool equalAsStr(CArrRef v2) const;
  bool equalAsStr(CObjRef v2) const;
  bool equalAsStr(CVarRef v2) const;

  bool less(bool    v2) const;
  bool less(int     v2) const;
  bool less(int64   v2) const;
  bool less(double  v2) const;
  bool less(litstr  v2) const;
  bool less(const StringData *v2) const;
  bool less(CStrRef v2) const;
  bool less(CArrRef v2) const;
  bool less(CObjRef v2) const;
  bool less(CVarRef v2) const;

  bool more(bool    v2) const;
  bool more(int     v2) const;
  bool more(int64   v2) const;
  bool more(double  v2) const;
  bool more(litstr  v2) const;
  bool more(const StringData *v2) const;
  bool more(CStrRef v2) const;
  bool more(CArrRef v2) const;
  bool more(CObjRef v2) const;
  bool more(CVarRef v2) const;

  /**
   * Output functions
   */
  void serialize(VariableSerializer *serializer,
                 bool isArrayKey = false,
                 bool skipNestCheck = false) const;
  void unserialize(VariableUnserializer *unserializer);

  /**
   * Used by SharedStore to save/restore a variant.
   */
  Variant share(bool save) const;

  /**
   * Get the wrapped SharedVariant, if any.
   */
  SharedVariant *getSharedVariant() const;

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(Variant);

  void dump() const;

  /**
   * Offset functions
   */
  Variant rvalAtHelper(int64 offset, ACCESSPARAMS_DECL) const;
  Variant rvalAt(bool offset, ACCESSPARAMS_DECL) const;
  Variant rvalAt(int offset, ACCESSPARAMS_DECL) const {
    return rvalAt((int64)offset, flags);
  }
  Variant rvalAt(int64 offset, ACCESSPARAMS_DECL) const {
    if (m_type == KindOfArray) {
      return m_data.parr->get(offset, flags & AccessFlags::Error);
    }
    return rvalAtHelper(offset, flags);
  }
  Variant rvalAt(double offset, ACCESSPARAMS_DECL) const;
  Variant rvalAt(litstr offset, ACCESSPARAMS_DECL) const;
  Variant rvalAt(CStrRef offset, ACCESSPARAMS_DECL) const;
  Variant rvalAt(CVarRef offset, ACCESSPARAMS_DECL) const;

  template <typename T>
  CVarRef rvalRefHelper(T offset, CVarRef tmp, ACCESSPARAMS_DECL) const;
  CVarRef rvalRef(int offset, CVarRef tmp, ACCESSPARAMS_DECL) const {
    return rvalRef((int64)offset, tmp, flags);
  }

  CVarRef rvalRef(int64 offset, CVarRef tmp, ACCESSPARAMS_DECL) const {
    if (m_type == KindOfArray) {
      return m_data.parr->get(offset, flags & AccessFlags::Error);
    }
    return rvalRefHelper(offset, tmp, flags);
  }
  CVarRef rvalRef(bool offset, CVarRef tmp, ACCESSPARAMS_DECL) const;
  CVarRef rvalRef(double offset, CVarRef tmp, ACCESSPARAMS_DECL) const;
  CVarRef rvalRef(litstr offset, CVarRef tmp, ACCESSPARAMS_DECL) const;
  CVarRef rvalRef(CStrRef offset, CVarRef tmp, ACCESSPARAMS_DECL) const;
  CVarRef rvalRef(CVarRef offset, CVarRef tmp, ACCESSPARAMS_DECL) const;

  // for when we know its an array or null
  template <typename T>
  CVarRef rvalAtRefHelper(T offset, ACCESSPARAMS_DECL) const;
  CVarRef rvalAtRef(bool offset, ACCESSPARAMS_DECL) const {
    return rvalAtRefHelper((int64)offset, flags);
  }
  CVarRef rvalAtRef(int offset, ACCESSPARAMS_DECL) const {
    return rvalAtRefHelper((int64)offset, flags);
  }
  CVarRef rvalAtRef(double offset, ACCESSPARAMS_DECL) const;
  CVarRef rvalAtRef(int64 offset, ACCESSPARAMS_DECL) const {
    return rvalAtRefHelper(offset, flags);
  }
  CVarRef rvalAtRef(CStrRef offset, ACCESSPARAMS_DECL) const {
    return rvalAtRefHelper<CStrRef>(offset, flags);
  }
  CVarRef rvalAtRef(CVarRef offset, ACCESSPARAMS_DECL) const {
    return rvalAtRefHelper<CVarRef>(offset, flags);
  }
  const Variant operator[](bool    key) const { return rvalAt(key);}
  const Variant operator[](int     key) const { return rvalAt(key);}
  const Variant operator[](int64   key) const { return rvalAt(key);}
  const Variant operator[](double  key) const { return rvalAt(key);}
  const Variant operator[](litstr  key) const { return rvalAt(key);}
  const Variant operator[](CStrRef key) const { return rvalAt(key);}
  const Variant operator[](CArrRef key) const { return rvalAt(key);}
  const Variant operator[](CObjRef key) const { return rvalAt(key);}
  const Variant operator[](CVarRef key) const { return rvalAt(key);}

  template<typename T>
  Variant &lval(const T &key) {
    if (m_type == KindOfRef) {
      return m_data.pref->var()->lval(key);
    }

    ASSERT(m_type == KindOfArray);
    Variant *ret = NULL;
    ArrayData *arr = m_data.parr;
    ArrayData *escalated = arr->lval(key, ret, arr->getCount() > 1);
    if (escalated) {
      set(escalated);
    }
    ASSERT(ret);
    return *ret;
  }

  Variant *lvalPtr(CStrRef key, bool forWrite, bool create);

  Variant &lvalAt();

  static Variant &lvalInvalid();
  static Variant &lvalBlackHole();

  Variant &lvalAt(bool    key, ACCESSPARAMS_DECL);
  Variant &lvalAt(int     key, ACCESSPARAMS_DECL);
  Variant &lvalAt(int64   key, ACCESSPARAMS_DECL);
  Variant &lvalAt(double  key, ACCESSPARAMS_DECL);
  Variant &lvalAt(litstr  key, ACCESSPARAMS_DECL);
  Variant &lvalAt(CStrRef key, ACCESSPARAMS_DECL);
  Variant &lvalAt(CVarRef key, ACCESSPARAMS_DECL);

  Variant &lvalRef(bool    key, Variant& tmp, ACCESSPARAMS_DECL);
  Variant &lvalRef(int     key, Variant& tmp, ACCESSPARAMS_DECL);
  Variant &lvalRef(int64   key, Variant& tmp, ACCESSPARAMS_DECL);
  Variant &lvalRef(double  key, Variant& tmp, ACCESSPARAMS_DECL);
  Variant &lvalRef(litstr  key, Variant& tmp, ACCESSPARAMS_DECL);
  Variant &lvalRef(CStrRef key, Variant& tmp, ACCESSPARAMS_DECL);
  Variant &lvalRef(CVarRef key, Variant& tmp, ACCESSPARAMS_DECL);

  Variant refvalAt(bool    key);
  Variant refvalAt(int     key);
  Variant refvalAt(int64   key);
  Variant refvalAt(double  key);
  Variant refvalAt(litstr  key, bool isString = false);
  Variant refvalAt(CStrRef key, bool isString = false);
  Variant refvalAt(CVarRef key);

  Variant argvalAt(bool byRef, bool    key) const;
  Variant argvalAt(bool byRef, int     key) const;
  Variant argvalAt(bool byRef, int64   key) const;
  Variant argvalAt(bool byRef, double  key) const;
  Variant argvalAt(bool byRef, litstr  key,
      bool isString = false) const;
  Variant argvalAt(bool byRef, CStrRef key,
      bool isString = false) const;
  Variant argvalAt(bool byRef, CVarRef key) const;

  Variant o_get(CStrRef propName, bool error = true,
                CStrRef context = null_string) const;
  Variant o_set(CStrRef s, CVarRef v, CStrRef context = null_string);
  Variant o_set(CStrRef s, RefResult v, CStrRef context = null_string) {
    return o_setRef(s, variant(v), context);
  }
  Variant o_setRef(CStrRef s, CVarRef v, CStrRef context = null_string);
  Variant o_getPublic(CStrRef propName, bool error = true) const;
  Variant o_setPublic(CStrRef s, CVarRef v);
  Variant o_setPublic(CStrRef s, RefResult v) {
    return o_setPublicRef(s, variant(v));
  }
  Variant o_setPublicRef(CStrRef s, CVarRef v);
  Variant &o_lval(CStrRef propName, CVarRef tmpForGet,
                  CStrRef context = null_string);
  Variant &o_unsetLval(CStrRef s, CVarRef tmpForGet,
                       CStrRef context = null_string);
  Variant o_argval(bool byRef, CStrRef propName, bool error = true,
      CStrRef context = null_string) const;
  bool o_empty(CStrRef propName, CStrRef context = null_string) const;
  bool o_isset(CStrRef propName, CStrRef context = null_string) const;
  void o_unset(CStrRef propName, CStrRef context = null_string);

  Variant o_invoke(CStrRef s, CArrRef params, int64 hash = -1);
  Variant o_root_invoke(CStrRef s, CArrRef params, int64 hash = -1);
  Variant o_invoke_ex(CStrRef clsname, CStrRef s, CArrRef params);
  Variant o_invoke_few_args(CStrRef s, int64 hash, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);
  Variant o_root_invoke_few_args(CStrRef s, int64 hash, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);
  bool o_get_call_info(MethodCallPackage &info, int64 hash = -1);

  template <typename T>
  inline ALWAYS_INLINE static CVarRef SetImpl(
    Variant *self, T key, CVarRef v, bool isKey);

  template <typename T>
  inline ALWAYS_INLINE static CVarRef SetRefImpl(
    Variant *self, T key, CVarRef v, bool isKey);

  CVarRef set(bool    key, CVarRef v);
  CVarRef set(int     key, CVarRef v) { return set((int64)key, v); }
  CVarRef set(int64   key, CVarRef v);
  CVarRef set(double  key, CVarRef v);
  CVarRef set(litstr  key, CVarRef v, bool isString = false) {
    return set(String(key), v, isString);
  }
  CVarRef set(CStrRef key, CVarRef v, bool isString = false);
  CVarRef set(CVarRef key, CVarRef v);

  CVarRef append(CVarRef v);

  CVarRef setRef(bool    key, CVarRef v);
  CVarRef setRef(int     key, CVarRef v) { return setRef((int64)key, v); }
  CVarRef setRef(int64   key, CVarRef v);
  CVarRef setRef(double  key, CVarRef v);
  CVarRef setRef(litstr  key, CVarRef v, bool isString = false) {
    return setRef(String(key), v, isString);
  }
  CVarRef setRef(CStrRef key, CVarRef v, bool isString = false);
  CVarRef setRef(CVarRef key, CVarRef v);

  CVarRef set(bool    key, RefResult v) { return setRef(key, variant(v)); }
  CVarRef set(int     key, RefResult v) { return setRef(key, variant(v)); }
  CVarRef set(int64   key, RefResult v) { return setRef(key, variant(v)); }
  CVarRef set(double  key, RefResult v) { return setRef(key, variant(v)); }
  CVarRef set(litstr  key, RefResult v, bool isString = false) {
    return setRef(key, variant(v), isString);
  }
  CVarRef set(CStrRef key, RefResult v, bool isString = false) {
    return setRef(key, variant(v), isString);
  }
  CVarRef set(CVarRef key, RefResult v) { return setRef(key, variant(v)); }

  CVarRef appendRef(CVarRef v);
  CVarRef append(RefResult v) { return appendRef(variant(v)); }

  CVarRef setOpEqual(int op, bool key, CVarRef v);
  CVarRef setOpEqual(int op, int key, CVarRef v) {
    return setOpEqual(op, (int64)key, v);
  }
  CVarRef setOpEqual(int op, int64 key, CVarRef v);
  CVarRef setOpEqual(int op, double key, CVarRef v);
  CVarRef setOpEqual(int op, litstr  key, CVarRef v, bool isString = false) {
    return setOpEqual(op, String(key), v, isString);
  }
  CVarRef setOpEqual(int op, CStrRef key, CVarRef v, bool isString = false);
  CVarRef setOpEqual(int op, CVarRef key, CVarRef v);
  CVarRef appendOpEqual(int op, CVarRef v);

  template<typename T, int op>
  T o_assign_op(CStrRef propName, CVarRef val, CStrRef context = null_string);

  void remove(bool    key) { removeImpl(key);}
  void remove(int     key) { removeImpl((int64)key);}
  void remove(int64   key) { removeImpl(key);}
  void remove(double  key) { removeImpl(key);}
  void remove(litstr  key, bool isString = false) {
    remove(String(key), isString);
  }
  void remove(CStrRef key, bool isString = false) {
    removeImpl(key, isString);
  }
  void remove(CVarRef key);

  void weakRemove(litstr key, bool isStr = false) {
    if (is(KindOfArray) ||
        (is(KindOfObject) && getObjectData()->o_instanceof("arrayaccess"))) {
      remove(key, isStr);
      return;
    }
    if (isString()) {
      raise_error("Cannot unset string offsets");
      return;
    }
  }

  void weakRemove(CStrRef key, bool isStr = false) {
    if (is(KindOfArray) ||
        (is(KindOfObject) && getObjectData()->o_instanceof("arrayaccess"))) {
      remove(key, isStr);
      return;
    }
    if (isString()) {
      raise_error("Cannot unset string offsets");
      return;
    }
  }

  template<typename T>
  void weakRemove(const T &key) {
    if (is(KindOfArray) ||
        (is(KindOfObject) && getObjectData()->o_instanceof("arrayaccess"))) {
      remove(key);
      return;
    }
    if (isString()) {
      raise_error("Cannot unset string offsets");
      return;
    }
  }

  /**
   * More array opeartions.
   */
  Variant pop();
  Variant dequeue();
  void prepend(CVarRef v);

  /**
   * Position-based iterations.
   */
  Variant array_iter_reset();
  Variant array_iter_prev();
  Variant array_iter_current() const;
  Variant array_iter_current_ref();
  Variant array_iter_next();
  Variant array_iter_end();
  Variant array_iter_key() const;
  Variant array_iter_each();

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
  int64 *getInt64Data() const {
    ASSERT(getType() == KindOfInt64);
    return m_type == KindOfRef ? &m_data.pref->var()->m_data.num : &m_data.num;
  }
  double *getDoubleData() const {
    ASSERT(getType() == KindOfDouble);
    return m_type == KindOfRef ? &m_data.pref->var()->m_data.dbl : &m_data.dbl;
  }
  StringData *getStringData() const {
    ASSERT(getType() == KindOfString || getType() == KindOfStaticString);
    return m_type == KindOfRef ? m_data.pref->var()->m_data.pstr : m_data.pstr;
  }
  StringData *getStringDataOrNull() const {
    // This is a necessary evil because getStringData() returns
    // an undefined result if this is a null variant
    ASSERT(isNull() || is(KindOfString) || is(KindOfStaticString));
    return m_type == KindOfRef ?
      (m_data.pref->var()->m_type <= KindOfNull ? NULL :
        m_data.pref->var()->m_data.pstr) :
      (m_type <= KindOfNull ? NULL : m_data.pstr);
  }
  ArrayData *getArrayData() const {
    ASSERT(is(KindOfArray));
    return m_type == KindOfRef ? m_data.pref->var()->m_data.parr : m_data.parr;
  }
  ArrayData *getArrayDataOrNull() const {
    // This is a necessary evil because getArrayData() returns
    // an undefined result if this is a null variant
    ASSERT(isNull() || is(KindOfArray));
    return m_type == KindOfRef ?
      (m_data.pref->var()->m_type <= KindOfNull ? NULL :
        m_data.pref->var()->m_data.parr) :
      (m_type <= KindOfNull ? NULL : m_data.parr);
  }
  ObjectData *getObjectData() const {
    ASSERT(is(KindOfObject));
    return m_type == KindOfRef ? m_data.pref->var()->m_data.pobj : m_data.pobj;
  }
  ObjectData *getObjectDataOrNull() const {
    // This is a necessary evil because getObjectData() returns
    // an undefined result if this is a null variant
    ASSERT(isNull() || is(KindOfObject));
    return m_type == KindOfRef ?
      (m_data.pref->var()->m_type <= KindOfNull ? NULL :
        m_data.pref->var()->m_data.pobj) :
      (m_type <= KindOfNull ? NULL : m_data.pobj);
  }
  Variant *getRefData() const {
    ASSERT(m_type == KindOfRef);
    return m_data.pref->var();
  }

  ObjectData *getArrayAccess() const;
  void callOffsetUnset(CVarRef key);
  int64 getNumData() const { return m_data.num; }
  bool isStatic() const { return _count == RefCountStaticValue; }
  void setEvalScalar() const;

  void setToDefaultObject();

  /*
   * Access this Variant as a TypedValue.  Differs slightly from
   * getTypedAccessor() by returning the outer TypedValue if it is
   * KindOfRef.
   */
  TypedValue* asTypedValue() {
    return reinterpret_cast<TypedValue*>(this);
  }
  const TypedValue* asTypedValue() const {
    return reinterpret_cast<const TypedValue*>(this);
  }

  /**
   * Based on the order in complex_types.h, TypedValue is defined before.
   * TypedValue is binary compatible with Variant
   */
  typedef struct TypedValue* TypedValueAccessor;
  TypedValueAccessor getTypedAccessor() const {
    const Variant *value = m_type == KindOfRef ? m_data.pref->var() : this;
    return (TypedValueAccessor)value;
  }
  static DataType GetAccessorType(TypedValueAccessor acc) {
    ASSERT(acc);
    return acc->m_type;
  }
  static bool GetBoolean(TypedValueAccessor acc) {
    ASSERT(acc && acc->m_type == KindOfBoolean);
    return acc->m_data.num;
  }
  static int64 GetInt64(TypedValueAccessor acc) {
    ASSERT(acc);
    ASSERT(acc->m_type == KindOfInt64);
    return acc->m_data.num;
  }
  static double GetDouble(TypedValueAccessor acc) {
    ASSERT(acc && acc->m_type == KindOfDouble);
    return acc->m_data.dbl;
  }
  static bool IsString(TypedValueAccessor acc) {
    return acc->m_type == KindOfString || acc->m_type == KindOfStaticString;
  }
  static StringData *GetStringData(TypedValueAccessor acc) {
    ASSERT(acc);
    ASSERT(acc->m_type == KindOfString || acc->m_type == KindOfStaticString);
    return acc->m_data.pstr;
  }
  static ArrayData *GetArrayData(TypedValueAccessor acc) {
    ASSERT(acc && acc->m_type == KindOfArray);
    return acc->m_data.parr;
  }
  static ObjectData *GetObjectData(TypedValueAccessor acc) {
    ASSERT(acc && acc->m_type == KindOfObject);
    return acc->m_data.pobj;
  }
  static ObjectData *GetArrayAccess(TypedValueAccessor acc) {
    ASSERT(acc && acc->m_type == KindOfObject);
    ObjectData *obj = acc->m_data.pobj;
    if (!obj->o_instanceof("ArrayAccess")) {
      throw InvalidOperandException("not ArrayAccess objects");
    }
    return obj;
  }
  static Array &GetAsArray(TypedValueAccessor acc) {
    ASSERT(acc && acc->m_type == KindOfArray);
    return *(Array*)acc;
  }

  static String &GetAsString(TypedValueAccessor acc) {
    ASSERT(IsString(acc));
    return *(String*)acc;
  }

  /**
   * The order of the data members is significant. The _count field must
   * be exactly FAST_REFCOUNT_OFFSET bytes from the beginning of the object.
   */
 protected:
  mutable union Data {
    int64        num;
    double       dbl;
    StringData  *pstr;
    ArrayData   *parr;
    ObjectData  *pobj;
    RefData     *pref; // shared data between strongly bound Variants
  } m_data;
 protected:
  union {
    // Anonymous: just use _count, m_type
    struct {
      mutable int32_t _count;
      mutable DataType m_type;
    };
    uint64 m_countAndTypeUnion;
  };

 private:
  bool isPrimitive() const { return !IS_REFCOUNTED_TYPE(m_type); }
  bool isObjectConvertable() {
    ASSERT(m_type != KindOfRef);
    return m_type <= KindOfNull ||
      (m_type == KindOfBoolean && !m_data.num) ||
      ((m_type == KindOfString || m_type == KindOfStaticString) &&
       m_data.pstr->empty());
  }

  void removeImpl(double key);
  void removeImpl(int64 key);
  void removeImpl(bool key);
  void removeImpl(CVarRef key, bool isString = false);
  void removeImpl(CStrRef key, bool isString = false);

  CVarRef set(bool    v);
  CVarRef set(int     v);
  CVarRef set(int64   v);
  CVarRef set(double  v);
  CVarRef set(litstr  v);
  CVarRef set(const std::string & v) {
    return set(String(v));
  }
  CVarRef set(StringData  *v);
  CVarRef set(ArrayData   *v);
  CVarRef set(ObjectData  *v);
  CVarRef set(const StringData  *v); // no definition
  CVarRef set(const ArrayData   *v); // no definition
  CVarRef set(const ObjectData  *v); // no definition

  CVarRef set(CStrRef v) { return set(v.get()); }
  CVarRef set(const StaticString & v);
  CVarRef set(CArrRef v) { return set(v.get()); }
  CVarRef set(CObjRef v) { return set(v.get()); }

  template<typename T>
  CVarRef set(const SmartObject<T> &v) {
    return set(v.get());
  }

  // only called from constructor
  void init(ObjectData *v);

  static inline ALWAYS_INLINE
  void AssignValHelper(Variant *self, const Variant *other) {
    if (self->m_type == KindOfRef) self = self->m_data.pref->var();
    if (other->m_type == KindOfRef) other = other->m_data.pref->var();
    if (self != other) {
      DataType otype = other->m_type;
      Data odata = other->m_data;
      Variant scopy(noInit);
      scopy.m_data = self->m_data;
      scopy.m_countAndTypeUnion = self->m_countAndTypeUnion;

      if (IS_REFCOUNTED_TYPE(otype)) {
        odata.pstr->incRefCount();
      }
      self->m_data = odata;
      self->m_type = otype == KindOfUninit ? KindOfNull : otype;
    }
  }

#ifdef INLINE_VARIANT_HELPER
public:
#endif
  static inline ALWAYS_INLINE void PromoteToRef(CVarRef v) {
    ASSERT(hhvm || !v.isVarNR());
    ASSERT(&v != &null_variant);
    if (v.m_type != KindOfRef) {
      RefData *ref = NEW(RefData)(v.m_type, v.m_data.num);
      const_cast<Variant&>(v).m_type = KindOfRef;
      const_cast<Variant&>(v).m_data.pref = ref;
    }
  }

  inline ALWAYS_INLINE void assignValHelper(CVarRef v) {
    AssignValHelper(this, &v);
  }

  inline ALWAYS_INLINE void assignRefHelper(CVarRef v) {
    PromoteToRef(v);
    RefData* r = v.m_data.pref;
    r->incRefCount(); // in case destruct() triggers deletion of v

    RefData* d = m_data.pref;
    DataType t = m_type;
    m_type = KindOfRef;
    m_data.pref = r;
    if (IS_REFCOUNTED_TYPE(t)) destructData(d, t);
  }

  inline ALWAYS_INLINE void constructRefHelper(CVarRef v) {
    PromoteToRef(v);
    v.m_data.pref->incRefCount();
    m_data.pref = v.m_data.pref;
    m_type = KindOfRef;
    _count = 0;
  }

  inline ALWAYS_INLINE void constructValHelper(CVarRef v) {
    const Variant *other =
      UNLIKELY(v.m_type == KindOfRef) ? v.m_data.pref->var() : &v;
    ASSERT(this != other);
    if (IS_REFCOUNTED_TYPE(other->m_type)) {
      other->m_data.pstr->incRefCount();
    }
    _count = 0;
    m_type = other->m_type != KindOfUninit ? other->m_type : KindOfNull;
    m_data = other->m_data;
  }

  inline ALWAYS_INLINE
  void setWithRefHelper(CVarRef v, const ArrayData *arr, bool destroy) {
    ASSERT(this != &v);

    CVarRef rhs = v.m_type == KindOfRef && v.m_data.pref->getCount() <= 1 &&
      (!arr || v.m_data.pref->var()->m_data.parr != arr) ?
      *v.m_data.pref->var() : v;
    if (IS_REFCOUNTED_TYPE(rhs.m_type)) {
      ASSERT(rhs.m_data.pstr);
      rhs.m_data.pstr->incRefCount();
    }

    RefData* d = m_data.pref;
    DataType t = m_type;
    m_type = rhs.m_type;
    if (m_type == KindOfUninit) m_type = KindOfNull; // drop uninit
    m_data.num = rhs.m_data.num;
    if (destroy) destructData(d, t);
  }

  inline ALWAYS_INLINE
  void constructWithRefHelper(CVarRef v, const ArrayData *arr) {
    _count = 0;
    setWithRefHelper(v, arr, false);
  }

#ifdef INLINE_VARIANT_HELPER
private:
#endif

  void split();  // breaking weak binding by making a real copy

  template<typename T>
  static inline ALWAYS_INLINE Variant &LvalAtImpl0(
      Variant *self, T key, Variant *tmp, bool blackHole, ACCESSPARAMS_DECL);

  template<typename T>
  inline ALWAYS_INLINE Variant &lvalAtImpl(T key, ACCESSPARAMS_DECL);

  template<typename T>
  Variant refvalAtImpl(const T &key) {
    if (m_type == KindOfRef) {
      return m_data.pref->var()->refvalAtImpl(key);
    }
    if (is(KindOfArray) || isObjectConvertable()) {
      return strongBind(lvalAt(key));
    } else {
      return rvalAt(key);
    }
  }

  Variant refvalAtImpl(CStrRef key, bool isString = false);

  template<class T>
  Variant argvalAtImpl(bool byRef, const T &key) {
    if (m_type == KindOfRef) {
      return m_data.pref->var()->argvalAtImpl(byRef, key);
    }
    if (byRef && (m_type == KindOfArray ||
                  isObjectConvertable())) {
      return strongBind(lvalAt(key));
    } else {
      return rvalAt(key);
    }
  }
  Variant argvalAtImpl(bool byRef, CStrRef key, bool isString = false);

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
    ASSERT(m_type == KindOfArray);
    if (m_data.parr->getCount() > 1) return true;
    if (v.m_type == KindOfArray) return m_data.parr == v.m_data.parr;
    if (v.m_type == KindOfRef) {
      return m_data.parr == v.m_data.pref->var()->m_data.parr;
    }
    return false;
  }

  bool needCopyForSetRef(CVarRef v) {
    ASSERT(m_type == KindOfArray);
    return m_data.parr->getCount() > 1;
  }

  bool   toBooleanHelper() const;
  int64  toInt64Helper(int base = 10) const;
  double toDoubleHelper() const;
  String toStringHelper() const;
  Array  toArrayHelper() const;
  Object toObjectHelper() const;

  static void compileTimeAssertions() {
    CT_ASSERT(offsetof(Variant,m_data) == offsetof(TypedValue,m_data));
    CT_ASSERT(offsetof(Variant,_count) == offsetof(TypedValue,_count));
    CT_ASSERT(offsetof(Variant,m_type) == offsetof(TypedValue,m_type));
    CT_ASSERT(offsetof(Variant,_count) == FAST_REFCOUNT_OFFSET);
  }
  DataType convertToNumeric(int64 *lval, double *dval) const;
};

class VariantVectorBase {
public:
  ~VariantVectorBase();
  Variant &operator[](int i) {
    return ((Variant*)(this+1))[i];
  }
  void pushWithRef(CVarRef v);
protected:
  union {
    int           m_size;
    TypedValue    force_alignment;
  };
};

template <int num>
class VariantVector : public VariantVectorBase {
public:
  VariantVector() { m_size = 0; }
  VariantVector(int n) {
    m_size = n;
    memset(m_elems, 0, n * sizeof(TypedValue));
  }
protected:
  TypedValue    m_elems[num];
};

class RefResultValue {
public:
  CVarRef get() const { return m_var; }
private:
  Variant m_var;
};

class VRefParamValue {
public:
  template <class T> VRefParamValue(const T &v) : m_var(v) {}

  VRefParamValue() : m_var(Variant::nullInit) {}
  VRefParamValue(RefResult v) : m_var(strongBind(v)) {}
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
  Variant *operator&() const { return &m_var; }
  Variant *operator->() const { return &m_var; }

  operator bool   () const { return m_var.toBoolean();}
  operator int    () const { return m_var.toInt32();}
  operator int64  () const { return m_var.toInt64();}
  operator double () const { return m_var.toDouble();}
  operator String () const { return m_var.toString();}
  operator Array  () const { return m_var.toArray();}
  operator Object () const { return m_var.toObject();}

  bool is(DataType type) const { return m_var.is(type); }
  bool isString() const { return m_var.isString(); }
  bool isObject() const { return m_var.isObject(); }
  bool isReferenced() const { return m_var.isReferenced(); }
  bool isNull() const { return m_var.isNull(); }

  bool toBoolean() const { return m_var.toBoolean(); }
  int64 toInt64() const { return m_var.toInt64(); }
  double toDouble() const { return m_var.toDouble(); }
  String toString() const { return m_var.toString(); }
  StringData *getStringData() const { return m_var.getStringData(); }
  Array toArray() const { return m_var.toArray(); }
  Object toObject() const { return m_var.toObject(); }
  ObjectData *getObjectData() const { return m_var.getObjectData(); }

  CVarRef append(CVarRef v) const { return m_var.append(v); }

  Variant pop() const { return m_var.pop(); }
  Variant dequeue() const { return m_var.dequeue(); }
  void prepend(CVarRef v) const { m_var.prepend(v); }

  bool isArray() const { return m_var.isArray(); }
  ArrNR toArrNR() const { return m_var.toArrNR(); }

  Variant array_iter_reset() const { return m_var.array_iter_reset(); }
  Variant array_iter_prev() const { return m_var.array_iter_prev(); }
  Variant array_iter_current() const { return m_var.array_iter_current(); }
  Variant array_iter_current_ref() const {
    return m_var.array_iter_current_ref();
  }
  Variant array_iter_next() const { return m_var.array_iter_next(); }
  Variant array_iter_end() const { return m_var.array_iter_end(); }
  Variant array_iter_key() const { return m_var.array_iter_key(); }
  Variant array_iter_each() const { return m_var.array_iter_each(); }

  Variant::TypedValueAccessor getTypedAccessor() const {
    return m_var.getTypedAccessor();
  }
private:
  mutable Variant m_var;
};

inline VRefParamValue vref(CVarRef v) {
  return strongBind(v);
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

template<int op> class AssignOp {
public:
  static Variant assign(Variant &var, CVarRef val);
};

///////////////////////////////////////////////////////////////////////////////
// VarNR

class VarNR : private TypedValue {
public:
  // Use to hold variant that do not need ref-counting
  explicit VarNR(bool    v) { init(KindOfBoolean); m_data.num = (v?1:0);}
  explicit VarNR(int     v) { init(KindOfInt64  ); m_data.num = v;}
  explicit VarNR(int64   v) { init(KindOfInt64  ); m_data.num = v;}
  explicit VarNR(uint64  v) { init(KindOfInt64  ); m_data.num = v;}
  explicit VarNR(long    v) { init(KindOfInt64  ); m_data.num = v;}
  explicit VarNR(double  v) { init(KindOfDouble ); m_data.dbl = v;}

  explicit VarNR(const StaticString &v) {
    init(KindOfStaticString);
    StringData *s = v.get();
    ASSERT(s);
    m_data.pstr = s;
  }

  explicit VarNR(CStrRef v);
  explicit VarNR(CArrRef v);
  explicit VarNR(CObjRef v);
  explicit VarNR(StringData *v);
  explicit VarNR(const StringData *v) {
    ASSERT(v && v->isStatic());
    init(KindOfString);
    m_data.pstr = const_cast<StringData*>(v);
  }
  explicit VarNR(ArrayData *v);
  explicit VarNR(ObjectData *v);

  VarNR(const VarNR &v) : TypedValue(v) {}

  explicit VarNR() { asVariant()->setUninitNull(); }

#ifdef DEBUG
  ~VarNR() { ASSERT(checkRefCount()); }
#endif

  operator CVarRef() const { return *asVariant(); }

  bool isNull() const {
    return asVariant()->isNull();
  }

  bool isVarNR() const {
    return asVariant()->isVarNR();
  }
private:
  VarNR(litstr  v); // not implemented
  VarNR(const std::string & v); // not implemented

  void init(DataType dt) { asVariant()->initForVarNR(dt); }
  const Variant *asVariant() const {
    return (const Variant*)this;
  }
  Variant *asVariant() {
    return (Variant*)this;
  }
  bool checkRefCount() {
    if (m_type == KindOfRef) return false;
    if (!IS_REFCOUNTED_TYPE(m_type)) return true;
    if (!isVarNR()) return false;
    switch (m_type) {
    case KindOfArray:
      return m_data.parr->getCount() > 0;
    case KindOfString:
      return m_data.pstr->getCount() > 0;
    case KindOfObject:
      return m_data.pobj->getCount() > 0;
    default:
      break;
    }
    return false;
  }
};

typedef struct VariantProxy {
  union {
    char m_data[sizeof(Variant)];
    void *m_dummyp;
    int64 m_dummyi;
  };
} VariantProxy;

///////////////////////////////////////////////////////////////////////////////
// breaking circular dependencies

template<typename T>
Variant Array::refvalAt(const T &key) {
  return strongBind(lvalAt(key));
}

template<typename T>
Variant Array::argvalAt(bool byRef, const T &key) const {
  if (byRef) {
    return strongBind(const_cast<Array*>(this)->lvalAt(key));
  } else {
    return rvalAtRef(key);
  }
}

inline const Variant Array::operator[](bool    key) const {
  return rvalAt(key);
}

inline const Variant Array::operator[](int     key) const {
  return rvalAt(key);
}

inline const Variant Array::operator[](int64   key) const {
  return rvalAt(key);
}

inline const Variant Array::operator[](double  key) const {
  return rvalAt(key);
}

inline const Variant Array::operator[](litstr  key) const {
  return rvalAt(key);
}

inline const Variant Array::operator[](CStrRef key) const {
  return rvalAt(key);
}

inline const Variant Array::operator[](CVarRef key) const {
  return rvalAt(key);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VARIANT_H__
