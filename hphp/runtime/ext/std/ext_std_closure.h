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

struct StandardExtension;

extern const StaticString s_Closure;

// native data for closures. Memory layout looks like this:
// [ClosureHdr][ObjectData, kind=Closure][captured vars]
struct ClosureHdr {
  void* ctx;
  HeaderWord<> hdr;
  uint32_t& size() { return hdr.hi32; }
  uint32_t size() const { return hdr.hi32; }
};

struct c_Closure final : ObjectData {

  static Class* classof() { assertx(cls_Closure); return cls_Closure; }
  static c_Closure* fromObject(ObjectData* obj) {
    assertx(obj->instanceof(classof()));
    return reinterpret_cast<c_Closure*>(obj);
  }

  /* closureInstanceCtor() skips this constructor call in debug mode.
   * Update that method if this assumption changes.
   */
  explicit c_Closure(Class* cls)
    : ObjectData(cls, IsCppBuiltin | HasClone, HeaderKind::Closure) {
    // hdr()->ctx must be initialized by init() or the TC.
    if (debug) setThis(reinterpret_cast<ObjectData*>(-uintptr_t(1)));
  }

  ClosureHdr* hdr() {
    return reinterpret_cast<ClosureHdr*>(this) - 1;
  }
  const ClosureHdr* hdr() const {
    return reinterpret_cast<const ClosureHdr*>(this) - 1;
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
  TypedValue* getUseVars() { return propVec(); }

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
  void* getThisOrClass() const { return hdr()->ctx; }
  void setThisOrClass(void* p) { hdr()->ctx = p; }
  ObjectData* getThisUnchecked() const {
    assertx(!ctxIsClass());
    return reinterpret_cast<ObjectData*>(hdr()->ctx);
  }
  ObjectData* getThis() const {
    if (UNLIKELY(ctxIsClass())) {
      return nullptr;
    }
    return getThisUnchecked();
  }
  void setThis(ObjectData* od) { hdr()->ctx = od; }
  bool hasThis() const { return hdr()->ctx && !ctxIsClass(); }

  Class* getClass() const {
    if (LIKELY(ctxIsClass())) {
      return reinterpret_cast<Class*>(
        reinterpret_cast<uintptr_t>(hdr()->ctx) & ~kClassBit
      );
    } else {
      return nullptr;
    }
  }
  void setClass(Class* cls) {
    assertx(cls);
    hdr()->ctx = reinterpret_cast<void*>(
      reinterpret_cast<uintptr_t>(cls) | kClassBit
    );
  }
  bool hasClass() const { return ctxIsClass(); }

  ObjectData* clone();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Offsets for the JIT.
   */
  static constexpr ssize_t ctxOffset() {
    return offsetof(ClosureHdr, ctx) - sizeof(ClosureHdr);
  }

private:
  friend struct Class;
  friend struct StandardExtension;

  static Class* cls_Closure;
  static void setAllocators(Class* cls);

  static constexpr uintptr_t kClassBit = 0x1;
  bool ctxIsClass() const {
    return reinterpret_cast<uintptr_t>(hdr()->ctx) & kClassBit;
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CLOSURE_H_
