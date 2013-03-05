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

#ifndef __HPHP_OBJECT_DATA_H__
#define __HPHP_OBJECT_DATA_H__

#include <runtime/base/util/countable.h>
#include <runtime/base/util/smart_ptr.h>
#include <runtime/base/types.h>
#include <runtime/base/macros.h>
#include <runtime/base/runtime_error.h>

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/int.hpp>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayIter;
class MutableArrayIter;

class HphpArray;
class TypedValue;
namespace VM {
  class PreClass;
  class Class;
}

extern StaticString ssIterator;

/**
 * Base class of all user-defined classes. All data members and methods in
 * this class should start with "o_" or "os_" to avoid name conflicts.
 *
 * Calling sequence:
 *
 * 1. Statically resolved properties and methods will be statically called.
 * 2. Dynamic properties:
 *    o_get() -> t___get() as fallback
 *    o_lval() -> t___get() as fallback (!really)
 *    o_set() -> t___set() as fallback
 * 3. Dynamic methods:
 *    o_invoke() -> t___call() as fallback
 * 4. Auto-generated jump-tables:
 *    o_realProp()
 *    o_realPropPublic()
 *    o_realPropPrivate() # non-virtual, only as needed
 */
class ObjectData : public CountableNF {
 public:
  enum Attribute {
    NoDestructor  = 0x0001, // __destruct()
    HasSleep      = 0x0002, // __sleep()
    UseSet        = 0x0004, // __set()
    UseGet        = 0x0008, // __get()
    UseIsset      = 0x0010, // __isset()
    UseUnset      = 0x0020, // __unset()
    HasLval       = 0x0040, // defines ___lval
    HasCall       = 0x0080, // defines __call
    HasCallStatic = 0x0100, // defines __callStatic
    // The top 3 bits of o_attributes are reserved to indicate the
    // type of collection
    CollectionTypeAttrMask = (7 << 13),
    VectorAttrInit = (Collection::VectorType << 13),
    MapAttrInit = (Collection::MapType << 13),
    StableMapAttrInit = (Collection::StableMapType << 13),
  };

  enum {
    RealPropCreate = 1,   // Property should be created if it doesnt exist
    RealPropWrite = 2,    // Property could be modified
    RealPropNoDynamic = 4,// Dont return dynamic properties
    RealPropUnchecked = 8,// Dont check property accessibility
    RealPropExist = 16,   // For property_exists
  };

  ObjectData(bool noId, VM::Class* type)
      : o_attribute(0), m_cls(type) {
    assert(!hhvm || uintptr_t(this) % sizeof(TypedValue) == 0);
    if (!noId) {
      o_id = ++(*os_max_id);
    }
  }

  void setId(const ObjectData *r) { if (r) o_id = r->o_id; }

  virtual ~ObjectData(); // all PHP classes need virtual tables

  HPHP::VM::Class* getVMClass() const {
    return m_cls;
  }
  static size_t getVMClassOffset() {
    // For assembly linkage.
    return offsetof(ObjectData, m_cls);
  }
  static size_t attributeOff() { return offsetof(ObjectData, o_attribute); }
  HPHP::VM::Class* instanceof(const HPHP::VM::PreClass* pc) const;
  bool instanceof(const HPHP::VM::Class* c) const;

  bool isCollection() const {
    return getCollectionType() != Collection::InvalidType;
  }
  int getCollectionType() const {
    // Return the upper 3 bits of o_attribute
    return (int)(o_attribute >> 13) & 7;
  }
  bool supportsUnsetElem() const {
    return isCollection() || o_instanceof("arrayaccess");
  }

  bool implementsIterator() {
    return (o_instanceof(ssIterator));
  }

  void setAttributes(int attrs) { o_attribute |= attrs; }
  void setAttributes(const ObjectData *o) { o_attribute |= o->o_attribute; }
  bool getAttribute(Attribute attr) const { return o_attribute & attr; }
  void setAttribute(Attribute attr) const { o_attribute |= attr;}
  void clearAttribute(Attribute attr) const { o_attribute &= ~attr;}
  bool noDestruct() const { return getAttribute(NoDestructor); }
  void setNoDestruct() { setAttribute(NoDestructor); }
  ObjectData *clearNoDestruct() { clearAttribute(NoDestructor); return this; }

  Object iterableObject(bool& isIterable, bool mayImplementIterator = true);
  ArrayIter begin(CStrRef context = null_string);
  MutableArrayIter begin(Variant *key, Variant &val,
                         CStrRef context = null_string);

  /**
   * o_instanceof() can be used for both classes and interfaces. Note
   * that o_instanceof() has not been thoroughly tested with redeclared
   * classes or classes that have a redeclared ancestor in the inheritance
   * hierarchy. It is also worth noting that o_instanceof will always return
   * false for classes that are descendents of ResourceData.
   */
  bool o_instanceof(CStrRef s) const;
  virtual ObjectData *getRedeclaredParent() const { return 0; }

  // class info
  CStrRef o_getClassName() const;
  CStrRef o_getParentName() const;
  virtual CStrRef o_getClassNameHook() const;
  static CStrRef GetParentName(CStrRef cls);
  virtual bool isResource() const { return false;}
  bool o_isClass(const char *s) const;
  int o_getId() const { return o_id;}

  // overridable casting
  virtual bool   o_toBoolean() const { return true;}
  virtual int64_t  o_toInt64() const;
  virtual double o_toDouble()  const { return o_toInt64();}

  void setDummy();

  virtual void init() {}
  virtual void destruct();

  // properties
  virtual Array o_toArray() const;
  virtual Array o_toIterArray(CStrRef context, bool getRef = false);
  virtual Array o_getDynamicProperties() const;
  Variant *o_realProp(CStrRef s, int flags,
                      CStrRef context = null_string) const;
  void *o_realPropTyped(CStrRef s, int flags,
                        CStrRef context, DataType* type) const;
  Variant *o_realPropPublic(CStrRef s, int flags) const;
  bool o_exists(CStrRef s, CStrRef context = null_string) const;
  Variant o_get(CStrRef s, bool error = true,
                CStrRef context = null_string);
  Variant o_getPublic(CStrRef s, bool error = true);
  Variant o_getUnchecked(CStrRef s, CStrRef context = null_string);

  Variant o_i_set(CStrRef s, CVarRef v);
  Variant o_i_setPublicWithRef(CStrRef s, CVarRef v);

  Variant o_set(CStrRef s, CVarRef v);
  Variant o_set(CStrRef s, RefResult v);
  Variant o_setRef(CStrRef s, CVarRef v);

  Variant o_set(CStrRef s, CVarRef v, CStrRef context);
  Variant o_set(CStrRef s, RefResult v, CStrRef context);
  Variant o_setRef(CStrRef s, CVarRef v, CStrRef context);
  Variant o_setPublic(CStrRef s, CVarRef v);
  Variant o_setPublic(CStrRef s, RefResult v);
  Variant o_setPublicRef(CStrRef s, CVarRef v);
  Variant o_setPublicWithRef(CStrRef s, CVarRef v);

  Variant &o_lval(CStrRef s, CVarRef tmpForGet, CStrRef context = null_string);
  Variant &o_unsetLval(CStrRef s, CVarRef tmpForGet,
                       CStrRef context = null_string) {
    return o_lval(s, tmpForGet, context);
  }
  Variant *o_weakLval(CStrRef s, CStrRef context = null_string);

  virtual void o_setArray(CArrRef properties);

  virtual void o_getArray(Array &props, bool pubOnly = false) const;
  void o_set(const Array properties);
  Variant o_argval(bool byRef, CStrRef s, bool error = true,
      CStrRef context = null_string);

  virtual Variant o_getError(CStrRef prop, CStrRef context);
  virtual Variant o_setError(CStrRef prop, CStrRef context);
  /**
   * This is different from o_exists(), which is isset() semantics. This one
   * is property_exists() semantics. ie, accessibility is ignored, and declared
   * but unset properties still return true.
   */
  bool o_propExists(CStrRef s, CStrRef context = null_string);

  /**
   * This is different from o_exists(), which is isset() semantics; this one
   * returns true even if the property is set to null.
   */
  bool o_propForIteration(CStrRef s, CStrRef context = null_string);

  static Object FromArray(ArrayData *properties);

  CVarRef set(CStrRef s, CVarRef v);

  // methods
  Variant o_invoke_ex(CStrRef clsname, CStrRef s,
                      CArrRef params, bool fatal = true);

  // method invocation with CStrRef
  Variant o_invoke(CStrRef s, CArrRef params, strhash_t hash = -1,
                   bool fatal = true);
  Variant o_invoke_few_args(CStrRef s, strhash_t hash, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);

  // misc
  Variant o_throw_fatal(const char *msg);
  void serialize(VariableSerializer *serializer) const;
  virtual void serializeImpl(VariableSerializer *serializer) const;
  bool hasInternalReference(PointerSet &vars, bool ds = false) const;
  virtual void dump() const;
  virtual ObjectData *clone();

  bool o_isset(CStrRef prop, CStrRef context = null_string);
  bool o_empty(CStrRef prop, CStrRef context = null_string);
  void o_unset(CStrRef prop, CStrRef context = null_string);

  // magic methods
  // __construct is handled in a special way
  virtual Variant t___destruct();
  virtual Variant t___call(Variant v_name, Variant v_arguments);
  virtual Variant t___set(Variant v_name, Variant v_value);
  virtual Variant t___get(Variant v_name);
  virtual Variant *___lval(Variant v_name);
  virtual Variant &___offsetget_lval(Variant key);
  virtual bool t___isset(Variant v_name);
  virtual Variant t___unset(Variant v_name);
  virtual Variant t___sleep();
  virtual Variant t___wakeup();
  virtual String t___tostring();
  virtual Variant t___clone();

  template<typename T, int op>
  T o_assign_op(CStrRef propName, CVarRef val, CStrRef context = null_string);

  static int GetMaxId() ATTRIBUTE_COLD;
 protected:
  virtual bool php_sleep(Variant &ret);
public:
  CArrRef getProperties() const { return o_properties; }
  void initProperties(int nProp);
 private:
  ObjectData(const ObjectData &) { assert(false);}
  inline Variant o_getImpl(CStrRef propName, int flags,
                           bool error = true, CStrRef context = null_string);
  static DECLARE_THREAD_LOCAL_NO_CHECK(int, os_max_id);
  template <typename T>
  inline Variant o_setImpl(CStrRef propName, T v,
                           bool forInit, CStrRef context);
  template <typename T>
  inline Variant o_setPublicImpl(CStrRef propName, T v, bool forInit);
 public:
  static const bool IsResourceClass = false;

  // this will be hopefully packed together with _count from parent class
 private:
  mutable int16_t o_attribute;     // various flags
 protected:
  // 16 bits of unused memory that can be reused by subclasses
  union {
    uint16_t u16;
    uint8_t u8[2];

  } o_subclassData;

 protected:
  HPHP::VM::Class* m_cls;

 protected:
  ArrNR         o_properties;    // dynamic properties (VM and hphpc)
  int           o_id;            // a numeric identifier of this object

 protected:
  void          cloneDynamic(ObjectData *orig);

 private:
  static void compileTimeAssertions() {
    static_assert(offsetof(ObjectData, _count) == FAST_REFCOUNT_OFFSET,
                  "Offset of ObjectData._count must be FAST_REFCOUNT_OFFSET");
  }

 public:
  void release() {
    assert(getCount() == 0);
    destruct();
    if (UNLIKELY(getCount() != 0)) {
      // Object was resurrected.
      return;
    }
    delete this;
  }
} __attribute__((aligned(16)));

template<> inline SmartPtr<ObjectData>::~SmartPtr() {}

typedef VM::GlobalNameValueTableWrapper GlobalVariables;

///////////////////////////////////////////////////////////////////////////////
// Calculate item sizes for object allocators

#define WORD_SIZE sizeof(TypedValue)
#define ALIGN_WORD(n) ((n) + (WORD_SIZE - (n) % WORD_SIZE) % WORD_SIZE)

// Mapping from index to size class for objects.  Mapping in the other
// direction is available from ObjectSizeClass<> below.
template<int Idx> class ObjectSizeTable {
  enum { prevSize = ObjectSizeTable<Idx - 1>::value };
public:
  enum {
    value = ALIGN_WORD(prevSize + (prevSize >> 1))
  };
};

template<> struct ObjectSizeTable<0> {
  enum { value = sizeof(ObjectData) };
};

#undef WORD_SIZE
#undef ALIGN_WORD

/*
 * This determines the highest size class we can have by looking for
 * the first entry in our table that is larger than the hard coded
 * SmartAllocator SLAB_SIZE.  This is because you can't (currently)
 * SmartAllocate chunks that are potentially bigger than a slab. If
 * you introduce a bigger size class, SmartAllocator will hit an
 * assertion at runtime.  The last size class currently goes up to
 * 97096 bytes -- enough room for 6064 TypedValues. Hopefully that's
 * enough.
 */
template<int Index>
struct DetermineLargestSizeClass {
  typedef typename boost::mpl::eval_if_c<
    (ObjectSizeTable<Index>::value > SLAB_SIZE),
    boost::mpl::int_<Index>,
    DetermineLargestSizeClass<Index + 1>
  >::type type;
};
const int NumObjectSizeClasses = DetermineLargestSizeClass<0>::type::value;

template<size_t Sz, int Index> struct LookupObjSizeIndex {
  enum { index =
    Sz <= ObjectSizeTable<Index>::value
      ? Index : LookupObjSizeIndex<Sz,Index + 1>::index };
};
template<size_t Sz> struct LookupObjSizeIndex<Sz,NumObjectSizeClasses> {
  enum { index = NumObjectSizeClasses };
};

template<size_t Sz>
struct ObjectSizeClass {
  enum {
    index = LookupObjSizeIndex<Sz,0>::index,
    value = ObjectSizeTable<index>::value
  };
};

typedef ObjectAllocatorBase *(*ObjectAllocatorBaseGetter)(void);

class ObjectAllocatorCollector {
public:
  static std::map<int, ObjectAllocatorBaseGetter> &getWrappers() {
    static std::map<int, ObjectAllocatorBaseGetter> wrappers;
    return wrappers;
  }
};

template <typename T>
void *ObjectAllocatorInitSetup() {
  ThreadLocalSingleton<ObjectAllocator<
    ObjectSizeClass<sizeof(T)>::value> > tls;
  int index = ObjectSizeClass<sizeof(T)>::index;
  ObjectAllocatorCollector::getWrappers()[index] =
    (ObjectAllocatorBaseGetter)tls.getCheck;
  GetAllocatorInitList().insert((AllocatorThreadLocalInit)(tls.getCheck));
  return (void*)tls.getNoCheck;
}

/*
 * Return the index in ThreadInfo::m_allocators for the allocator
 * responsible for a given object size.
 *
 * There is a maximum limit on the size of allocatable objects.  If
 * this is reached, this function returns -1.
 */
int object_alloc_size_to_index(size_t size);

///////////////////////////////////////////////////////////////////////////////
// Attribute helpers
class AttributeSetter {
public:
  AttributeSetter(ObjectData::Attribute a, ObjectData *o) : m_a(a), m_o(o) {
    o->setAttribute(a);
  }
  ~AttributeSetter() {
    m_o->clearAttribute(m_a);
  }
private:
  ObjectData::Attribute m_a;
  ObjectData *m_o;
};

class AttributeClearer {
public:
  AttributeClearer(ObjectData::Attribute a, ObjectData *o) : m_a(a), m_o(o) {
    o->clearAttribute(a);
  }
  ~AttributeClearer() {
    m_o->setAttribute(m_a);
  }
private:
  ObjectData::Attribute m_a;
  ObjectData *m_o;
};

ALWAYS_INLINE inline void decRefObj(ObjectData* obj) {
  if (obj->decRefCount() == 0) obj->release();
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_OBJECT_DATA_H__
