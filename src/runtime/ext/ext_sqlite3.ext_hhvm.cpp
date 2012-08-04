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
#include <runtime/vm/exception_gate.h>
#include <exception>

namespace HPHP {

HPHP::VM::Instance* new_SQLite3_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SQLite3) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SQLite3(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_SQLite3::t___construct(HPHP::String const&, long long, HPHP::String const&)
_ZN4HPHP9c_SQLite313t___constructERKNS_6StringExS3_

this_ => rdi
filename => rsi
flags => rdx
encryption_key => rcx
*/

void th_7SQLite3___construct(ObjectData* this_, Value* filename, long long flags, Value* encryption_key) asm("_ZN4HPHP9c_SQLite313t___constructERKNS_6StringExS3_");

TypedValue* tg1_7SQLite3___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_7SQLite3___construct((this_), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(k_SQLITE3_OPEN_READWRITE|k_SQLITE3_OPEN_CREATE), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_7SQLite3___construct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_7SQLite3___construct((this_), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(k_SQLITE3_OPEN_READWRITE|k_SQLITE3_OPEN_CREATE), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::__construct", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
void HPHP::c_SQLite3::t_open(HPHP::String const&, long long, HPHP::String const&)
_ZN4HPHP9c_SQLite36t_openERKNS_6StringExS3_

this_ => rdi
filename => rsi
flags => rdx
encryption_key => rcx
*/

void th_7SQLite3_open(ObjectData* this_, Value* filename, long long flags, Value* encryption_key) asm("_ZN4HPHP9c_SQLite36t_openERKNS_6StringExS3_");

TypedValue* tg1_7SQLite3_open(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_open(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_7SQLite3_open((this_), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(k_SQLITE3_OPEN_READWRITE|k_SQLITE3_OPEN_CREATE), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_7SQLite3_open(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_7SQLite3_open((this_), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(k_SQLITE3_OPEN_READWRITE|k_SQLITE3_OPEN_CREATE), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_open(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::open", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::open");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3::t_busytimeout(long long)
_ZN4HPHP9c_SQLite313t_busytimeoutEx

(return value) => rax
this_ => rdi
msecs => rsi
*/

bool th_7SQLite3_busytimeout(ObjectData* this_, long long msecs) asm("_ZN4HPHP9c_SQLite313t_busytimeoutEx");

TypedValue* tg1_7SQLite3_busytimeout(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_busytimeout(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (th_7SQLite3_busytimeout((this_), (long long)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_7SQLite3_busytimeout(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_7SQLite3_busytimeout((this_), (long long)(args[-0].m_data.num))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_busytimeout(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::busytimeout", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::busytimeout");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3::t_close()
_ZN4HPHP9c_SQLite37t_closeEv

(return value) => rax
this_ => rdi
*/

bool th_7SQLite3_close(ObjectData* this_) asm("_ZN4HPHP9c_SQLite37t_closeEv");

TypedValue* tg_7SQLite3_close(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_7SQLite3_close((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3::close", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::close");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3::t_exec(HPHP::String const&)
_ZN4HPHP9c_SQLite36t_execERKNS_6StringE

(return value) => rax
this_ => rdi
sql => rsi
*/

bool th_7SQLite3_exec(ObjectData* this_, Value* sql) asm("_ZN4HPHP9c_SQLite36t_execERKNS_6StringE");

TypedValue* tg1_7SQLite3_exec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_exec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_7SQLite3_exec((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_7SQLite3_exec(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_7SQLite3_exec((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_exec(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::exec", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::exec");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Array HPHP::c_SQLite3::t_version()
_ZN4HPHP9c_SQLite39t_versionEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_7SQLite3_version(Value* _rv, ObjectData* this_) asm("_ZN4HPHP9c_SQLite39t_versionEv");

TypedValue* tg_7SQLite3_version(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_7SQLite3_version((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3::version", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::version");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
long long HPHP::c_SQLite3::t_lastinsertrowid()
_ZN4HPHP9c_SQLite317t_lastinsertrowidEv

(return value) => rax
this_ => rdi
*/

long long th_7SQLite3_lastinsertrowid(ObjectData* this_) asm("_ZN4HPHP9c_SQLite317t_lastinsertrowidEv");

TypedValue* tg_7SQLite3_lastinsertrowid(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_7SQLite3_lastinsertrowid((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3::lastinsertrowid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::lastinsertrowid");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
long long HPHP::c_SQLite3::t_lasterrorcode()
_ZN4HPHP9c_SQLite315t_lasterrorcodeEv

(return value) => rax
this_ => rdi
*/

long long th_7SQLite3_lasterrorcode(ObjectData* this_) asm("_ZN4HPHP9c_SQLite315t_lasterrorcodeEv");

TypedValue* tg_7SQLite3_lasterrorcode(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_7SQLite3_lasterrorcode((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3::lasterrorcode", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::lasterrorcode");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::String HPHP::c_SQLite3::t_lasterrormsg()
_ZN4HPHP9c_SQLite314t_lasterrormsgEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_7SQLite3_lasterrormsg(Value* _rv, ObjectData* this_) asm("_ZN4HPHP9c_SQLite314t_lasterrormsgEv");

TypedValue* tg_7SQLite3_lasterrormsg(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_7SQLite3_lasterrormsg((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3::lasterrormsg", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::lasterrormsg");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3::t_loadextension(HPHP::String const&)
_ZN4HPHP9c_SQLite315t_loadextensionERKNS_6StringE

(return value) => rax
this_ => rdi
extension => rsi
*/

bool th_7SQLite3_loadextension(ObjectData* this_, Value* extension) asm("_ZN4HPHP9c_SQLite315t_loadextensionERKNS_6StringE");

TypedValue* tg1_7SQLite3_loadextension(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_loadextension(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_7SQLite3_loadextension((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_7SQLite3_loadextension(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_7SQLite3_loadextension((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_loadextension(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::loadextension", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::loadextension");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
long long HPHP::c_SQLite3::t_changes()
_ZN4HPHP9c_SQLite39t_changesEv

(return value) => rax
this_ => rdi
*/

long long th_7SQLite3_changes(ObjectData* this_) asm("_ZN4HPHP9c_SQLite39t_changesEv");

TypedValue* tg_7SQLite3_changes(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_7SQLite3_changes((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3::changes", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::changes");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::String HPHP::c_SQLite3::t_escapestring(HPHP::String const&)
_ZN4HPHP9c_SQLite314t_escapestringERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
sql => rdx
*/

Value* th_7SQLite3_escapestring(Value* _rv, ObjectData* this_, Value* sql) asm("_ZN4HPHP9c_SQLite314t_escapestringERKNS_6StringE");

TypedValue* tg1_7SQLite3_escapestring(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_escapestring(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  th_7SQLite3_escapestring((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7SQLite3_escapestring(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfString;
          th_7SQLite3_escapestring((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_escapestring(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::escapestring", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::escapestring");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Variant HPHP::c_SQLite3::t_prepare(HPHP::String const&)
_ZN4HPHP9c_SQLite39t_prepareERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
sql => rdx
*/

TypedValue* th_7SQLite3_prepare(TypedValue* _rv, ObjectData* this_, Value* sql) asm("_ZN4HPHP9c_SQLite39t_prepareERKNS_6StringE");

TypedValue* tg1_7SQLite3_prepare(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_prepare(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_7SQLite3_prepare((rv), (this_), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7SQLite3_prepare(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_7SQLite3_prepare((&(rv)), (this_), (Value*)(args-0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_prepare(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::prepare", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::prepare");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Variant HPHP::c_SQLite3::t_query(HPHP::String const&)
_ZN4HPHP9c_SQLite37t_queryERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
sql => rdx
*/

TypedValue* th_7SQLite3_query(TypedValue* _rv, ObjectData* this_, Value* sql) asm("_ZN4HPHP9c_SQLite37t_queryERKNS_6StringE");

TypedValue* tg1_7SQLite3_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_7SQLite3_query((rv), (this_), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7SQLite3_query(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_7SQLite3_query((&(rv)), (this_), (Value*)(args-0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_query(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::query", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::query");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Variant HPHP::c_SQLite3::t_querysingle(HPHP::String const&, bool)
_ZN4HPHP9c_SQLite313t_querysingleERKNS_6StringEb

(return value) => rax
_rv => rdi
this_ => rsi
sql => rdx
entire_row => rcx
*/

TypedValue* th_7SQLite3_querysingle(TypedValue* _rv, ObjectData* this_, Value* sql, bool entire_row) asm("_ZN4HPHP9c_SQLite313t_querysingleERKNS_6StringEb");

TypedValue* tg1_7SQLite3_querysingle(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_querysingle(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_7SQLite3_querysingle((rv), (this_), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7SQLite3_querysingle(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-0)->m_type)) {
          th_7SQLite3_querysingle((&(rv)), (this_), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_querysingle(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::querysingle", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::querysingle");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3::t_createfunction(HPHP::String const&, HPHP::Variant const&, long long)
_ZN4HPHP9c_SQLite316t_createfunctionERKNS_6StringERKNS_7VariantEx

(return value) => rax
this_ => rdi
name => rsi
callback => rdx
argcount => rcx
*/

bool th_7SQLite3_createfunction(ObjectData* this_, Value* name, TypedValue* callback, long long argcount) asm("_ZN4HPHP9c_SQLite316t_createfunctionERKNS_6StringERKNS_7VariantEx");

TypedValue* tg1_7SQLite3_createfunction(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_createfunction(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_7SQLite3_createfunction((this_), (Value*)(args-0), (args-1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_7SQLite3_createfunction(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_7SQLite3_createfunction((this_), (Value*)(args-0), (args-1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_createfunction(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::createfunction", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::createfunction");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3::t_createaggregate(HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&, long long)
_ZN4HPHP9c_SQLite317t_createaggregateERKNS_6StringERKNS_7VariantES6_x

(return value) => rax
this_ => rdi
name => rsi
step => rdx
final => rcx
argcount => r8
*/

bool th_7SQLite3_createaggregate(ObjectData* this_, Value* name, TypedValue* step, TypedValue* final, long long argcount) asm("_ZN4HPHP9c_SQLite317t_createaggregateERKNS_6StringERKNS_7VariantES6_x");

TypedValue* tg1_7SQLite3_createaggregate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_createaggregate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_7SQLite3_createaggregate((this_), (Value*)(args-0), (args-1), (args-2), (count > 3) ? (long long)(args[-3].m_data.num) : (long long)(-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_7SQLite3_createaggregate(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 3LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_7SQLite3_createaggregate((this_), (Value*)(args-0), (args-1), (args-2), (count > 3) ? (long long)(args[-3].m_data.num) : (long long)(-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_createaggregate(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::createaggregate", count, 3, 4, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::createaggregate");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3::t_openblob(HPHP::String const&, HPHP::String const&, long long, HPHP::String const&)
_ZN4HPHP9c_SQLite310t_openblobERKNS_6StringES3_xS3_

(return value) => rax
this_ => rdi
table => rsi
column => rdx
rowid => rcx
dbname => r8
*/

bool th_7SQLite3_openblob(ObjectData* this_, Value* table, Value* column, long long rowid, Value* dbname) asm("_ZN4HPHP9c_SQLite310t_openblobERKNS_6StringES3_xS3_");

TypedValue* tg1_7SQLite3_openblob(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7SQLite3_openblob(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_7SQLite3_openblob((this_), (Value*)(args-0), (Value*)(args-1), (long long)(args[-2].m_data.num), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_7SQLite3_openblob(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 3LL && count <= 4LL) {
        if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (args-2)->m_type == KindOfInt64 && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_7SQLite3_openblob((this_), (Value*)(args-0), (Value*)(args-1), (long long)(args[-2].m_data.num), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7SQLite3_openblob(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3::openblob", count, 3, 4, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::openblob");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Variant HPHP::c_SQLite3::t___destruct()
_ZN4HPHP9c_SQLite312t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_7SQLite3___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP9c_SQLite312t___destructEv");

TypedValue* tg_7SQLite3___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_7SQLite3___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3::__destruct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

HPHP::VM::Instance* new_SQLite3Stmt_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SQLite3Stmt) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SQLite3Stmt(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_SQLite3Stmt::t___construct(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13c_SQLite3Stmt13t___constructERKNS_6ObjectERKNS_6StringE

this_ => rdi
dbobject => rsi
statement => rdx
*/

void th_11SQLite3Stmt___construct(ObjectData* this_, Value* dbobject, Value* statement) asm("_ZN4HPHP13c_SQLite3Stmt13t___constructERKNS_6ObjectERKNS_6StringE");

TypedValue* tg1_11SQLite3Stmt___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11SQLite3Stmt___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  th_11SQLite3Stmt___construct((this_), (Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* tg_11SQLite3Stmt___construct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_11SQLite3Stmt___construct((this_), (Value*)(args-0), (Value*)(args-1));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11SQLite3Stmt___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3Stmt::__construct", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Stmt::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
long long HPHP::c_SQLite3Stmt::t_paramcount()
_ZN4HPHP13c_SQLite3Stmt12t_paramcountEv

(return value) => rax
this_ => rdi
*/

long long th_11SQLite3Stmt_paramcount(ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt12t_paramcountEv");

TypedValue* tg_11SQLite3Stmt_paramcount(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_11SQLite3Stmt_paramcount((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Stmt::paramcount", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Stmt::paramcount");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3Stmt::t_close()
_ZN4HPHP13c_SQLite3Stmt7t_closeEv

(return value) => rax
this_ => rdi
*/

bool th_11SQLite3Stmt_close(ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt7t_closeEv");

TypedValue* tg_11SQLite3Stmt_close(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_11SQLite3Stmt_close((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Stmt::close", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Stmt::close");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3Stmt::t_reset()
_ZN4HPHP13c_SQLite3Stmt7t_resetEv

(return value) => rax
this_ => rdi
*/

bool th_11SQLite3Stmt_reset(ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt7t_resetEv");

TypedValue* tg_11SQLite3Stmt_reset(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_11SQLite3Stmt_reset((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Stmt::reset", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Stmt::reset");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3Stmt::t_clear()
_ZN4HPHP13c_SQLite3Stmt7t_clearEv

(return value) => rax
this_ => rdi
*/

bool th_11SQLite3Stmt_clear(ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt7t_clearEv");

TypedValue* tg_11SQLite3Stmt_clear(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_11SQLite3Stmt_clear((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Stmt::clear", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Stmt::clear");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3Stmt::t_bindparam(HPHP::Variant const&, HPHP::VRefParamValue const&, long long)
_ZN4HPHP13c_SQLite3Stmt11t_bindparamERKNS_7VariantERKNS_14VRefParamValueEx

(return value) => rax
this_ => rdi
name => rsi
parameter => rdx
type => rcx
*/

bool th_11SQLite3Stmt_bindparam(ObjectData* this_, TypedValue* name, TypedValue* parameter, long long type) asm("_ZN4HPHP13c_SQLite3Stmt11t_bindparamERKNS_7VariantERKNS_14VRefParamValueEx");

TypedValue* tg1_11SQLite3Stmt_bindparam(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11SQLite3Stmt_bindparam(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-2);
  rv->m_data.num = (th_11SQLite3Stmt_bindparam((this_), (args-0), (args-1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(k_SQLITE3_TEXT))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_11SQLite3Stmt_bindparam(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_11SQLite3Stmt_bindparam((this_), (args-0), (args-1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(k_SQLITE3_TEXT))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11SQLite3Stmt_bindparam(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3Stmt::bindparam", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Stmt::bindparam");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3Stmt::t_bindvalue(HPHP::Variant const&, HPHP::Variant const&, long long)
_ZN4HPHP13c_SQLite3Stmt11t_bindvalueERKNS_7VariantES3_x

(return value) => rax
this_ => rdi
name => rsi
parameter => rdx
type => rcx
*/

bool th_11SQLite3Stmt_bindvalue(ObjectData* this_, TypedValue* name, TypedValue* parameter, long long type) asm("_ZN4HPHP13c_SQLite3Stmt11t_bindvalueERKNS_7VariantES3_x");

TypedValue* tg1_11SQLite3Stmt_bindvalue(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11SQLite3Stmt_bindvalue(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-2);
  rv->m_data.num = (th_11SQLite3Stmt_bindvalue((this_), (args-0), (args-1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(k_SQLITE3_TEXT))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_11SQLite3Stmt_bindvalue(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_11SQLite3Stmt_bindvalue((this_), (args-0), (args-1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(k_SQLITE3_TEXT))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11SQLite3Stmt_bindvalue(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3Stmt::bindvalue", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Stmt::bindvalue");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Variant HPHP::c_SQLite3Stmt::t_execute()
_ZN4HPHP13c_SQLite3Stmt9t_executeEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_11SQLite3Stmt_execute(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt9t_executeEv");

TypedValue* tg_11SQLite3Stmt_execute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_11SQLite3Stmt_execute((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Stmt::execute", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Stmt::execute");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Variant HPHP::c_SQLite3Stmt::t___destruct()
_ZN4HPHP13c_SQLite3Stmt12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_11SQLite3Stmt___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt12t___destructEv");

TypedValue* tg_11SQLite3Stmt___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_11SQLite3Stmt___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Stmt::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Stmt::__destruct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

HPHP::VM::Instance* new_SQLite3Result_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SQLite3Result) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SQLite3Result(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_SQLite3Result::t___construct()
_ZN4HPHP15c_SQLite3Result13t___constructEv

this_ => rdi
*/

void th_13SQLite3Result___construct(ObjectData* this_) asm("_ZN4HPHP15c_SQLite3Result13t___constructEv");

TypedValue* tg_13SQLite3Result___construct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_13SQLite3Result___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Result::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Result::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
long long HPHP::c_SQLite3Result::t_numcolumns()
_ZN4HPHP15c_SQLite3Result12t_numcolumnsEv

(return value) => rax
this_ => rdi
*/

long long th_13SQLite3Result_numcolumns(ObjectData* this_) asm("_ZN4HPHP15c_SQLite3Result12t_numcolumnsEv");

TypedValue* tg_13SQLite3Result_numcolumns(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_13SQLite3Result_numcolumns((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Result::numcolumns", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Result::numcolumns");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::String HPHP::c_SQLite3Result::t_columnname(long long)
_ZN4HPHP15c_SQLite3Result12t_columnnameEx

(return value) => rax
_rv => rdi
this_ => rsi
column => rdx
*/

Value* th_13SQLite3Result_columnname(Value* _rv, ObjectData* this_, long long column) asm("_ZN4HPHP15c_SQLite3Result12t_columnnameEx");

TypedValue* tg1_13SQLite3Result_columnname(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_13SQLite3Result_columnname(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  th_13SQLite3Result_columnname((Value*)(rv), (this_), (long long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_13SQLite3Result_columnname(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfString;
          th_13SQLite3Result_columnname((Value*)(&(rv)), (this_), (long long)(args[-0].m_data.num));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_13SQLite3Result_columnname(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3Result::columnname", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Result::columnname");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
long long HPHP::c_SQLite3Result::t_columntype(long long)
_ZN4HPHP15c_SQLite3Result12t_columntypeEx

(return value) => rax
this_ => rdi
column => rsi
*/

long long th_13SQLite3Result_columntype(ObjectData* this_, long long column) asm("_ZN4HPHP15c_SQLite3Result12t_columntypeEx");

TypedValue* tg1_13SQLite3Result_columntype(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_13SQLite3Result_columntype(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (long long)th_13SQLite3Result_columntype((this_), (long long)(args[-0].m_data.num));
  return rv;
}

TypedValue* tg_13SQLite3Result_columntype(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfInt64;
          rv.m_data.num = (long long)th_13SQLite3Result_columntype((this_), (long long)(args[-0].m_data.num));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_13SQLite3Result_columntype(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SQLite3Result::columntype", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Result::columntype");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Variant HPHP::c_SQLite3Result::t_fetcharray(long long)
_ZN4HPHP15c_SQLite3Result12t_fetcharrayEx

(return value) => rax
_rv => rdi
this_ => rsi
mode => rdx
*/

TypedValue* th_13SQLite3Result_fetcharray(TypedValue* _rv, ObjectData* this_, long long mode) asm("_ZN4HPHP15c_SQLite3Result12t_fetcharrayEx");

TypedValue* tg1_13SQLite3Result_fetcharray(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_13SQLite3Result_fetcharray(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_13SQLite3Result_fetcharray((rv), (this_), (count > 0) ? (long long)(args[-0].m_data.num) : (long long)(k_SQLITE3_BOTH));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_13SQLite3Result_fetcharray(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
          th_13SQLite3Result_fetcharray((&(rv)), (this_), (count > 0) ? (long long)(args[-0].m_data.num) : (long long)(k_SQLITE3_BOTH));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_13SQLite3Result_fetcharray(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("SQLite3Result::fetcharray", 1, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Result::fetcharray");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3Result::t_reset()
_ZN4HPHP15c_SQLite3Result7t_resetEv

(return value) => rax
this_ => rdi
*/

bool th_13SQLite3Result_reset(ObjectData* this_) asm("_ZN4HPHP15c_SQLite3Result7t_resetEv");

TypedValue* tg_13SQLite3Result_reset(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_13SQLite3Result_reset((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Result::reset", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Result::reset");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SQLite3Result::t_finalize()
_ZN4HPHP15c_SQLite3Result10t_finalizeEv

(return value) => rax
this_ => rdi
*/

bool th_13SQLite3Result_finalize(ObjectData* this_) asm("_ZN4HPHP15c_SQLite3Result10t_finalizeEv");

TypedValue* tg_13SQLite3Result_finalize(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_13SQLite3Result_finalize((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Result::finalize", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Result::finalize");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Variant HPHP::c_SQLite3Result::t___destruct()
_ZN4HPHP15c_SQLite3Result12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_13SQLite3Result___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP15c_SQLite3Result12t___destructEv");

TypedValue* tg_13SQLite3Result___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_13SQLite3Result___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SQLite3Result::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SQLite3Result::__destruct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}


} // !HPHP

