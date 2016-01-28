/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StandardExtension;

extern const StaticString s_Closure;

struct c_Closure : ObjectData {

  static Class* classof() { assertx(cls_Closure); return cls_Closure; }
  static c_Closure* fromObject(ObjectData* obj) {
    assertx(obj->instanceof(classof()));
    return reinterpret_cast<c_Closure*>(obj);
  }

  /* closureInstanceCtor() skips this constructor call in debug mode.
   * Update that method if this assumption changes.
   */
  explicit c_Closure(Class* cls = classof()): ObjectData(cls) {
    // m_ctx must be initialized by init() or the TC.
    if (debug) setThis(reinterpret_cast<ObjectData*>(-uintptr_t(1)));
  }

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
  const Func* getInvokeFunc() const { return getVMClass()->getCachedInvoke(); }

  /*
   * The Class scope the closure was defined in.
   */
  Class* getScope() { return getInvokeFunc()->cls(); }

  /*
   * Use and static local variables.
   *
   * Returns obj->propVec()
   * but with runtime generalized checks replaced with assertions
   */
  TypedValue* getUseVars() {
    assertx(getVMClass()->builtinODTailSize() == sizeof(void*));

    auto ret = reinterpret_cast<TypedValue*>(this + 1);
    // Sanity check in dbg mode to make sure ObjectData's shape hasn't changed
    assertx(ret == propVec());

    return ret;
  }
  TypedValue* getStaticVar(Slot s) {
    assertx(getVMClass()->numDeclProperties() > s);
    return getUseVars() + s;
  }
  int32_t getNumUseVars() const {
    return getVMClass()->numDeclProperties() -
           getInvokeFunc()->numStaticLocals();
  }

  /*
   * The bound context of the Closure---either a $this or a late bound class,
   * just like in the ActRec.
   */
  void* getThisOrClass() const { return m_ctx; }

  ObjectData* getThis() const {
    if (UNLIKELY(ctxIsClass())) {
      return nullptr;
    } else {
      return reinterpret_cast<ObjectData*>(m_ctx);
    }
  }
  void setThis(ObjectData* od) { m_ctx = od; }
  bool hasThis() const { return m_ctx && !ctxIsClass(); }

  Class* getClass() const {
    if (LIKELY(ctxIsClass())) {
      return reinterpret_cast<Class*>(
        reinterpret_cast<uintptr_t>(m_ctx) & ~kClassBit
      );
    } else {
      return nullptr;
    }
  }
  void setClass(Class* cls) {
    m_ctx = reinterpret_cast<void*>(
      reinterpret_cast<uintptr_t>(cls) | kClassBit
    );
  }
  bool hasClass() const { return m_ctx && ctxIsClass(); }

  ObjectData* clone();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Offsets for the JIT.
   */
  static constexpr ssize_t ctxOffset() {
    // c_Closure must be precisely one pointer larger than ObjectData
    // so that we use up precisely one slotted declared property slot
    static_assert(sizeof(c_Closure) == sizeof(void*) + sizeof(ObjectData),
                  "c_Closure size mismatch");
    return offsetof(c_Closure, m_ctx);
  }

private:
  friend class Class;
  friend class StandardExtension;

  static Class* cls_Closure;
  static void setAllocators(Class* cls);

  static constexpr uintptr_t kClassBit = 0x1;
  bool ctxIsClass() const {
    return reinterpret_cast<uintptr_t>(m_ctx) & kClassBit;
  }

  void* m_ctx;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CLOSURE_H_
