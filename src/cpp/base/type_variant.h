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

#ifndef __HPHP_VARIANT_H__
#define __HPHP_VARIANT_H__

#include <cpp/base/types.h>
#include <cpp/base/memory/smart_allocator.h>
#include <cpp/base/array/array_data.h>
#include <cpp/base/array/array_iterator.h>
#include <cpp/base/macros.h>

namespace HPHP {

#define null Variant()
#define null_variant Variant::s_nullVariant

///////////////////////////////////////////////////////////////////////////////


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
class Variant : public Countable {
public:
  Variant() : m_type(KindOfNull) { m_data.num = 1 /* uninitialized */;}

  static const Variant s_nullVariant;

  // g++ does not inline !isPrimitive()
  ~Variant() { if (m_type > LiteralString) destruct();}
  void reset(); // only for special memory sweeping!

  /**
   * Constructors. We can't really use template<T> here, since that will make
   * Variant being able to take many other external types, messing up those
   * operator overloads.
   */
  Variant(CVarRef v);
  Variant(bool    v) : m_type(KindOfBoolean)  { m_data.num = (v?1:0);}
  Variant(char    v) : m_type(KindOfByte  )   { m_data.num = v;}
  Variant(short   v) : m_type(KindOfInt16 )   { m_data.num = v;}
  Variant(int     v) : m_type(KindOfInt32 )   { m_data.num = v;}
  Variant(int64   v) : m_type(KindOfInt64 )   { m_data.num = v;}
  Variant(uint64  v) : m_type(KindOfInt64 )   { m_data.num = v;}
  Variant(ssize_t v) : m_type(KindOfInt64 )   { m_data.num = v;}
  Variant(double  v) : m_type(KindOfDouble )  { m_data.dbl = v;}
  Variant(litstr  v) : m_type(LiteralString)  { m_data.str = v;}

  Variant(CStrRef v);
  Variant(CArrRef v);
  Variant(CObjRef v);
  Variant(StringData *v);
  Variant(ArrayData *v);
  Variant(ObjectData *v);
  Variant(Variant *v);

  template<typename T>
  Variant(const SmartObject<T> &v) : m_type(KindOfNull) {
    set(v);
  }

  /**
   * Break bindings and set to null.
   */
  void unset() {
    destruct();
    m_data.num = 0;
    m_type = KindOfNull;
  }

  /**
   * set to null without breaking bindings (if any), faster than v_a = null;
   */
  void setNull();

  /**
   * Type testing functions
   */
  DataType getType() const {
    return m_type == KindOfVariant ? m_data.pvar->getType() : m_type;
  }
  bool is(DataType type) const {
    return getType() == type;
  }
  bool isInitialized() const {
    return m_type != KindOfNull || m_data.num != 1 /* uninitialized */;
  }
  bool isNull() const {
    return getType() == KindOfNull;
  }
  bool isBoolean() const {
    return getType() == KindOfBoolean;
  }
  bool isDouble() const {
    return getType() == KindOfDouble;
  }
  bool isString() const {
    DataType type = getType();
    return type == LiteralString || type == KindOfString;
  }
  bool isPrimitive() const { return m_type <= LiteralString; }
  bool isInteger() const;
  bool isNumeric(bool checkString = false) const;
  bool isScalar() const;
  bool isObject () const {
    return getType() == KindOfObject;
  }
  bool isIntVal() const {
    return isInteger() || isNull() || isBoolean() || isObject();
  }
  bool isArray() const {
    return getType() == KindOfArray;
  }
  bool isResource() const;
  bool instanceof(const char *s) const;

  /**
   * Borrowing Countable::_count for contagious bit, and this is okay, since
   * outer Variant never uses reference counting.
   */
  void setContagious() const { _count = -1;}
  void clearContagious() const { _count = 0;}
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

  double getDouble() const {
    ASSERT(m_type == KindOfDouble);
    return m_data.dbl;
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
  Variant &assign(CVarRef v, bool deep);
  Variant &operator=(CVarRef v) {
    return assign(v, true);
  }
  template<typename T> Variant &operator=(const T &v) {
    if (m_type != KindOfVariant) {
      set(v);
    } else {
      m_data.pvar->set(v);
    }
    return *this;
  }

  Variant  operator +  ();
  Variant unary_plus() const { return Variant(*this).operator+();}
  Variant  operator +  (CVarRef v) const;
  Variant &operator += (CVarRef v);
  Variant &operator += (char    n) { return operator+=((int64)n);}
  Variant &operator += (short   n) { return operator+=((int64)n);}
  Variant &operator += (int     n) { return operator+=((int64)n);}
  Variant &operator += (int64   n);
  Variant &operator += (double  n);

  Variant negate() const { return Variant(*this).operator-();}
  Variant  operator -  ();
  Variant  operator -  (CVarRef v) const;
  Variant &operator -= (CVarRef v);
  Variant &operator -= (char    n) { return operator-=((int64)n);}
  Variant &operator -= (short   n) { return operator-=((int64)n);}
  Variant &operator -= (int     n) { return operator-=((int64)n);}
  Variant &operator -= (int64   n);
  Variant &operator -= (double  n);

  Variant  operator *  (CVarRef v) const;
  Variant &operator *= (CVarRef v);
  Variant &operator *= (char    n) { return operator*=((int64)n);}
  Variant &operator *= (short   n) { return operator*=((int64)n);}
  Variant &operator *= (int     n) { return operator*=((int64)n);}
  Variant &operator *= (int64   n);
  Variant &operator *= (double  n);

  Variant  operator /  (CVarRef v) const;
  Variant &operator /= (CVarRef v);
  Variant &operator /= (char    n) { return operator/=((int64)n);}
  Variant &operator /= (short   n) { return operator/=((int64)n);}
  Variant &operator /= (int     n) { return operator/=((int64)n);}
  Variant &operator /= (int64   n);
  Variant &operator /= (double  n);

  int64    operator %  (CVarRef v) const;
  Variant &operator %= (CVarRef v);
  Variant &operator %= (char    n) { return operator%=((int64)n);}
  Variant &operator %= (short   n) { return operator%=((int64)n);}
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
   * escalate() will escalate me to become VectorVariant or MapVariant, so that
   * getValueRef() can be called to take a reference to an array element.
   */
  ArrayIterPtr begin(const char *context = NULL) const;
  // used by generated code
  MutableArrayIterPtr begin(Variant *key, Variant &val);
  void escalate();

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
  bool   toBoolean() const;
  char   toByte   () const { return (char)toInt64();}
  short  toInt16  (int base = 10) const { return (short)toInt64(base);}
  int    toInt32  (int base = 10) const { return (int)toInt64(base);}
  int64  toInt64  (int base = 10) const;
  double toDouble () const;
  String toString () const;
  Array  toArray  () const;
  Object toObject () const;
  Variant toKey   () const;

  /**
   * Comparisons
   */
  bool same(bool    v2) const;
  bool same(char    v2) const;
  bool same(short   v2) const;
  bool same(int     v2) const;
  bool same(int64   v2) const;
  bool same(double  v2) const;
  bool same(litstr  v2) const;
  bool same(CStrRef v2) const;
  bool same(CArrRef v2) const;
  bool same(CObjRef v2) const;
  bool same(CVarRef v2) const;

  bool equal(bool    v2) const;
  bool equal(char    v2) const;
  bool equal(short   v2) const;
  bool equal(int     v2) const;
  bool equal(int64   v2) const;
  bool equal(double  v2) const;
  bool equal(litstr  v2) const;
  bool equal(CStrRef v2) const;
  bool equal(CArrRef v2) const;
  bool equal(CObjRef v2) const;
  bool equal(CVarRef v2) const;

  bool less(bool    v2) const;
  bool less(char    v2) const;
  bool less(short   v2) const;
  bool less(int     v2) const;
  bool less(int64   v2) const;
  bool less(double  v2) const;
  bool less(litstr  v2) const;
  bool less(CStrRef v2) const;
  bool less(CArrRef v2) const;
  bool less(CObjRef v2) const;
  bool less(CVarRef v2) const;

  bool more(bool    v2) const;
  bool more(char    v2) const;
  bool more(short   v2) const;
  bool more(int     v2) const;
  bool more(int64   v2) const;
  bool more(double  v2) const;
  bool more(litstr  v2) const;
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
   * Debugging functions.
   */
  static const char *getTypeString(DataType type);
  std::string getDebugDump() const;

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION_NOCALLBACKS(Variant);
  void dump();

  /**
   * Offset functions
   */
  Variant rvalAt(bool    offset, int64 prehash = -1) const {
    return rvalAt(offset ? 1LL : 0LL, prehash);
  }
  Variant rvalAt(char    offset, int64 prehash = -1) const {
    return rvalAt((int64)offset, prehash);
  }
  Variant rvalAt(short   offset, int64 prehash = -1) const {
    return rvalAt((int64)offset, prehash);
  }
  Variant rvalAt(int     offset, int64 prehash = -1) const {
    return rvalAt((int64)offset, prehash);
  }
  Variant rvalAt(int64   offset, int64 prehash = -1) const;
  Variant rvalAt(double  offset, int64 prehash = -1) const {
    return rvalAt((int64)offset, prehash);
  }
  Variant rvalAt(litstr  offset, int64 prehash = -1) const;
  Variant rvalAt(CStrRef offset, int64 prehash = -1) const;
  Variant rvalAt(CVarRef offset, int64 prehash = -1) const;

  const Variant operator[](bool    key) const { return rvalAt(key);}
  const Variant operator[](char    key) const { return rvalAt(key);}
  const Variant operator[](short   key) const { return rvalAt(key);}
  const Variant operator[](int     key) const { return rvalAt(key);}
  const Variant operator[](int64   key) const { return rvalAt(key);}
  const Variant operator[](double  key) const { return rvalAt(key);}
  const Variant operator[](litstr  key) const { return rvalAt(key);}
  const Variant operator[](CStrRef key) const { return rvalAt(key);}
  const Variant operator[](CArrRef key) const { return rvalAt(key);}
  const Variant operator[](CObjRef key) const { return rvalAt(key);}
  const Variant operator[](CVarRef key) const { return rvalAt(key);}

  /**
   * Called from VariantOffset for taking lval().
   */
  Variant &lval() {
    if (m_type == KindOfVariant) {
      return m_data.pvar->lval();
    }

    ASSERT(is(KindOfArray));
    Variant *ret = NULL;
    ArrayData *arr = m_data.parr;
    ArrayData *escalated = arr->lval(ret, arr->getCount() > 1);
    if (escalated) {
      set(escalated);
    }
    ASSERT(ret);
    return *ret;
  }

  template<typename T>
    Variant &lval(const T &key) {
    if (m_type == KindOfVariant) {
      return m_data.pvar->lval(key);
    }

    ASSERT(is(KindOfArray));
    Variant *ret = NULL;
    ArrayData *arr = m_data.parr;
    ArrayData *escalated = arr->lval(key, ret, arr->getCount() > 1);
    if (escalated) {
      set(escalated);
    }
    ASSERT(ret);
    return *ret;
  }

  Variant &lvalAt() {
    append(null);
    return lval();
  }

  static Variant &lvalInvalid();
  static Variant &lvalBlackHole();

  Variant &lvalAt(bool    key, int64 prehash = -1);
  Variant &lvalAt(char    key, int64 prehash = -1);
  Variant &lvalAt(short   key, int64 prehash = -1);
  Variant &lvalAt(int     key, int64 prehash = -1);
  Variant &lvalAt(int64   key, int64 prehash = -1);
  Variant &lvalAt(double  key, int64 prehash = -1);
  Variant &lvalAt(litstr  key, int64 prehash = -1);
  Variant &lvalAt(CStrRef key, int64 prehash = -1);
  Variant &lvalAt(CVarRef key, int64 prehash = -1);

  Variant refvalAt(bool    key, int64 prehash = -1);
  Variant refvalAt(char    key, int64 prehash = -1);
  Variant refvalAt(short   key, int64 prehash = -1);
  Variant refvalAt(int     key, int64 prehash = -1);
  Variant refvalAt(int64   key, int64 prehash = -1);
  Variant refvalAt(double  key, int64 prehash = -1);
  Variant refvalAt(litstr  key, int64 prehash = -1);
  Variant refvalAt(CStrRef key, int64 prehash = -1);
  Variant refvalAt(CVarRef key, int64 prehash = -1);

  Variant o_get(CStrRef propName, int64 prehash = -1) const;
  ObjectOffset o_lval(CStrRef propName, int64 prehash = -1);

  Variant o_invoke(const char *s, CArrRef params, int64 hash);
  Variant o_root_invoke(const char *s, CArrRef params, int64 hash);
  Variant o_invoke_ex(const char *clsname, const char *s,
                      CArrRef params, int64 hash);

  Variant o_invoke_few_args(const char *s, int64 hash, int count,
                            CVarRef a0 = null_variant,
                            CVarRef a1 = null_variant,
                            CVarRef a2 = null_variant
#if INVOKE_FEW_ARGS_COUNT > 3
                            ,CVarRef a3 = null_variant,
                            CVarRef a4 = null_variant,
                            CVarRef a5 = null_variant
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
                            ,CVarRef a6 = null_variant,
                            CVarRef a7 = null_variant,
                            CVarRef a8 = null_variant,
                            CVarRef a9 = null_variant
#endif
);
  Variant o_root_invoke_few_args(const char *s, int64 hash, int count,
                                 CVarRef a0 = null_variant,
                                 CVarRef a1 = null_variant,
                                 CVarRef a2 = null_variant
#if INVOKE_FEW_ARGS_COUNT > 3
                                 ,CVarRef a3 = null_variant,
                                 CVarRef a4 = null_variant,
                                 CVarRef a5 = null_variant
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
                                 ,CVarRef a6 = null_variant,
                                 CVarRef a7 = null_variant,
                                 CVarRef a8 = null_variant,
                                 CVarRef a9 = null_variant
#endif
);

  /**
   * The whole purpose of VariantOffset is to collect "v" parameter to call
   * this function.
   */
  CVarRef set(bool    key, CVarRef v, int64 prehash = -1) {
    return set(key ? 1LL : 0LL, v, prehash);
  }
  CVarRef set(char    key, CVarRef v, int64 prehash = -1) {
    return set((int64)key, v, prehash);
  }
  CVarRef set(short   key, CVarRef v, int64 prehash = -1) {
    return set((int64)key, v, prehash);
  }
  CVarRef set(int     key, CVarRef v, int64 prehash = -1) {
    return set((int64)key, v, prehash);
  }
  CVarRef set(int64   key, CVarRef v, int64 prehash = -1);
  CVarRef set(double  key, CVarRef v, int64 prehash = -1) {
    return set((int64)key, v, prehash);
  }
  CVarRef set(litstr  key, CVarRef v, int64 prehash = -1);
  CVarRef set(CStrRef key, CVarRef v, int64 prehash = -1);
  CVarRef set(CVarRef key, CVarRef v, int64 prehash = -1);

  CVarRef append(CVarRef v);

  template<typename T>
  void removeImpl(const T &key, int64 prehash) {
    switch (getType()) {
    case KindOfNull:
      break;
    case KindOfArray:
      {
        ArrayData *arr = getArrayData();
        if (arr) {
          ArrayData *escalated = arr->remove(key, (arr->getCount() > 1),
                                             prehash);
          if (escalated) {
            set(escalated);
          }
        }
      }
      break;
    case KindOfObject:
      callOffsetUnset(key);
      break;
    default:
      lvalInvalid();
      break;
    }
  }
  void remove(int64   key, int64 prehash = -1) { removeImpl(key, prehash);}
  void remove(litstr  key, int64 prehash = -1);
  void remove(CStrRef key, int64 prehash = -1);
  void remove(CVarRef key, int64 prehash = -1);

  // implemented in object_data.h
  template<typename T>
  void weakRemove(const T &key, int64 prehash = -1);

  /**
   * More array opeartions.
   */
  Variant pop();
  Variant dequeue();
  void insert(int pos, CVarRef v);

  /**
   * Position-based iterations.
   */
  Variant array_iter_reset();
  Variant array_iter_prev();
  Variant array_iter_current() const;
  Variant array_iter_next();
  Variant array_iter_end();
  Variant array_iter_key() const;
  Variant array_iter_value(ssize_t &pos) const;
  Variant array_iter_each();

  /**
   * Low level access that should be restricted to internal use.
   */
  litstr getLiteralString() const {
    ASSERT(getType() == LiteralString);
    return m_type == KindOfVariant ? m_data.pvar->m_data.str : m_data.str;
  }

  StringData *getStringData() const {
    ASSERT(getType() == KindOfString);
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
      shared->bind(*this);
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
  void setStatic() const;

 private:
  mutable DataType m_type;
  mutable union {
    int64        num;
    double       dbl;
    litstr       str;
    StringData  *pstr;
    ArrayData   *parr;
    ObjectData  *pobj;
    Variant     *pvar; // shared data between strongly bound Variants
  } m_data;

  void destruct();

  CVarRef set(bool    v);
  CVarRef set(char    v);
  CVarRef set(short   v);
  CVarRef set(int     v);
  CVarRef set(int64   v);
  CVarRef set(double  v);
  CVarRef set(litstr  v);
  CVarRef set(CStrRef v);
  CVarRef set(CArrRef v);
  CVarRef set(CObjRef v);
  CVarRef set(const ObjectOffset& v);
  CVarRef set(StringData  *v);
  CVarRef set(ArrayData   *v);
  CVarRef set(ObjectData  *v);
  void escalateString() const;

  template<typename T>
  CVarRef set(const SmartObject<T> &v) {
    return set(v.get());
  }

  CVarRef setAtImpl(int64 key, CVarRef v, int64 prehash);
  CVarRef setAtImpl(CStrRef key, CVarRef v, int64 prehash);
  CVarRef setAtImpl(CVarRef key, CVarRef v, int64 prehash);

  /**
   * When "deep" is true, if v is strongly bound, we will follow the link to
   * make a real copy of the data. This is the default mode for all cases.
   *
   * When "deep" is false, we will strongly bind to the same data. This is the
   * mode we use when we copy array elements or object members, because we
   * always make shallow copy of them when they are referenced.
   */
  void copy(CVarRef v, bool deep); // making a real copy of another Variant
  void bind2(CVarRef v); // internal helper function for bind
  void bind(CVarRef  v); // weakly binding a variable
  void bind(Variant *v); // strongly binding a variable
  void split();          // breaking weak binding by making a real copy

  // implemented in object_data.h
  template<typename T>
  Variant &lvalAtImpl(const T &key, int64 prehash = -1);
  template<typename T>
  Variant refvalAtImpl(const T &key, int64 prehash = -1);
};

///////////////////////////////////////////////////////////////////////////////

inline Variant operator+(char    n, CVarRef v) { return Variant(v) += n;}
inline Variant operator+(short   n, CVarRef v) { return Variant(v) += n;}
inline Variant operator+(int     n, CVarRef v) { return Variant(v) += n;}
inline Variant operator+(int64   n, CVarRef v) { return Variant(v) += n;}
inline Variant operator+(double  n, CVarRef v) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, char    n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, short   n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, int     n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, int64   n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, double  n) { return Variant(v) += n;}

inline Variant operator-(char    n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(short   n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(int     n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(int64   n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(double  n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(CVarRef v, char    n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, short   n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, int     n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, int64   n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, double  n) { return Variant(v) -= n;}

inline Variant operator*(char    n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(short   n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(int     n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(int64   n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(double  n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, char    n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, short   n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, int     n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, int64   n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, double  n) { return Variant(v) *= n;}

double operator/(double n, CVarRef v);
double operator/(int n, CVarRef v);
inline Variant operator/(CVarRef v, char    n) { return Variant(v) /= n;}
inline Variant operator/(CVarRef v, short   n) { return Variant(v) /= n;}
inline Variant operator/(CVarRef v, int     n) { return Variant(v) /= n;}
inline Variant operator/(CVarRef v, int64   n) { return Variant(v) /= n;}
inline Variant operator/(CVarRef v, double  n) { return Variant(v) /= n;}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VARIANT_H__
