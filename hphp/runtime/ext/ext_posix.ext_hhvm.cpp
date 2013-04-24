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

/*
bool HPHP::f_posix_access(HPHP::String const&, int)
_ZN4HPHP14f_posix_accessERKNS_6StringEi

(return value) => rax
file => rdi
mode => rsi
*/

bool fh_posix_access(Value* file, int mode) asm("_ZN4HPHP14f_posix_accessERKNS_6StringEi");

TypedValue * fg1_posix_access(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_access(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_posix_access(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_access(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_access(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_access(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_access", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_posix_ctermid()
_ZN4HPHP15f_posix_ctermidEv

(return value) => rax
_rv => rdi
*/

Value* fh_posix_ctermid(Value* _rv) asm("_ZN4HPHP15f_posix_ctermidEv");

TypedValue* fg_posix_ctermid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_posix_ctermid((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_ctermid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_posix_get_last_error()
_ZN4HPHP22f_posix_get_last_errorEv

(return value) => rax
*/

long fh_posix_get_last_error() asm("_ZN4HPHP22f_posix_get_last_errorEv");

TypedValue* fg_posix_get_last_error(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_posix_get_last_error();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_get_last_error", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_posix_getcwd()
_ZN4HPHP14f_posix_getcwdEv

(return value) => rax
_rv => rdi
*/

Value* fh_posix_getcwd(Value* _rv) asm("_ZN4HPHP14f_posix_getcwdEv");

TypedValue* fg_posix_getcwd(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_posix_getcwd((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getcwd", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_posix_getegid()
_ZN4HPHP15f_posix_getegidEv

(return value) => rax
*/

long fh_posix_getegid() asm("_ZN4HPHP15f_posix_getegidEv");

TypedValue* fg_posix_getegid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_posix_getegid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getegid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_posix_geteuid()
_ZN4HPHP15f_posix_geteuidEv

(return value) => rax
*/

long fh_posix_geteuid() asm("_ZN4HPHP15f_posix_geteuidEv");

TypedValue* fg_posix_geteuid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_posix_geteuid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_geteuid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_posix_getgid()
_ZN4HPHP14f_posix_getgidEv

(return value) => rax
*/

long fh_posix_getgid() asm("_ZN4HPHP14f_posix_getgidEv");

TypedValue* fg_posix_getgid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_posix_getgid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getgid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_getgrgid(int)
_ZN4HPHP16f_posix_getgrgidEi

(return value) => rax
_rv => rdi
gid => rsi
*/

TypedValue* fh_posix_getgrgid(TypedValue* _rv, int gid) asm("_ZN4HPHP16f_posix_getgrgidEi");

TypedValue * fg1_posix_getgrgid(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_getgrgid(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_posix_getgrgid((rv), (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_posix_getgrgid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        fh_posix_getgrgid((&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_getgrgid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_getgrgid", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_getgrnam(HPHP::String const&)
_ZN4HPHP16f_posix_getgrnamERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

TypedValue* fh_posix_getgrnam(TypedValue* _rv, Value* name) asm("_ZN4HPHP16f_posix_getgrnamERKNS_6StringE");

TypedValue * fg1_posix_getgrnam(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_getgrnam(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_posix_getgrnam((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_posix_getgrnam(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_posix_getgrnam((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_getgrnam(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_getgrnam", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_getgroups()
_ZN4HPHP17f_posix_getgroupsEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_getgroups(TypedValue* _rv) asm("_ZN4HPHP17f_posix_getgroupsEv");

TypedValue* fg_posix_getgroups(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_posix_getgroups((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getgroups", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_getlogin()
_ZN4HPHP16f_posix_getloginEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_getlogin(TypedValue* _rv) asm("_ZN4HPHP16f_posix_getloginEv");

TypedValue* fg_posix_getlogin(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_posix_getlogin((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getlogin", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_getpgid(int)
_ZN4HPHP15f_posix_getpgidEi

(return value) => rax
_rv => rdi
pid => rsi
*/

TypedValue* fh_posix_getpgid(TypedValue* _rv, int pid) asm("_ZN4HPHP15f_posix_getpgidEi");

TypedValue * fg1_posix_getpgid(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_getpgid(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_posix_getpgid((rv), (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_posix_getpgid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        fh_posix_getpgid((&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_getpgid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_getpgid", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_posix_getpgrp()
_ZN4HPHP15f_posix_getpgrpEv

(return value) => rax
*/

long fh_posix_getpgrp() asm("_ZN4HPHP15f_posix_getpgrpEv");

TypedValue* fg_posix_getpgrp(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_posix_getpgrp();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getpgrp", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_posix_getpid()
_ZN4HPHP14f_posix_getpidEv

(return value) => rax
*/

long fh_posix_getpid() asm("_ZN4HPHP14f_posix_getpidEv");

TypedValue* fg_posix_getpid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_posix_getpid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getpid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_posix_getppid()
_ZN4HPHP15f_posix_getppidEv

(return value) => rax
*/

long fh_posix_getppid() asm("_ZN4HPHP15f_posix_getppidEv");

TypedValue* fg_posix_getppid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_posix_getppid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getppid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_getpwnam(HPHP::String const&)
_ZN4HPHP16f_posix_getpwnamERKNS_6StringE

(return value) => rax
_rv => rdi
username => rsi
*/

TypedValue* fh_posix_getpwnam(TypedValue* _rv, Value* username) asm("_ZN4HPHP16f_posix_getpwnamERKNS_6StringE");

TypedValue * fg1_posix_getpwnam(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_getpwnam(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_posix_getpwnam((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_posix_getpwnam(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_posix_getpwnam((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_getpwnam(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_getpwnam", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_getpwuid(int)
_ZN4HPHP16f_posix_getpwuidEi

(return value) => rax
_rv => rdi
uid => rsi
*/

TypedValue* fh_posix_getpwuid(TypedValue* _rv, int uid) asm("_ZN4HPHP16f_posix_getpwuidEi");

TypedValue * fg1_posix_getpwuid(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_getpwuid(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_posix_getpwuid((rv), (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_posix_getpwuid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        fh_posix_getpwuid((&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_getpwuid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_getpwuid", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_getrlimit()
_ZN4HPHP17f_posix_getrlimitEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_getrlimit(TypedValue* _rv) asm("_ZN4HPHP17f_posix_getrlimitEv");

TypedValue* fg_posix_getrlimit(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_posix_getrlimit((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getrlimit", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_getsid(int)
_ZN4HPHP14f_posix_getsidEi

(return value) => rax
_rv => rdi
pid => rsi
*/

TypedValue* fh_posix_getsid(TypedValue* _rv, int pid) asm("_ZN4HPHP14f_posix_getsidEi");

TypedValue * fg1_posix_getsid(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_getsid(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_posix_getsid((rv), (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_posix_getsid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        fh_posix_getsid((&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_getsid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_getsid", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_posix_getuid()
_ZN4HPHP14f_posix_getuidEv

(return value) => rax
*/

long fh_posix_getuid() asm("_ZN4HPHP14f_posix_getuidEv");

TypedValue* fg_posix_getuid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_posix_getuid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_getuid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_posix_initgroups(HPHP::String const&, int)
_ZN4HPHP18f_posix_initgroupsERKNS_6StringEi

(return value) => rax
name => rdi
base_group_id => rsi
*/

bool fh_posix_initgroups(Value* name, int base_group_id) asm("_ZN4HPHP18f_posix_initgroupsERKNS_6StringEi");

TypedValue * fg1_posix_initgroups(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_initgroups(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_posix_initgroups(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_initgroups(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_initgroups(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_initgroups(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_initgroups", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_posix_isatty(HPHP::Variant const&)
_ZN4HPHP14f_posix_isattyERKNS_7VariantE

(return value) => rax
fd => rdi
*/

bool fh_posix_isatty(TypedValue* fd) asm("_ZN4HPHP14f_posix_isattyERKNS_7VariantE");

TypedValue* fg_posix_isatty(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_posix_isatty((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("posix_isatty", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_posix_kill(int, int)
_ZN4HPHP12f_posix_killEii

(return value) => rax
pid => rdi
sig => rsi
*/

bool fh_posix_kill(int pid, int sig) asm("_ZN4HPHP12f_posix_killEii");

TypedValue * fg1_posix_kill(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_kill(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_posix_kill((int)(args[-0].m_data.num), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_kill(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_kill((int)(args[-0].m_data.num), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_kill(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_kill", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_posix_mkfifo(HPHP::String const&, int)
_ZN4HPHP14f_posix_mkfifoERKNS_6StringEi

(return value) => rax
pathname => rdi
mode => rsi
*/

bool fh_posix_mkfifo(Value* pathname, int mode) asm("_ZN4HPHP14f_posix_mkfifoERKNS_6StringEi");

TypedValue * fg1_posix_mkfifo(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_mkfifo(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_posix_mkfifo(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_mkfifo(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_mkfifo(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_mkfifo(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_mkfifo", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_posix_mknod(HPHP::String const&, int, int, int)
_ZN4HPHP13f_posix_mknodERKNS_6StringEiii

(return value) => rax
pathname => rdi
mode => rsi
major => rdx
minor => rcx
*/

bool fh_posix_mknod(Value* pathname, int mode, int major, int minor) asm("_ZN4HPHP13f_posix_mknodERKNS_6StringEiii");

TypedValue * fg1_posix_mknod(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_mknod(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_posix_mknod(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_mknod(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_mknod(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_mknod(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_mknod", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_posix_setegid(int)
_ZN4HPHP15f_posix_setegidEi

(return value) => rax
gid => rdi
*/

bool fh_posix_setegid(int gid) asm("_ZN4HPHP15f_posix_setegidEi");

TypedValue * fg1_posix_setegid(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_setegid(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_posix_setegid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_setegid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_setegid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_setegid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_setegid", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_posix_seteuid(int)
_ZN4HPHP15f_posix_seteuidEi

(return value) => rax
uid => rdi
*/

bool fh_posix_seteuid(int uid) asm("_ZN4HPHP15f_posix_seteuidEi");

TypedValue * fg1_posix_seteuid(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_seteuid(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_posix_seteuid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_seteuid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_seteuid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_seteuid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_seteuid", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_posix_setgid(int)
_ZN4HPHP14f_posix_setgidEi

(return value) => rax
gid => rdi
*/

bool fh_posix_setgid(int gid) asm("_ZN4HPHP14f_posix_setgidEi");

TypedValue * fg1_posix_setgid(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_setgid(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_posix_setgid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_setgid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_setgid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_setgid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_setgid", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_posix_setpgid(int, int)
_ZN4HPHP15f_posix_setpgidEii

(return value) => rax
pid => rdi
pgid => rsi
*/

bool fh_posix_setpgid(int pid, int pgid) asm("_ZN4HPHP15f_posix_setpgidEii");

TypedValue * fg1_posix_setpgid(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_setpgid(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_posix_setpgid((int)(args[-0].m_data.num), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_setpgid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_setpgid((int)(args[-0].m_data.num), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_setpgid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_setpgid", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_posix_setsid()
_ZN4HPHP14f_posix_setsidEv

(return value) => rax
*/

long fh_posix_setsid() asm("_ZN4HPHP14f_posix_setsidEv");

TypedValue* fg_posix_setsid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_posix_setsid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_setsid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_posix_setuid(int)
_ZN4HPHP14f_posix_setuidEi

(return value) => rax
uid => rdi
*/

bool fh_posix_setuid(int uid) asm("_ZN4HPHP14f_posix_setuidEi");

TypedValue * fg1_posix_setuid(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_setuid(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_posix_setuid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_posix_setuid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_posix_setuid((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_setuid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_setuid", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_posix_strerror(int)
_ZN4HPHP16f_posix_strerrorEi

(return value) => rax
_rv => rdi
errnum => rsi
*/

Value* fh_posix_strerror(Value* _rv, int errnum) asm("_ZN4HPHP16f_posix_strerrorEi");

TypedValue * fg1_posix_strerror(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_posix_strerror(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_posix_strerror((&rv->m_data), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_posix_strerror(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfString;
        fh_posix_strerror((&rv.m_data), (int)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_posix_strerror(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("posix_strerror", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_times()
_ZN4HPHP13f_posix_timesEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_times(TypedValue* _rv) asm("_ZN4HPHP13f_posix_timesEv");

TypedValue* fg_posix_times(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_posix_times((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_times", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_ttyname(HPHP::Variant const&)
_ZN4HPHP15f_posix_ttynameERKNS_7VariantE

(return value) => rax
_rv => rdi
fd => rsi
*/

TypedValue* fh_posix_ttyname(TypedValue* _rv, TypedValue* fd) asm("_ZN4HPHP15f_posix_ttynameERKNS_7VariantE");

TypedValue* fg_posix_ttyname(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_posix_ttyname((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("posix_ttyname", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_posix_uname()
_ZN4HPHP13f_posix_unameEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_uname(TypedValue* _rv) asm("_ZN4HPHP13f_posix_unameEv");

TypedValue* fg_posix_uname(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_posix_uname((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("posix_uname", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

