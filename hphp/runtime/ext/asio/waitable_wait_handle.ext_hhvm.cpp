/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/array/array_init.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/class.h>
#include <runtime/vm/runtime.h>
#include <exception>

namespace HPHP {

IMPLEMENT_CLASS(WaitableWaitHandle);
/*
void HPHP::c_WaitableWaitHandle::t___construct()
_ZN4HPHP20c_WaitableWaitHandle13t___constructEv

this_ => rdi
*/

void th_18WaitableWaitHandle___construct(ObjectData* this_) asm("_ZN4HPHP20c_WaitableWaitHandle13t___constructEv");

TypedValue* tg_18WaitableWaitHandle___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_18WaitableWaitHandle___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitableWaitHandle::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitableWaitHandle::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
int HPHP::c_WaitableWaitHandle::t_getcontextidx()
_ZN4HPHP20c_WaitableWaitHandle15t_getcontextidxEv

(return value) => rax
this_ => rdi
*/

int th_18WaitableWaitHandle_getContextIdx(ObjectData* this_) asm("_ZN4HPHP20c_WaitableWaitHandle15t_getcontextidxEv");

TypedValue* tg_18WaitableWaitHandle_getContextIdx(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_18WaitableWaitHandle_getContextIdx((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitableWaitHandle::getContextIdx", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitableWaitHandle::getContextIdx");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_WaitableWaitHandle::t_getcreator()
_ZN4HPHP20c_WaitableWaitHandle12t_getcreatorEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_18WaitableWaitHandle_getCreator(Value* _rv, ObjectData* this_) asm("_ZN4HPHP20c_WaitableWaitHandle12t_getcreatorEv");

TypedValue* tg_18WaitableWaitHandle_getCreator(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfObject;
        th_18WaitableWaitHandle_getCreator((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitableWaitHandle::getCreator", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitableWaitHandle::getCreator");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_WaitableWaitHandle::t_getparents()
_ZN4HPHP20c_WaitableWaitHandle12t_getparentsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_18WaitableWaitHandle_getParents(Value* _rv, ObjectData* this_) asm("_ZN4HPHP20c_WaitableWaitHandle12t_getparentsEv");

TypedValue* tg_18WaitableWaitHandle_getParents(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_18WaitableWaitHandle_getParents((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitableWaitHandle::getParents", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitableWaitHandle::getParents");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_WaitableWaitHandle::t_getstacktrace()
_ZN4HPHP20c_WaitableWaitHandle15t_getstacktraceEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_18WaitableWaitHandle_getStackTrace(Value* _rv, ObjectData* this_) asm("_ZN4HPHP20c_WaitableWaitHandle15t_getstacktraceEv");

TypedValue* tg_18WaitableWaitHandle_getStackTrace(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_18WaitableWaitHandle_getStackTrace((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("WaitableWaitHandle::getStackTrace", 0, 1);
      }
    } else {
      throw_instance_method_fatal("WaitableWaitHandle::getStackTrace");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

