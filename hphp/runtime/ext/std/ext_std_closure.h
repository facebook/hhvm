/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const StaticString s_Closure;

struct Closure {

  static Class* classof() {
    static auto cls = Unit::lookupClass(s_Closure.get());
    return cls;
  }

  Closure();
  Closure& operator=(const Closure& src);
  ~Closure();
  void sweep() {}

  /*
   * Initialization function used by the interpreter.  The JIT inlines these
   * operations in the TC.
   *
   * `sp' points to the last used variable on the evaluation stack.
   */
  void init(int numArgs, ActRec* ar, TypedValue* sp);

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
  static constexpr ssize_t ctxOffset() {
    return offsetof(Closure, m_ctx) - sizeof(Closure);
  }

private:
  void* m_ctx;
};

///////////////////////////////////////////////////////////////////////////////

inline Closure::Closure() {
  // m_ctx must be initialized by init() or the TC.
  if (debug) m_ctx = reinterpret_cast<ObjectData*>(-uintptr_t(1));
}

inline const Func* Closure::getInvokeFunc() const {
  return Native::object<Closure>(this)->getVMClass()->getCachedInvoke();
}

inline Class* Closure::getScope() {
  return getInvokeFunc()->cls();
}

inline TypedValue* Closure::getUseVars() {
  return Native::object<Closure>(this)->propVec();
}

inline TypedValue* Closure::getStaticVar(Slot s) {
  return Native::object<Closure>(this)->propVec() + s;
}

inline int32_t Closure::getNumUseVars() const {
  return Native::object<Closure>(this)->getVMClass()->numDeclProperties() -
         getInvokeFunc()->numStaticLocals();
}

inline void* Closure::getThisOrClass() const {
  return m_ctx;
}

inline ObjectData* Closure::getThis() const {
  return ActRec::decodeThis(m_ctx);
}

inline void Closure::setThis(ObjectData* od) {
  m_ctx = ActRec::encodeThis(od);
}

inline bool Closure::hasThis() const {
  return getThis() != nullptr;
}

inline Class* Closure::getClass() const {
  return ActRec::decodeClass(m_ctx);
}

inline void Closure::setClass(Class* cls) {
  m_ctx = ActRec::encodeClass(cls);
}

inline bool Closure::hasClass() const {
  return getClass() != nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CLOSURE_H_
