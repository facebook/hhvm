/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct StandardExtension;

extern const StaticString s_Closure;

// native data for closures. Memory layout looks like this:
// [ClosureHdr][ObjectData, kind=Closure][captured vars]
struct ClosureHdr : HeapObject {
  enum class NoThrow {};
  explicit ClosureHdr(uint32_t size) {
    initHeader_32(HeaderKind::ClosureHdr, size);
    // we need to set this here, because the next thing 'new Closure'
    // will do is call the constructor, which will throw, and the
    // destructor will examine this field.
    ctx = nullptr;
  }
  ClosureHdr(uint32_t size, NoThrow) {
    initHeader_32(HeaderKind::ClosureHdr, size);
  }
  uint32_t& size() { return m_aux32; }
  uint32_t size() const { return m_aux32; }
  TYPE_SCAN_CUSTOM_FIELD(ctx) {
    scanner.scan(ctx_this);
  }
 public:
  union {
    void* ctx;
    Class* ctx_class;
    ObjectData* ctx_this;
  };
};

struct c_Closure final : ObjectData {

  static bool initialized() { return cls_Closure != nullptr; }
  static Class* classof() { assertx(cls_Closure); return cls_Closure; }
  static c_Closure* fromObject(ObjectData* obj) {
    assertx(obj->instanceof(classof()));
    return reinterpret_cast<c_Closure*>(obj);
  }

  explicit c_Closure(Class* cls)
    : ObjectData(cls, 0, HeaderKind::Closure) {
    // hdr()->ctx must be initialized by init() or the TC.
    if (debug) setThis(reinterpret_cast<ObjectData*>(-uintptr_t(1)));
  }
  c_Closure(Class* cls, InitRaw)
    : ObjectData(cls, InitRaw{}, 0, HeaderKind::Closure) {
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

  /*
   * Initialization function used by the interpreter. The JIT inlines these
   * operations in the TC.
   *
   * Returns the number of cells copied to the stack.
   */
  static int initActRecFromClosure(ActRec* ar, TypedValue* sp);
  /////////////////////////////////////////////////////////////////////////////

  static ObjectData* instanceCtor(Class* cls);
  static void instanceDtor(ObjectData* obj, const Class* cls);

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
   * The bound context of the Closure---either a $this or a late bound class,
   * just like in the ActRec.
   */
  void* getThisOrClass() const { return hdr()->ctx; }
  void setThisOrClass(void* p) { hdr()->ctx = p; }
  ObjectData* getThisUnchecked() const {
    assertx(hasThis());
    return hdr()->ctx_this;
  }
  ObjectData* getThis() const {
    return UNLIKELY(hasThis()) ? getThisUnchecked() : nullptr;
  }
  void setThis(ObjectData* od) { hdr()->ctx_this = od; }
  bool hasThis() const {
    if (use_lowptr) return !is_low_mem(hdr()->ctx);
    return
      getInvokeFunc() && getInvokeFunc()->cls() && !getInvokeFunc()->isStatic();
  }

  Class* getClass() const {
    return LIKELY(hasClass()) ? hdr()->ctx_class : nullptr;
  }
  void setClass(Class* cls) {
    assertx(cls);
    hdr()->ctx_class = cls;
  }
  bool hasClass() const {
    if (use_lowptr) return hdr()->ctx && is_low_mem(hdr()->ctx);
    return
      getInvokeFunc() && getInvokeFunc()->cls() && getInvokeFunc()->isStatic();
  }

  size_t coeffectsPropSlot() const;
  RuntimeCoeffects getCoeffects() const;
  void setCoeffects(RuntimeCoeffects);

  ObjectData* clone();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Size of a closure allocation for a given class.
   */
  static size_t size(const Class* cls) {
    return sizeof(ClosureHdr) +
           ObjectData::sizeForNProps(cls->numDeclProperties());
  }

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
};

ObjectData* createClosureRepoAuthRawSmall(Class* cls, size_t size,
                                          size_t index);
ObjectData* createClosureRepoAuth(Class* cls);
ObjectData* createClosure(Class* cls);

///////////////////////////////////////////////////////////////////////////////
}

