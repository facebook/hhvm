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

#include "runtime/vm/translator/translator-runtime.h"

#include "runtime/ext/ext_function.h"
#include "runtime/vm/member_operations.h"
#include "runtime/vm/type_constraint.h"

namespace HPHP { namespace VM { namespace Transl {

ArrayData* addElemIntKeyHelper(ArrayData* ad,
                               int64_t key,
                               TypedValue value) {
  // this does not re-enter
  return array_setm_ik1_v0(0, ad, key, &value);
}

ArrayData* addElemStringKeyHelper(ArrayData* ad,
                                  StringData* key,
                                  TypedValue value) {
  // this does not re-enter
  return array_setm_s0k1_v0(0, ad, key, &value);
}

HOT_FUNC_VM TypedValue setNewElem(TypedValue* base, Cell val) {
  SetNewElem<true>(base, &val);
  return val;
}

void bindNewElemIR(TypedValue* base, RefData* val, MInstrState* mis) {
  base = NewElem(mis->tvScratch, mis->tvRef, base);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBindRef(val, base);
  }
}

// TODO: Kill this #2031980
HOT_FUNC_VM RefData* box_value(TypedValue tv) {
  return tvBoxHelper(tv.m_type, tv.m_data.num);
}

void raisePropertyOnNonObject() {
  raise_warning("Cannot access property on non-object");
}

void raiseUndefProp(ObjectData* base, const StringData* name) {
  static_cast<Instance*>(base)->raiseUndefProp(name);
}

void raise_error_sd(const StringData *msg) {
  raise_error("%s", msg->data());
}

void VerifyParamTypeFail(int paramNum) {
  VMRegAnchor _;
  const ActRec* ar = curFrame();
  const Func* func = ar->m_func;
  const TypeConstraint& tc = func->params()[paramNum].typeConstraint();
  TypedValue* tv = frame_local(ar, paramNum);
  assert(!tc.check(tv, func));
  tc.verifyFail(func, paramNum, tv);
}

void VerifyParamTypeCallable(TypedValue value, int param) {
  if (UNLIKELY(!f_is_callable(tvAsCVarRef(&value)))) {
    VerifyParamTypeFail(param);
  }
}

HOT_FUNC_VM
void VerifyParamTypeSlow(const Class* cls, const Class* constraint, int param) {
  if (UNLIKELY(!(constraint && cls->classof(constraint)))) {
    VerifyParamTypeFail(param);
  }
}

} } }
