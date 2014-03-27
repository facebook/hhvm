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

#ifndef incl_HPHP_EXT_CONTINUATION_H_
#define incl_HPHP_EXT_CONTINUATION_H_


#include "hphp/runtime/base/base-includes.h"
#include "hphp/system/systemlib.h"

typedef unsigned char* TCA; // "Translation cache address."

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FORWARD_DECLARE_CLASS(Continuation);

///////////////////////////////////////////////////////////////////////////////
// class Continuation

struct c_Continuation : ExtObjectDataFlags<ObjectData::HasClone> {
  DECLARE_CLASS_NO_ALLOCATION(Continuation)

  static constexpr ptrdiff_t startedOff() {
    return offsetof(c_Continuation, o_subclassData);
  }
  bool started() const { return o_subclassData.u8[0]; }
  void start() { o_subclassData.u8[0] = true; }

  enum ContState : uint8_t {
    Running = 1,
    Done    = 2
  };
  static constexpr ptrdiff_t stateOff() {
    return offsetof(c_Continuation, o_subclassData) + 1;
  }

  bool done() const { return o_subclassData.u8[1] & ContState::Done; }
  void setDone() { o_subclassData.u8[1]  =  ContState::Done; }

  bool running() const { return o_subclassData.u8[1] & ContState::Running; }
  void setRunning() { o_subclassData.u8[1]  =  ContState::Running; }
  void setStopped() { o_subclassData.u8[1] &= ~ContState::Running; }

  void t___construct();
  void suspend(Offset offset, const Cell& value);
  void suspend(Offset offset, const Cell& value, const Cell& key);
  Object t_getwaithandle();
  Variant t_current();
  Variant t_key();
  void t_next();
  void t_rewind();
  bool t_valid();
  void t_send(const Variant& v);
  void t_raise(const Variant& v);
  String t_getorigfuncname();
  String t_getcalledclass();

  static c_Continuation* Clone(ObjectData* obj);

 private:
  /*
   * The memory allocated by c_Continuation::alloc is laid out as follows:
   *
   *                +--------------------------+ high address
   *                |  c_Continuation object   |
   *                +--------------------------+
   *                |         ActRec           |
   *                +--------------------------+
   *                |                          |
   *                |  Function locals and     |
   *                |  iterators               |
   * malloc ptr ->  +--------------------------+ low address
   *
   * Given a c_Continuation object, the ActRec is just below
   * the object in memory, followed by the functions locals
   * and iterators.
   *
   */
  static c_Continuation* Alloc(const Func* func, Offset offset) {
    assert(func);
    assert(func->isAsync() || func->isGenerator());
    assert(func->contains(offset));

    size_t contOffset = getContOffset(func);
    size_t objectSize = contOffset + sizeof(c_Continuation);
    void* mem = MM().objMallocLogged(objectSize);
    void* objMem = (char*)mem + contOffset;
    auto const cont = new (objMem) c_Continuation();
    memset(mem, 0, contOffset);
    cont->m_offset = offset;
    cont->m_size = objectSize;
    assert(cont->getObjectSize() == objectSize);
    return cont;
  }

  static c_Continuation* Create(const Func* func, Offset offset) {
    auto const cont = c_Continuation::Alloc(func, offset);
    cont->incRefCount();
    cont->setNoDestruct();

    // The ActRec corresponding to the generator body lives as long as the
    // object does. We set it up once, here, and then just change FP to point
    // to it when we enter the generator body.
    auto ar = cont->actRec();
    ar->m_func = func;
    ar->initNumArgsInGenerator(0);
    ar->setVarEnv(nullptr);
    return cont;
  }

 public:
  static ObjectData* CreateFunc(const Func* func, Offset offset) {
    auto cont = Create(func, offset);
    cont->actRec()->setThis(nullptr);
    return cont;
  }

  static ObjectData* CreateMeth(const Func* func, void* objOrCls,
                                Offset offset) {
    auto cont = Create(func, offset);
    auto ar = cont->actRec();
    ar->setThisOrClass(objOrCls);
    if (ar->hasThis()) {
      ar->getThis()->incRefCount();
    }
    return cont;
  }

  static ptrdiff_t getArOffset() {
    return -sizeof(ActRec);
  }

  static ptrdiff_t getContOffset(const Func* func) {
    assert(func->isAsync() || func->isGenerator());
    return sizeof(Iter) * func->numIterators() +
           sizeof(TypedValue) * func->numLocals() +
           sizeof(ActRec);
  }

  /**
   * Get adjusted generator function base() where the real user code starts.
   *
   * Skips CreateCont, RetC and PopC opcodes.
   */
  static Offset userBase(const Func* func) {
    assert(func->isGenerator());
    auto base = func->base();

    DEBUG_ONLY auto op = reinterpret_cast<const Op*>(func->unit()->at(base));
    assert(op[0] == OpCreateCont);
    assert(op[1 + sizeof(Offset)] == OpRetC);
    assert(op[2 + sizeof(Offset)] == OpPopC);

    return base + 3 + sizeof(Offset);
  }

  inline void preNext() {
    if (done()) {
      throw_exception(Object(SystemLib::AllocExceptionObject(
                               "Continuation is already finished")));
    }
    if (running()) {
      throw_exception(Object(SystemLib::AllocExceptionObject(
                               "Continuation is already running")));
    }
    setRunning();
    start();
  }

  inline void startedCheck() {
    if (!started()) {
      throw_exception(
        Object(SystemLib::AllocExceptionObject("Need to call next() first")));
    }
  }

  Offset offset() const {
    assert(!running());
    assert(actRec()->func()->contains(m_offset));
    return m_offset;
  }

private:
  explicit c_Continuation(Class* cls = c_Continuation::classof());
  ~c_Continuation();

  void* getMallocBase() {
    return (char*)(this + 1) - getObjectSize();
  }

  size_t getObjectSize() {
    return m_size;
  }

  void dupContVar(const StringData *name, TypedValue *src);
  void copyContinuationVars(ActRec *fp);

public:
  /* 32-bit o_id from ObjectData */
  Offset m_offset;
  int64_t m_index;
  Cell m_key;
  Cell m_value;
  int32_t m_size;

  /* temporary storage used to save the SP when inlining into a continuation */
  void* m_stashedSP;

  String& getCalledClass() { not_reached(); }

  ActRec* actRec() const {
    return (ActRec *)((char*)this + getArOffset());
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CONTINUATION_H_
