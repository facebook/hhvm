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

#ifndef __HPHP_OBJECT_DATA_H__
#define __HPHP_OBJECT_DATA_H__

#include <cpp/base/util/countable.h>
#include <cpp/base/types.h>
#include <cpp/base/memory/unsafe_pointer.h>
#include <cpp/base/type_variant.h>
#include <cpp/base/string_data.h>
#include <cpp/base/macros.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Needed for eval
namespace Eval {
class MethodStatement;
class FunctionCallExpression;
class VariableEnvironment;
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
 *    o_lval() -> o_set() -> t___set() as fallback
 * 3. Dynamic methods:
 *    o_invoke() -> t___call() as fallback
 * 4. Auto-generated jump-tables:
 *    o_exists()
 *    o_get()
 *    o_set()
 *    o_lval()
 *    o_invoke()
 */
class ObjectData : public Countable {
  public:
  enum Attribute {
    inConstructor = 1, // __construct()
    inDestructor  = 2, // __destruct()
  };

  public:
  ObjectData();
  virtual ~ObjectData(); // all PHP classes need virtual tables

  bool getAttribute(Attribute attr) const { return o_attribute & attr; }
  void setAttribute(Attribute attr) { o_attribute |= attr;}
  void clearAttribute(Attribute attr) { o_attribute &= ~attr;}
  bool inDtor() { return getAttribute(inDestructor); }
  bool inCtor() { return getAttribute(inConstructor); }
  bool inCtorDtor() { return inCtor() || inDtor(); }
  void setInDtor() { setAttribute(inDestructor); }
  void setInCall(CStrRef name);
  void clearInCall() { --o_inCall;}
  bool gasInCtor(bool inCtor) { // get and set inConstructor
    bool oldInCtor = getAttribute(inConstructor);
    if (inCtor) {
      setAttribute(inConstructor);
    } else {
      clearAttribute(inConstructor);
    }
    return oldInCtor;
  }

  // class info
  virtual const char *o_getClassName() const = 0;
  virtual bool o_instanceof(const char *s) const = 0;
  virtual bool isResource() const { return false;}
  virtual int64 o_toInt64() const { return 1;}
  bool o_isClass(const char *s) const;
  int o_getId() const { return o_id;}

  virtual void init() {}
  ObjectData *create() { init(); return this;}
  ObjectData *dynCreate(const Array &params, bool init = true) {
    if (init) return create();
    return this;
  }
  virtual void dynConstruct(CArrRef params) {}
  virtual void release() { destruct(); delete this; } // for SmartPtr<T>
  virtual void destruct() {}

  virtual const
    Eval::MethodStatement *getMethodStatement(const char* name) const;

  // static methods and properties
  static Variant os_get(const char *s, int64 hash);
  static Variant &os_lval(const char *s, int64 hash);
  static Variant os_invoke(const char *c, const char *s,
                           CArrRef params, int64 hash, bool fatal = true);
  static Variant os_constant(const char *s);

  static Variant os_invoke_from_eval(const char *c, const char *s,
                                     Eval::VariableEnvironment &env,
                                     const Eval::FunctionCallExpression *call,
                                     int64 hash,
                                     bool fatal /* = true */);

  // properties
  virtual Array o_toArray() const;
  virtual Array o_toIterArray(const char *context);
  virtual Array o_getDynamicProperties() const;
  virtual bool o_exists(CStrRef s, int64 hash) const;
  virtual void o_get(std::vector<ArrayElement *> &props) const {}
  virtual Variant o_get(CStrRef s, int64 hash);
  virtual Variant o_getUnchecked(CStrRef s, int64 hash);
  virtual Variant o_set(CStrRef s, int64 hash, CVarRef v, bool forInit = false);
  virtual Variant &o_lval(CStrRef s, int64 hash);
  void o_set(const Array properties);

  CVarRef set(CStrRef s, CVarRef v);

  // methods
  virtual Variant o_invoke(const char *s, CArrRef params, int64 hash,
                           bool fatal = true);
  virtual Variant o_root_invoke(const char *s, CArrRef params, int64 hash,
                                bool fatal = false);
  virtual Variant o_invoke_ex(const char *clsname, const char *s,
                              CArrRef params, int64 hash, bool fatal = true);

  virtual Variant o_invoke_few_args(const char *s, int64 hash, int count,
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
  virtual Variant o_root_invoke_few_args(const char *s, int64 hash, int count,
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
  virtual Variant o_invoke_from_eval(const char *s,
                                     Eval::VariableEnvironment &env,
                                     const Eval::FunctionCallExpression *call,
                                     int64 hash,
                                     bool fatal /* = true */);
  // misc
  Variant o_throw_fatal(const char *msg);
  virtual void serialize(VariableSerializer *serializer) const;
  virtual void dump() const;
  virtual ObjectData *clone();
  virtual void setRoot(ObjectData *root) {}
  /**
   * If __call is defined, then this gets overridden to call it.
   * Otherwise, it just throws if fatal else returns false.
   */
  virtual Variant doCall(Variant v_name, Variant v_arguments, bool fatal);

  // magic methods
  // __construct is handled in a special way
  virtual Variant t___destruct();
  virtual Variant t___call(Variant v_name, Variant v_arguments);
  virtual Variant t___set(Variant v_name, Variant v_value);
  virtual Variant t___get(Variant v_name);
  virtual Variant &___lval(Variant v_name);
  virtual Variant &___offsetget_lval(Variant v_name);
  virtual bool t___isset(Variant v_name);
  virtual Variant t___unset(Variant v_name);
  virtual Variant t___sleep();
  virtual Variant t___wakeup();
  virtual Variant t___set_state(Variant v_properties);
  virtual String t___tostring();
  virtual Variant t___clone();

  protected:
  virtual ObjectData* cloneImpl() = 0;
  void cloneSet(ObjectData *clone);
  int o_id;                      // a numeric identifier of this object
  mutable Array *o_properties;   // dynamic properties

  private:
  mutable int16  o_attribute;    // vairous flags
  mutable int16  o_inCall;       // counter for __call() recursion checking

private:
  ObjectData(const ObjectData &) { ASSERT(false);}
};

typedef ObjectData c_ObjectData; // purely for easier code generation

///////////////////////////////////////////////////////////////////////////////
// breaking circular dependencies

template<typename T>
void Variant::weakRemove(const T &key, int64 prehash /* = -1 */) {
  if (is(KindOfArray) ||
      (is(KindOfObject) && getObjectData()->o_instanceof("arrayaccess"))) {
    remove(key, prehash);
  }
}

template<typename T>
Variant &Variant::lvalAtImpl(const T &key, int64 prehash /* = -1 */) {
  if (m_type == KindOfVariant) {
    return m_data.pvar->lvalAtImpl(key, prehash);
  }
  if (isNull() ||
      (is(KindOfBoolean) && !toBoolean()) ||
      (is(LiteralString) && !*getLiteralString()) ||
      (is(KindOfString) && getStringData()->empty())) {
    unset();
    set(toArray());
  }
  if (is(KindOfArray)) {
    Variant *ret = NULL;
    ArrayData *arr = m_data.parr;
    ArrayData *escalated = arr->lval(key, ret, arr->getCount() > 1, prehash);
    if (escalated) {
      set(escalated);
    }
    ASSERT(ret);
    return *ret;
  }
  if (is(KindOfObject)) {
    return getArrayAccess()->___offsetget_lval(key);
  }
  return lvalInvalid();
}

template<typename T>
Variant Variant::refvalAtImpl(const T &key, int64 prehash /* = -1 */) {
  if (m_type == KindOfVariant) {
    return m_data.pvar->refvalAtImpl(key, prehash);
  }
  if (is(KindOfArray) || isNull() ||
      (is(KindOfBoolean) && !toBoolean()) ||
      (is(LiteralString) && !*getLiteralString()) ||
      (is(KindOfString) && getStringData()->empty())) {
    return ref(lvalAt(key, prehash));
  } else {
    return rvalAt(key, prehash);
  }

}

///////////////////////////////////////////////////////////////////////////////
// Calculate item sizes for object allocators

#define WORD_SIZE sizeof(void *)
#define ALIGN_WORD(n) (n + (WORD_SIZE - n % WORD_SIZE) % WORD_SIZE)

template<int M>
class ItemSize {
public:
  enum {
    prev = (M + M + 3) / 3 < M - 1 ? (M + M + 3) / 3 : M - 1,
    value = (ItemSize<prev>::value < M ?
             ALIGN_WORD(ItemSize<prev>::value + (ItemSize<prev>::value >> 1)) :
             ItemSize<prev>::value)
  };
};

template<>
class ItemSize<0> {
public:
  enum {
    prev = 0,
    value = sizeof(ObjectData)
  };
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_OBJECT_DATA_H__
