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

#include "runtime/ext_hhvm/ext_hhvm.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/base/array/array_init.h"
#include "runtime/ext/ext.h"
#include "runtime/vm/class.h"
#include "runtime/vm/runtime.h"
#include <exception>

namespace HPHP {


HPHP::VM::Instance* new_SQLite3_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SQLite3) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SQLite3(cls);
  return inst;
}

IMPLEMENT_CLASS(SQLite3);
void th_7SQLite3___construct(ObjectData* this_, Value* filename, long flags, Value* encryption_key) asm("_ZN4HPHP9c_SQLite313t___constructERKNS_6StringElS3_");

void tg1_7SQLite3___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfNull;
  th_7SQLite3___construct((this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(k_SQLITE3_OPEN_READWRITE|k_SQLITE3_OPEN_CREATE), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
}

TypedValue* tg_7SQLite3___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 3) {
      if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_7SQLite3___construct((this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(k_SQLITE3_OPEN_READWRITE|k_SQLITE3_OPEN_CREATE), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
      } else {
        tg1_7SQLite3___construct(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::__construct", count, 1, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::__construct");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_7SQLite3_open(ObjectData* this_, Value* filename, long flags, Value* encryption_key) asm("_ZN4HPHP9c_SQLite36t_openERKNS_6StringElS3_");

void tg1_7SQLite3_open(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_open(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfNull;
  th_7SQLite3_open((this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(k_SQLITE3_OPEN_READWRITE|k_SQLITE3_OPEN_CREATE), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
}

TypedValue* tg_7SQLite3_open(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 3) {
      if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_7SQLite3_open((this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(k_SQLITE3_OPEN_READWRITE|k_SQLITE3_OPEN_CREATE), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
      } else {
        tg1_7SQLite3_open(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::open", count, 1, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::open");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_7SQLite3_busytimeout(ObjectData* this_, long msecs) asm("_ZN4HPHP9c_SQLite313t_busytimeoutEl");

void tg1_7SQLite3_busytimeout(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_busytimeout(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_7SQLite3_busytimeout((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* tg_7SQLite3_busytimeout(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_7SQLite3_busytimeout((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
      } else {
        tg1_7SQLite3_busytimeout(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::busytimeout", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::busytimeout");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_7SQLite3_close(ObjectData* this_) asm("_ZN4HPHP9c_SQLite37t_closeEv");

TypedValue* tg_7SQLite3_close(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_7SQLite3_close((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("SQLite3::close", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::close");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_7SQLite3_exec(ObjectData* this_, Value* sql) asm("_ZN4HPHP9c_SQLite36t_execERKNS_6StringE");

void tg1_7SQLite3_exec(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_exec(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_7SQLite3_exec((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_7SQLite3_exec(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_7SQLite3_exec((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_7SQLite3_exec(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::exec", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::exec");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_7SQLite3_version(Value* _rv, ObjectData* this_) asm("_ZN4HPHP9c_SQLite39t_versionEv");

TypedValue* tg_7SQLite3_version(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfArray;
      th_7SQLite3_version(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SQLite3::version", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::version");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_7SQLite3_lastinsertrowid(ObjectData* this_) asm("_ZN4HPHP9c_SQLite317t_lastinsertrowidEv");

TypedValue* tg_7SQLite3_lastinsertrowid(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_7SQLite3_lastinsertrowid((this_));
    } else {
      throw_toomany_arguments_nr("SQLite3::lastinsertrowid", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::lastinsertrowid");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_7SQLite3_lasterrorcode(ObjectData* this_) asm("_ZN4HPHP9c_SQLite315t_lasterrorcodeEv");

TypedValue* tg_7SQLite3_lasterrorcode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_7SQLite3_lasterrorcode((this_));
    } else {
      throw_toomany_arguments_nr("SQLite3::lasterrorcode", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::lasterrorcode");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_7SQLite3_lasterrormsg(Value* _rv, ObjectData* this_) asm("_ZN4HPHP9c_SQLite314t_lasterrormsgEv");

TypedValue* tg_7SQLite3_lasterrormsg(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_7SQLite3_lasterrormsg(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SQLite3::lasterrormsg", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::lasterrormsg");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_7SQLite3_loadextension(ObjectData* this_, Value* extension) asm("_ZN4HPHP9c_SQLite315t_loadextensionERKNS_6StringE");

void tg1_7SQLite3_loadextension(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_loadextension(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_7SQLite3_loadextension((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_7SQLite3_loadextension(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_7SQLite3_loadextension((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_7SQLite3_loadextension(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::loadextension", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::loadextension");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_7SQLite3_changes(ObjectData* this_) asm("_ZN4HPHP9c_SQLite39t_changesEv");

TypedValue* tg_7SQLite3_changes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_7SQLite3_changes((this_));
    } else {
      throw_toomany_arguments_nr("SQLite3::changes", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::changes");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_7SQLite3_escapestring(Value* _rv, ObjectData* this_, Value* sql) asm("_ZN4HPHP9c_SQLite314t_escapestringERKNS_6StringE");

void tg1_7SQLite3_escapestring(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_escapestring(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  th_7SQLite3_escapestring(&(rv->m_data), (this_), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_7SQLite3_escapestring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfString;
        th_7SQLite3_escapestring(&(rv->m_data), (this_), &args[-0].m_data);
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_7SQLite3_escapestring(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::escapestring", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::escapestring");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_7SQLite3_prepare(TypedValue* _rv, ObjectData* this_, Value* sql) asm("_ZN4HPHP9c_SQLite39t_prepareERKNS_6StringE");

void tg1_7SQLite3_prepare(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_prepare(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_7SQLite3_prepare(rv, (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_7SQLite3_prepare(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        th_7SQLite3_prepare(rv, (this_), &args[-0].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_7SQLite3_prepare(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::prepare", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::prepare");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_SQLite3Stmt_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SQLite3Stmt) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SQLite3Stmt(cls);
  return inst;
}

IMPLEMENT_CLASS(SQLite3Stmt);
void th_11SQLite3Stmt___construct(ObjectData* this_, Value* dbobject, Value* statement) asm("_ZN4HPHP13c_SQLite3Stmt13t___constructERKNS_6ObjectERKNS_6StringE");

void tg1_11SQLite3Stmt___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_11SQLite3Stmt___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  th_11SQLite3Stmt___construct((this_), &args[-0].m_data, &args[-1].m_data);
}

TypedValue* tg_11SQLite3Stmt___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          (args - 0)->m_type == KindOfObject) {
        rv->m_type = KindOfNull;
        th_11SQLite3Stmt___construct((this_), &args[-0].m_data, &args[-1].m_data);
      } else {
        tg1_11SQLite3Stmt___construct(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3Stmt::__construct", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Stmt::__construct");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_7SQLite3_query(TypedValue* _rv, ObjectData* this_, Value* sql) asm("_ZN4HPHP9c_SQLite37t_queryERKNS_6StringE");

void tg1_7SQLite3_query(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_query(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_7SQLite3_query(rv, (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_7SQLite3_query(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        th_7SQLite3_query(rv, (this_), &args[-0].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_7SQLite3_query(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::query", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::query");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_11SQLite3Stmt_execute(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt9t_executeEv");

TypedValue* tg_11SQLite3Stmt_execute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_11SQLite3Stmt_execute(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SQLite3Stmt::execute", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Stmt::execute");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_7SQLite3_querysingle(TypedValue* _rv, ObjectData* this_, Value* sql, bool entire_row) asm("_ZN4HPHP9c_SQLite313t_querysingleERKNS_6StringEb");

void tg1_7SQLite3_querysingle(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_querysingle(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
  th_7SQLite3_querysingle(rv, (this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_7SQLite3_querysingle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        th_7SQLite3_querysingle(rv, (this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_7SQLite3_querysingle(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::querysingle", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::querysingle");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_7SQLite3_createfunction(ObjectData* this_, Value* name, TypedValue* callback, long argcount) asm("_ZN4HPHP9c_SQLite316t_createfunctionERKNS_6StringERKNS_7VariantEl");

void tg1_7SQLite3_createfunction(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_createfunction(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_7SQLite3_createfunction((this_), &args[-0].m_data, (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1))) ? 1LL : 0LL;
}

TypedValue* tg_7SQLite3_createfunction(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 3) {
      if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_7SQLite3_createfunction((this_), &args[-0].m_data, (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(-1))) ? 1LL : 0LL;
      } else {
        tg1_7SQLite3_createfunction(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::createfunction", count, 2, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::createfunction");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_7SQLite3_createaggregate(ObjectData* this_, Value* name, TypedValue* step, TypedValue* final, long argcount) asm("_ZN4HPHP9c_SQLite317t_createaggregateERKNS_6StringERKNS_7VariantES6_l");

void tg1_7SQLite3_createaggregate(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_createaggregate(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_7SQLite3_createaggregate((this_), &args[-0].m_data, (args-1), (args-2), (count > 3) ? (long)(args[-3].m_data.num) : (long)(-1))) ? 1LL : 0LL;
}

TypedValue* tg_7SQLite3_createaggregate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 3 && count <= 4) {
      if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_7SQLite3_createaggregate((this_), &args[-0].m_data, (args-1), (args-2), (count > 3) ? (long)(args[-3].m_data.num) : (long)(-1))) ? 1LL : 0LL;
      } else {
        tg1_7SQLite3_createaggregate(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::createaggregate", count, 3, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::createaggregate");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_7SQLite3_openblob(ObjectData* this_, Value* table, Value* column, long rowid, Value* dbname) asm("_ZN4HPHP9c_SQLite310t_openblobERKNS_6StringES3_lS3_");

void tg1_7SQLite3_openblob(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_7SQLite3_openblob(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_7SQLite3_openblob((this_), &args[-0].m_data, &args[-1].m_data, (long)(args[-2].m_data.num), (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* tg_7SQLite3_openblob(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 3 && count <= 4) {
      if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
          (args - 2)->m_type == KindOfInt64 &&
          IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_7SQLite3_openblob((this_), &args[-0].m_data, &args[-1].m_data, (long)(args[-2].m_data.num), (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
      } else {
        tg1_7SQLite3_openblob(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3::openblob", count, 3, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3::openblob");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_11SQLite3Stmt_paramcount(ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt12t_paramcountEv");

TypedValue* tg_11SQLite3Stmt_paramcount(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_11SQLite3Stmt_paramcount((this_));
    } else {
      throw_toomany_arguments_nr("SQLite3Stmt::paramcount", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Stmt::paramcount");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_11SQLite3Stmt_close(ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt7t_closeEv");

TypedValue* tg_11SQLite3Stmt_close(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_11SQLite3Stmt_close((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("SQLite3Stmt::close", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Stmt::close");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_11SQLite3Stmt_reset(ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt7t_resetEv");

TypedValue* tg_11SQLite3Stmt_reset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_11SQLite3Stmt_reset((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("SQLite3Stmt::reset", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Stmt::reset");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_11SQLite3Stmt_clear(ObjectData* this_) asm("_ZN4HPHP13c_SQLite3Stmt7t_clearEv");

TypedValue* tg_11SQLite3Stmt_clear(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_11SQLite3Stmt_clear((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("SQLite3Stmt::clear", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Stmt::clear");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_11SQLite3Stmt_bindparam(ObjectData* this_, TypedValue* name, TypedValue* parameter, long type) asm("_ZN4HPHP13c_SQLite3Stmt11t_bindparamERKNS_7VariantERKNS_14VRefParamValueEl");

void tg1_11SQLite3Stmt_bindparam(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_11SQLite3Stmt_bindparam(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-2);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_11SQLite3Stmt_bindparam((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(k_SQLITE3_TEXT))) ? 1LL : 0LL;
}

TypedValue* tg_11SQLite3Stmt_bindparam(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 3) {
      if ((count <= 2 || (args - 2)->m_type == KindOfInt64)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_11SQLite3Stmt_bindparam((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(k_SQLITE3_TEXT))) ? 1LL : 0LL;
      } else {
        tg1_11SQLite3Stmt_bindparam(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3Stmt::bindparam", count, 2, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Stmt::bindparam");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_11SQLite3Stmt_bindvalue(ObjectData* this_, TypedValue* name, TypedValue* parameter, long type) asm("_ZN4HPHP13c_SQLite3Stmt11t_bindvalueERKNS_7VariantES3_l");

void tg1_11SQLite3Stmt_bindvalue(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_11SQLite3Stmt_bindvalue(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-2);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_11SQLite3Stmt_bindvalue((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(k_SQLITE3_TEXT))) ? 1LL : 0LL;
}

TypedValue* tg_11SQLite3Stmt_bindvalue(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 3) {
      if ((count <= 2 || (args - 2)->m_type == KindOfInt64)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_11SQLite3Stmt_bindvalue((this_), (args-0), (args-1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(k_SQLITE3_TEXT))) ? 1LL : 0LL;
      } else {
        tg1_11SQLite3Stmt_bindvalue(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3Stmt::bindvalue", count, 2, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Stmt::bindvalue");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_SQLite3Result_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SQLite3Result) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SQLite3Result(cls);
  return inst;
}

IMPLEMENT_CLASS(SQLite3Result);
void th_13SQLite3Result___construct(ObjectData* this_) asm("_ZN4HPHP15c_SQLite3Result13t___constructEv");

TypedValue* tg_13SQLite3Result___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_13SQLite3Result___construct((this_));
    } else {
      throw_toomany_arguments_nr("SQLite3Result::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Result::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_13SQLite3Result_numcolumns(ObjectData* this_) asm("_ZN4HPHP15c_SQLite3Result12t_numcolumnsEv");

TypedValue* tg_13SQLite3Result_numcolumns(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_13SQLite3Result_numcolumns((this_));
    } else {
      throw_toomany_arguments_nr("SQLite3Result::numcolumns", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Result::numcolumns");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_13SQLite3Result_columnname(Value* _rv, ObjectData* this_, long column) asm("_ZN4HPHP15c_SQLite3Result12t_columnnameEl");

void tg1_13SQLite3Result_columnname(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_13SQLite3Result_columnname(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  th_13SQLite3Result_columnname(&(rv->m_data), (this_), (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_13SQLite3Result_columnname(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfString;
        th_13SQLite3Result_columnname(&(rv->m_data), (this_), (long)(args[-0].m_data.num));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_13SQLite3Result_columnname(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3Result::columnname", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Result::columnname");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_13SQLite3Result_columntype(ObjectData* this_, long column) asm("_ZN4HPHP15c_SQLite3Result12t_columntypeEl");

void tg1_13SQLite3Result_columntype(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_13SQLite3Result_columntype(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)th_13SQLite3Result_columntype((this_), (long)(args[-0].m_data.num));
}

TypedValue* tg_13SQLite3Result_columntype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfInt64;
        rv->m_data.num = (int64_t)th_13SQLite3Result_columntype((this_), (long)(args[-0].m_data.num));
      } else {
        tg1_13SQLite3Result_columntype(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SQLite3Result::columntype", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Result::columntype");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_13SQLite3Result_fetcharray(TypedValue* _rv, ObjectData* this_, long mode) asm("_ZN4HPHP15c_SQLite3Result12t_fetcharrayEl");

void tg1_13SQLite3Result_fetcharray(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_13SQLite3Result_fetcharray(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_13SQLite3Result_fetcharray(rv, (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(k_SQLITE3_BOTH));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_13SQLite3Result_fetcharray(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || (args - 0)->m_type == KindOfInt64)) {
        th_13SQLite3Result_fetcharray(rv, (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(k_SQLITE3_BOTH));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_13SQLite3Result_fetcharray(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("SQLite3Result::fetcharray", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Result::fetcharray");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_13SQLite3Result_reset(ObjectData* this_) asm("_ZN4HPHP15c_SQLite3Result7t_resetEv");

TypedValue* tg_13SQLite3Result_reset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_13SQLite3Result_reset((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("SQLite3Result::reset", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Result::reset");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_13SQLite3Result_finalize(ObjectData* this_) asm("_ZN4HPHP15c_SQLite3Result10t_finalizeEv");

TypedValue* tg_13SQLite3Result_finalize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_13SQLite3Result_finalize((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("SQLite3Result::finalize", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SQLite3Result::finalize");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
