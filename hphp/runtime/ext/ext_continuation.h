/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

  static constexpr uint startedOffset() {
    return offsetof(c_Continuation, o_subclassData);
  }
  bool started() const { return o_subclassData.u8[0]; }
  void start() { o_subclassData.u8[0] = true; }

  enum ContState : uint8_t {
    Running = 1,
    Done    = 2
  };
  static constexpr uint stateOffset() {
    return offsetof(c_Continuation, o_subclassData) + 1;
  }
  bool done() const { return o_subclassData.u8[1] & ContState::Done; }
  void setDone() { o_subclassData.u8[1]  =  ContState::Done; }

  bool running() const { return o_subclassData.u8[1] & ContState::Running; }
  void setRunning() { o_subclassData.u8[1]  =  ContState::Running; }
  void setStopped() { o_subclassData.u8[1] &= ~ContState::Running; }

  void t___construct();
  void t_update(int64_t label, CVarRef value);
  void t_update_key(int64_t label, CVarRef value, CVarRef key);
  Object t_getwaithandle();
  int64_t t_getlabel();
  Variant t_current();
  Variant t_key();
  void t_next();
  void t_rewind();
  bool t_valid();
  void t_send(CVarRef v);
  void t_raise(CVarRef v);
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
  static c_Continuation* Alloc(const Func* genFunc) {
    assert(genFunc);
    assert(genFunc->isGenerator());

    size_t contOffset = getContOffset(genFunc);
    size_t objectSize = contOffset + sizeof(c_Continuation);
    void* mem = MM().objMallocLogged(objectSize);
    void* objMem = (char*)mem + contOffset;
    auto const cont = new (objMem) c_Continuation();
    memset(mem, 0, contOffset);
    cont->m_entry = genFunc->getPrologue(0);
    cont->m_size = objectSize;
    assert(cont->getObjectSize() == objectSize);
    return cont;
  }

  static c_Continuation* Create(const Func* genFunc) {
    auto const cont = c_Continuation::Alloc(genFunc);
    cont->incRefCount();
    cont->setNoDestruct();

    // The ActRec corresponding to the generator body lives as long as the
    // object does. We set it up once, here, and then just change FP to point
    // to it when we enter the generator body.
    auto ar = cont->actRec();
    ar->m_func = genFunc;
    ar->initNumArgs(0);
    ar->setVarEnv(nullptr);
    return cont;
  }

 public:
  static ObjectData* CreateFunc(const Func* genFunc) {
    auto cont = Create(genFunc);
    cont->actRec()->setThis(nullptr);
    return cont;
  }

  static ObjectData* CreateMeth(const Func* genFunc, void* objOrCls) {
    auto cont = Create(genFunc);
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

  static ptrdiff_t getContOffset(const Func* genFunc) {
    assert(genFunc->isGenerator());
    return sizeof(Iter) * genFunc->numIterators() +
           sizeof(TypedValue) * genFunc->numLocals() +
           sizeof(ActRec);
  }

  void call_send(Cell& v);
  void call_raise(ObjectData* e);

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

  Offset getExecutionOffset(int32_t label) const;
  Offset getNextExecutionOffset() const;

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
  int32_t m_label;
  int64_t m_index;
  Variant m_key;
  Variant m_value;
  int32_t m_size;

  /* TCA for function entry */
  TCA m_entry;

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
