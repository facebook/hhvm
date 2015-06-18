/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_CLOSURE_H_
#define incl_HPHP_EXT_CLOSURE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/bytecode.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct c_Closure : ExtObjectDataFlags<ObjectData::HasClone> {

  DECLARE_CLASS_NO_SWEEP(Closure)

  c_Closure(Class* cls = c_Closure::classof());
  ~c_Closure();

  /*
   * Initialization function used by the interpreter.  The JIT inlines these
   * operations in the TC.
   *
   * `sp' points to the last used variable on the evaluation stack.
   */
  void init(int numArgs, ActRec* ar, TypedValue* sp);

  /*
   * Glub.
   */
  static c_Closure* Clone(ObjectData* obj);

  /////////////////////////////////////////////////////////////////////////////
  // ObjectData overrides.

  void t___construct(); // must not be called for Closures

  // Closure object can't have properties.
  Variant t___get(Variant member);
  Variant t___set(Variant member, Variant value);
  bool t___isset(Variant name);
  Variant t___unset(Variant name);

  Array t___debuginfo();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * The closure's underlying function.
   */
  const Func* getInvokeFunc() const;

  /*
   * The Class scope the closure was defined in.
   */
  Class* getScope();

  /*
   * Use and static local variables.
   */
  TypedValue* getUseVars();
  TypedValue* getStaticVar(Slot s);
  int32_t getNumUseVars() const;

  /*
   * The bound context of the Closure---either a $this or a late bound class,
   * just like in the ActRec.
   */
  void* getThisOrClass() const;

  ObjectData* getThis() const;
  void setThis(ObjectData* od);
  bool hasThis() const;

  Class* getClass() const;
  void setClass(Class* cls);
  bool hasClass() const;

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Offsets for the JIT.
   */
  static constexpr size_t ctxOffset() { return offsetof(c_Closure, m_ctx); }

  /*
   * PHP API.
   */
  static Object ti_bind(const Variant& closure, const Variant& newthis,
                        const Variant& scope);
  Object t_bindto(const Variant& newthis, const Variant& scope);


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  void* m_ctx;
};

///////////////////////////////////////////////////////////////////////////////

inline c_Closure::c_Closure(Class* cls)
  : ExtObjectDataFlags(cls)
{
  // m_ctx must be initialized by init() or the TC.
  if (debug) m_ctx = reinterpret_cast<ObjectData*>(-uintptr_t(1));
}

inline const Func* c_Closure::getInvokeFunc() const {
  return getVMClass()->getCachedInvoke();
}

inline Class* c_Closure::getScope() {
  return getInvokeFunc()->cls();
}

inline TypedValue* c_Closure::getUseVars() {
  return propVec();
}

inline TypedValue* c_Closure::getStaticVar(Slot s) {
  return propVec() + s;
}

inline int32_t c_Closure::getNumUseVars() const {
  return getVMClass()->numDeclProperties() -
         getInvokeFunc()->numStaticLocals();
}

inline void* c_Closure::getThisOrClass() const {
  return m_ctx;
}

inline ObjectData* c_Closure::getThis() const {
  return ActRec::decodeThis(m_ctx);
}

inline void c_Closure::setThis(ObjectData* od) {
  m_ctx = ActRec::encodeThis(od);
}

inline bool c_Closure::hasThis() const {
  return getThis() != nullptr;
}

inline Class* c_Closure::getClass() const {
  return ActRec::decodeClass(m_ctx);
}

inline void c_Closure::setClass(Class* cls) {
  m_ctx = ActRec::encodeClass(cls);
}

inline bool c_Closure::hasClass() const {
  return getClass() != nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CLOSURE_H_
