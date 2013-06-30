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

#ifndef incl_HPHP_OBJECT_DATA_H_
#define incl_HPHP_OBJECT_DATA_H_

#include "hphp/runtime/base/util/countable.h"
#include "hphp/runtime/base/util/smart_ptr.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/system/systemlib.h"
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/int.hpp>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayIter;
class MutableArrayIter;

class HphpArray;
class TypedValue;
class PreClass;
class Class;

extern StaticString ssIterator;

/**
 * Base class of all PHP objects and PHP resources.
 *
 * 1. Properties:
 *    o_get() -> t___get() as fallback
 *    o_set() -> t___set() as fallback
 * 2. Methods:
 *    o_invoke() -> t___call() as fallback
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
    HasCall       = 0x0080, // defines __call
    HasCallStatic = 0x0100, // defines __callStatic
    CallToImpl    = 0x0200, // call o_to{Boolean,Int64,Double}Impl
    // The top 3 bits of o_attributes are reserved to indicate the
    // type of collection
    CollectionTypeAttrMask = (7 << 13),
    VectorAttrInit = (Collection::VectorType << 13),
    MapAttrInit = (Collection::MapType << 13),
    StableMapAttrInit = (Collection::StableMapType << 13),
    SetAttrInit = (Collection::SetType << 13),
    PairAttrInit = (Collection::PairType << 13),
  };

  enum {
    RealPropCreate = 1,    // Property should be created if it doesn't exist
    RealPropNoDynamic = 4, // Don't return dynamic properties
    RealPropUnchecked = 8, // Don't check property accessibility
    RealPropExist = 16,    // For property_exists
  };

  ObjectData(bool noId, Class* type)
      : o_attribute(0), m_cls(type) {
    assert(uintptr_t(this) % sizeof(TypedValue) == 0);
    if (!noId) {
      o_id = ++(*os_max_id);
    }
  }

  void setId(const ObjectData *r) { if (r) o_id = r->o_id; }

  virtual ~ObjectData(); // all PHP classes need virtual tables

  Class* getVMClass() const {
    return m_cls;
  }
  static size_t getVMClassOffset() {
    // For assembly linkage.
    return offsetof(ObjectData, m_cls);
  }
  static size_t attributeOff() { return offsetof(ObjectData, o_attribute); }
  bool instanceof(const Class* c) const;

  bool isCollection() const {
    return getCollectionType() != Collection::InvalidType;
  }
  Collection::Type getCollectionType() const {
    // Return the upper 3 bits of o_attribute
    return (Collection::Type)((uint16_t)(o_attribute >> 13) & 7);
  }

  bool implementsIterator() {
    return (instanceof(SystemLib::s_IteratorClass));
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
   * o_instanceof() can be used for both classes and interfaces.
   * It is also worth noting that o_instanceof will always return
   * false for classes that are descendents of ResourceData.
   */
  bool o_instanceof(CStrRef s) const;

  // class info
  CStrRef o_getClassName() const;
  CStrRef o_getParentName() const;
  virtual CStrRef o_getClassNameHook() const;
  virtual bool isResource() const { return false; }
  int o_getId() const { return o_id;}

  bool o_toBoolean() const {
    if (getAttribute(CallToImpl)) {
      return o_toBooleanImpl();
    }
    return true;
  }

  static void raiseObjToIntNotice(const char*);

  int64_t o_toInt64() const {
    if (getAttribute(CallToImpl)) {
      return o_toInt64Impl();
    }
    raiseObjToIntNotice(o_getClassName().data());
    return 1;
  }

  double o_toDouble() const {
    if (getAttribute(CallToImpl)) {
      return o_toDoubleImpl();
    }
    return o_toInt64();
  }

  // overridable casting
  virtual bool o_toBooleanImpl() const noexcept;
  virtual int64_t o_toInt64Impl() const noexcept;
  virtual double o_toDoubleImpl() const noexcept;

  void destruct();

  // properties
  virtual Array o_toArray() const;
  Array o_toIterArray(CStrRef context, bool getRef = false);

  Array o_getDynamicProperties() const {
    return o_properties;
  }

  Variant* o_realProp(CStrRef s, int flags,
                      CStrRef context = null_string) const;

  Variant o_get(CStrRef s, bool error = true,
                CStrRef context = null_string);

  Variant o_set(CStrRef s, CVarRef v);
  Variant o_set(CStrRef s, RefResult v);
  Variant o_setRef(CStrRef s, CVarRef v);

  Variant o_set(CStrRef s, CVarRef v, CStrRef context);
  Variant o_set(CStrRef s, RefResult v, CStrRef context);
  Variant o_setRef(CStrRef s, CVarRef v, CStrRef context);

  void o_setArray(CArrRef properties);
  void o_getArray(Array &props, bool pubOnly = false) const;

  static Object FromArray(ArrayData *properties);

  // method invocation with CStrRef
  Variant o_invoke(CStrRef s, CArrRef params, bool fatal = true);
  Variant o_invoke_few_args(CStrRef s, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);

  // misc
  void serialize(VariableSerializer *serializer) const;
  virtual void serializeImpl(VariableSerializer *serializer) const;
  bool hasInternalReference(PointerSet &vars, bool ds = false) const;
  virtual void dump() const;
  virtual ObjectData *clone();

  Variant offsetGet(Variant key);

  // magic methods
  // __construct is handled in a special way
  virtual Variant t___destruct();
  virtual Variant t___call(Variant v_name, Variant v_arguments);
  virtual Variant t___set(Variant v_name, Variant v_value);
  virtual Variant t___get(Variant v_name);
  virtual bool t___isset(Variant v_name);
  virtual Variant t___unset(Variant v_name);
  virtual Variant t___sleep();
  virtual Variant t___wakeup();
  virtual String t___tostring();
  virtual Variant t___clone();

  static int GetMaxId() ATTRIBUTE_COLD;
 protected:
  virtual bool php_sleep(Variant &ret);
 public:
  CArrRef getProperties() const { return o_properties; }
  void initProperties(int nProp);
 private:
  static DECLARE_THREAD_LOCAL_NO_CHECK(int, os_max_id);
  ObjectData(const ObjectData &) { assert(false);}
  inline Variant o_getImpl(CStrRef propName, int flags,
                           bool error = true, CStrRef context = null_string);
  template <typename T>
  inline Variant o_setImpl(CStrRef propName, T v,
                           bool forInit, CStrRef context);
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
  Class* m_cls;

 protected:
  ArrNR         o_properties;    // dynamic properties (VM and hphpc)
  int           o_id;            // a numeric identifier of this object

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

 public:
  void getChildren(std::vector<TypedValue *> &out) {
    ArrayData *props = o_properties.get();
    if (props) {
      props->getChildren(out);
    }
  }
} __attribute__((aligned(16)));

template<> inline SmartPtr<ObjectData>::~SmartPtr() {}

typedef GlobalNameValueTableWrapper GlobalVariables;

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

void deepInitHelper(TypedValue* propVec, const TypedValueAux* propData,
                    size_t nProps);

class Instance : public ObjectData {
  // Do not declare any fields directly in Instance; instead embed them in
  // ObjectData, so that a property vector can always reside immediately past
  // the end of an object.

 private:
  // This constructor is used for all pure classes that are not
  // descendents of cppext classes
  explicit Instance(Class* cls) : ObjectData(false, cls) {
    instanceInit(cls);
  }

  enum class NoInit { noinit };
  explicit Instance(Class* cls, NoInit) : ObjectData(false, cls) {}

 public:
  // This constructor is used for all cppext classes (including resources)
  // and their descendents.
  Instance(Class* cls, bool isResource)
      : ObjectData(isResource, cls) {
    instanceInit(cls);
  }

  virtual ~Instance() {}

  static int ObjAllocatorSizeClassCount;

  static void raiseAbstractClassError(Class* cls);

  // Call newInstance() to instantiate an Instance
  static Instance* newInstance(Class* cls) {
    if (cls->m_InstanceCtor) {
      return cls->m_InstanceCtor(cls);
    }
    Attr attrs = cls->attrs();
    if (UNLIKELY(attrs & (AttrAbstract | AttrInterface | AttrTrait))) {
      raiseAbstractClassError(cls);
    }
    size_t nProps = cls->numDeclProperties();
    size_t size = sizeForNProps(nProps);
    Instance* obj = (Instance*)ALLOCOBJSZ(size);
    new (obj) Instance(cls);
    if (UNLIKELY(cls->callsCustomInstanceInit())) {
      /*
        This must happen after the constructor finishes,
        because it can leak references to obj AND it can
        throw exceptions. If we have this in the Instance
        constructor, and it throws, obj will be partially
        destroyed (ie ~ObjectData will be called, resetting
        the vtable pointer) leaving dangling references
        to the object (eg in backtraces).
      */
      obj->callCustomInstanceInit();
    }
    return obj;
  }

  // Given a Class that is assumed to be a concrete, regular (not a
  // trait or interface), pure PHP class, and an allocator index,
  // return a new, uninitialized object of that class.
  static Instance* newInstanceRaw(Class* cls, int idx);

 private:
  void instanceInit(Class* cls) {
    setAttributes(cls->getODAttrs());
    size_t nProps = cls->numDeclProperties();
    if (cls->needInitialization()) {
      cls->initialize();
    }
    if (nProps > 0) {
      if (cls->pinitVec().size() > 0) {
        const Class::PropInitVec* propInitVec = m_cls->getPropData();
        assert(propInitVec != nullptr);
        assert(nProps == propInitVec->size());
        if (!cls->hasDeepInitProps()) {
          memcpy(propVec(), &(*propInitVec)[0], nProps * sizeof(TypedValue));
        } else {
          deepInitHelper(propVec(), &(*propInitVec)[0], nProps);
        }
      } else {
        assert(nProps == cls->declPropInit().size());
        memcpy(propVec(), &cls->declPropInit()[0], nProps * sizeof(TypedValue));
      }
    }
  }

 protected:
  TypedValue* propVec();
  const TypedValue* propVec() const;

 public:
  Instance* callCustomInstanceInit();

  void operator delete(void* p);

  //============================================================================
  // Virtual ObjectData methods that we need to override

 public:
  virtual Variant t___destruct();
  virtual Variant t___call(Variant v_name, Variant v_arguments);
  virtual Variant t___set(Variant v_name, Variant v_value);
  virtual Variant t___get(Variant v_name);
  virtual bool t___isset(Variant v_name);
  virtual Variant t___unset(Variant v_name);
  virtual Variant t___sleep();
  virtual Variant t___wakeup();
  virtual Variant t___set_state(Variant v_properties);
  virtual String t___tostring();
  virtual Variant t___clone();

  //============================================================================
  // Miscellaneous.

  void cloneSet(ObjectData* clone);
  ObjectData* cloneImpl();

  void invokeUserMethod(TypedValue* retval, const Func* method,
                        CArrRef params);

  const Func* methodNamed(const StringData* sd) const {
    return getVMClass()->lookupMethod(sd);
  }

  static size_t sizeForNProps(Slot nProps) {
    size_t sz = sizeof(Instance) + (sizeof(TypedValue) * nProps);
    assert((sz & (sizeof(TypedValue) - 1)) == 0);
    return sz;
  }

  static Object FromArray(ArrayData *properties);

  //============================================================================
  // Properties.
 public:
  int builtinPropSize() const {
    return m_cls->builtinPropSize();
  }

  // public for ObjectData access
  void initDynProps(int numDynamic = 0);
  Slot declPropInd(TypedValue* prop) const;
 private:
  template <bool declOnly>
  TypedValue* getPropImpl(Class* ctx, const StringData* key, bool& visible,
                          bool& accessible, bool& unset);
 public:
  TypedValue* getProp(Class* ctx, const StringData* key, bool& visible,
                      bool& accessible, bool& unset);
  TypedValue* getDeclProp(Class* ctx, const StringData* key, bool& visible,
                          bool& accessible, bool& unset);
 private:
  template <bool warn, bool define>
  void propImpl(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
                const StringData* key);
  void invokeSet(TypedValue* retval, const StringData* key, TypedValue* val);
  void invokeGet(TypedValue* retval, const StringData* key);
  void invokeGetProp(TypedValue*& retval, TypedValue& tvRef,
                     const StringData* key);
  void invokeIsset(TypedValue* retval, const StringData* key);
  void invokeUnset(TypedValue* retval, const StringData* key);
  void getProp(const Class* klass, bool pubOnly, const PreClass::Prop* prop,
               Array& props, std::vector<bool>& inserted) const;
  void getProps(const Class* klass, bool pubOnly, const PreClass* pc,
                Array& props, std::vector<bool>& inserted) const;
 public:
  void prop(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
            const StringData* key);
  void propD(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
             const StringData* key);
  void propW(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
             const StringData* key);
  void propWD(TypedValue*& retval, TypedValue& tvRef, Class* ctx,
              const StringData* key);
  bool propIsset(Class* ctx, const StringData* key);
  bool propEmpty(Class* ctx, const StringData* key);

  TypedValue* setProp(Class* ctx, const StringData* key, TypedValue* val,
                      bool bindingAssignment = false);
  TypedValue* setOpProp(TypedValue& tvRef, Class* ctx, unsigned char op,
                        const StringData* key, Cell* val);
 private:
  template <bool setResult>
  void incDecPropImpl(TypedValue& tvRef, Class* ctx, unsigned char op,
                      const StringData* key, TypedValue& dest);
 public:
  template <bool setResult>
  void incDecProp(TypedValue& tvRef, Class* ctx, unsigned char op,
                  const StringData* key, TypedValue& dest);
  void unsetProp(Class* ctx, const StringData* key);

  void raiseUndefProp(const StringData* name);

  friend class ObjectData;
};

inline Instance* instanceFromTv(TypedValue* tv) {
  assert(dynamic_cast<Instance*>(tv->m_data.pobj));
  return static_cast<Instance*>(tv->m_data.pobj);
}

class ExtObjectData : public HPHP::Instance {
 public:
  explicit ExtObjectData(HPHP::Class* cls)
    : HPHP::Instance(cls, false) {
    assert(!m_cls->callsCustomInstanceInit());
  }
};

template <int flags> class ExtObjectDataFlags : public ExtObjectData {
 public:
  explicit ExtObjectDataFlags(HPHP::Class* cb) : ExtObjectData(cb) {
    ObjectData::setAttributes(flags);
  }
};

} // HPHP

#endif
