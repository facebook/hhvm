/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/types.h>
#include <runtime/base/hphp_value.h>
#include <runtime/base/type_string.h>
#include <runtime/base/type_object.h>
#include <runtime/base/type_array.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/macros.h>

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
 * underlying data. Perhaps "contagious" bit is the hardest to understand, but
 * it's really just a temporary flag to indicate that next assignment should
 * set both variables to be strongly bound together. For example,
 *
 *   a = ref(b);
 *
 * Here, ref() simply sets "contagious" flag on b. Then assignment a = b will
 * check contagious flag. If on, both will be bound together. Then contagious
 * flag will be cleared, so not to affect future assignments. Code generation
 * made sure both "a" and "b" are Variants, otherwise we won't do references.
 *
 * In this class, strong binding is done through "pvar" member variable. All
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
 *   pvar    strong    x (pointer)                      x
 */

#define null ((Variant()))

class Variant {
 public:
  friend class Array;

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

  void destruct();

  ~Variant() { if (IS_REFCOUNTED_TYPE(m_type)) destruct(); }

  void reset(); // only for special memory sweeping!

  /**
   * Constructors. We can't really use template<T> here, since that will make
   * Variant being able to take many other external types, messing up those
   * operator overloads.
   */
  Variant(bool    v) : _count(0), m_type(KindOfBoolean) { m_data.num = (v?1:0);}
  Variant(int     v) : _count(0), m_type(KindOfInt32  ) { m_data.num = v;}
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
  Variant(Variant *v);

  template<typename T>
  Variant(const SmartObject<T> &v) : _count(0) { init((ObjectData *)v.get()); }

  inline ALWAYS_INLINE void VariantHelper(CVarRef v) {
    setUninitNull();
    if (v.isContagious()) {
      assignContagious(v);
      return;
    }
    bind(v);
  }

#ifdef INLINE_VARIANT_HELPER
  inline ALWAYS_INLINE Variant(CVarRef v) { VariantHelper(v);}
#else
  Variant(CVarRef v);
#endif

 private:
  friend class VarNR;
  // This helper is only used to construct VarNR
  static const int NR_FLAG = 1 << 29;
  void initForVarNR(DataType dt) { _count = NR_FLAG; m_type = dt; }

 public:
  bool isVarNR() const { return _count == NR_FLAG; }

  /**
   * Break bindings and set to null.
   */
  void unset() {
    if (IS_REFCOUNTED_TYPE(m_type)) destruct();
    m_type = KindOfUninit;
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
  const String & asCStrRef() const {
    ASSERT(m_type == KindOfString || m_type == KindOfStaticString);
    return *(const String*)(this);
  }

  String & asStrRef() {
    ASSERT(m_type == KindOfString || m_type == KindOfStaticString);
    return *(String*)(this);
  }

  const Array & asCArrRef() const {
    ASSERT(m_type == KindOfArray);
    return *(const Array*)(this);
  }

  Array & asArrRef() {
    ASSERT(m_type == KindOfArray);
    return *(Array*)(this);
  }

  const Object & asCObjRef() const {
    ASSERT(m_type == KindOfObject);
    return *(const Object*)(this);
  }

  Object & asObjRef() {
    ASSERT(m_type == KindOfObject);
    return *(Object*)(this);
  }

  ObjectData *objectForCall() const {
    if (m_type == KindOfObject) return m_data.pobj;
    if (m_type == KindOfVariant) {
      Variant *t = m_data.pvar;
      if (t->m_type == KindOfObject) return t->m_data.pobj;
    }
    throw_call_non_object();
    return 0;
  }

  /**
   * Type testing functions
   */
  DataType getType() const {
    return m_type == KindOfVariant ? m_data.pvar->m_type : m_type;
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
    return getType() <= KindOfNull;
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
      case KindOfInt32:
      case KindOfInt64:
      case KindOfObject:
        return true;
      case KindOfVariant:
        return m_data.pvar->isIntVal();
      default:
        break;
    }
    return false;
  }
  bool isArray() const {
    return getType() == KindOfArray;
  }
  bool isResource() const;
  bool instanceof(CStrRef s) const;

  /**
   * Borrowing Countable::_count for contagious bit, and this is okay, since
   * outer Variant never uses reference counting.
   */
  void setContagious() const {
    ASSERT(this != &null_variant);
    ASSERT(!isVarNR());
    _count = -1;
  }
  void clearContagious() const { ASSERT(!isVarNR()); _count = 0;}
  bool isContagious() const { return _count == -1;}

  /**
   * Whether or not there are at least two variables that are strongly bound.
   */
  bool isReferenced() const {
    return m_type == KindOfVariant && m_data.pvar->getCount() > 1;
  }

  /**
   * Get reference count of weak or strong binding. For debugging purpose.
   */
  int getRefCount() const;

  bool getBoolean() const {
    ASSERT(getType() == KindOfBoolean);
    bool val = m_type == KindOfVariant ? m_data.pvar->m_data.num : m_data.num;
    return val;
  }
  int64 getInt64() const {
    ASSERT(getType() == KindOfInt32   ||
           getType() == KindOfInt64);
    return m_type == KindOfVariant ? m_data.pvar->m_data.num : m_data.num;
  }
  double getDouble() const {
    ASSERT(getType() == KindOfDouble);
    return m_type == KindOfVariant ? m_data.pvar->m_data.dbl : m_data.dbl;
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

  Variant &operator=(CVarRef v) {
    return assign(v);
  }
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
  ArrayIter begin(CStrRef context = null_string,
                  bool setIterDirty = false) const;
  // used by generated code
  MutableArrayIter begin(Variant *key, Variant &val,
                         CStrRef context = null_string,
                         bool setIterDirty = false);

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
    if (m_type <= KindOfNull) return false;
    if (m_type <= KindOfInt64) return m_data.num;
    return toBooleanHelper();
  }
  char   toByte   () const { return (char)toInt64();}
  short  toInt16  (int base = 10) const { return (short)toInt64(base);}
  int    toInt32  (int base = 10) const { return (int)toInt64(base);}
  int64  toInt64  () const {
    if (m_type <= KindOfNull) return 0;
    if (m_type <= KindOfInt64) return m_data.num;
    return toInt64Helper(10);
  }
  int64  toInt64  (int base) const {
    if (m_type <= KindOfNull) return 0;
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
  VarNR toKey   () const;
  /* Creating a temporary Array, String, or Object with no ref-counting and
   * no type checking, use it only when we have checked the variant type and
   * we are sure the internal data will have a reference until the temporary
   * one gets out-of-scope.
   */
  StrNR toStrNR () const {
    return getStringData();
  }
  ArrNR toArrNR () const {
    return getArrayData();
  }
  ObjNR toObjNR() const {
    return getObjectData();
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
                 bool isArrayKey = false) const;
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
   * Marshaling/Unmarshaling between request thread and fiber thread.
   */
  Variant fiberMarshal(FiberReferenceMap &refMap) const;
  Variant fiberUnmarshal(FiberReferenceMap &refMap) const;

  /**
   * Debugging functions.
   */
  static const char *getTypeString(DataType type);
  std::string getDebugDump() const;

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION_NOCALLBACKS(Variant);

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
  CVarRef rvalRef(bool offset, CVarRef tmp, ACCESSPARAMS_DECL) const {
    return rvalRef((int64)offset, tmp, flags);
  }
  CVarRef rvalRef(int offset, CVarRef tmp, ACCESSPARAMS_DECL) const {
    return rvalRef((int64)offset, tmp, flags);
  }
  CVarRef rvalRef(int64 offset, CVarRef tmp, ACCESSPARAMS_DECL) const {
    if (m_type == KindOfArray) {
      return m_data.parr->get(offset, flags & AccessFlags::Error);
    }
    return rvalRefHelper(offset, tmp, flags);
  }
  CVarRef rvalRef(double offset, CVarRef tmp, ACCESSPARAMS_DECL) const;
  CVarRef rvalRef(litstr offset, CVarRef tmp, ACCESSPARAMS_DECL) const;
  CVarRef rvalRef(CStrRef offset, CVarRef tmp, ACCESSPARAMS_DECL) const;
  CVarRef rvalRef(CVarRef offset, CVarRef tmp, ACCESSPARAMS_DECL) const;

  const Variant operator[](bool    key) const { return rvalAt(key);}
  const Variant operator[](int     key) const { return rvalAt(key);}
  const Variant operator[](int64   key) const { return rvalAt(key);}
  const Variant operator[](double  key) const { return rvalAt(key);}
  const Variant operator[](litstr  key) const { return rvalAt(key);}
  const Variant operator[](CStrRef key) const { return rvalAt(key);}
  const Variant operator[](StringData *key) const {
    assert(false);
    return rvalAt(String(key));
  }
  const Variant operator[](CArrRef key) const { return rvalAt(key);}
  const Variant operator[](CObjRef key) const { return rvalAt(key);}
  const Variant operator[](CVarRef key) const { return rvalAt(key);}

  template<typename T>
  Variant &lval(const T &key) {
    if (m_type == KindOfVariant) {
      return m_data.pvar->lval(key);
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

  Variant refvalAt(bool    key);
  Variant refvalAt(int     key);
  Variant refvalAt(int64   key);
  Variant refvalAt(double  key);
  Variant refvalAt(litstr  key, bool isString = false);
  Variant refvalAt(CStrRef key, bool isString = false);
  Variant refvalAt(CVarRef key);

  Variant argvalAt(bool byRef, bool    key);
  Variant argvalAt(bool byRef, int     key);
  Variant argvalAt(bool byRef, int64   key);
  Variant argvalAt(bool byRef, double  key);
  Variant argvalAt(bool byRef, litstr  key,
      bool isString = false);
  Variant argvalAt(bool byRef, CStrRef key,
      bool isString = false);
  Variant argvalAt(bool byRef, CVarRef key);

  Variant o_get(CStrRef propName, bool error = true,
                CStrRef context = null_string) const;
  Variant o_set(CStrRef s, CVarRef v, CStrRef context = null_string);
  Variant o_getPublic(CStrRef propName, bool error = true) const;
  Variant o_setPublic(CStrRef s, CVarRef v);
  Variant &o_lval(CStrRef propName, CVarRef tmpForGet,
                  CStrRef context = null_string);
  Variant &o_unsetLval(CStrRef s, CVarRef tmpForGet,
                       CStrRef context = null_string);
  Variant o_argval(bool byRef, CStrRef propName, bool error = true,
      CStrRef context = null_string) const;
  bool o_empty(CStrRef propName, CStrRef context = null_string) const;
  bool o_isset(CStrRef propName, CStrRef context = null_string) const;
  void o_unset(CStrRef propName, CStrRef context = null_string);

  Variant o_invoke(const char *s, CArrRef params, int64 hash = -1);
  Variant o_invoke(CStrRef s, CArrRef params, int64 hash = -1);
  Variant o_root_invoke(const char *s, CArrRef params, int64 hash = -1);
  Variant o_root_invoke(CStrRef s, CArrRef params, int64 hash = -1);
  Variant o_invoke_ex(CStrRef clsname, CStrRef s, CArrRef params);
  Variant o_invoke_few_args(const char *s, int64 hash, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);
  Variant o_invoke_few_args(CStrRef s, int64 hash, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);
  Variant o_root_invoke_few_args(const char *s, int64 hash, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);
  Variant o_root_invoke_few_args(CStrRef s, int64 hash, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);
  bool o_get_call_info(MethodCallPackage &info, int64 hash = -1);

  /**
   * The whole purpose of VariantOffset is to collect "v" parameter to call
   * this function.
   */
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
   * Purely for reset() missing error.
   */
  void array_iter_dirty_set() const;
  void array_iter_dirty_reset() const;
  void array_iter_dirty_check() const;

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
    return m_type == KindOfVariant ? &m_data.pvar->m_data.num : &m_data.num;
  }
  double *getDoubleData() const {
    ASSERT(getType() == KindOfDouble);
    return m_type == KindOfVariant ? &m_data.pvar->m_data.dbl : &m_data.dbl;
  }
  StringData *getStringData() const {
    ASSERT(getType() == KindOfString || getType() == KindOfStaticString);
    return m_type == KindOfVariant ? m_data.pvar->m_data.pstr : m_data.pstr;
  }
  ArrayData *getArrayData() const {
    ASSERT(is(KindOfArray));
    return m_type == KindOfVariant ? m_data.pvar->m_data.parr : m_data.parr;
  }
  ObjectData *getObjectData() const {
    ASSERT(is(KindOfObject));
    return m_type == KindOfVariant ? m_data.pvar->m_data.pobj : m_data.pobj;
  }
  Variant *getVariantData() const {
    // Wrap into a referenceable form, if it isn't already.
    if (m_type != KindOfVariant) {
      Variant *shared = NEW(Variant)();
      shared->bindNoVariant(*this);
      shared->_count = 1;

      _count = 0;
      m_data.pvar = shared;
      m_type = KindOfVariant;
    }
    ASSERT(m_type == KindOfVariant);
    return m_data.pvar;
  }

  ObjectData *getArrayAccess() const;
  void callOffsetUnset(CVarRef key);
  int64 getNumData() const { return m_data.num; }
  bool isStatic() const { return _count == (1 << 30); }
  void setStatic() const;

  /**
   * Based on the order in complex_types.h, TypedValue is defined before.
   * TypedValue is binary compatible with Variant
   */
  typedef struct TypedValue* TypedValueAccessor;
  TypedValueAccessor getTypedAccessor() const {
    const Variant *value = m_type == KindOfVariant ? m_data.pvar : this;
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
    ASSERT(acc->m_type == KindOfInt32 ||
           acc->m_type == KindOfInt64);
    return acc->m_data.num;
  }
  static double GetDouble(TypedValueAccessor acc) {
    ASSERT(acc && acc->m_type == KindOfDouble);
    return acc->m_data.dbl;
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


  /**
   * The order of the data members is significant. The _count field must
   * be exactly FAST_REFCOUNT_OFFSET bytes from the beginning of the object.
   */
 protected:
  mutable union {
    int64        num;
    double       dbl;
    litstr       str;
    StringData  *pstr;
    ArrayData   *parr;
    ObjectData  *pobj;
    Variant     *pvar; // shared data between strongly bound Variants
  } m_data;
 protected:
  union {
    // Anonymous: just use _count, m_type
    struct {
      mutable int _count;
      mutable DataType m_type;
    };
    uint64 m_countAndTypeUnion;
  };

 private:
  bool isPrimitive() const { return !IS_REFCOUNTED_TYPE(m_type); }
  bool isObjectConvertable() {
    ASSERT(m_type != KindOfVariant);
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
  CVarRef set(char    v);
  CVarRef set(short   v);
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

  void bindNoVariant(CVarRef v) {
    ASSERT(v.m_type != KindOfVariant);
    m_type = v.m_type;
    m_data.num = v.m_data.num;
    if (IS_REFCOUNTED_TYPE(v.m_type)) {
#ifdef FAST_REFCOUNT_FOR_VARIANT
      m_data.pvar->incRefCount();
#else
      switch (m_type) {
      case KindOfString:
        ASSERT(m_data.pstr);
        m_data.pstr->incRefCount();
        break;
      case KindOfArray:
        ASSERT(m_data.parr);
        m_data.parr->incRefCount();
        break;
      case KindOfObject:
        ASSERT(m_data.pobj);
        m_data.pobj->incRefCount();
        break;
      default:
        ASSERT(false);
      }
#endif
    } else if (m_type == KindOfUninit) {
      m_type = KindOfNull; // drop uninit
    }
  }

  // Internal helper for weakly binding a variable. m_type should be viewed
  // as KindOfNull and for complex types the old data already released.
  void bind(CVarRef v) {
    if (!IS_REFCOUNTED_TYPE(v.m_type)) {
      m_type = v.m_type;
      if (m_type == KindOfUninit) m_type = KindOfNull; // drop uninit
      m_data.num = v.m_data.num;
      return;
    }
#ifdef FAST_REFCOUNT_FOR_VARIANT
    Variant *var = v.m_data.pvar;
    ASSERT(var);
    if (v.m_type != KindOfVariant) {
      m_type = v.m_type;
      m_data.pvar = var;
      /**
       * This is safe because we have compile time assertions that
       * guarantee that the _count field will always be exactly
       * FAST_REFCOUNT_OFFSET bytes from the beginning of the object
       * for the StringData, ArrayData, ObjectData, and Variant classes.
       */
      var->incRefCount();
    } else {
      bindNoVariant(*var);
    }
#else
    switch (v.m_type) {
    // copy-on-write: ref counting complex types
    case KindOfString: {
      StringData *str = v.m_data.pstr;
      ASSERT(str);
      m_type = KindOfString;
      m_data.pstr = str;
      str->incRefCount();
      break;
    }
    case KindOfArray: {
      ArrayData *arr = v.m_data.parr;
      ASSERT(arr);
      m_type = KindOfArray;
      m_data.parr = arr;
      arr->incRefCount();
      break;
    }
    case KindOfObject: {
      ObjectData *obj = v.m_data.pobj;
      ASSERT(obj);
      m_type = KindOfObject;
      m_data.pobj = obj;
      obj->incRefCount();
      break;
    }
    case KindOfVariant:
      bindNoVariant(*v.m_data.pvar);
      break;
    default:
      ASSERT(false);
    }
#endif
  }

  // Internal helper for strongly binding a variable
  void strongBind(Variant *v) {
    ASSERT(v != this);
    if (v) {
      v->incRefCount(); // in case destruct() triggers deletion of v
      if (IS_REFCOUNTED_TYPE(m_type)) destruct();
      m_type = KindOfVariant;
      m_data.pvar = v;
    } else {
      unset();
    }
  }

  // Internal helper for handling contagious assignment
  void assignContagious(CVarRef v) {
    ASSERT(v.isContagious());
    // do it early to avoid strongBind() triggers deletion of v
    v.clearContagious();
    // we have to wrap up v into a sharable form
    if (v.m_type != KindOfVariant) {
      Variant *shared = NEW(Variant)();
      shared->bindNoVariant(v);
      const_cast<Variant&>(v).strongBind(shared);
    }
    // then we can share v.m_data.pvar
    strongBind(v.m_data.pvar);
  }

  void split();  // breaking weak binding by making a real copy

  template<typename T>
  Variant &lvalAtImpl(T key, ACCESSPARAMS_DECL);

  template<typename T>
  Variant refvalAtImpl(const T &key) {
    if (m_type == KindOfVariant) {
      return m_data.pvar->refvalAtImpl(key);
    }
    if (is(KindOfArray) || isObjectConvertable()) {
      return ref(lvalAt(key));
    } else {
      return rvalAt(key);
    }
  }

  Variant refvalAtImpl(CStrRef key, bool isString = false);

  template<class T>
  Variant argvalAtImpl(bool byRef, const T &key) {
    if (m_type == KindOfVariant) {
      return m_data.pvar->argvalAtImpl(byRef, key);
    }
    if (byRef && (is(KindOfArray) || isNull() ||
          (is(KindOfBoolean) && !toBoolean()) ||
          (is(KindOfStaticString) && getStringData()->empty()) ||
          (is(KindOfString) && getStringData()->empty()))) {
      return ref(lvalAt(key));
    } else {
      return rvalAt(key);
    }
  }
  Variant argvalAtImpl(bool byRef, CStrRef key, bool isString = false);

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
    if (!v.isContagious()) {
      if (v.m_type == KindOfArray) return m_data.parr == v.m_data.parr;
      if (v.m_type == KindOfVariant) {
        return m_data.parr == v.m_data.pvar->m_data.parr;
      }
    }
    return false;
  }

 private:
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
#ifdef FAST_REFCOUNT_FOR_VARIANT
    CT_ASSERT(offsetof(Variant,_count) == FAST_REFCOUNT_OFFSET);
#endif
  }
  DataType convertToNumeric(int64 *lval, double *dval) const;
};

template<int op> class AssignOp {
public:
  static Variant assign(Variant &var, CVarRef val);
};

///////////////////////////////////////////////////////////////////////////////
// VarNR

class VarNR : private TypedValue {
public:
  // Use to hold variant that do not need ref-counting
  VarNR(bool    v) { init(KindOfBoolean); m_data.num = (v?1:0);}
  VarNR(int     v) { init(KindOfInt32  ); m_data.num = v;}
  VarNR(int64   v) { init(KindOfInt64  ); m_data.num = v;}
  VarNR(uint64  v) { init(KindOfInt64  ); m_data.num = v;}
  VarNR(long    v) { init(KindOfInt64  ); m_data.num = v;}
  VarNR(double  v) { init(KindOfDouble ); m_data.dbl = v;}

  VarNR(const StaticString &v) {
    init(KindOfStaticString);
    StringData *s = v.get();
    ASSERT(s);
    m_data.pstr = s;
  }

  VarNR(CStrRef v);
  VarNR(CArrRef v);
  VarNR(CObjRef v);
  VarNR(StringData *v);
  VarNR(ArrayData *v);
  VarNR(ObjectData *v);

  VarNR(const VarNR &v) : TypedValue(v) {}

  VarNR() { asVariant()->setUninitNull(); }

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
    if (m_type == KindOfVariant) return false;
    if (!IS_REFCOUNTED_TYPE(m_type)) return true;
    if (!((Variant*)this)->isVarNR()) return false;
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

///////////////////////////////////////////////////////////////////////////////
// breaking circular dependencies

template<typename T>
CVarRef Array::setImpl(const T &key, CVarRef v) {
  if (!m_px) {
    ArrayData *data = ArrayData::Create(key, v);
    SmartPtr<ArrayData>::operator=(data);
  } else {
    if (v.isContagious()) {
      escalate();
    }
    ArrayData *escalated =
      m_px->set(key, v, (m_px->getCount() > 1));
    if (escalated) {
      SmartPtr<ArrayData>::operator=(escalated);
    }
  }
  return v;
}

template<typename T>
CVarRef Array::addImpl(const T &key, CVarRef v) {
  if (!m_px) {
    ArrayData *data = ArrayData::Create(key, v);
    SmartPtr<ArrayData>::operator=(data);
  } else {
    if (v.isContagious()) escalate();
    ArrayData *escalated = m_px->add(key, v, (m_px->getCount() > 1));
    if (escalated) {
      SmartPtr<ArrayData>::operator=(escalated);
    }
  }
  return v;
}

template<typename T>
Variant Array::refvalAt(const T &key) {
  return ref(lvalAt(key));
}

template<typename T>
Variant Array::argvalAt(bool byRef, const T &key) {
  if (byRef) {
    return ref(lvalAt(key));
  } else {
    return rvalAt(key);
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

inline const Variant Array::operator[](const StringData *key) const {
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
