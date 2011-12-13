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
#include <runtime/base/memory/unsafe_pointer.h>
#include <runtime/base/macros.h>
#include <runtime/base/runtime_error.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Needed for eval
namespace Eval {
class MethodStatement;
class FunctionCallExpression;
class VariableEnvironment;
}

class ArrayIter;
class MutableArrayIter;
class ClassPropTable;
struct ObjectStaticCallbacks;
struct MethodCallInfoTable;

class HphpArray;
class TypedValue;
namespace VM {
  class PreClass;
  class Class;
}

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
    InConstructor = 0x0001, // __construct()
    InDestructor  = 0x0002, // __destruct()
    HasSleep      = 0x0004, // __sleep()
    UseSet        = 0x0008, // __set()
    UseGet        = 0x0010, // __get()
    UseIsset      = 0x0020, // __isset()
    UseUnset      = 0x0040, // __unset()
    HasLval       = 0x0080, // defines ___lval
    HasCall       = 0x0100, // defines __call
    HasCallStatic = 0x0200, // defines __callStatic
    IsInstance    = 0x0400  // isInstance()
  };
  enum {
    RealPropCreate = 1,   // Property should be created if it doesnt exist
    RealPropWrite = 2,    // Property could be modified
    RealPropNoDynamic = 4,// Dont return dynamic properties
    RealPropUnchecked = 8,// Dont check property accessibility
  };

  ObjectData(const ObjectStaticCallbacks *cb = NULL, bool isResource = false,
             VM::Class* type = NULL)
      : o_attribute(0), o_callbacks(cb)
#ifdef HHVM
        , m_cls(type), m_propMap(NULL), m_propVec(NULL)
#endif
        {
    if (!isResource) {
      o_id = ++(*os_max_id);
    }
  }

  virtual ~ObjectData(); // all PHP classes need virtual tables

  HPHP::VM::Class* getVMClass() const {
    const_assert(hhvm);
    return m_cls;
  }
  static size_t getVMClassOffset() {
    // For assembly linkage.
    const_assert(hhvm);
    return offsetof(ObjectData, m_cls);
  }
  HPHP::VM::Class* instanceof(const HPHP::VM::PreClass* pc) const;
  bool instanceof(const HPHP::VM::Class* c) const;

  void setAttributes(int attrs) { o_attribute |= attrs; }
  void setAttributes(const ObjectData *o) { o_attribute |= o->o_attribute; }
  bool getAttribute(Attribute attr) const { return o_attribute & attr; }
  void setAttribute(Attribute attr) const { o_attribute |= attr;}
  void clearAttribute(Attribute attr) const { o_attribute &= ~attr;}
  bool inDtor() { return getAttribute(InDestructor); }
  bool inCtor() { return getAttribute(InConstructor); }
  bool inCtorDtor() { return inCtor() || inDtor(); }
  void setInDtor() { setAttribute(InDestructor); }
  bool gasInCtor(bool inCtor) { // get and set InConstructor
    bool oldInCtor = getAttribute(InConstructor);
    if (inCtor) {
      setAttribute(InConstructor);
    } else {
      clearAttribute(InConstructor);
    }
    return oldInCtor;
  }

  Object iterableObject(bool& isInstanceofIterator);
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
  virtual bool o_instanceof_hook(CStrRef s) const;
  virtual ObjectData *getRedeclaredParent() const { return 0; }

  // class info
#ifdef HHVM
  virtual
#endif
  CStrRef o_getClassName() const;
#ifdef HHVM
  virtual
#endif
  CStrRef o_getParentName() const;
  virtual CStrRef o_getClassNameHook() const;
  static CStrRef GetParentName(CStrRef cls);
  static CallInfo *GetCallHandler();
  virtual bool isResource() const { return false;}
  bool o_isClass(const char *s) const;
  int o_getId() const { return o_id;}

  // overridable casting
  virtual bool   o_toBoolean() const { return true;}
  virtual int64  o_toInt64() const;
  virtual double o_toDouble()  const { return o_toInt64();}

  template<typename T>
  T *bindClass(ThreadInfo *info) {
    bindThis(info);
    return static_cast<T*>(this);
  }
  void bindThis(ThreadInfo *info);

  void setDummy();
  static Variant ifa_dummy(MethodCallPackage &mcp, int count,
                           INVOKE_FEW_ARGS_IMPL_ARGS,
                           Variant (*ifa)(MethodCallPackage &mcp, int count,
                                          INVOKE_FEW_ARGS_IMPL_ARGS),
                           ObjectData *(*coo)(ObjectData*));
  static Variant i_dummy(MethodCallPackage &mcp, CArrRef params,
                         Variant (*i)(MethodCallPackage &mcp,
                                      CArrRef params),
                         ObjectData *(*coo)(ObjectData*));
  static Variant ifa_dummy(MethodCallPackage &mcp, int count,
                           INVOKE_FEW_ARGS_IMPL_ARGS,
                           Variant (*ifa)(MethodCallPackage &mcp, int count,
                                          INVOKE_FEW_ARGS_IMPL_ARGS),
                           ObjectData *(*coo)());
  static Variant i_dummy(MethodCallPackage &mcp, CArrRef params,
                         Variant (*i)(MethodCallPackage &mcp,
                                      CArrRef params),
                         ObjectData *(*coo)());

  virtual void init() {}
  ObjectData *create() { CountableHelper h(this); init(); return this;}
  virtual void getConstructor(MethodCallPackage &mcp);
  void release();
  virtual void destruct();
  virtual void destructNoThrow();

  virtual const Eval::MethodStatement* getConstructorStatement() const;
  virtual const Eval::MethodStatement* getMethodStatement(CStrRef name,
    int &access) const;

  static Variant os_invoke(CStrRef c, CStrRef s,
                           CArrRef params, int64 hash, bool fatal = true);

  // properties
  virtual Array o_toArray() const;
  virtual Array o_toIterArray(CStrRef context, bool getRef = false);
  virtual Array o_getDynamicProperties() const;
  Variant *o_realProp(CStrRef s, int flags,
                      CStrRef context = null_string) const;
  Variant *o_realPropPublic(CStrRef s, int flags) const;
  virtual Variant *o_realPropHook(CStrRef s, int flags,
                                  CStrRef context = null_string) const;
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
  const ClassPropTable *o_getClassPropTable() const;
  void o_set(const Array properties);
  Variant o_argval(bool byRef, CStrRef s, bool error = true,
      CStrRef context = null_string);

  virtual Variant o_getError(CStrRef prop, CStrRef context);
  virtual Variant o_setError(CStrRef prop, CStrRef context);
  /**
   * This is different from o_exists(), which is isset() semantics. This one
   * is property_exists() semantics that check whether it was unset before.
   * This is used for deciding what property_exists() returns and whether or
   * not this property should be part of an iteration in foreach ($obj as ...)
   */
  bool o_propExists(CStrRef s, CStrRef context = null_string);

  static Object FromArray(ArrayData *properties);

  CVarRef set(CStrRef s, CVarRef v);

  // methods
  Variant o_invoke_ex(CStrRef clsname, CStrRef s,
                      CArrRef params, bool fatal = true);

  // method invocation with CStrRef
  Variant o_invoke(CStrRef s, CArrRef params, int64 hash = -1,
                   bool fatal = true);
  Variant o_root_invoke(CStrRef s, CArrRef params, int64 hash = -1,
                        bool fatal = false);
  Variant o_invoke_few_args(CStrRef s, int64 hash, int count,
                            INVOKE_FEW_ARGS_DECL_ARGS);
  Variant o_root_invoke_few_args(CStrRef s, int64 hash, int count,
                                 INVOKE_FEW_ARGS_DECL_ARGS);
  bool o_get_call_info(MethodCallPackage &mcp, int64 hash = -1);
  const ObjectStaticCallbacks *o_get_callbacks() const {
    return o_callbacks;
  }
  bool o_get_call_info_ex(const char *clsname,
                          MethodCallPackage &mcp, int64 hash = -1);
  virtual bool o_get_call_info_hook(const char *clsname,
                                    MethodCallPackage &mcp, int64 hash = -1);

  // misc
  Variant o_throw_fatal(const char *msg);
  virtual void serialize(VariableSerializer *serializer) const;
  virtual void dump() const;
  virtual ObjectData *clone();
  virtual void setRoot(ObjectData *root) {}
  virtual ObjectData *getRoot();
  /**
   * If __call is defined, then this gets overridden to call it.
   * Otherwise, it just throws if fatal else returns false.
   */
  virtual Variant doCall(Variant v_name, Variant v_arguments, bool fatal);
  virtual Variant doRootCall(Variant v_name, Variant v_arguments, bool fatal);

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
  virtual Variant &___offsetget_lval(Variant v_name);
  virtual bool t___isset(Variant v_name);
  virtual Variant t___unset(Variant v_name);
  virtual Variant t___sleep();
  virtual Variant t___wakeup();
  virtual String t___tostring();
  virtual Variant t___clone();
  virtual const CallInfo *t___invokeCallInfoHelper(void *&extra);

  template<typename T, int op>
  T o_assign_op(CStrRef propName, CVarRef val, CStrRef context = null_string);

  /**
   * Marshaling/Unmarshaling between request thread and fiber thread.
   */
  virtual Object fiberMarshal(FiberReferenceMap &refMap) const;
  virtual Object fiberUnmarshal(FiberReferenceMap &refMap) const;

  static Variant callHandler(MethodCallPackage &info, CArrRef params);
  static Variant callHandlerFewArgs(MethodCallPackage &info, int count,
      INVOKE_FEW_ARGS_IMPL_ARGS);
  static Variant NullConstructor(MethodCallPackage &info, CArrRef params);
  static Variant NullConstructorFewArgs(MethodCallPackage &info, int count,
      INVOKE_FEW_ARGS_IMPL_ARGS);
  static int GetMaxId() ATTRIBUTE_COLD;
 protected:
  virtual bool php_sleep(Variant &ret);
public:
  bool hasCall();
  bool hasCallStatic();

 private:
  ObjectData(const ObjectData &) { ASSERT(false);}
  inline Variant o_getImpl(CStrRef propName, int flags,
                           bool error = true, CStrRef context = null_string);
  static DECLARE_THREAD_LOCAL_NO_CHECK(int, os_max_id);
  template <typename T>
  inline Variant o_setImpl(CStrRef propName, T v,
                           bool forInit, CStrRef context);
  template <typename T>
  inline Variant o_setPublicImpl(CStrRef propName, T v, bool forInit);

  static Variant *RealPropPublicHelper(CStrRef propName, int64 hash, int flags,
                                       const ObjectData *obj,
                                       const ObjectStaticCallbacks *osc);
 protected:
  int           o_id;            // a numeric identifier of this object
 private:
  mutable int16 o_attribute;     // various flags
 protected:
  Array         o_properties;    // dynamic properties
 protected:
#ifndef HHVM
  // For non-HHVM builds, avoid the memory overhead of HHVM-specific fields by
  // overlaying them onto a field that exists regardless.  This is sufficient
  // for compilation to succeed, but it is critical that the HHVM-specific
  // fields only be used if hhvm is true.
  union {
#endif
    const ObjectStaticCallbacks *o_callbacks;

    // All of the fields that would ordinarily reside in HPHP::VM::Instance must
    // reside here, so that it is possible to dynamically compute the offset of
    // the property vector that follows an "Instance", regardless of whether the
    // Instance is actually an extension class that derives from ObjectData.
#ifdef HHVM
   public:
#endif
    HPHP::VM::Class* m_cls;
    HphpArray* m_propMap;
    TypedValue* m_propVec;
#ifndef HHVM
  };
#endif
 protected:
  void          cloneDynamic(ObjectData *orig);

#ifdef FAST_REFCOUNT_FOR_VARIANT
 private:
  static void compileTimeAssertions() {
    CT_ASSERT(offsetof(ObjectData, _count) == FAST_REFCOUNT_OFFSET);
  }
#endif
 public:
  // true : pure user class or user class deriving from builtin
  // false: pure builtin
  bool isInstance() const {
    return getAttribute(IsInstance);
  }

  template <bool canThrow>
  void releaseImpl() {
    ASSERT(getCount() == 0);
    if (canThrow) {
      destruct();
    } else {
      destructNoThrow();
    }
    if (UNLIKELY(getCount() != 0)) {
      // Object was resurrected.  Make a note to avoid re-running __destruct()
      setAttribute(InDestructor);
      return;
    }
    delete this;
  }
};

template<> inline SmartPtr<ObjectData>::~SmartPtr() {}

typedef ObjectData c_ObjectData; // purely for easier code generation

struct MethodCallInfoTable {
  int64       hash;
  int         flags;
  int         len;
  const char *name;
  CallInfo   *ci;
};

struct InstanceOfInfo {
  int64                       hash;
  int                         flags;
  const char                  *name;
  const ObjectStaticCallbacks *cb;
};

class GlobalVariables;

// Callback structure for functions related to static methods
struct ObjectStaticCallbacks {
  Object create(CArrRef params, bool init = true,
                ObjectData* root = NULL) const;
  Object createOnly(ObjectData *root = NULL) const;
  inline bool os_get_call_info(MethodCallPackage &info, int64 hash = -1) const {
    return GetCallInfo(this, info, hash);
  }
  Variant os_getInit(CStrRef s) const;
  Variant os_get(CStrRef s) const;
  Variant &os_lval(CStrRef s) const;
  Variant os_constant(const char *s) const;
  static bool GetCallInfo(const ObjectStaticCallbacks *osc,
                          MethodCallPackage &mcp, int64 hash);
  static bool GetCallInfoEx(const char *cls,
                            const ObjectStaticCallbacks *osc,
                            MethodCallPackage &mcp, int64 hash);

  bool checkAttribute(int attrs) const;
  operator const ObjectStaticCallbacks*() const { return this; }
  const ObjectStaticCallbacks* operator->() const { return this; }
  GlobalVariables *lazy_initializer(GlobalVariables *g) const;

  ObjectData *(*createOnlyNoInit)(ObjectData* root);

  const MethodCallInfoTable   *mcit;
  const int                   *mcit_ix;
  const InstanceOfInfo        *instanceof_table;
  const int                   *instanceof_index;
  const StaticString          *cls;
  const ClassPropTable        *cpt;
  const CallInfo              *constructor;
  int64                       redeclaredParent;
  const ObjectStaticCallbacks *parent;
  int                         attributes;
  HPHP::VM::Class**           os_cls_ptr;
};

struct RedeclaredObjectStaticCallbacks {
  ObjectStaticCallbacks oscb;
  int id;

  operator const ObjectStaticCallbacks*() const { return &oscb; }
  int getRedeclaringId() const { return id; }

  Variant os_getInit(CStrRef s) const;
  Variant os_get(CStrRef s) const;
  Variant &os_lval(CStrRef s) const;
  Variant os_constant(const char *s) const;
  bool os_get_call_info(MethodCallPackage &info, int64 hash = -1) const;
  ObjectData *createOnlyNoInit(ObjectData* root = NULL) const;
  Object create(CArrRef params, bool init = true,
                ObjectData* root = NULL) const;
  Object createOnly(ObjectData *root = NULL) const;
  const RedeclaredObjectStaticCallbacks* operator->() const { return this; }
};

typedef const RedeclaredObjectStaticCallbacks
RedeclaredObjectStaticCallbacksConst;

ObjectData *coo_ObjectData(ObjectData *);

///////////////////////////////////////////////////////////////////////////////
// Calculate item sizes for object allocators

#define WORD_SIZE sizeof(void *)
#define ALIGN_WORD(n) ((n) + (WORD_SIZE - (n) % WORD_SIZE) % WORD_SIZE)
#define UNIT_SIZE sizeof(ObjectData)

template<int M>
class ItemSize {
 private:
  enum {
    prev = (M + M + 3) / 3 >= UNIT_SIZE ? (M + M + 3) / 3 : UNIT_SIZE,
    pval = ItemSize<prev>::value
  };
 public:
  enum {
    index = ItemSize<prev>::index + (int)(pval < M),
    value = (pval < M ? ALIGN_WORD(pval + (pval >> 1)) : pval)
  };
};

template<>
class ItemSize<UNIT_SIZE> {
 public:
  enum {
    index = 0,
    value = UNIT_SIZE
  };
};

typedef ObjectAllocatorBase *(*ObjectAllocatorBaseGetter)(void);

class ObjectAllocatorCollector {
public:
  static int AllocSizeToIndex(int size);
  static std::map<int, ObjectAllocatorBaseGetter> &getWrappers() {
    static std::map<int, ObjectAllocatorBaseGetter> wrappers;
    return wrappers;
  }
};

template <typename T>
void *ObjectAllocatorInitSetup() {
  ThreadLocalSingleton<ObjectAllocator<ItemSize<sizeof(T)>::value> > tls;
  if (hhvm) {
    ObjectAllocatorCollector::getWrappers()[ItemSize<sizeof(T)>::index] =
      (ObjectAllocatorBaseGetter)tls.getCheck;
  }
  GetAllocatorInitList().insert((AllocatorThreadLocalInit)(tls.getCheck));
  return (void*)tls.getNoCheck;
}

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

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_OBJECT_DATA_H__
