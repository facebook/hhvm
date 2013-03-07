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
HPHP::String HPHP::f_magickgetcopyright()
_ZN4HPHP20f_magickgetcopyrightEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgetcopyright(Value* _rv) asm("_ZN4HPHP20f_magickgetcopyrightEv");

TypedValue* fg_magickgetcopyright(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_magickgetcopyright((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("magickgetcopyright", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgethomeurl()
_ZN4HPHP18f_magickgethomeurlEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgethomeurl(Value* _rv) asm("_ZN4HPHP18f_magickgethomeurlEv");

TypedValue* fg_magickgethomeurl(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_magickgethomeurl((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("magickgethomeurl", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetpackagename()
_ZN4HPHP22f_magickgetpackagenameEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgetpackagename(Value* _rv) asm("_ZN4HPHP22f_magickgetpackagenameEv");

TypedValue* fg_magickgetpackagename(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_magickgetpackagename((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("magickgetpackagename", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetquantumdepth()
_ZN4HPHP23f_magickgetquantumdepthEv

(return value) => xmm0
*/

double fh_magickgetquantumdepth() asm("_ZN4HPHP23f_magickgetquantumdepthEv");

TypedValue* fg_magickgetquantumdepth(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfDouble;
      rv.m_data.dbl = fh_magickgetquantumdepth();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("magickgetquantumdepth", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetreleasedate()
_ZN4HPHP22f_magickgetreleasedateEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgetreleasedate(Value* _rv) asm("_ZN4HPHP22f_magickgetreleasedateEv");

TypedValue* fg_magickgetreleasedate(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_magickgetreleasedate((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("magickgetreleasedate", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetresourcelimit(int)
_ZN4HPHP24f_magickgetresourcelimitEi

(return value) => xmm0
resource_type => rdi
*/

double fh_magickgetresourcelimit(int resource_type) asm("_ZN4HPHP24f_magickgetresourcelimitEi");

TypedValue * fg1_magickgetresourcelimit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetresourcelimit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToInt64InPlace(args-0);
  rv->m_data.dbl = fh_magickgetresourcelimit((int)(args[-0].m_data.num));
  return rv;
}

TypedValue* fg_magickgetresourcelimit(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetresourcelimit((int)(args[-0].m_data.num));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetresourcelimit(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetresourcelimit", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetversion()
_ZN4HPHP18f_magickgetversionEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgetversion(Value* _rv) asm("_ZN4HPHP18f_magickgetversionEv");

TypedValue* fg_magickgetversion(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      fh_magickgetversion((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("magickgetversion", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetversionnumber()
_ZN4HPHP24f_magickgetversionnumberEv

(return value) => rax
*/

long fh_magickgetversionnumber() asm("_ZN4HPHP24f_magickgetversionnumberEv");

TypedValue* fg_magickgetversionnumber(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_magickgetversionnumber();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("magickgetversionnumber", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetversionstring()
_ZN4HPHP24f_magickgetversionstringEv

(return value) => rax
_rv => rdi
*/

Value* fh_magickgetversionstring(Value* _rv) asm("_ZN4HPHP24f_magickgetversionstringEv");

TypedValue* fg_magickgetversionstring(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_magickgetversionstring((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("magickgetversionstring", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickqueryconfigureoption(HPHP::String const&)
_ZN4HPHP28f_magickqueryconfigureoptionERKNS_6StringE

(return value) => rax
_rv => rdi
option => rsi
*/

Value* fh_magickqueryconfigureoption(Value* _rv, Value* option) asm("_ZN4HPHP28f_magickqueryconfigureoptionERKNS_6StringE");

TypedValue * fg1_magickqueryconfigureoption(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickqueryconfigureoption(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_magickqueryconfigureoption((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickqueryconfigureoption(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfString;
        fh_magickqueryconfigureoption((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickqueryconfigureoption(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickqueryconfigureoption", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickqueryconfigureoptions(HPHP::String const&)
_ZN4HPHP29f_magickqueryconfigureoptionsERKNS_6StringE

(return value) => rax
_rv => rdi
pattern => rsi
*/

Value* fh_magickqueryconfigureoptions(Value* _rv, Value* pattern) asm("_ZN4HPHP29f_magickqueryconfigureoptionsERKNS_6StringE");

TypedValue * fg1_magickqueryconfigureoptions(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickqueryconfigureoptions(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToStringInPlace(args-0);
  fh_magickqueryconfigureoptions((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickqueryconfigureoptions(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfArray;
        fh_magickqueryconfigureoptions((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickqueryconfigureoptions(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickqueryconfigureoptions", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickqueryfonts(HPHP::String const&)
_ZN4HPHP18f_magickqueryfontsERKNS_6StringE

(return value) => rax
_rv => rdi
pattern => rsi
*/

Value* fh_magickqueryfonts(Value* _rv, Value* pattern) asm("_ZN4HPHP18f_magickqueryfontsERKNS_6StringE");

TypedValue * fg1_magickqueryfonts(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickqueryfonts(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToStringInPlace(args-0);
  fh_magickqueryfonts((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickqueryfonts(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfArray;
        fh_magickqueryfonts((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickqueryfonts(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickqueryfonts", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickqueryformats(HPHP::String const&)
_ZN4HPHP20f_magickqueryformatsERKNS_6StringE

(return value) => rax
_rv => rdi
pattern => rsi
*/

Value* fh_magickqueryformats(Value* _rv, Value* pattern) asm("_ZN4HPHP20f_magickqueryformatsERKNS_6StringE");

TypedValue * fg1_magickqueryformats(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickqueryformats(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToStringInPlace(args-0);
  fh_magickqueryformats((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickqueryformats(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfArray;
        fh_magickqueryformats((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickqueryformats(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickqueryformats", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetresourcelimit(int, double)
_ZN4HPHP24f_magicksetresourcelimitEid

(return value) => rax
resource_type => rdi
limit => xmm0
*/

bool fh_magicksetresourcelimit(int resource_type, double limit) asm("_ZN4HPHP24f_magicksetresourcelimitEid");

TypedValue * fg1_magicksetresourcelimit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetresourcelimit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetresourcelimit((int)(args[-0].m_data.num), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetresourcelimit(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetresourcelimit((int)(args[-0].m_data.num), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetresourcelimit(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetresourcelimit", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_newdrawingwand()
_ZN4HPHP16f_newdrawingwandEv

(return value) => rax
_rv => rdi
*/

Value* fh_newdrawingwand(Value* _rv) asm("_ZN4HPHP16f_newdrawingwandEv");

TypedValue* fg_newdrawingwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfObject;
      fh_newdrawingwand((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("newdrawingwand", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_newmagickwand()
_ZN4HPHP15f_newmagickwandEv

(return value) => rax
_rv => rdi
*/

Value* fh_newmagickwand(Value* _rv) asm("_ZN4HPHP15f_newmagickwandEv");

TypedValue* fg_newmagickwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfObject;
      fh_newmagickwand((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("newmagickwand", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_newpixeliterator(HPHP::Object const&)
_ZN4HPHP18f_newpixeliteratorERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_newpixeliterator(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP18f_newpixeliteratorERKNS_6ObjectE");

TypedValue * fg1_newpixeliterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_newpixeliterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_newpixeliterator((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_newpixeliterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_newpixeliterator((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_newpixeliterator(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("newpixeliterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_newpixelregioniterator(HPHP::Object const&, int, int, int, int)
_ZN4HPHP24f_newpixelregioniteratorERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
mgck_wnd => rsi
x => rdx
y => rcx
columns => r8
rows => r9
*/

Value* fh_newpixelregioniterator(Value* _rv, Value* mgck_wnd, int x, int y, int columns, int rows) asm("_ZN4HPHP24f_newpixelregioniteratorERKNS_6ObjectEiiii");

TypedValue * fg1_newpixelregioniterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_newpixelregioniterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_newpixelregioniterator((Value*)(rv), (Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_newpixelregioniterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_newpixelregioniterator((Value*)(&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_newpixelregioniterator(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("newpixelregioniterator", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_newpixelwand(HPHP::String const&)
_ZN4HPHP14f_newpixelwandERKNS_6StringE

(return value) => rax
_rv => rdi
imagemagick_col_str => rsi
*/

Value* fh_newpixelwand(Value* _rv, Value* imagemagick_col_str) asm("_ZN4HPHP14f_newpixelwandERKNS_6StringE");

TypedValue * fg1_newpixelwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_newpixelwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  fh_newpixelwand((Value*)(rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_newpixelwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv.m_type = KindOfObject;
        fh_newpixelwand((Value*)(&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_newpixelwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("newpixelwand", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_newpixelwandarray(int)
_ZN4HPHP19f_newpixelwandarrayEi

(return value) => rax
_rv => rdi
num_pxl_wnds => rsi
*/

Value* fh_newpixelwandarray(Value* _rv, int num_pxl_wnds) asm("_ZN4HPHP19f_newpixelwandarrayEi");

TypedValue * fg1_newpixelwandarray(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_newpixelwandarray(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToInt64InPlace(args-0);
  fh_newpixelwandarray((Value*)(rv), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_newpixelwandarray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfArray;
        fh_newpixelwandarray((Value*)(&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_newpixelwandarray(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("newpixelwandarray", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_newpixelwands(int)
_ZN4HPHP15f_newpixelwandsEi

(return value) => rax
_rv => rdi
num_pxl_wnds => rsi
*/

Value* fh_newpixelwands(Value* _rv, int num_pxl_wnds) asm("_ZN4HPHP15f_newpixelwandsEi");

TypedValue * fg1_newpixelwands(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_newpixelwands(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToInt64InPlace(args-0);
  fh_newpixelwands((Value*)(rv), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_newpixelwands(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfArray;
        fh_newpixelwands((Value*)(&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_newpixelwands(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("newpixelwands", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_destroydrawingwand(HPHP::Object const&)
_ZN4HPHP20f_destroydrawingwandERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_destroydrawingwand(Value* drw_wnd) asm("_ZN4HPHP20f_destroydrawingwandERKNS_6ObjectE");

TypedValue * fg1_destroydrawingwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_destroydrawingwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_destroydrawingwand((Value*)(args-0));
  return rv;
}

TypedValue* fg_destroydrawingwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_destroydrawingwand((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_destroydrawingwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("destroydrawingwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_destroymagickwand(HPHP::Object const&)
_ZN4HPHP19f_destroymagickwandERKNS_6ObjectE

mgck_wnd => rdi
*/

void fh_destroymagickwand(Value* mgck_wnd) asm("_ZN4HPHP19f_destroymagickwandERKNS_6ObjectE");

TypedValue * fg1_destroymagickwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_destroymagickwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_destroymagickwand((Value*)(args-0));
  return rv;
}

TypedValue* fg_destroymagickwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_destroymagickwand((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_destroymagickwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("destroymagickwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_destroypixeliterator(HPHP::Object const&)
_ZN4HPHP22f_destroypixeliteratorERKNS_6ObjectE

pxl_iter => rdi
*/

void fh_destroypixeliterator(Value* pxl_iter) asm("_ZN4HPHP22f_destroypixeliteratorERKNS_6ObjectE");

TypedValue * fg1_destroypixeliterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_destroypixeliterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_destroypixeliterator((Value*)(args-0));
  return rv;
}

TypedValue* fg_destroypixeliterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_destroypixeliterator((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_destroypixeliterator(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("destroypixeliterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_destroypixelwand(HPHP::Object const&)
_ZN4HPHP18f_destroypixelwandERKNS_6ObjectE

pxl_wnd => rdi
*/

void fh_destroypixelwand(Value* pxl_wnd) asm("_ZN4HPHP18f_destroypixelwandERKNS_6ObjectE");

TypedValue * fg1_destroypixelwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_destroypixelwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_destroypixelwand((Value*)(args-0));
  return rv;
}

TypedValue* fg_destroypixelwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_destroypixelwand((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_destroypixelwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("destroypixelwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_destroypixelwandarray(HPHP::Array const&)
_ZN4HPHP23f_destroypixelwandarrayERKNS_5ArrayE

pxl_wnd_array => rdi
*/

void fh_destroypixelwandarray(Value* pxl_wnd_array) asm("_ZN4HPHP23f_destroypixelwandarrayERKNS_5ArrayE");

TypedValue * fg1_destroypixelwandarray(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_destroypixelwandarray(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToArrayInPlace(args-0);
  fh_destroypixelwandarray((Value*)(args-0));
  return rv;
}

TypedValue* fg_destroypixelwandarray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfArray) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_destroypixelwandarray((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_destroypixelwandarray(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("destroypixelwandarray", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_destroypixelwands(HPHP::Array const&)
_ZN4HPHP19f_destroypixelwandsERKNS_5ArrayE

pxl_wnd_array => rdi
*/

void fh_destroypixelwands(Value* pxl_wnd_array) asm("_ZN4HPHP19f_destroypixelwandsERKNS_5ArrayE");

TypedValue * fg1_destroypixelwands(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_destroypixelwands(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToArrayInPlace(args-0);
  fh_destroypixelwands((Value*)(args-0));
  return rv;
}

TypedValue* fg_destroypixelwands(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfArray) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_destroypixelwands((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_destroypixelwands(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("destroypixelwands", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_isdrawingwand(HPHP::Variant const&)
_ZN4HPHP15f_isdrawingwandERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_isdrawingwand(TypedValue* var) asm("_ZN4HPHP15f_isdrawingwandERKNS_7VariantE");

TypedValue* fg_isdrawingwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_isdrawingwand((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("isdrawingwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_ismagickwand(HPHP::Variant const&)
_ZN4HPHP14f_ismagickwandERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_ismagickwand(TypedValue* var) asm("_ZN4HPHP14f_ismagickwandERKNS_7VariantE");

TypedValue* fg_ismagickwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_ismagickwand((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("ismagickwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_ispixeliterator(HPHP::Variant const&)
_ZN4HPHP17f_ispixeliteratorERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_ispixeliterator(TypedValue* var) asm("_ZN4HPHP17f_ispixeliteratorERKNS_7VariantE");

TypedValue* fg_ispixeliterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_ispixeliterator((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("ispixeliterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_ispixelwand(HPHP::Variant const&)
_ZN4HPHP13f_ispixelwandERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_ispixelwand(TypedValue* var) asm("_ZN4HPHP13f_ispixelwandERKNS_7VariantE");

TypedValue* fg_ispixelwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_ispixelwand((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("ispixelwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_cleardrawingwand(HPHP::Object const&)
_ZN4HPHP18f_cleardrawingwandERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_cleardrawingwand(Value* drw_wnd) asm("_ZN4HPHP18f_cleardrawingwandERKNS_6ObjectE");

TypedValue * fg1_cleardrawingwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_cleardrawingwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_cleardrawingwand((Value*)(args-0));
  return rv;
}

TypedValue* fg_cleardrawingwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_cleardrawingwand((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_cleardrawingwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("cleardrawingwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_clearmagickwand(HPHP::Object const&)
_ZN4HPHP17f_clearmagickwandERKNS_6ObjectE

mgck_wnd => rdi
*/

void fh_clearmagickwand(Value* mgck_wnd) asm("_ZN4HPHP17f_clearmagickwandERKNS_6ObjectE");

TypedValue * fg1_clearmagickwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_clearmagickwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_clearmagickwand((Value*)(args-0));
  return rv;
}

TypedValue* fg_clearmagickwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_clearmagickwand((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_clearmagickwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("clearmagickwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_clearpixeliterator(HPHP::Object const&)
_ZN4HPHP20f_clearpixeliteratorERKNS_6ObjectE

pxl_iter => rdi
*/

void fh_clearpixeliterator(Value* pxl_iter) asm("_ZN4HPHP20f_clearpixeliteratorERKNS_6ObjectE");

TypedValue * fg1_clearpixeliterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_clearpixeliterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_clearpixeliterator((Value*)(args-0));
  return rv;
}

TypedValue* fg_clearpixeliterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_clearpixeliterator((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_clearpixeliterator(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("clearpixeliterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_clearpixelwand(HPHP::Object const&)
_ZN4HPHP16f_clearpixelwandERKNS_6ObjectE

pxl_wnd => rdi
*/

void fh_clearpixelwand(Value* pxl_wnd) asm("_ZN4HPHP16f_clearpixelwandERKNS_6ObjectE");

TypedValue * fg1_clearpixelwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_clearpixelwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_clearpixelwand((Value*)(args-0));
  return rv;
}

TypedValue* fg_clearpixelwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_clearpixelwand((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_clearpixelwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("clearpixelwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_clonedrawingwand(HPHP::Object const&)
_ZN4HPHP18f_clonedrawingwandERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_clonedrawingwand(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_clonedrawingwandERKNS_6ObjectE");

TypedValue * fg1_clonedrawingwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_clonedrawingwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_clonedrawingwand((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_clonedrawingwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_clonedrawingwand((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_clonedrawingwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("clonedrawingwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_clonemagickwand(HPHP::Object const&)
_ZN4HPHP17f_clonemagickwandERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_clonemagickwand(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP17f_clonemagickwandERKNS_6ObjectE");

TypedValue * fg1_clonemagickwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_clonemagickwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_clonemagickwand((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_clonemagickwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_clonemagickwand((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_clonemagickwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("clonemagickwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_wandgetexception(HPHP::Object const&)
_ZN4HPHP18f_wandgetexceptionERKNS_6ObjectE

(return value) => rax
_rv => rdi
wnd => rsi
*/

Value* fh_wandgetexception(Value* _rv, Value* wnd) asm("_ZN4HPHP18f_wandgetexceptionERKNS_6ObjectE");

TypedValue * fg1_wandgetexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_wandgetexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_wandgetexception((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_wandgetexception(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_wandgetexception((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_wandgetexception(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("wandgetexception", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_wandgetexceptionstring(HPHP::Object const&)
_ZN4HPHP24f_wandgetexceptionstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
wnd => rsi
*/

Value* fh_wandgetexceptionstring(Value* _rv, Value* wnd) asm("_ZN4HPHP24f_wandgetexceptionstringERKNS_6ObjectE");

TypedValue * fg1_wandgetexceptionstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_wandgetexceptionstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_wandgetexceptionstring((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_wandgetexceptionstring(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_wandgetexceptionstring((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_wandgetexceptionstring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("wandgetexceptionstring", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_wandgetexceptiontype(HPHP::Object const&)
_ZN4HPHP22f_wandgetexceptiontypeERKNS_6ObjectE

(return value) => rax
wnd => rdi
*/

long fh_wandgetexceptiontype(Value* wnd) asm("_ZN4HPHP22f_wandgetexceptiontypeERKNS_6ObjectE");

TypedValue * fg1_wandgetexceptiontype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_wandgetexceptiontype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_wandgetexceptiontype((Value*)(args-0));
  return rv;
}

TypedValue* fg_wandgetexceptiontype(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_wandgetexceptiontype((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_wandgetexceptiontype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("wandgetexceptiontype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_wandhasexception(HPHP::Object const&)
_ZN4HPHP18f_wandhasexceptionERKNS_6ObjectE

(return value) => rax
wnd => rdi
*/

bool fh_wandhasexception(Value* wnd) asm("_ZN4HPHP18f_wandhasexceptionERKNS_6ObjectE");

TypedValue * fg1_wandhasexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_wandhasexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_wandhasexception((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_wandhasexception(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_wandhasexception((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_wandhasexception(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("wandhasexception", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawaffine(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP12f_drawaffineERKNS_6ObjectEdddddd

drw_wnd => rdi
sx => xmm0
sy => xmm1
rx => xmm2
ry => xmm3
tx => xmm4
ty => xmm5
*/

void fh_drawaffine(Value* drw_wnd, double sx, double sy, double rx, double ry, double tx, double ty) asm("_ZN4HPHP12f_drawaffineERKNS_6ObjectEdddddd");

TypedValue * fg1_drawaffine(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawaffine(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawaffine((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
  return rv;
}

TypedValue* fg_drawaffine(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 7LL) {
      if ((args-6)->m_type == KindOfDouble && (args-5)->m_type == KindOfDouble && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawaffine((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawaffine(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawaffine", count, 7, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawannotation(HPHP::Object const&, double, double, HPHP::String const&)
_ZN4HPHP16f_drawannotationERKNS_6ObjectEddRKNS_6StringE

drw_wnd => rdi
x => xmm0
y => xmm1
text => rsi
*/

void fh_drawannotation(Value* drw_wnd, double x, double y, Value* text) asm("_ZN4HPHP16f_drawannotationERKNS_6ObjectEddRKNS_6StringE");

TypedValue * fg1_drawannotation(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawannotation(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawannotation((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (Value*)(args-3));
  return rv;
}

TypedValue* fg_drawannotation(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-3)->m_type) && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawannotation((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (Value*)(args-3));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawannotation(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawannotation", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawarc(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP9f_drawarcERKNS_6ObjectEdddddd

drw_wnd => rdi
sx => xmm0
sy => xmm1
ex => xmm2
ey => xmm3
sd => xmm4
ed => xmm5
*/

void fh_drawarc(Value* drw_wnd, double sx, double sy, double ex, double ey, double sd, double ed) asm("_ZN4HPHP9f_drawarcERKNS_6ObjectEdddddd");

TypedValue * fg1_drawarc(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawarc(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawarc((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
  return rv;
}

TypedValue* fg_drawarc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 7LL) {
      if ((args-6)->m_type == KindOfDouble && (args-5)->m_type == KindOfDouble && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawarc((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawarc(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawarc", count, 7, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawbezier(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP12f_drawbezierERKNS_6ObjectERKNS_5ArrayE

drw_wnd => rdi
x_y_points_array => rsi
*/

void fh_drawbezier(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP12f_drawbezierERKNS_6ObjectERKNS_5ArrayE");

TypedValue * fg1_drawbezier(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawbezier(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawbezier((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_drawbezier(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawbezier((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawbezier(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawbezier", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawcircle(HPHP::Object const&, double, double, double, double)
_ZN4HPHP12f_drawcircleERKNS_6ObjectEdddd

drw_wnd => rdi
ox => xmm0
oy => xmm1
px => xmm2
py => xmm3
*/

void fh_drawcircle(Value* drw_wnd, double ox, double oy, double px, double py) asm("_ZN4HPHP12f_drawcircleERKNS_6ObjectEdddd");

TypedValue * fg1_drawcircle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawcircle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawcircle((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
  return rv;
}

TypedValue* fg_drawcircle(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawcircle((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawcircle(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawcircle", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawcolor(HPHP::Object const&, double, double, int)
_ZN4HPHP11f_drawcolorERKNS_6ObjectEddi

drw_wnd => rdi
x => xmm0
y => xmm1
paint_method => rsi
*/

void fh_drawcolor(Value* drw_wnd, double x, double y, int paint_method) asm("_ZN4HPHP11f_drawcolorERKNS_6ObjectEddi");

TypedValue * fg1_drawcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawcolor((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num));
  return rv;
}

TypedValue* fg_drawcolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawcolor((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawcolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawcolor", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawcomment(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13f_drawcommentERKNS_6ObjectERKNS_6StringE

drw_wnd => rdi
comment => rsi
*/

void fh_drawcomment(Value* drw_wnd, Value* comment) asm("_ZN4HPHP13f_drawcommentERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_drawcomment(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawcomment(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawcomment((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_drawcomment(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawcomment((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawcomment(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawcomment", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_drawcomposite(HPHP::Object const&, int, double, double, double, double, HPHP::Object const&)
_ZN4HPHP15f_drawcompositeERKNS_6ObjectEiddddS2_

(return value) => rax
drw_wnd => rdi
composite_operator => rsi
x => xmm0
y => xmm1
width => xmm2
height => xmm3
mgck_wnd => rdx
*/

bool fh_drawcomposite(Value* drw_wnd, int composite_operator, double x, double y, double width, double height, Value* mgck_wnd) asm("_ZN4HPHP15f_drawcompositeERKNS_6ObjectEiddddS2_");

TypedValue * fg1_drawcomposite(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawcomposite(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-6)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_drawcomposite((Value*)(args-0), (int)(args[-1].m_data.num), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (Value*)(args-6))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_drawcomposite(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 7LL) {
      if ((args-6)->m_type == KindOfObject && (args-5)->m_type == KindOfDouble && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_drawcomposite((Value*)(args-0), (int)(args[-1].m_data.num), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (Value*)(args-6))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawcomposite(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawcomposite", count, 7, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawellipse(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP13f_drawellipseERKNS_6ObjectEdddddd

drw_wnd => rdi
ox => xmm0
oy => xmm1
rx => xmm2
ry => xmm3
start => xmm4
end => xmm5
*/

void fh_drawellipse(Value* drw_wnd, double ox, double oy, double rx, double ry, double start, double end) asm("_ZN4HPHP13f_drawellipseERKNS_6ObjectEdddddd");

TypedValue * fg1_drawellipse(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawellipse(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawellipse((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
  return rv;
}

TypedValue* fg_drawellipse(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 7LL) {
      if ((args-6)->m_type == KindOfDouble && (args-5)->m_type == KindOfDouble && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawellipse((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawellipse(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawellipse", count, 7, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_drawgetclippath(HPHP::Object const&)
_ZN4HPHP17f_drawgetclippathERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetclippath(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP17f_drawgetclippathERKNS_6ObjectE");

TypedValue * fg1_drawgetclippath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetclippath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_drawgetclippath((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgetclippath(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_drawgetclippath((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetclippath(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetclippath", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgetcliprule(HPHP::Object const&)
_ZN4HPHP17f_drawgetclipruleERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgetcliprule(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetclipruleERKNS_6ObjectE");

TypedValue * fg1_drawgetcliprule(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetcliprule(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgetcliprule((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetcliprule(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgetcliprule((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetcliprule(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetcliprule", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgetclipunits(HPHP::Object const&)
_ZN4HPHP18f_drawgetclipunitsERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgetclipunits(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetclipunitsERKNS_6ObjectE");

TypedValue * fg1_drawgetclipunits(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetclipunits(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgetclipunits((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetclipunits(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgetclipunits((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetclipunits(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetclipunits", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_drawgetexception(HPHP::Object const&)
_ZN4HPHP18f_drawgetexceptionERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetexception(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_drawgetexceptionERKNS_6ObjectE");

TypedValue * fg1_drawgetexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_drawgetexception((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgetexception(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_drawgetexception((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetexception(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetexception", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_drawgetexceptionstring(HPHP::Object const&)
_ZN4HPHP24f_drawgetexceptionstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetexceptionstring(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP24f_drawgetexceptionstringERKNS_6ObjectE");

TypedValue * fg1_drawgetexceptionstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetexceptionstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_drawgetexceptionstring((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgetexceptionstring(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_drawgetexceptionstring((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetexceptionstring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetexceptionstring", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgetexceptiontype(HPHP::Object const&)
_ZN4HPHP22f_drawgetexceptiontypeERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgetexceptiontype(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetexceptiontypeERKNS_6ObjectE");

TypedValue * fg1_drawgetexceptiontype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetexceptiontype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgetexceptiontype((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetexceptiontype(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgetexceptiontype((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetexceptiontype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetexceptiontype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_drawgetfillalpha(HPHP::Object const&)
_ZN4HPHP18f_drawgetfillalphaERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetfillalpha(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfillalphaERKNS_6ObjectE");

TypedValue * fg1_drawgetfillalpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetfillalpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_drawgetfillalpha((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetfillalpha(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_drawgetfillalpha((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetfillalpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetfillalpha", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_drawgetfillcolor(HPHP::Object const&)
_ZN4HPHP18f_drawgetfillcolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetfillcolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfillcolorERKNS_6ObjectE");

TypedValue * fg1_drawgetfillcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetfillcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_drawgetfillcolor((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgetfillcolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_drawgetfillcolor((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetfillcolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetfillcolor", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_drawgetfillopacity(HPHP::Object const&)
_ZN4HPHP20f_drawgetfillopacityERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetfillopacity(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetfillopacityERKNS_6ObjectE");

TypedValue * fg1_drawgetfillopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetfillopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_drawgetfillopacity((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetfillopacity(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_drawgetfillopacity((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetfillopacity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetfillopacity", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgetfillrule(HPHP::Object const&)
_ZN4HPHP17f_drawgetfillruleERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgetfillrule(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetfillruleERKNS_6ObjectE");

TypedValue * fg1_drawgetfillrule(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetfillrule(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgetfillrule((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetfillrule(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgetfillrule((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetfillrule(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetfillrule", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_drawgetfont(HPHP::Object const&)
_ZN4HPHP13f_drawgetfontERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetfont(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP13f_drawgetfontERKNS_6ObjectE");

TypedValue * fg1_drawgetfont(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetfont(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_drawgetfont((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgetfont(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_drawgetfont((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetfont(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetfont", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_drawgetfontfamily(HPHP::Object const&)
_ZN4HPHP19f_drawgetfontfamilyERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetfontfamily(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP19f_drawgetfontfamilyERKNS_6ObjectE");

TypedValue * fg1_drawgetfontfamily(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetfontfamily(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_drawgetfontfamily((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgetfontfamily(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_drawgetfontfamily((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetfontfamily(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetfontfamily", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_drawgetfontsize(HPHP::Object const&)
_ZN4HPHP17f_drawgetfontsizeERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetfontsize(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetfontsizeERKNS_6ObjectE");

TypedValue * fg1_drawgetfontsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetfontsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_drawgetfontsize((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetfontsize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_drawgetfontsize((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetfontsize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetfontsize", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgetfontstretch(HPHP::Object const&)
_ZN4HPHP20f_drawgetfontstretchERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgetfontstretch(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetfontstretchERKNS_6ObjectE");

TypedValue * fg1_drawgetfontstretch(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetfontstretch(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgetfontstretch((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetfontstretch(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgetfontstretch((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetfontstretch(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetfontstretch", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgetfontstyle(HPHP::Object const&)
_ZN4HPHP18f_drawgetfontstyleERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgetfontstyle(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfontstyleERKNS_6ObjectE");

TypedValue * fg1_drawgetfontstyle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetfontstyle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgetfontstyle((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetfontstyle(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgetfontstyle((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetfontstyle(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetfontstyle", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_drawgetfontweight(HPHP::Object const&)
_ZN4HPHP19f_drawgetfontweightERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetfontweight(Value* drw_wnd) asm("_ZN4HPHP19f_drawgetfontweightERKNS_6ObjectE");

TypedValue * fg1_drawgetfontweight(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetfontweight(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_drawgetfontweight((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetfontweight(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_drawgetfontweight((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetfontweight(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetfontweight", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgetgravity(HPHP::Object const&)
_ZN4HPHP16f_drawgetgravityERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgetgravity(Value* drw_wnd) asm("_ZN4HPHP16f_drawgetgravityERKNS_6ObjectE");

TypedValue * fg1_drawgetgravity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetgravity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgetgravity((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetgravity(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgetgravity((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetgravity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetgravity", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_drawgetstrokealpha(HPHP::Object const&)
_ZN4HPHP20f_drawgetstrokealphaERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetstrokealpha(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokealphaERKNS_6ObjectE");

TypedValue * fg1_drawgetstrokealpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetstrokealpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_drawgetstrokealpha((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetstrokealpha(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_drawgetstrokealpha((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetstrokealpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetstrokealpha", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_drawgetstrokeantialias(HPHP::Object const&)
_ZN4HPHP24f_drawgetstrokeantialiasERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

bool fh_drawgetstrokeantialias(Value* drw_wnd) asm("_ZN4HPHP24f_drawgetstrokeantialiasERKNS_6ObjectE");

TypedValue * fg1_drawgetstrokeantialias(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetstrokeantialias(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_drawgetstrokeantialias((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_drawgetstrokeantialias(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_drawgetstrokeantialias((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetstrokeantialias(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetstrokeantialias", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_drawgetstrokecolor(HPHP::Object const&)
_ZN4HPHP20f_drawgetstrokecolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetstrokecolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokecolorERKNS_6ObjectE");

TypedValue * fg1_drawgetstrokecolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetstrokecolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_drawgetstrokecolor((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgetstrokecolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_drawgetstrokecolor((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetstrokecolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetstrokecolor", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_drawgetstrokedasharray(HPHP::Object const&)
_ZN4HPHP24f_drawgetstrokedasharrayERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetstrokedasharray(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP24f_drawgetstrokedasharrayERKNS_6ObjectE");

TypedValue * fg1_drawgetstrokedasharray(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetstrokedasharray(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_drawgetstrokedasharray((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgetstrokedasharray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_drawgetstrokedasharray((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetstrokedasharray(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetstrokedasharray", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_drawgetstrokedashoffset(HPHP::Object const&)
_ZN4HPHP25f_drawgetstrokedashoffsetERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetstrokedashoffset(Value* drw_wnd) asm("_ZN4HPHP25f_drawgetstrokedashoffsetERKNS_6ObjectE");

TypedValue * fg1_drawgetstrokedashoffset(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetstrokedashoffset(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_drawgetstrokedashoffset((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetstrokedashoffset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_drawgetstrokedashoffset((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetstrokedashoffset(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetstrokedashoffset", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgetstrokelinecap(HPHP::Object const&)
_ZN4HPHP22f_drawgetstrokelinecapERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgetstrokelinecap(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetstrokelinecapERKNS_6ObjectE");

TypedValue * fg1_drawgetstrokelinecap(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetstrokelinecap(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgetstrokelinecap((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetstrokelinecap(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgetstrokelinecap((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetstrokelinecap(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetstrokelinecap", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgetstrokelinejoin(HPHP::Object const&)
_ZN4HPHP23f_drawgetstrokelinejoinERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgetstrokelinejoin(Value* drw_wnd) asm("_ZN4HPHP23f_drawgetstrokelinejoinERKNS_6ObjectE");

TypedValue * fg1_drawgetstrokelinejoin(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetstrokelinejoin(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgetstrokelinejoin((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetstrokelinejoin(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgetstrokelinejoin((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetstrokelinejoin(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetstrokelinejoin", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_drawgetstrokemiterlimit(HPHP::Object const&)
_ZN4HPHP25f_drawgetstrokemiterlimitERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetstrokemiterlimit(Value* drw_wnd) asm("_ZN4HPHP25f_drawgetstrokemiterlimitERKNS_6ObjectE");

TypedValue * fg1_drawgetstrokemiterlimit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetstrokemiterlimit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_drawgetstrokemiterlimit((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetstrokemiterlimit(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_drawgetstrokemiterlimit((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetstrokemiterlimit(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetstrokemiterlimit", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_drawgetstrokeopacity(HPHP::Object const&)
_ZN4HPHP22f_drawgetstrokeopacityERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetstrokeopacity(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetstrokeopacityERKNS_6ObjectE");

TypedValue * fg1_drawgetstrokeopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetstrokeopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_drawgetstrokeopacity((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetstrokeopacity(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_drawgetstrokeopacity((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetstrokeopacity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetstrokeopacity", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_drawgetstrokewidth(HPHP::Object const&)
_ZN4HPHP20f_drawgetstrokewidthERKNS_6ObjectE

(return value) => xmm0
drw_wnd => rdi
*/

double fh_drawgetstrokewidth(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokewidthERKNS_6ObjectE");

TypedValue * fg1_drawgetstrokewidth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetstrokewidth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_drawgetstrokewidth((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgetstrokewidth(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_drawgetstrokewidth((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetstrokewidth(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetstrokewidth", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgettextalignment(HPHP::Object const&)
_ZN4HPHP22f_drawgettextalignmentERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgettextalignment(Value* drw_wnd) asm("_ZN4HPHP22f_drawgettextalignmentERKNS_6ObjectE");

TypedValue * fg1_drawgettextalignment(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgettextalignment(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgettextalignment((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgettextalignment(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgettextalignment((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgettextalignment(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgettextalignment", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_drawgettextantialias(HPHP::Object const&)
_ZN4HPHP22f_drawgettextantialiasERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

bool fh_drawgettextantialias(Value* drw_wnd) asm("_ZN4HPHP22f_drawgettextantialiasERKNS_6ObjectE");

TypedValue * fg1_drawgettextantialias(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgettextantialias(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_drawgettextantialias((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_drawgettextantialias(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_drawgettextantialias((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgettextantialias(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgettextantialias", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_drawgettextdecoration(HPHP::Object const&)
_ZN4HPHP23f_drawgettextdecorationERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

long fh_drawgettextdecoration(Value* drw_wnd) asm("_ZN4HPHP23f_drawgettextdecorationERKNS_6ObjectE");

TypedValue * fg1_drawgettextdecoration(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgettextdecoration(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_drawgettextdecoration((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawgettextdecoration(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_drawgettextdecoration((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgettextdecoration(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgettextdecoration", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_drawgettextencoding(HPHP::Object const&)
_ZN4HPHP21f_drawgettextencodingERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgettextencoding(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP21f_drawgettextencodingERKNS_6ObjectE");

TypedValue * fg1_drawgettextencoding(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgettextencoding(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_drawgettextencoding((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgettextencoding(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_drawgettextencoding((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgettextencoding(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgettextencoding", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_drawgettextundercolor(HPHP::Object const&)
_ZN4HPHP23f_drawgettextundercolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgettextundercolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP23f_drawgettextundercolorERKNS_6ObjectE");

TypedValue * fg1_drawgettextundercolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgettextundercolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_drawgettextundercolor((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgettextundercolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_drawgettextundercolor((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgettextundercolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgettextundercolor", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_drawgetvectorgraphics(HPHP::Object const&)
_ZN4HPHP23f_drawgetvectorgraphicsERKNS_6ObjectE

(return value) => rax
_rv => rdi
drw_wnd => rsi
*/

Value* fh_drawgetvectorgraphics(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP23f_drawgetvectorgraphicsERKNS_6ObjectE");

TypedValue * fg1_drawgetvectorgraphics(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawgetvectorgraphics(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_drawgetvectorgraphics((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_drawgetvectorgraphics(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_drawgetvectorgraphics((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawgetvectorgraphics(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawgetvectorgraphics", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawline(HPHP::Object const&, double, double, double, double)
_ZN4HPHP10f_drawlineERKNS_6ObjectEdddd

drw_wnd => rdi
sx => xmm0
sy => xmm1
ex => xmm2
ey => xmm3
*/

void fh_drawline(Value* drw_wnd, double sx, double sy, double ex, double ey) asm("_ZN4HPHP10f_drawlineERKNS_6ObjectEdddd");

TypedValue * fg1_drawline(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawline(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawline((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
  return rv;
}

TypedValue* fg_drawline(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawline((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawline(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawline", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawmatte(HPHP::Object const&, double, double, int)
_ZN4HPHP11f_drawmatteERKNS_6ObjectEddi

drw_wnd => rdi
x => xmm0
y => xmm1
paint_method => rsi
*/

void fh_drawmatte(Value* drw_wnd, double x, double y, int paint_method) asm("_ZN4HPHP11f_drawmatteERKNS_6ObjectEddi");

TypedValue * fg1_drawmatte(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawmatte(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawmatte((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num));
  return rv;
}

TypedValue* fg_drawmatte(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawmatte((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawmatte(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawmatte", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathclose(HPHP::Object const&)
_ZN4HPHP15f_drawpathcloseERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpathclose(Value* drw_wnd) asm("_ZN4HPHP15f_drawpathcloseERKNS_6ObjectE");

TypedValue * fg1_drawpathclose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathclose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_drawpathclose((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawpathclose(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathclose((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathclose(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathclose", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathcurvetoabsolute(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP25f_drawpathcurvetoabsoluteERKNS_6ObjectEdddddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x2 => xmm2
y2 => xmm3
x => xmm4
y => xmm5
*/

void fh_drawpathcurvetoabsolute(Value* drw_wnd, double x1, double y1, double x2, double y2, double x, double y) asm("_ZN4HPHP25f_drawpathcurvetoabsoluteERKNS_6ObjectEdddddd");

TypedValue * fg1_drawpathcurvetoabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathcurvetoabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathcurvetoabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathcurvetoabsolute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 7LL) {
      if ((args-6)->m_type == KindOfDouble && (args-5)->m_type == KindOfDouble && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathcurvetoabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathcurvetoabsolute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathcurvetoabsolute", count, 7, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathcurvetoquadraticbezierabsolute(HPHP::Object const&, double, double, double, double)
_ZN4HPHP40f_drawpathcurvetoquadraticbezierabsoluteERKNS_6ObjectEdddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x => xmm2
y => xmm3
*/

void fh_drawpathcurvetoquadraticbezierabsolute(Value* drw_wnd, double x1, double y1, double x, double y) asm("_ZN4HPHP40f_drawpathcurvetoquadraticbezierabsoluteERKNS_6ObjectEdddd");

TypedValue * fg1_drawpathcurvetoquadraticbezierabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathcurvetoquadraticbezierabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathcurvetoquadraticbezierabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathcurvetoquadraticbezierabsolute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathcurvetoquadraticbezierabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathcurvetoquadraticbezierabsolute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathcurvetoquadraticbezierabsolute", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathcurvetoquadraticbezierrelative(HPHP::Object const&, double, double, double, double)
_ZN4HPHP40f_drawpathcurvetoquadraticbezierrelativeERKNS_6ObjectEdddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x => xmm2
y => xmm3
*/

void fh_drawpathcurvetoquadraticbezierrelative(Value* drw_wnd, double x1, double y1, double x, double y) asm("_ZN4HPHP40f_drawpathcurvetoquadraticbezierrelativeERKNS_6ObjectEdddd");

TypedValue * fg1_drawpathcurvetoquadraticbezierrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathcurvetoquadraticbezierrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathcurvetoquadraticbezierrelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathcurvetoquadraticbezierrelative(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathcurvetoquadraticbezierrelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathcurvetoquadraticbezierrelative(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathcurvetoquadraticbezierrelative", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathcurvetoquadraticbeziersmoothabsolute(HPHP::Object const&, double, double)
_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothabsoluteERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathcurvetoquadraticbeziersmoothabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothabsoluteERKNS_6ObjectEdd");

TypedValue * fg1_drawpathcurvetoquadraticbeziersmoothabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathcurvetoquadraticbeziersmoothabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathcurvetoquadraticbeziersmoothabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathcurvetoquadraticbeziersmoothabsolute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathcurvetoquadraticbeziersmoothabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathcurvetoquadraticbeziersmoothabsolute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathcurvetoquadraticbeziersmoothabsolute", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathcurvetoquadraticbeziersmoothrelative(HPHP::Object const&, double, double)
_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothrelativeERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathcurvetoquadraticbeziersmoothrelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothrelativeERKNS_6ObjectEdd");

TypedValue * fg1_drawpathcurvetoquadraticbeziersmoothrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathcurvetoquadraticbeziersmoothrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathcurvetoquadraticbeziersmoothrelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathcurvetoquadraticbeziersmoothrelative(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathcurvetoquadraticbeziersmoothrelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathcurvetoquadraticbeziersmoothrelative(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathcurvetoquadraticbeziersmoothrelative", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathcurvetorelative(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP25f_drawpathcurvetorelativeERKNS_6ObjectEdddddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x2 => xmm2
y2 => xmm3
x => xmm4
y => xmm5
*/

void fh_drawpathcurvetorelative(Value* drw_wnd, double x1, double y1, double x2, double y2, double x, double y) asm("_ZN4HPHP25f_drawpathcurvetorelativeERKNS_6ObjectEdddddd");

TypedValue * fg1_drawpathcurvetorelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathcurvetorelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathcurvetorelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathcurvetorelative(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 7LL) {
      if ((args-6)->m_type == KindOfDouble && (args-5)->m_type == KindOfDouble && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathcurvetorelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathcurvetorelative(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathcurvetorelative", count, 7, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathcurvetosmoothabsolute(HPHP::Object const&, double, double, double, double)
_ZN4HPHP31f_drawpathcurvetosmoothabsoluteERKNS_6ObjectEdddd

drw_wnd => rdi
x2 => xmm0
y2 => xmm1
x => xmm2
y => xmm3
*/

void fh_drawpathcurvetosmoothabsolute(Value* drw_wnd, double x2, double y2, double x, double y) asm("_ZN4HPHP31f_drawpathcurvetosmoothabsoluteERKNS_6ObjectEdddd");

TypedValue * fg1_drawpathcurvetosmoothabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathcurvetosmoothabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathcurvetosmoothabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathcurvetosmoothabsolute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathcurvetosmoothabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathcurvetosmoothabsolute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathcurvetosmoothabsolute", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathcurvetosmoothrelative(HPHP::Object const&, double, double, double, double)
_ZN4HPHP31f_drawpathcurvetosmoothrelativeERKNS_6ObjectEdddd

drw_wnd => rdi
x2 => xmm0
y2 => xmm1
x => xmm2
y => xmm3
*/

void fh_drawpathcurvetosmoothrelative(Value* drw_wnd, double x2, double y2, double x, double y) asm("_ZN4HPHP31f_drawpathcurvetosmoothrelativeERKNS_6ObjectEdddd");

TypedValue * fg1_drawpathcurvetosmoothrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathcurvetosmoothrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathcurvetosmoothrelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathcurvetosmoothrelative(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathcurvetosmoothrelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathcurvetosmoothrelative(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathcurvetosmoothrelative", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathellipticarcabsolute(HPHP::Object const&, double, double, double, bool, bool, double, double)
_ZN4HPHP29f_drawpathellipticarcabsoluteERKNS_6ObjectEdddbbdd

drw_wnd => rdi
rx => xmm0
ry => xmm1
x_axis_rotation => xmm2
large_arc_flag => rsi
sweep_flag => rdx
x => xmm3
y => xmm4
*/

void fh_drawpathellipticarcabsolute(Value* drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) asm("_ZN4HPHP29f_drawpathellipticarcabsoluteERKNS_6ObjectEdddbbdd");

TypedValue * fg1_drawpathellipticarcabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathellipticarcabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-7)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-7);
  }
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathellipticarcabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num), (args[-6].m_data.dbl), (args[-7].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathellipticarcabsolute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 8LL) {
      if ((args-7)->m_type == KindOfDouble && (args-6)->m_type == KindOfDouble && (args-5)->m_type == KindOfBoolean && (args-4)->m_type == KindOfBoolean && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathellipticarcabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num), (args[-6].m_data.dbl), (args[-7].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathellipticarcabsolute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathellipticarcabsolute", count, 8, 8, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 8);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathellipticarcrelative(HPHP::Object const&, double, double, double, bool, bool, double, double)
_ZN4HPHP29f_drawpathellipticarcrelativeERKNS_6ObjectEdddbbdd

drw_wnd => rdi
rx => xmm0
ry => xmm1
x_axis_rotation => xmm2
large_arc_flag => rsi
sweep_flag => rdx
x => xmm3
y => xmm4
*/

void fh_drawpathellipticarcrelative(Value* drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) asm("_ZN4HPHP29f_drawpathellipticarcrelativeERKNS_6ObjectEdddbbdd");

TypedValue * fg1_drawpathellipticarcrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathellipticarcrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-7)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-7);
  }
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathellipticarcrelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num), (args[-6].m_data.dbl), (args[-7].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathellipticarcrelative(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 8LL) {
      if ((args-7)->m_type == KindOfDouble && (args-6)->m_type == KindOfDouble && (args-5)->m_type == KindOfBoolean && (args-4)->m_type == KindOfBoolean && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathellipticarcrelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num), (args[-6].m_data.dbl), (args[-7].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathellipticarcrelative(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathellipticarcrelative", count, 8, 8, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 8);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathfinish(HPHP::Object const&)
_ZN4HPHP16f_drawpathfinishERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpathfinish(Value* drw_wnd) asm("_ZN4HPHP16f_drawpathfinishERKNS_6ObjectE");

TypedValue * fg1_drawpathfinish(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathfinish(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_drawpathfinish((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawpathfinish(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathfinish((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathfinish(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathfinish", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathlinetoabsolute(HPHP::Object const&, double, double)
_ZN4HPHP24f_drawpathlinetoabsoluteERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathlinetoabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathlinetoabsoluteERKNS_6ObjectEdd");

TypedValue * fg1_drawpathlinetoabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathlinetoabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathlinetoabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathlinetoabsolute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathlinetoabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathlinetoabsolute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathlinetoabsolute", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathlinetohorizontalabsolute(HPHP::Object const&, double)
_ZN4HPHP34f_drawpathlinetohorizontalabsoluteERKNS_6ObjectEd

drw_wnd => rdi
x => xmm0
*/

void fh_drawpathlinetohorizontalabsolute(Value* drw_wnd, double x) asm("_ZN4HPHP34f_drawpathlinetohorizontalabsoluteERKNS_6ObjectEd");

TypedValue * fg1_drawpathlinetohorizontalabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathlinetohorizontalabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathlinetohorizontalabsolute((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathlinetohorizontalabsolute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathlinetohorizontalabsolute((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathlinetohorizontalabsolute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathlinetohorizontalabsolute", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathlinetohorizontalrelative(HPHP::Object const&, double)
_ZN4HPHP34f_drawpathlinetohorizontalrelativeERKNS_6ObjectEd

drw_wnd => rdi
x => xmm0
*/

void fh_drawpathlinetohorizontalrelative(Value* drw_wnd, double x) asm("_ZN4HPHP34f_drawpathlinetohorizontalrelativeERKNS_6ObjectEd");

TypedValue * fg1_drawpathlinetohorizontalrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathlinetohorizontalrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathlinetohorizontalrelative((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathlinetohorizontalrelative(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathlinetohorizontalrelative((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathlinetohorizontalrelative(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathlinetohorizontalrelative", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathlinetorelative(HPHP::Object const&, double, double)
_ZN4HPHP24f_drawpathlinetorelativeERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathlinetorelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathlinetorelativeERKNS_6ObjectEdd");

TypedValue * fg1_drawpathlinetorelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathlinetorelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathlinetorelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathlinetorelative(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathlinetorelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathlinetorelative(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathlinetorelative", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathlinetoverticalabsolute(HPHP::Object const&, double)
_ZN4HPHP32f_drawpathlinetoverticalabsoluteERKNS_6ObjectEd

drw_wnd => rdi
y => xmm0
*/

void fh_drawpathlinetoverticalabsolute(Value* drw_wnd, double y) asm("_ZN4HPHP32f_drawpathlinetoverticalabsoluteERKNS_6ObjectEd");

TypedValue * fg1_drawpathlinetoverticalabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathlinetoverticalabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathlinetoverticalabsolute((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathlinetoverticalabsolute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathlinetoverticalabsolute((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathlinetoverticalabsolute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathlinetoverticalabsolute", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathlinetoverticalrelative(HPHP::Object const&, double)
_ZN4HPHP32f_drawpathlinetoverticalrelativeERKNS_6ObjectEd

drw_wnd => rdi
y => xmm0
*/

void fh_drawpathlinetoverticalrelative(Value* drw_wnd, double y) asm("_ZN4HPHP32f_drawpathlinetoverticalrelativeERKNS_6ObjectEd");

TypedValue * fg1_drawpathlinetoverticalrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathlinetoverticalrelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathlinetoverticalrelative((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathlinetoverticalrelative(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathlinetoverticalrelative((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathlinetoverticalrelative(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathlinetoverticalrelative", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathmovetoabsolute(HPHP::Object const&, double, double)
_ZN4HPHP24f_drawpathmovetoabsoluteERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathmovetoabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathmovetoabsoluteERKNS_6ObjectEdd");

TypedValue * fg1_drawpathmovetoabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathmovetoabsolute(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathmovetoabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathmovetoabsolute(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathmovetoabsolute((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathmovetoabsolute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathmovetoabsolute", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathmovetorelative(HPHP::Object const&, double, double)
_ZN4HPHP24f_drawpathmovetorelativeERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpathmovetorelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathmovetorelativeERKNS_6ObjectEdd");

TypedValue * fg1_drawpathmovetorelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathmovetorelative(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpathmovetorelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpathmovetorelative(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathmovetorelative((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathmovetorelative(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathmovetorelative", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpathstart(HPHP::Object const&)
_ZN4HPHP15f_drawpathstartERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpathstart(Value* drw_wnd) asm("_ZN4HPHP15f_drawpathstartERKNS_6ObjectE");

TypedValue * fg1_drawpathstart(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpathstart(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_drawpathstart((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawpathstart(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpathstart((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpathstart(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpathstart", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpoint(HPHP::Object const&, double, double)
_ZN4HPHP11f_drawpointERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawpoint(Value* drw_wnd, double x, double y) asm("_ZN4HPHP11f_drawpointERKNS_6ObjectEdd");

TypedValue * fg1_drawpoint(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpoint(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpoint((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpoint(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpoint((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpoint(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpoint", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpolygon(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP13f_drawpolygonERKNS_6ObjectERKNS_5ArrayE

drw_wnd => rdi
x_y_points_array => rsi
*/

void fh_drawpolygon(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP13f_drawpolygonERKNS_6ObjectERKNS_5ArrayE");

TypedValue * fg1_drawpolygon(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpolygon(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpolygon((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_drawpolygon(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpolygon((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpolygon(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpolygon", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpolyline(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP14f_drawpolylineERKNS_6ObjectERKNS_5ArrayE

drw_wnd => rdi
x_y_points_array => rsi
*/

void fh_drawpolyline(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP14f_drawpolylineERKNS_6ObjectERKNS_5ArrayE");

TypedValue * fg1_drawpolyline(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpolyline(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpolyline((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_drawpolyline(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpolyline((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpolyline(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpolyline", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawrectangle(HPHP::Object const&, double, double, double, double)
_ZN4HPHP15f_drawrectangleERKNS_6ObjectEdddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x2 => xmm2
y2 => xmm3
*/

void fh_drawrectangle(Value* drw_wnd, double x1, double y1, double x2, double y2) asm("_ZN4HPHP15f_drawrectangleERKNS_6ObjectEdddd");

TypedValue * fg1_drawrectangle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawrectangle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawrectangle((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
  return rv;
}

TypedValue* fg_drawrectangle(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawrectangle((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawrectangle(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawrectangle", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_drawrender(HPHP::Object const&)
_ZN4HPHP12f_drawrenderERKNS_6ObjectE

(return value) => rax
drw_wnd => rdi
*/

bool fh_drawrender(Value* drw_wnd) asm("_ZN4HPHP12f_drawrenderERKNS_6ObjectE");

TypedValue * fg1_drawrender(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawrender(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_drawrender((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_drawrender(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_drawrender((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawrender(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawrender", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawrotate(HPHP::Object const&, double)
_ZN4HPHP12f_drawrotateERKNS_6ObjectEd

drw_wnd => rdi
degrees => xmm0
*/

void fh_drawrotate(Value* drw_wnd, double degrees) asm("_ZN4HPHP12f_drawrotateERKNS_6ObjectEd");

TypedValue * fg1_drawrotate(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawrotate(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawrotate((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawrotate(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawrotate((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawrotate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawrotate", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawroundrectangle(HPHP::Object const&, double, double, double, double, double, double)
_ZN4HPHP20f_drawroundrectangleERKNS_6ObjectEdddddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x2 => xmm2
y2 => xmm3
rx => xmm4
ry => xmm5
*/

void fh_drawroundrectangle(Value* drw_wnd, double x1, double y1, double x2, double y2, double rx, double ry) asm("_ZN4HPHP20f_drawroundrectangleERKNS_6ObjectEdddddd");

TypedValue * fg1_drawroundrectangle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawroundrectangle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawroundrectangle((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
  return rv;
}

TypedValue* fg_drawroundrectangle(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 7LL) {
      if ((args-6)->m_type == KindOfDouble && (args-5)->m_type == KindOfDouble && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawroundrectangle((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawroundrectangle(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawroundrectangle", count, 7, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawscale(HPHP::Object const&, double, double)
_ZN4HPHP11f_drawscaleERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawscale(Value* drw_wnd, double x, double y) asm("_ZN4HPHP11f_drawscaleERKNS_6ObjectEdd");

TypedValue * fg1_drawscale(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawscale(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawscale((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  return rv;
}

TypedValue* fg_drawscale(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawscale((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawscale(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawscale", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_drawsetclippath(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_drawsetclippathERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
clip_path => rsi
*/

bool fh_drawsetclippath(Value* drw_wnd, Value* clip_path) asm("_ZN4HPHP17f_drawsetclippathERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_drawsetclippath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetclippath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_drawsetclippath((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_drawsetclippath(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_drawsetclippath((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetclippath(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetclippath", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetcliprule(HPHP::Object const&, int)
_ZN4HPHP17f_drawsetclipruleERKNS_6ObjectEi

drw_wnd => rdi
fill_rule => rsi
*/

void fh_drawsetcliprule(Value* drw_wnd, int fill_rule) asm("_ZN4HPHP17f_drawsetclipruleERKNS_6ObjectEi");

TypedValue * fg1_drawsetcliprule(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetcliprule(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetcliprule((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_drawsetcliprule(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetcliprule((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetcliprule(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetcliprule", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetclipunits(HPHP::Object const&, int)
_ZN4HPHP18f_drawsetclipunitsERKNS_6ObjectEi

drw_wnd => rdi
clip_path_units => rsi
*/

void fh_drawsetclipunits(Value* drw_wnd, int clip_path_units) asm("_ZN4HPHP18f_drawsetclipunitsERKNS_6ObjectEi");

TypedValue * fg1_drawsetclipunits(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetclipunits(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetclipunits((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_drawsetclipunits(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetclipunits((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetclipunits(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetclipunits", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetfillalpha(HPHP::Object const&, double)
_ZN4HPHP18f_drawsetfillalphaERKNS_6ObjectEd

drw_wnd => rdi
fill_opacity => xmm0
*/

void fh_drawsetfillalpha(Value* drw_wnd, double fill_opacity) asm("_ZN4HPHP18f_drawsetfillalphaERKNS_6ObjectEd");

TypedValue * fg1_drawsetfillalpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfillalpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetfillalpha((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawsetfillalpha(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetfillalpha((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfillalpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfillalpha", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetfillcolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP18f_drawsetfillcolorERKNS_6ObjectES2_

drw_wnd => rdi
fill_pxl_wnd => rsi
*/

void fh_drawsetfillcolor(Value* drw_wnd, Value* fill_pxl_wnd) asm("_ZN4HPHP18f_drawsetfillcolorERKNS_6ObjectES2_");

TypedValue * fg1_drawsetfillcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfillcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetfillcolor((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_drawsetfillcolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetfillcolor((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfillcolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfillcolor", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetfillopacity(HPHP::Object const&, double)
_ZN4HPHP20f_drawsetfillopacityERKNS_6ObjectEd

drw_wnd => rdi
fill_opacity => xmm0
*/

void fh_drawsetfillopacity(Value* drw_wnd, double fill_opacity) asm("_ZN4HPHP20f_drawsetfillopacityERKNS_6ObjectEd");

TypedValue * fg1_drawsetfillopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfillopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetfillopacity((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawsetfillopacity(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetfillopacity((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfillopacity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfillopacity", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_drawsetfillpatternurl(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP23f_drawsetfillpatternurlERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
fill_url => rsi
*/

bool fh_drawsetfillpatternurl(Value* drw_wnd, Value* fill_url) asm("_ZN4HPHP23f_drawsetfillpatternurlERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_drawsetfillpatternurl(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfillpatternurl(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_drawsetfillpatternurl((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_drawsetfillpatternurl(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_drawsetfillpatternurl((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfillpatternurl(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfillpatternurl", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetfillrule(HPHP::Object const&, int)
_ZN4HPHP17f_drawsetfillruleERKNS_6ObjectEi

drw_wnd => rdi
fill_rule => rsi
*/

void fh_drawsetfillrule(Value* drw_wnd, int fill_rule) asm("_ZN4HPHP17f_drawsetfillruleERKNS_6ObjectEi");

TypedValue * fg1_drawsetfillrule(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfillrule(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetfillrule((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_drawsetfillrule(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetfillrule((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfillrule(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfillrule", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_drawsetfont(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13f_drawsetfontERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
font_file => rsi
*/

bool fh_drawsetfont(Value* drw_wnd, Value* font_file) asm("_ZN4HPHP13f_drawsetfontERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_drawsetfont(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfont(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_drawsetfont((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_drawsetfont(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_drawsetfont((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfont(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfont", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_drawsetfontfamily(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP19f_drawsetfontfamilyERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
font_family => rsi
*/

bool fh_drawsetfontfamily(Value* drw_wnd, Value* font_family) asm("_ZN4HPHP19f_drawsetfontfamilyERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_drawsetfontfamily(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfontfamily(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_drawsetfontfamily((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_drawsetfontfamily(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_drawsetfontfamily((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfontfamily(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfontfamily", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetfontsize(HPHP::Object const&, double)
_ZN4HPHP17f_drawsetfontsizeERKNS_6ObjectEd

drw_wnd => rdi
pointsize => xmm0
*/

void fh_drawsetfontsize(Value* drw_wnd, double pointsize) asm("_ZN4HPHP17f_drawsetfontsizeERKNS_6ObjectEd");

TypedValue * fg1_drawsetfontsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfontsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetfontsize((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawsetfontsize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetfontsize((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfontsize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfontsize", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetfontstretch(HPHP::Object const&, int)
_ZN4HPHP20f_drawsetfontstretchERKNS_6ObjectEi

drw_wnd => rdi
stretch_type => rsi
*/

void fh_drawsetfontstretch(Value* drw_wnd, int stretch_type) asm("_ZN4HPHP20f_drawsetfontstretchERKNS_6ObjectEi");

TypedValue * fg1_drawsetfontstretch(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfontstretch(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetfontstretch((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_drawsetfontstretch(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetfontstretch((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfontstretch(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfontstretch", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetfontstyle(HPHP::Object const&, int)
_ZN4HPHP18f_drawsetfontstyleERKNS_6ObjectEi

drw_wnd => rdi
style_type => rsi
*/

void fh_drawsetfontstyle(Value* drw_wnd, int style_type) asm("_ZN4HPHP18f_drawsetfontstyleERKNS_6ObjectEi");

TypedValue * fg1_drawsetfontstyle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfontstyle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetfontstyle((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_drawsetfontstyle(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetfontstyle((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfontstyle(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfontstyle", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetfontweight(HPHP::Object const&, double)
_ZN4HPHP19f_drawsetfontweightERKNS_6ObjectEd

drw_wnd => rdi
font_weight => xmm0
*/

void fh_drawsetfontweight(Value* drw_wnd, double font_weight) asm("_ZN4HPHP19f_drawsetfontweightERKNS_6ObjectEd");

TypedValue * fg1_drawsetfontweight(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetfontweight(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetfontweight((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawsetfontweight(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetfontweight((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetfontweight(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetfontweight", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetgravity(HPHP::Object const&, int)
_ZN4HPHP16f_drawsetgravityERKNS_6ObjectEi

drw_wnd => rdi
gravity_type => rsi
*/

void fh_drawsetgravity(Value* drw_wnd, int gravity_type) asm("_ZN4HPHP16f_drawsetgravityERKNS_6ObjectEi");

TypedValue * fg1_drawsetgravity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetgravity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetgravity((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_drawsetgravity(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetgravity((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetgravity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetgravity", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetstrokealpha(HPHP::Object const&, double)
_ZN4HPHP20f_drawsetstrokealphaERKNS_6ObjectEd

drw_wnd => rdi
stroke_opacity => xmm0
*/

void fh_drawsetstrokealpha(Value* drw_wnd, double stroke_opacity) asm("_ZN4HPHP20f_drawsetstrokealphaERKNS_6ObjectEd");

TypedValue * fg1_drawsetstrokealpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokealpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetstrokealpha((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawsetstrokealpha(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetstrokealpha((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokealpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokealpha", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetstrokeantialias(HPHP::Object const&, bool)
_ZN4HPHP24f_drawsetstrokeantialiasERKNS_6ObjectEb

drw_wnd => rdi
stroke_antialias => rsi
*/

void fh_drawsetstrokeantialias(Value* drw_wnd, bool stroke_antialias) asm("_ZN4HPHP24f_drawsetstrokeantialiasERKNS_6ObjectEb");

TypedValue * fg1_drawsetstrokeantialias(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokeantialias(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetstrokeantialias((Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  return rv;
}

TypedValue* fg_drawsetstrokeantialias(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetstrokeantialias((Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokeantialias(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokeantialias", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetstrokecolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP20f_drawsetstrokecolorERKNS_6ObjectES2_

drw_wnd => rdi
strokecolor_pxl_wnd => rsi
*/

void fh_drawsetstrokecolor(Value* drw_wnd, Value* strokecolor_pxl_wnd) asm("_ZN4HPHP20f_drawsetstrokecolorERKNS_6ObjectES2_");

TypedValue * fg1_drawsetstrokecolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokecolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetstrokecolor((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_drawsetstrokecolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetstrokecolor((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokecolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokecolor", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetstrokedasharray(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP24f_drawsetstrokedasharrayERKNS_6ObjectERKNS_5ArrayE

drw_wnd => rdi
dash_array => rsi
*/

void fh_drawsetstrokedasharray(Value* drw_wnd, Value* dash_array) asm("_ZN4HPHP24f_drawsetstrokedasharrayERKNS_6ObjectERKNS_5ArrayE");

TypedValue * fg1_drawsetstrokedasharray(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokedasharray(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetstrokedasharray((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_array));
  return rv;
}

TypedValue* fg_drawsetstrokedasharray(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfArray) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetstrokedasharray((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_array));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokedasharray(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokedasharray", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetstrokedashoffset(HPHP::Object const&, double)
_ZN4HPHP25f_drawsetstrokedashoffsetERKNS_6ObjectEd

drw_wnd => rdi
dash_offset => xmm0
*/

void fh_drawsetstrokedashoffset(Value* drw_wnd, double dash_offset) asm("_ZN4HPHP25f_drawsetstrokedashoffsetERKNS_6ObjectEd");

TypedValue * fg1_drawsetstrokedashoffset(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokedashoffset(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetstrokedashoffset((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawsetstrokedashoffset(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetstrokedashoffset((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokedashoffset(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokedashoffset", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetstrokelinecap(HPHP::Object const&, int)
_ZN4HPHP22f_drawsetstrokelinecapERKNS_6ObjectEi

drw_wnd => rdi
line_cap => rsi
*/

void fh_drawsetstrokelinecap(Value* drw_wnd, int line_cap) asm("_ZN4HPHP22f_drawsetstrokelinecapERKNS_6ObjectEi");

TypedValue * fg1_drawsetstrokelinecap(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokelinecap(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetstrokelinecap((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_drawsetstrokelinecap(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetstrokelinecap((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokelinecap(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokelinecap", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetstrokelinejoin(HPHP::Object const&, int)
_ZN4HPHP23f_drawsetstrokelinejoinERKNS_6ObjectEi

drw_wnd => rdi
line_join => rsi
*/

void fh_drawsetstrokelinejoin(Value* drw_wnd, int line_join) asm("_ZN4HPHP23f_drawsetstrokelinejoinERKNS_6ObjectEi");

TypedValue * fg1_drawsetstrokelinejoin(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokelinejoin(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetstrokelinejoin((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_drawsetstrokelinejoin(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetstrokelinejoin((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokelinejoin(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokelinejoin", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetstrokemiterlimit(HPHP::Object const&, double)
_ZN4HPHP25f_drawsetstrokemiterlimitERKNS_6ObjectEd

drw_wnd => rdi
miterlimit => xmm0
*/

void fh_drawsetstrokemiterlimit(Value* drw_wnd, double miterlimit) asm("_ZN4HPHP25f_drawsetstrokemiterlimitERKNS_6ObjectEd");

TypedValue * fg1_drawsetstrokemiterlimit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokemiterlimit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetstrokemiterlimit((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawsetstrokemiterlimit(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetstrokemiterlimit((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokemiterlimit(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokemiterlimit", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetstrokeopacity(HPHP::Object const&, double)
_ZN4HPHP22f_drawsetstrokeopacityERKNS_6ObjectEd

drw_wnd => rdi
stroke_opacity => xmm0
*/

void fh_drawsetstrokeopacity(Value* drw_wnd, double stroke_opacity) asm("_ZN4HPHP22f_drawsetstrokeopacityERKNS_6ObjectEd");

TypedValue * fg1_drawsetstrokeopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokeopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetstrokeopacity((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawsetstrokeopacity(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetstrokeopacity((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokeopacity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokeopacity", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_drawsetstrokepatternurl(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP25f_drawsetstrokepatternurlERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
stroke_url => rsi
*/

bool fh_drawsetstrokepatternurl(Value* drw_wnd, Value* stroke_url) asm("_ZN4HPHP25f_drawsetstrokepatternurlERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_drawsetstrokepatternurl(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokepatternurl(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_drawsetstrokepatternurl((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_drawsetstrokepatternurl(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_drawsetstrokepatternurl((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokepatternurl(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokepatternurl", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetstrokewidth(HPHP::Object const&, double)
_ZN4HPHP20f_drawsetstrokewidthERKNS_6ObjectEd

drw_wnd => rdi
stroke_width => xmm0
*/

void fh_drawsetstrokewidth(Value* drw_wnd, double stroke_width) asm("_ZN4HPHP20f_drawsetstrokewidthERKNS_6ObjectEd");

TypedValue * fg1_drawsetstrokewidth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetstrokewidth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetstrokewidth((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawsetstrokewidth(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetstrokewidth((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetstrokewidth(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetstrokewidth", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsettextalignment(HPHP::Object const&, int)
_ZN4HPHP22f_drawsettextalignmentERKNS_6ObjectEi

drw_wnd => rdi
align_type => rsi
*/

void fh_drawsettextalignment(Value* drw_wnd, int align_type) asm("_ZN4HPHP22f_drawsettextalignmentERKNS_6ObjectEi");

TypedValue * fg1_drawsettextalignment(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsettextalignment(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsettextalignment((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_drawsettextalignment(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsettextalignment((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsettextalignment(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsettextalignment", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsettextantialias(HPHP::Object const&, bool)
_ZN4HPHP22f_drawsettextantialiasERKNS_6ObjectEb

drw_wnd => rdi
text_antialias => rsi
*/

void fh_drawsettextantialias(Value* drw_wnd, bool text_antialias) asm("_ZN4HPHP22f_drawsettextantialiasERKNS_6ObjectEb");

TypedValue * fg1_drawsettextantialias(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsettextantialias(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsettextantialias((Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  return rv;
}

TypedValue* fg_drawsettextantialias(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsettextantialias((Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsettextantialias(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsettextantialias", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsettextdecoration(HPHP::Object const&, int)
_ZN4HPHP23f_drawsettextdecorationERKNS_6ObjectEi

drw_wnd => rdi
decoration_type => rsi
*/

void fh_drawsettextdecoration(Value* drw_wnd, int decoration_type) asm("_ZN4HPHP23f_drawsettextdecorationERKNS_6ObjectEi");

TypedValue * fg1_drawsettextdecoration(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsettextdecoration(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsettextdecoration((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_drawsettextdecoration(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsettextdecoration((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsettextdecoration(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsettextdecoration", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsettextencoding(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21f_drawsettextencodingERKNS_6ObjectERKNS_6StringE

drw_wnd => rdi
encoding => rsi
*/

void fh_drawsettextencoding(Value* drw_wnd, Value* encoding) asm("_ZN4HPHP21f_drawsettextencodingERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_drawsettextencoding(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsettextencoding(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsettextencoding((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_drawsettextencoding(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsettextencoding((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsettextencoding(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsettextencoding", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsettextundercolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP23f_drawsettextundercolorERKNS_6ObjectES2_

drw_wnd => rdi
undercolor_pxl_wnd => rsi
*/

void fh_drawsettextundercolor(Value* drw_wnd, Value* undercolor_pxl_wnd) asm("_ZN4HPHP23f_drawsettextundercolorERKNS_6ObjectES2_");

TypedValue * fg1_drawsettextundercolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsettextundercolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsettextundercolor((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_drawsettextundercolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsettextundercolor((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsettextundercolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsettextundercolor", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_drawsetvectorgraphics(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP23f_drawsetvectorgraphicsERKNS_6ObjectERKNS_6StringE

(return value) => rax
drw_wnd => rdi
vector_graphics => rsi
*/

bool fh_drawsetvectorgraphics(Value* drw_wnd, Value* vector_graphics) asm("_ZN4HPHP23f_drawsetvectorgraphicsERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_drawsetvectorgraphics(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetvectorgraphics(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_drawsetvectorgraphics((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_drawsetvectorgraphics(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_drawsetvectorgraphics((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetvectorgraphics(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetvectorgraphics", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawsetviewbox(HPHP::Object const&, double, double, double, double)
_ZN4HPHP16f_drawsetviewboxERKNS_6ObjectEdddd

drw_wnd => rdi
x1 => xmm0
y1 => xmm1
x2 => xmm2
y2 => xmm3
*/

void fh_drawsetviewbox(Value* drw_wnd, double x1, double y1, double x2, double y2) asm("_ZN4HPHP16f_drawsetviewboxERKNS_6ObjectEdddd");

TypedValue * fg1_drawsetviewbox(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawsetviewbox(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawsetviewbox((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
  return rv;
}

TypedValue* fg_drawsetviewbox(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawsetviewbox((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawsetviewbox(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawsetviewbox", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawskewx(HPHP::Object const&, double)
_ZN4HPHP11f_drawskewxERKNS_6ObjectEd

drw_wnd => rdi
degrees => xmm0
*/

void fh_drawskewx(Value* drw_wnd, double degrees) asm("_ZN4HPHP11f_drawskewxERKNS_6ObjectEd");

TypedValue * fg1_drawskewx(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawskewx(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawskewx((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawskewx(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawskewx((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawskewx(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawskewx", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawskewy(HPHP::Object const&, double)
_ZN4HPHP11f_drawskewyERKNS_6ObjectEd

drw_wnd => rdi
degrees => xmm0
*/

void fh_drawskewy(Value* drw_wnd, double degrees) asm("_ZN4HPHP11f_drawskewyERKNS_6ObjectEd");

TypedValue * fg1_drawskewy(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawskewy(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawskewy((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_drawskewy(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawskewy((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawskewy(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawskewy", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawtranslate(HPHP::Object const&, double, double)
_ZN4HPHP15f_drawtranslateERKNS_6ObjectEdd

drw_wnd => rdi
x => xmm0
y => xmm1
*/

void fh_drawtranslate(Value* drw_wnd, double x, double y) asm("_ZN4HPHP15f_drawtranslateERKNS_6ObjectEdd");

TypedValue * fg1_drawtranslate(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawtranslate(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawtranslate((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
  return rv;
}

TypedValue* fg_drawtranslate(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawtranslate((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawtranslate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawtranslate", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pushdrawingwand(HPHP::Object const&)
_ZN4HPHP17f_pushdrawingwandERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_pushdrawingwand(Value* drw_wnd) asm("_ZN4HPHP17f_pushdrawingwandERKNS_6ObjectE");

TypedValue * fg1_pushdrawingwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pushdrawingwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_pushdrawingwand((Value*)(args-0));
  return rv;
}

TypedValue* fg_pushdrawingwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pushdrawingwand((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pushdrawingwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pushdrawingwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpushclippath(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP18f_drawpushclippathERKNS_6ObjectERKNS_6StringE

drw_wnd => rdi
clip_path_id => rsi
*/

void fh_drawpushclippath(Value* drw_wnd, Value* clip_path_id) asm("_ZN4HPHP18f_drawpushclippathERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_drawpushclippath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpushclippath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpushclippath((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_drawpushclippath(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpushclippath((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpushclippath(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpushclippath", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpushdefs(HPHP::Object const&)
_ZN4HPHP14f_drawpushdefsERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpushdefs(Value* drw_wnd) asm("_ZN4HPHP14f_drawpushdefsERKNS_6ObjectE");

TypedValue * fg1_drawpushdefs(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpushdefs(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_drawpushdefs((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawpushdefs(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpushdefs((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpushdefs(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpushdefs", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpushpattern(HPHP::Object const&, HPHP::String const&, double, double, double, double)
_ZN4HPHP17f_drawpushpatternERKNS_6ObjectERKNS_6StringEdddd

drw_wnd => rdi
pattern_id => rsi
x => xmm0
y => xmm1
width => xmm2
height => xmm3
*/

void fh_drawpushpattern(Value* drw_wnd, Value* pattern_id, double x, double y, double width, double height) asm("_ZN4HPHP17f_drawpushpatternERKNS_6ObjectERKNS_6StringEdddd");

TypedValue * fg1_drawpushpattern(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpushpattern(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_drawpushpattern((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl));
  return rv;
}

TypedValue* fg_drawpushpattern(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfDouble && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpushpattern((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpushpattern(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpushpattern", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_popdrawingwand(HPHP::Object const&)
_ZN4HPHP16f_popdrawingwandERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_popdrawingwand(Value* drw_wnd) asm("_ZN4HPHP16f_popdrawingwandERKNS_6ObjectE");

TypedValue * fg1_popdrawingwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_popdrawingwand(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_popdrawingwand((Value*)(args-0));
  return rv;
}

TypedValue* fg_popdrawingwand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_popdrawingwand((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_popdrawingwand(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("popdrawingwand", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpopclippath(HPHP::Object const&)
_ZN4HPHP17f_drawpopclippathERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpopclippath(Value* drw_wnd) asm("_ZN4HPHP17f_drawpopclippathERKNS_6ObjectE");

TypedValue * fg1_drawpopclippath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpopclippath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_drawpopclippath((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawpopclippath(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpopclippath((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpopclippath(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpopclippath", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpopdefs(HPHP::Object const&)
_ZN4HPHP13f_drawpopdefsERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpopdefs(Value* drw_wnd) asm("_ZN4HPHP13f_drawpopdefsERKNS_6ObjectE");

TypedValue * fg1_drawpopdefs(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpopdefs(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_drawpopdefs((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawpopdefs(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpopdefs((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpopdefs(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpopdefs", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_drawpoppattern(HPHP::Object const&)
_ZN4HPHP16f_drawpoppatternERKNS_6ObjectE

drw_wnd => rdi
*/

void fh_drawpoppattern(Value* drw_wnd) asm("_ZN4HPHP16f_drawpoppatternERKNS_6ObjectE");

TypedValue * fg1_drawpoppattern(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_drawpoppattern(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_drawpoppattern((Value*)(args-0));
  return rv;
}

TypedValue* fg_drawpoppattern(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_drawpoppattern((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_drawpoppattern(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("drawpoppattern", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickadaptivethresholdimage(HPHP::Object const&, double, double, double)
_ZN4HPHP30f_magickadaptivethresholdimageERKNS_6ObjectEddd

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
offset => xmm2
*/

bool fh_magickadaptivethresholdimage(Value* mgck_wnd, double width, double height, double offset) asm("_ZN4HPHP30f_magickadaptivethresholdimageERKNS_6ObjectEddd");

TypedValue * fg1_magickadaptivethresholdimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickadaptivethresholdimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickadaptivethresholdimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickadaptivethresholdimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickadaptivethresholdimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickadaptivethresholdimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickadaptivethresholdimage", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickaddimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP16f_magickaddimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
add_wand => rsi
*/

bool fh_magickaddimage(Value* mgck_wnd, Value* add_wand) asm("_ZN4HPHP16f_magickaddimageERKNS_6ObjectES2_");

TypedValue * fg1_magickaddimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickaddimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickaddimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickaddimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickaddimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickaddimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickaddimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickaddnoiseimage(HPHP::Object const&, int)
_ZN4HPHP21f_magickaddnoiseimageERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
noise_type => rsi
*/

bool fh_magickaddnoiseimage(Value* mgck_wnd, int noise_type) asm("_ZN4HPHP21f_magickaddnoiseimageERKNS_6ObjectEi");

TypedValue * fg1_magickaddnoiseimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickaddnoiseimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickaddnoiseimage((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickaddnoiseimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickaddnoiseimage((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickaddnoiseimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickaddnoiseimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickaffinetransformimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP28f_magickaffinetransformimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
drw_wnd => rsi
*/

bool fh_magickaffinetransformimage(Value* mgck_wnd, Value* drw_wnd) asm("_ZN4HPHP28f_magickaffinetransformimageERKNS_6ObjectES2_");

TypedValue * fg1_magickaffinetransformimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickaffinetransformimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickaffinetransformimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickaffinetransformimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickaffinetransformimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickaffinetransformimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickaffinetransformimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickannotateimage(HPHP::Object const&, HPHP::Object const&, double, double, double, HPHP::String const&)
_ZN4HPHP21f_magickannotateimageERKNS_6ObjectES2_dddRKNS_6StringE

(return value) => rax
mgck_wnd => rdi
drw_wnd => rsi
x => xmm0
y => xmm1
angle => xmm2
text => rdx
*/

bool fh_magickannotateimage(Value* mgck_wnd, Value* drw_wnd, double x, double y, double angle, Value* text) asm("_ZN4HPHP21f_magickannotateimageERKNS_6ObjectES2_dddRKNS_6StringE");

TypedValue * fg1_magickannotateimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickannotateimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-5)->m_type)) {
    tvCastToStringInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickannotateimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (Value*)(args-5))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickannotateimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if (IS_STRING_TYPE((args-5)->m_type) && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickannotateimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (Value*)(args-5))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickannotateimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickannotateimage", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickappendimages(HPHP::Object const&, bool)
_ZN4HPHP20f_magickappendimagesERKNS_6ObjectEb

(return value) => rax
_rv => rdi
mgck_wnd => rsi
stack_vertical => rdx
*/

Value* fh_magickappendimages(Value* _rv, Value* mgck_wnd, bool stack_vertical) asm("_ZN4HPHP20f_magickappendimagesERKNS_6ObjectEb");

TypedValue * fg1_magickappendimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickappendimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickappendimages((Value*)(rv), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickappendimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickappendimages((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickappendimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickappendimages", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickaverageimages(HPHP::Object const&)
_ZN4HPHP21f_magickaverageimagesERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickaverageimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickaverageimagesERKNS_6ObjectE");

TypedValue * fg1_magickaverageimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickaverageimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_magickaverageimages((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickaverageimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickaverageimages((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickaverageimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickaverageimages", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickblackthresholdimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP27f_magickblackthresholdimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
threshold_pxl_wnd => rsi
*/

bool fh_magickblackthresholdimage(Value* mgck_wnd, Value* threshold_pxl_wnd) asm("_ZN4HPHP27f_magickblackthresholdimageERKNS_6ObjectES2_");

TypedValue * fg1_magickblackthresholdimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickblackthresholdimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickblackthresholdimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickblackthresholdimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickblackthresholdimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickblackthresholdimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickblackthresholdimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickblurimage(HPHP::Object const&, double, double, int)
_ZN4HPHP17f_magickblurimageERKNS_6ObjectEddi

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
channel_type => rsi
*/

bool fh_magickblurimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP17f_magickblurimageERKNS_6ObjectEddi");

TypedValue * fg1_magickblurimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickblurimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickblurimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickblurimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickblurimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickblurimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickblurimage", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickborderimage(HPHP::Object const&, HPHP::Object const&, double, double)
_ZN4HPHP19f_magickborderimageERKNS_6ObjectES2_dd

(return value) => rax
mgck_wnd => rdi
bordercolor => rsi
width => xmm0
height => xmm1
*/

bool fh_magickborderimage(Value* mgck_wnd, Value* bordercolor, double width, double height) asm("_ZN4HPHP19f_magickborderimageERKNS_6ObjectES2_dd");

TypedValue * fg1_magickborderimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickborderimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickborderimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickborderimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickborderimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickborderimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickborderimage", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickcharcoalimage(HPHP::Object const&, double, double)
_ZN4HPHP21f_magickcharcoalimageERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
*/

bool fh_magickcharcoalimage(Value* mgck_wnd, double radius, double sigma) asm("_ZN4HPHP21f_magickcharcoalimageERKNS_6ObjectEdd");

TypedValue * fg1_magickcharcoalimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcharcoalimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickcharcoalimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickcharcoalimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickcharcoalimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcharcoalimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcharcoalimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickchopimage(HPHP::Object const&, double, double, int, int)
_ZN4HPHP17f_magickchopimageERKNS_6ObjectEddii

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
x => rsi
y => rdx
*/

bool fh_magickchopimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP17f_magickchopimageERKNS_6ObjectEddii");

TypedValue * fg1_magickchopimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickchopimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickchopimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickchopimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickchopimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickchopimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickchopimage", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickclipimage(HPHP::Object const&)
_ZN4HPHP17f_magickclipimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickclipimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickclipimageERKNS_6ObjectE");

TypedValue * fg1_magickclipimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickclipimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickclipimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickclipimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickclipimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickclipimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickclipimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickclippathimage(HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP21f_magickclippathimageERKNS_6ObjectERKNS_6StringEb

(return value) => rax
mgck_wnd => rdi
pathname => rsi
inside => rdx
*/

bool fh_magickclippathimage(Value* mgck_wnd, Value* pathname, bool inside) asm("_ZN4HPHP21f_magickclippathimageERKNS_6ObjectERKNS_6StringEb");

TypedValue * fg1_magickclippathimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickclippathimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickclippathimage((Value*)(args-0), (Value*)(args-1), (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickclippathimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfBoolean && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickclippathimage((Value*)(args-0), (Value*)(args-1), (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickclippathimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickclippathimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickcoalesceimages(HPHP::Object const&)
_ZN4HPHP22f_magickcoalesceimagesERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickcoalesceimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP22f_magickcoalesceimagesERKNS_6ObjectE");

TypedValue * fg1_magickcoalesceimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcoalesceimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_magickcoalesceimages((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickcoalesceimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickcoalesceimages((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcoalesceimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcoalesceimages", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickcolorfloodfillimage(HPHP::Object const&, HPHP::Object const&, double, HPHP::Object const&, int, int)
_ZN4HPHP27f_magickcolorfloodfillimageERKNS_6ObjectES2_dS2_ii

(return value) => rax
mgck_wnd => rdi
fillcolor_pxl_wnd => rsi
fuzz => xmm0
bordercolor_pxl_wnd => rdx
x => rcx
y => r8
*/

bool fh_magickcolorfloodfillimage(Value* mgck_wnd, Value* fillcolor_pxl_wnd, double fuzz, Value* bordercolor_pxl_wnd, int x, int y) asm("_ZN4HPHP27f_magickcolorfloodfillimageERKNS_6ObjectES2_dS2_ii");

TypedValue * fg1_magickcolorfloodfillimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcolorfloodfillimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickcolorfloodfillimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (Value*)(args-3), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickcolorfloodfillimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfObject && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickcolorfloodfillimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (Value*)(args-3), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcolorfloodfillimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcolorfloodfillimage", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickcolorizeimage(HPHP::Object const&, HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21f_magickcolorizeimageERKNS_6ObjectES2_S2_

(return value) => rax
mgck_wnd => rdi
colorize => rsi
opacity_pxl_wnd => rdx
*/

bool fh_magickcolorizeimage(Value* mgck_wnd, Value* colorize, Value* opacity_pxl_wnd) asm("_ZN4HPHP21f_magickcolorizeimageERKNS_6ObjectES2_S2_");

TypedValue * fg1_magickcolorizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcolorizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickcolorizeimage((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickcolorizeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfObject && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickcolorizeimage((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcolorizeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcolorizeimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickcombineimages(HPHP::Object const&, int)
_ZN4HPHP21f_magickcombineimagesERKNS_6ObjectEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
channel_type => rdx
*/

Value* fh_magickcombineimages(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP21f_magickcombineimagesERKNS_6ObjectEi");

TypedValue * fg1_magickcombineimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcombineimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickcombineimages((Value*)(rv), (Value*)(args-0), (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickcombineimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickcombineimages((Value*)(&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcombineimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcombineimages", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickcommentimage(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP20f_magickcommentimageERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
comment => rsi
*/

bool fh_magickcommentimage(Value* mgck_wnd, Value* comment) asm("_ZN4HPHP20f_magickcommentimageERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magickcommentimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcommentimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickcommentimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickcommentimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickcommentimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcommentimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcommentimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickcompareimages(HPHP::Object const&, HPHP::Object const&, int, int)
_ZN4HPHP21f_magickcompareimagesERKNS_6ObjectES2_ii

(return value) => rax
_rv => rdi
mgck_wnd => rsi
reference_wnd => rdx
metric_type => rcx
channel_type => r8
*/

Value* fh_magickcompareimages(Value* _rv, Value* mgck_wnd, Value* reference_wnd, int metric_type, int channel_type) asm("_ZN4HPHP21f_magickcompareimagesERKNS_6ObjectES2_ii");

TypedValue * fg1_magickcompareimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcompareimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickcompareimages((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickcompareimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickcompareimages((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcompareimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcompareimages", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickcompositeimage(HPHP::Object const&, HPHP::Object const&, int, int, int)
_ZN4HPHP22f_magickcompositeimageERKNS_6ObjectES2_iii

(return value) => rax
mgck_wnd => rdi
composite_wnd => rsi
composite_operator => rdx
x => rcx
y => r8
*/

bool fh_magickcompositeimage(Value* mgck_wnd, Value* composite_wnd, int composite_operator, int x, int y) asm("_ZN4HPHP22f_magickcompositeimageERKNS_6ObjectES2_iii");

TypedValue * fg1_magickcompositeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcompositeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickcompositeimage((Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickcompositeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickcompositeimage((Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcompositeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcompositeimage", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickconstituteimage(HPHP::Object const&, double, double, HPHP::String const&, int, HPHP::Array const&)
_ZN4HPHP23f_magickconstituteimageERKNS_6ObjectEddRKNS_6StringEiRKNS_5ArrayE

(return value) => rax
mgck_wnd => rdi
columns => xmm0
rows => xmm1
smap => rsi
storage_type => rdx
pixel_array => rcx
*/

bool fh_magickconstituteimage(Value* mgck_wnd, double columns, double rows, Value* smap, int storage_type, Value* pixel_array) asm("_ZN4HPHP23f_magickconstituteimageERKNS_6ObjectEddRKNS_6StringEiRKNS_5ArrayE");

TypedValue * fg1_magickconstituteimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickconstituteimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickconstituteimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (Value*)(args-3), (int)(args[-4].m_data.num), (Value*)(args-5))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickconstituteimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfArray && (args-4)->m_type == KindOfInt64 && IS_STRING_TYPE((args-3)->m_type) && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickconstituteimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (Value*)(args-3), (int)(args[-4].m_data.num), (Value*)(args-5))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickconstituteimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickconstituteimage", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickcontrastimage(HPHP::Object const&, bool)
_ZN4HPHP21f_magickcontrastimageERKNS_6ObjectEb

(return value) => rax
mgck_wnd => rdi
sharpen => rsi
*/

bool fh_magickcontrastimage(Value* mgck_wnd, bool sharpen) asm("_ZN4HPHP21f_magickcontrastimageERKNS_6ObjectEb");

TypedValue * fg1_magickcontrastimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcontrastimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickcontrastimage((Value*)(args-0), (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickcontrastimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickcontrastimage((Value*)(args-0), (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcontrastimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcontrastimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickconvolveimage(HPHP::Object const&, HPHP::Array const&, int)
_ZN4HPHP21f_magickconvolveimageERKNS_6ObjectERKNS_5ArrayEi

(return value) => rax
mgck_wnd => rdi
kernel_array => rsi
channel_type => rdx
*/

bool fh_magickconvolveimage(Value* mgck_wnd, Value* kernel_array, int channel_type) asm("_ZN4HPHP21f_magickconvolveimageERKNS_6ObjectERKNS_5ArrayEi");

TypedValue * fg1_magickconvolveimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickconvolveimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickconvolveimage((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickconvolveimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickconvolveimage((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickconvolveimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickconvolveimage", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickcropimage(HPHP::Object const&, double, double, int, int)
_ZN4HPHP17f_magickcropimageERKNS_6ObjectEddii

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
x => rsi
y => rdx
*/

bool fh_magickcropimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP17f_magickcropimageERKNS_6ObjectEddii");

TypedValue * fg1_magickcropimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcropimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickcropimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickcropimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickcropimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcropimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcropimage", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickcyclecolormapimage(HPHP::Object const&, int)
_ZN4HPHP26f_magickcyclecolormapimageERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
num_positions => rsi
*/

bool fh_magickcyclecolormapimage(Value* mgck_wnd, int num_positions) asm("_ZN4HPHP26f_magickcyclecolormapimageERKNS_6ObjectEi");

TypedValue * fg1_magickcyclecolormapimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickcyclecolormapimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickcyclecolormapimage((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickcyclecolormapimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickcyclecolormapimage((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickcyclecolormapimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickcyclecolormapimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickdeconstructimages(HPHP::Object const&)
_ZN4HPHP25f_magickdeconstructimagesERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickdeconstructimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickdeconstructimagesERKNS_6ObjectE");

TypedValue * fg1_magickdeconstructimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickdeconstructimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_magickdeconstructimages((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickdeconstructimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickdeconstructimages((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickdeconstructimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickdeconstructimages", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickdescribeimage(HPHP::Object const&)
_ZN4HPHP21f_magickdescribeimageERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickdescribeimage(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickdescribeimageERKNS_6ObjectE");

TypedValue * fg1_magickdescribeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickdescribeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickdescribeimage((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickdescribeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickdescribeimage((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickdescribeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickdescribeimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickdespeckleimage(HPHP::Object const&)
_ZN4HPHP22f_magickdespeckleimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickdespeckleimage(Value* mgck_wnd) asm("_ZN4HPHP22f_magickdespeckleimageERKNS_6ObjectE");

TypedValue * fg1_magickdespeckleimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickdespeckleimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickdespeckleimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickdespeckleimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickdespeckleimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickdespeckleimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickdespeckleimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickdrawimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP17f_magickdrawimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
drw_wnd => rsi
*/

bool fh_magickdrawimage(Value* mgck_wnd, Value* drw_wnd) asm("_ZN4HPHP17f_magickdrawimageERKNS_6ObjectES2_");

TypedValue * fg1_magickdrawimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickdrawimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickdrawimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickdrawimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickdrawimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickdrawimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickdrawimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickechoimageblob(HPHP::Object const&)
_ZN4HPHP21f_magickechoimageblobERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickechoimageblob(Value* mgck_wnd) asm("_ZN4HPHP21f_magickechoimageblobERKNS_6ObjectE");

TypedValue * fg1_magickechoimageblob(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickechoimageblob(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickechoimageblob((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickechoimageblob(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickechoimageblob((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickechoimageblob(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickechoimageblob", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickechoimagesblob(HPHP::Object const&)
_ZN4HPHP22f_magickechoimagesblobERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickechoimagesblob(Value* mgck_wnd) asm("_ZN4HPHP22f_magickechoimagesblobERKNS_6ObjectE");

TypedValue * fg1_magickechoimagesblob(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickechoimagesblob(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickechoimagesblob((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickechoimagesblob(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickechoimagesblob((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickechoimagesblob(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickechoimagesblob", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickedgeimage(HPHP::Object const&, double)
_ZN4HPHP17f_magickedgeimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
*/

bool fh_magickedgeimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP17f_magickedgeimageERKNS_6ObjectEd");

TypedValue * fg1_magickedgeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickedgeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickedgeimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickedgeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickedgeimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickedgeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickedgeimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickembossimage(HPHP::Object const&, double, double)
_ZN4HPHP19f_magickembossimageERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
*/

bool fh_magickembossimage(Value* mgck_wnd, double radius, double sigma) asm("_ZN4HPHP19f_magickembossimageERKNS_6ObjectEdd");

TypedValue * fg1_magickembossimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickembossimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickembossimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickembossimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickembossimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickembossimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickembossimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickenhanceimage(HPHP::Object const&)
_ZN4HPHP20f_magickenhanceimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickenhanceimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickenhanceimageERKNS_6ObjectE");

TypedValue * fg1_magickenhanceimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickenhanceimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickenhanceimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickenhanceimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickenhanceimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickenhanceimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickenhanceimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickequalizeimage(HPHP::Object const&)
_ZN4HPHP21f_magickequalizeimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickequalizeimage(Value* mgck_wnd) asm("_ZN4HPHP21f_magickequalizeimageERKNS_6ObjectE");

TypedValue * fg1_magickequalizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickequalizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickequalizeimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickequalizeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickequalizeimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickequalizeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickequalizeimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickevaluateimage(HPHP::Object const&, int, double, int)
_ZN4HPHP21f_magickevaluateimageERKNS_6ObjectEidi

(return value) => rax
mgck_wnd => rdi
evaluate_op => rsi
constant => xmm0
channel_type => rdx
*/

bool fh_magickevaluateimage(Value* mgck_wnd, int evaluate_op, double constant, int channel_type) asm("_ZN4HPHP21f_magickevaluateimageERKNS_6ObjectEidi");

TypedValue * fg1_magickevaluateimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickevaluateimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickevaluateimage((Value*)(args-0), (int)(args[-1].m_data.num), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickevaluateimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickevaluateimage((Value*)(args-0), (int)(args[-1].m_data.num), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickevaluateimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickevaluateimage", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickflattenimages(HPHP::Object const&)
_ZN4HPHP21f_magickflattenimagesERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickflattenimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickflattenimagesERKNS_6ObjectE");

TypedValue * fg1_magickflattenimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickflattenimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_magickflattenimages((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickflattenimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickflattenimages((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickflattenimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickflattenimages", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickflipimage(HPHP::Object const&)
_ZN4HPHP17f_magickflipimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickflipimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickflipimageERKNS_6ObjectE");

TypedValue * fg1_magickflipimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickflipimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickflipimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickflipimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickflipimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickflipimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickflipimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickflopimage(HPHP::Object const&)
_ZN4HPHP17f_magickflopimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickflopimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickflopimageERKNS_6ObjectE");

TypedValue * fg1_magickflopimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickflopimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickflopimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickflopimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickflopimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickflopimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickflopimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickframeimage(HPHP::Object const&, HPHP::Object const&, double, double, int, int)
_ZN4HPHP18f_magickframeimageERKNS_6ObjectES2_ddii

(return value) => rax
mgck_wnd => rdi
matte_color => rsi
width => xmm0
height => xmm1
inner_bevel => rdx
outer_bevel => rcx
*/

bool fh_magickframeimage(Value* mgck_wnd, Value* matte_color, double width, double height, int inner_bevel, int outer_bevel) asm("_ZN4HPHP18f_magickframeimageERKNS_6ObjectES2_ddii");

TypedValue * fg1_magickframeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickframeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickframeimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickframeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickframeimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickframeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickframeimage", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickfximage(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP15f_magickfximageERKNS_6ObjectERKNS_6StringEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
expression => rdx
channel_type => rcx
*/

Value* fh_magickfximage(Value* _rv, Value* mgck_wnd, Value* expression, int channel_type) asm("_ZN4HPHP15f_magickfximageERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_magickfximage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickfximage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickfximage((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickfximage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickfximage((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickfximage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickfximage", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickgammaimage(HPHP::Object const&, double, int)
_ZN4HPHP18f_magickgammaimageERKNS_6ObjectEdi

(return value) => rax
mgck_wnd => rdi
gamma => xmm0
channel_type => rsi
*/

bool fh_magickgammaimage(Value* mgck_wnd, double gamma, int channel_type) asm("_ZN4HPHP18f_magickgammaimageERKNS_6ObjectEdi");

TypedValue * fg1_magickgammaimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgammaimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickgammaimage((Value*)(args-0), (args[-1].m_data.dbl), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickgammaimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickgammaimage((Value*)(args-0), (args[-1].m_data.dbl), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgammaimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgammaimage", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickgaussianblurimage(HPHP::Object const&, double, double, int)
_ZN4HPHP25f_magickgaussianblurimageERKNS_6ObjectEddi

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
channel_type => rsi
*/

bool fh_magickgaussianblurimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP25f_magickgaussianblurimageERKNS_6ObjectEddi");

TypedValue * fg1_magickgaussianblurimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgaussianblurimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickgaussianblurimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickgaussianblurimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickgaussianblurimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgaussianblurimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgaussianblurimage", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetcharheight(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP21f_magickgetcharheightERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgetcharheight(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP21f_magickgetcharheightERKNS_6ObjectES2_RKNS_6StringEb");

TypedValue * fg1_magickgetcharheight(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetcharheight(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.dbl = fh_magickgetcharheight((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_magickgetcharheight(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetcharheight((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetcharheight(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetcharheight", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetcharwidth(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP20f_magickgetcharwidthERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgetcharwidth(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP20f_magickgetcharwidthERKNS_6ObjectES2_RKNS_6StringEb");

TypedValue * fg1_magickgetcharwidth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetcharwidth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.dbl = fh_magickgetcharwidth((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_magickgetcharwidth(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetcharwidth((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetcharwidth(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetcharwidth", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetexception(HPHP::Object const&)
_ZN4HPHP20f_magickgetexceptionERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetexception(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetexceptionERKNS_6ObjectE");

TypedValue * fg1_magickgetexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_magickgetexception((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetexception(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetexception((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetexception(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetexception", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetexceptionstring(HPHP::Object const&)
_ZN4HPHP26f_magickgetexceptionstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetexceptionstring(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetexceptionstringERKNS_6ObjectE");

TypedValue * fg1_magickgetexceptionstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetexceptionstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickgetexceptionstring((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetexceptionstring(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetexceptionstring((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetexceptionstring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetexceptionstring", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetexceptiontype(HPHP::Object const&)
_ZN4HPHP24f_magickgetexceptiontypeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetexceptiontype(Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetexceptiontypeERKNS_6ObjectE");

TypedValue * fg1_magickgetexceptiontype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetexceptiontype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetexceptiontype((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetexceptiontype(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetexceptiontype((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetexceptiontype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetexceptiontype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetfilename(HPHP::Object const&)
_ZN4HPHP19f_magickgetfilenameERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetfilename(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetfilenameERKNS_6ObjectE");

TypedValue * fg1_magickgetfilename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetfilename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickgetfilename((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetfilename(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetfilename((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetfilename(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetfilename", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetformat(HPHP::Object const&)
_ZN4HPHP17f_magickgetformatERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetformat(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP17f_magickgetformatERKNS_6ObjectE");

TypedValue * fg1_magickgetformat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetformat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickgetformat((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetformat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetformat((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetformat(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetformat", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickgetimage(HPHP::Object const&)
_ZN4HPHP16f_magickgetimageERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimage(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP16f_magickgetimageERKNS_6ObjectE");

TypedValue * fg1_magickgetimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimage((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickgetimage((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickgetimagebackgroundcolor(HPHP::Object const&)
_ZN4HPHP31f_magickgetimagebackgroundcolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagebackgroundcolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimagebackgroundcolorERKNS_6ObjectE");

TypedValue * fg1_magickgetimagebackgroundcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagebackgroundcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimagebackgroundcolor((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagebackgroundcolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickgetimagebackgroundcolor((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagebackgroundcolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagebackgroundcolor", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetimageblob(HPHP::Object const&)
_ZN4HPHP20f_magickgetimageblobERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimageblob(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimageblobERKNS_6ObjectE");

TypedValue * fg1_magickgetimageblob(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageblob(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimageblob((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimageblob(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetimageblob((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageblob(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageblob", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetimageblueprimary(HPHP::Object const&)
_ZN4HPHP27f_magickgetimageblueprimaryERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimageblueprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimageblueprimaryERKNS_6ObjectE");

TypedValue * fg1_magickgetimageblueprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageblueprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimageblueprimary((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimageblueprimary(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetimageblueprimary((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageblueprimary(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageblueprimary", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickgetimagebordercolor(HPHP::Object const&)
_ZN4HPHP27f_magickgetimagebordercolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagebordercolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimagebordercolorERKNS_6ObjectE");

TypedValue * fg1_magickgetimagebordercolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagebordercolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimagebordercolor((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagebordercolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickgetimagebordercolor((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagebordercolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagebordercolor", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetimagechannelmean(HPHP::Object const&, int)
_ZN4HPHP27f_magickgetimagechannelmeanERKNS_6ObjectEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
channel_type => rdx
*/

Value* fh_magickgetimagechannelmean(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP27f_magickgetimagechannelmeanERKNS_6ObjectEi");

TypedValue * fg1_magickgetimagechannelmean(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagechannelmean(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickgetimagechannelmean((Value*)(rv), (Value*)(args-0), (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagechannelmean(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetimagechannelmean((Value*)(&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagechannelmean(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagechannelmean", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickgetimagecolormapcolor(HPHP::Object const&, double)
_ZN4HPHP29f_magickgetimagecolormapcolorERKNS_6ObjectEd

(return value) => rax
_rv => rdi
mgck_wnd => rsi
index => xmm0
*/

Value* fh_magickgetimagecolormapcolor(Value* _rv, Value* mgck_wnd, double index) asm("_ZN4HPHP29f_magickgetimagecolormapcolorERKNS_6ObjectEd");

TypedValue * fg1_magickgetimagecolormapcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagecolormapcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickgetimagecolormapcolor((Value*)(rv), (Value*)(args-0), (args[-1].m_data.dbl));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagecolormapcolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickgetimagecolormapcolor((Value*)(&(rv)), (Value*)(args-0), (args[-1].m_data.dbl));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagecolormapcolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagecolormapcolor", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetimagecolors(HPHP::Object const&)
_ZN4HPHP22f_magickgetimagecolorsERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagecolors(Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimagecolorsERKNS_6ObjectE");

TypedValue * fg1_magickgetimagecolors(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagecolors(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_magickgetimagecolors((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagecolors(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetimagecolors((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagecolors(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagecolors", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimagecolorspace(HPHP::Object const&)
_ZN4HPHP26f_magickgetimagecolorspaceERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimagecolorspace(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagecolorspaceERKNS_6ObjectE");

TypedValue * fg1_magickgetimagecolorspace(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagecolorspace(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimagecolorspace((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagecolorspace(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimagecolorspace((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagecolorspace(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagecolorspace", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimagecompose(HPHP::Object const&)
_ZN4HPHP23f_magickgetimagecomposeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimagecompose(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetimagecomposeERKNS_6ObjectE");

TypedValue * fg1_magickgetimagecompose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagecompose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimagecompose((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagecompose(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimagecompose((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagecompose(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagecompose", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimagecompression(HPHP::Object const&)
_ZN4HPHP27f_magickgetimagecompressionERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimagecompression(Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimagecompressionERKNS_6ObjectE");

TypedValue * fg1_magickgetimagecompression(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagecompression(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimagecompression((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagecompression(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimagecompression((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagecompression(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagecompression", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetimagecompressionquality(HPHP::Object const&)
_ZN4HPHP34f_magickgetimagecompressionqualityERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagecompressionquality(Value* mgck_wnd) asm("_ZN4HPHP34f_magickgetimagecompressionqualityERKNS_6ObjectE");

TypedValue * fg1_magickgetimagecompressionquality(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagecompressionquality(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_magickgetimagecompressionquality((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagecompressionquality(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetimagecompressionquality((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagecompressionquality(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagecompressionquality", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetimagedelay(HPHP::Object const&)
_ZN4HPHP21f_magickgetimagedelayERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagedelay(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagedelayERKNS_6ObjectE");

TypedValue * fg1_magickgetimagedelay(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagedelay(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_magickgetimagedelay((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagedelay(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetimagedelay((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagedelay(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagedelay", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetimagedepth(HPHP::Object const&, int)
_ZN4HPHP21f_magickgetimagedepthERKNS_6ObjectEi

(return value) => xmm0
mgck_wnd => rdi
channel_type => rsi
*/

double fh_magickgetimagedepth(Value* mgck_wnd, int channel_type) asm("_ZN4HPHP21f_magickgetimagedepthERKNS_6ObjectEi");

TypedValue * fg1_magickgetimagedepth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagedepth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.dbl = fh_magickgetimagedepth((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  return rv;
}

TypedValue* fg_magickgetimagedepth(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetimagedepth((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagedepth(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagedepth", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimagedispose(HPHP::Object const&)
_ZN4HPHP23f_magickgetimagedisposeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimagedispose(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetimagedisposeERKNS_6ObjectE");

TypedValue * fg1_magickgetimagedispose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagedispose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimagedispose((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagedispose(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimagedispose((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagedispose(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagedispose", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetimageextrema(HPHP::Object const&, int)
_ZN4HPHP23f_magickgetimageextremaERKNS_6ObjectEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
channel_type => rdx
*/

Value* fh_magickgetimageextrema(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP23f_magickgetimageextremaERKNS_6ObjectEi");

TypedValue * fg1_magickgetimageextrema(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageextrema(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickgetimageextrema((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimageextrema(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetimageextrema((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageextrema(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageextrema", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetimagefilename(HPHP::Object const&)
_ZN4HPHP24f_magickgetimagefilenameERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagefilename(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetimagefilenameERKNS_6ObjectE");

TypedValue * fg1_magickgetimagefilename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagefilename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimagefilename((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagefilename(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetimagefilename((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagefilename(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagefilename", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetimageformat(HPHP::Object const&)
_ZN4HPHP22f_magickgetimageformatERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimageformat(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimageformatERKNS_6ObjectE");

TypedValue * fg1_magickgetimageformat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageformat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimageformat((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimageformat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetimageformat((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageformat(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageformat", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetimagegamma(HPHP::Object const&)
_ZN4HPHP21f_magickgetimagegammaERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagegamma(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagegammaERKNS_6ObjectE");

TypedValue * fg1_magickgetimagegamma(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagegamma(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_magickgetimagegamma((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagegamma(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetimagegamma((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagegamma(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagegamma", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetimagegreenprimary(HPHP::Object const&)
_ZN4HPHP28f_magickgetimagegreenprimaryERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagegreenprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP28f_magickgetimagegreenprimaryERKNS_6ObjectE");

TypedValue * fg1_magickgetimagegreenprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagegreenprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimagegreenprimary((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagegreenprimary(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetimagegreenprimary((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagegreenprimary(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagegreenprimary", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetimageheight(HPHP::Object const&)
_ZN4HPHP22f_magickgetimageheightERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimageheight(Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimageheightERKNS_6ObjectE");

TypedValue * fg1_magickgetimageheight(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageheight(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_magickgetimageheight((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimageheight(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetimageheight((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageheight(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageheight", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetimagehistogram(HPHP::Object const&)
_ZN4HPHP25f_magickgetimagehistogramERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagehistogram(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickgetimagehistogramERKNS_6ObjectE");

TypedValue * fg1_magickgetimagehistogram(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagehistogram(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimagehistogram((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagehistogram(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetimagehistogram((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagehistogram(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagehistogram", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimageindex(HPHP::Object const&)
_ZN4HPHP21f_magickgetimageindexERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimageindex(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimageindexERKNS_6ObjectE");

TypedValue * fg1_magickgetimageindex(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageindex(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimageindex((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimageindex(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimageindex((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageindex(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageindex", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimageinterlacescheme(HPHP::Object const&)
_ZN4HPHP31f_magickgetimageinterlaceschemeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimageinterlacescheme(Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimageinterlaceschemeERKNS_6ObjectE");

TypedValue * fg1_magickgetimageinterlacescheme(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageinterlacescheme(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimageinterlacescheme((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimageinterlacescheme(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimageinterlacescheme((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageinterlacescheme(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageinterlacescheme", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetimageiterations(HPHP::Object const&)
_ZN4HPHP26f_magickgetimageiterationsERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimageiterations(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageiterationsERKNS_6ObjectE");

TypedValue * fg1_magickgetimageiterations(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageiterations(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_magickgetimageiterations((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimageiterations(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetimageiterations((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageiterations(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageiterations", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickgetimagemattecolor(HPHP::Object const&)
_ZN4HPHP26f_magickgetimagemattecolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagemattecolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagemattecolorERKNS_6ObjectE");

TypedValue * fg1_magickgetimagemattecolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagemattecolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimagemattecolor((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagemattecolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickgetimagemattecolor((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagemattecolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagemattecolor", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetimagemimetype(HPHP::Object const&)
_ZN4HPHP24f_magickgetimagemimetypeERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagemimetype(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetimagemimetypeERKNS_6ObjectE");

TypedValue * fg1_magickgetimagemimetype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagemimetype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimagemimetype((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagemimetype(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetimagemimetype((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagemimetype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagemimetype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetimagepixels(HPHP::Object const&, int, int, double, double, HPHP::String const&, int)
_ZN4HPHP22f_magickgetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
x_offset => rdx
y_offset => rcx
columns => xmm0
rows => xmm1
smap => r8
storage_type => r9
*/

Value* fh_magickgetimagepixels(Value* _rv, Value* mgck_wnd, int x_offset, int y_offset, double columns, double rows, Value* smap, int storage_type) asm("_ZN4HPHP22f_magickgetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEi");

TypedValue * fg1_magickgetimagepixels(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagepixels(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if (!IS_STRING_TYPE((args-5)->m_type)) {
    tvCastToStringInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickgetimagepixels((Value*)(rv), (Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (Value*)(args-5), (int)(args[-6].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagepixels(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 7LL) {
      if ((args-6)->m_type == KindOfInt64 && IS_STRING_TYPE((args-5)->m_type) && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetimagepixels((Value*)(&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (Value*)(args-5), (int)(args[-6].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagepixels(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagepixels", count, 7, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetimageprofile(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP23f_magickgetimageprofileERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
name => rdx
*/

Value* fh_magickgetimageprofile(Value* _rv, Value* mgck_wnd, Value* name) asm("_ZN4HPHP23f_magickgetimageprofileERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magickgetimageprofile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageprofile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickgetimageprofile((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimageprofile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetimageprofile((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageprofile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageprofile", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetimageredprimary(HPHP::Object const&)
_ZN4HPHP26f_magickgetimageredprimaryERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimageredprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageredprimaryERKNS_6ObjectE");

TypedValue * fg1_magickgetimageredprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageredprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimageredprimary((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimageredprimary(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetimageredprimary((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageredprimary(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageredprimary", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimagerenderingintent(HPHP::Object const&)
_ZN4HPHP31f_magickgetimagerenderingintentERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimagerenderingintent(Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimagerenderingintentERKNS_6ObjectE");

TypedValue * fg1_magickgetimagerenderingintent(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagerenderingintent(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimagerenderingintent((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagerenderingintent(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimagerenderingintent((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagerenderingintent(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagerenderingintent", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetimageresolution(HPHP::Object const&)
_ZN4HPHP26f_magickgetimageresolutionERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimageresolution(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageresolutionERKNS_6ObjectE");

TypedValue * fg1_magickgetimageresolution(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageresolution(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimageresolution((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimageresolution(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetimageresolution((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageresolution(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageresolution", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetimagescene(HPHP::Object const&)
_ZN4HPHP21f_magickgetimagesceneERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagescene(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagesceneERKNS_6ObjectE");

TypedValue * fg1_magickgetimagescene(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagescene(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_magickgetimagescene((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagescene(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetimagescene((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagescene(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagescene", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetimagesignature(HPHP::Object const&)
_ZN4HPHP25f_magickgetimagesignatureERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagesignature(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickgetimagesignatureERKNS_6ObjectE");

TypedValue * fg1_magickgetimagesignature(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagesignature(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimagesignature((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagesignature(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetimagesignature((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagesignature(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagesignature", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimagesize(HPHP::Object const&)
_ZN4HPHP20f_magickgetimagesizeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimagesize(Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimagesizeERKNS_6ObjectE");

TypedValue * fg1_magickgetimagesize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagesize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimagesize((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagesize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimagesize((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagesize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagesize", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimagetype(HPHP::Object const&)
_ZN4HPHP20f_magickgetimagetypeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimagetype(Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimagetypeERKNS_6ObjectE");

TypedValue * fg1_magickgetimagetype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagetype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimagetype((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagetype(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimagetype((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagetype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagetype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimageunits(HPHP::Object const&)
_ZN4HPHP21f_magickgetimageunitsERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimageunits(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimageunitsERKNS_6ObjectE");

TypedValue * fg1_magickgetimageunits(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimageunits(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimageunits((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimageunits(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimageunits((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimageunits(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimageunits", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetimagevirtualpixelmethod(HPHP::Object const&)
_ZN4HPHP34f_magickgetimagevirtualpixelmethodERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetimagevirtualpixelmethod(Value* mgck_wnd) asm("_ZN4HPHP34f_magickgetimagevirtualpixelmethodERKNS_6ObjectE");

TypedValue * fg1_magickgetimagevirtualpixelmethod(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagevirtualpixelmethod(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetimagevirtualpixelmethod((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagevirtualpixelmethod(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetimagevirtualpixelmethod((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagevirtualpixelmethod(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagevirtualpixelmethod", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetimagewhitepoint(HPHP::Object const&)
_ZN4HPHP26f_magickgetimagewhitepointERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagewhitepoint(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagewhitepointERKNS_6ObjectE");

TypedValue * fg1_magickgetimagewhitepoint(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagewhitepoint(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimagewhitepoint((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagewhitepoint(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetimagewhitepoint((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagewhitepoint(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagewhitepoint", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetimagewidth(HPHP::Object const&)
_ZN4HPHP21f_magickgetimagewidthERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetimagewidth(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagewidthERKNS_6ObjectE");

TypedValue * fg1_magickgetimagewidth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagewidth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_magickgetimagewidth((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetimagewidth(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetimagewidth((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagewidth(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagewidth", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetimagesblob(HPHP::Object const&)
_ZN4HPHP21f_magickgetimagesblobERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetimagesblob(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagesblobERKNS_6ObjectE");

TypedValue * fg1_magickgetimagesblob(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetimagesblob(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickgetimagesblob((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetimagesblob(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetimagesblob((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetimagesblob(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetimagesblob", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_magickgetinterlacescheme(HPHP::Object const&)
_ZN4HPHP26f_magickgetinterlaceschemeERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

long fh_magickgetinterlacescheme(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetinterlaceschemeERKNS_6ObjectE");

TypedValue * fg1_magickgetinterlacescheme(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetinterlacescheme(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_magickgetinterlacescheme((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetinterlacescheme(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_magickgetinterlacescheme((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetinterlacescheme(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetinterlacescheme", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetmaxtextadvance(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP25f_magickgetmaxtextadvanceERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgetmaxtextadvance(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP25f_magickgetmaxtextadvanceERKNS_6ObjectES2_RKNS_6StringEb");

TypedValue * fg1_magickgetmaxtextadvance(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetmaxtextadvance(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.dbl = fh_magickgetmaxtextadvance((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_magickgetmaxtextadvance(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetmaxtextadvance((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetmaxtextadvance(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetmaxtextadvance", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickgetmimetype(HPHP::Object const&)
_ZN4HPHP19f_magickgetmimetypeERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetmimetype(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetmimetypeERKNS_6ObjectE");

TypedValue * fg1_magickgetmimetype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetmimetype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_magickgetmimetype((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetmimetype(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickgetmimetype((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetmimetype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetmimetype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetnumberimages(HPHP::Object const&)
_ZN4HPHP23f_magickgetnumberimagesERKNS_6ObjectE

(return value) => xmm0
mgck_wnd => rdi
*/

double fh_magickgetnumberimages(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetnumberimagesERKNS_6ObjectE");

TypedValue * fg1_magickgetnumberimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetnumberimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_magickgetnumberimages((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickgetnumberimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetnumberimages((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetnumberimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetnumberimages", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetsamplingfactors(HPHP::Object const&)
_ZN4HPHP26f_magickgetsamplingfactorsERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetsamplingfactors(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetsamplingfactorsERKNS_6ObjectE");

TypedValue * fg1_magickgetsamplingfactors(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetsamplingfactors(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_magickgetsamplingfactors((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetsamplingfactors(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetsamplingfactors((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetsamplingfactors(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetsamplingfactors", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetsize(HPHP::Object const&)
_ZN4HPHP15f_magickgetsizeERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetsize(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP15f_magickgetsizeERKNS_6ObjectE");

TypedValue * fg1_magickgetsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_magickgetsize((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetsize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetsize((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetsize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetsize", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetstringheight(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP23f_magickgetstringheightERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgetstringheight(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP23f_magickgetstringheightERKNS_6ObjectES2_RKNS_6StringEb");

TypedValue * fg1_magickgetstringheight(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetstringheight(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.dbl = fh_magickgetstringheight((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_magickgetstringheight(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetstringheight((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetstringheight(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetstringheight", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgetstringwidth(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP22f_magickgetstringwidthERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgetstringwidth(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP22f_magickgetstringwidthERKNS_6ObjectES2_RKNS_6StringEb");

TypedValue * fg1_magickgetstringwidth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetstringwidth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.dbl = fh_magickgetstringwidth((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_magickgetstringwidth(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgetstringwidth((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetstringwidth(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetstringwidth", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgettextascent(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP21f_magickgettextascentERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgettextascent(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP21f_magickgettextascentERKNS_6ObjectES2_RKNS_6StringEb");

TypedValue * fg1_magickgettextascent(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgettextascent(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.dbl = fh_magickgettextascent((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_magickgettextascent(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgettextascent((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgettextascent(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgettextascent", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_magickgettextdescent(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP22f_magickgettextdescentERKNS_6ObjectES2_RKNS_6StringEb

(return value) => xmm0
mgck_wnd => rdi
drw_wnd => rsi
txt => rdx
multiline => rcx
*/

double fh_magickgettextdescent(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP22f_magickgettextdescentERKNS_6ObjectES2_RKNS_6StringEb");

TypedValue * fg1_magickgettextdescent(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgettextdescent(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.dbl = fh_magickgettextdescent((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_magickgettextdescent(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_magickgettextdescent((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgettextdescent(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgettextdescent", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickgetwandsize(HPHP::Object const&)
_ZN4HPHP19f_magickgetwandsizeERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickgetwandsize(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetwandsizeERKNS_6ObjectE");

TypedValue * fg1_magickgetwandsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickgetwandsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_magickgetwandsize((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickgetwandsize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickgetwandsize((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickgetwandsize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickgetwandsize", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickhasnextimage(HPHP::Object const&)
_ZN4HPHP20f_magickhasnextimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickhasnextimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickhasnextimageERKNS_6ObjectE");

TypedValue * fg1_magickhasnextimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickhasnextimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickhasnextimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickhasnextimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickhasnextimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickhasnextimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickhasnextimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickhaspreviousimage(HPHP::Object const&)
_ZN4HPHP24f_magickhaspreviousimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickhaspreviousimage(Value* mgck_wnd) asm("_ZN4HPHP24f_magickhaspreviousimageERKNS_6ObjectE");

TypedValue * fg1_magickhaspreviousimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickhaspreviousimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickhaspreviousimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickhaspreviousimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickhaspreviousimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickhaspreviousimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickhaspreviousimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickimplodeimage(HPHP::Object const&, double)
_ZN4HPHP20f_magickimplodeimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
amount => xmm0
*/

bool fh_magickimplodeimage(Value* mgck_wnd, double amount) asm("_ZN4HPHP20f_magickimplodeimageERKNS_6ObjectEd");

TypedValue * fg1_magickimplodeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickimplodeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickimplodeimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickimplodeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickimplodeimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickimplodeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickimplodeimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicklabelimage(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP18f_magicklabelimageERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
label => rsi
*/

bool fh_magicklabelimage(Value* mgck_wnd, Value* label) asm("_ZN4HPHP18f_magicklabelimageERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magicklabelimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicklabelimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicklabelimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicklabelimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicklabelimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicklabelimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicklabelimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicklevelimage(HPHP::Object const&, double, double, double, int)
_ZN4HPHP18f_magicklevelimageERKNS_6ObjectEdddi

(return value) => rax
mgck_wnd => rdi
black_point => xmm0
gamma => xmm1
white_point => xmm2
channel_type => rsi
*/

bool fh_magicklevelimage(Value* mgck_wnd, double black_point, double gamma, double white_point, int channel_type) asm("_ZN4HPHP18f_magicklevelimageERKNS_6ObjectEdddi");

TypedValue * fg1_magicklevelimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicklevelimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    break;
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicklevelimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicklevelimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfInt64) && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicklevelimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicklevelimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicklevelimage", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickmagnifyimage(HPHP::Object const&)
_ZN4HPHP20f_magickmagnifyimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickmagnifyimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickmagnifyimageERKNS_6ObjectE");

TypedValue * fg1_magickmagnifyimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickmagnifyimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickmagnifyimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickmagnifyimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickmagnifyimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickmagnifyimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickmagnifyimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickmapimage(HPHP::Object const&, HPHP::Object const&, bool)
_ZN4HPHP16f_magickmapimageERKNS_6ObjectES2_b

(return value) => rax
mgck_wnd => rdi
map_wand => rsi
dither => rdx
*/

bool fh_magickmapimage(Value* mgck_wnd, Value* map_wand, bool dither) asm("_ZN4HPHP16f_magickmapimageERKNS_6ObjectES2_b");

TypedValue * fg1_magickmapimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickmapimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickmapimage((Value*)(args-0), (Value*)(args-1), (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickmapimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfBoolean && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickmapimage((Value*)(args-0), (Value*)(args-1), (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickmapimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickmapimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickmattefloodfillimage(HPHP::Object const&, double, double, HPHP::Object const&, int, int)
_ZN4HPHP27f_magickmattefloodfillimageERKNS_6ObjectEddS2_ii

(return value) => rax
mgck_wnd => rdi
opacity => xmm0
fuzz => xmm1
bordercolor_pxl_wnd => rsi
x => rdx
y => rcx
*/

bool fh_magickmattefloodfillimage(Value* mgck_wnd, double opacity, double fuzz, Value* bordercolor_pxl_wnd, int x, int y) asm("_ZN4HPHP27f_magickmattefloodfillimageERKNS_6ObjectEddS2_ii");

TypedValue * fg1_magickmattefloodfillimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickmattefloodfillimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickmattefloodfillimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (Value*)(args-3), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickmattefloodfillimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfObject && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickmattefloodfillimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (Value*)(args-3), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickmattefloodfillimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickmattefloodfillimage", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickmedianfilterimage(HPHP::Object const&, double)
_ZN4HPHP25f_magickmedianfilterimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
*/

bool fh_magickmedianfilterimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP25f_magickmedianfilterimageERKNS_6ObjectEd");

TypedValue * fg1_magickmedianfilterimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickmedianfilterimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickmedianfilterimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickmedianfilterimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickmedianfilterimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickmedianfilterimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickmedianfilterimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickminifyimage(HPHP::Object const&)
_ZN4HPHP19f_magickminifyimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickminifyimage(Value* mgck_wnd) asm("_ZN4HPHP19f_magickminifyimageERKNS_6ObjectE");

TypedValue * fg1_magickminifyimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickminifyimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickminifyimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickminifyimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickminifyimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickminifyimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickminifyimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickmodulateimage(HPHP::Object const&, double, double, double)
_ZN4HPHP21f_magickmodulateimageERKNS_6ObjectEddd

(return value) => rax
mgck_wnd => rdi
brightness => xmm0
saturation => xmm1
hue => xmm2
*/

bool fh_magickmodulateimage(Value* mgck_wnd, double brightness, double saturation, double hue) asm("_ZN4HPHP21f_magickmodulateimageERKNS_6ObjectEddd");

TypedValue * fg1_magickmodulateimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickmodulateimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickmodulateimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickmodulateimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickmodulateimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickmodulateimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickmodulateimage", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickmontageimage(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP20f_magickmontageimageERKNS_6ObjectES2_RKNS_6StringES5_iS5_

(return value) => rax
_rv => rdi
mgck_wnd => rsi
drw_wnd => rdx
tile_geometry => rcx
thumbnail_geometry => r8
montage_mode => r9
frame => st0
*/

Value* fh_magickmontageimage(Value* _rv, Value* mgck_wnd, Value* drw_wnd, Value* tile_geometry, Value* thumbnail_geometry, int montage_mode, Value* frame) asm("_ZN4HPHP20f_magickmontageimageERKNS_6ObjectES2_RKNS_6StringES5_iS5_");

TypedValue * fg1_magickmontageimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickmontageimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if (!IS_STRING_TYPE((args-5)->m_type)) {
    tvCastToStringInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickmontageimage((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3), (int)(args[-4].m_data.num), (Value*)(args-5));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickmontageimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if (IS_STRING_TYPE((args-5)->m_type) && (args-4)->m_type == KindOfInt64 && IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickmontageimage((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3), (int)(args[-4].m_data.num), (Value*)(args-5));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickmontageimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickmontageimage", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickmorphimages(HPHP::Object const&, double)
_ZN4HPHP19f_magickmorphimagesERKNS_6ObjectEd

(return value) => rax
_rv => rdi
mgck_wnd => rsi
number_frames => xmm0
*/

Value* fh_magickmorphimages(Value* _rv, Value* mgck_wnd, double number_frames) asm("_ZN4HPHP19f_magickmorphimagesERKNS_6ObjectEd");

TypedValue * fg1_magickmorphimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickmorphimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickmorphimages((Value*)(rv), (Value*)(args-0), (args[-1].m_data.dbl));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickmorphimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickmorphimages((Value*)(&(rv)), (Value*)(args-0), (args[-1].m_data.dbl));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickmorphimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickmorphimages", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickmosaicimages(HPHP::Object const&)
_ZN4HPHP20f_magickmosaicimagesERKNS_6ObjectE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
*/

Value* fh_magickmosaicimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickmosaicimagesERKNS_6ObjectE");

TypedValue * fg1_magickmosaicimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickmosaicimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_magickmosaicimages((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickmosaicimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickmosaicimages((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickmosaicimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickmosaicimages", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickmotionblurimage(HPHP::Object const&, double, double, double)
_ZN4HPHP23f_magickmotionblurimageERKNS_6ObjectEddd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
angle => xmm2
*/

bool fh_magickmotionblurimage(Value* mgck_wnd, double radius, double sigma, double angle) asm("_ZN4HPHP23f_magickmotionblurimageERKNS_6ObjectEddd");

TypedValue * fg1_magickmotionblurimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickmotionblurimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickmotionblurimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickmotionblurimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickmotionblurimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickmotionblurimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickmotionblurimage", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicknegateimage(HPHP::Object const&, bool, int)
_ZN4HPHP19f_magicknegateimageERKNS_6ObjectEbi

(return value) => rax
mgck_wnd => rdi
only_the_gray => rsi
channel_type => rdx
*/

bool fh_magicknegateimage(Value* mgck_wnd, bool only_the_gray, int channel_type) asm("_ZN4HPHP19f_magicknegateimageERKNS_6ObjectEbi");

TypedValue * fg1_magicknegateimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicknegateimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicknegateimage((Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicknegateimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicknegateimage((Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicknegateimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicknegateimage", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicknewimage(HPHP::Object const&, double, double, HPHP::String const&)
_ZN4HPHP16f_magicknewimageERKNS_6ObjectEddRKNS_6StringE

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
imagemagick_col_str => rsi
*/

bool fh_magicknewimage(Value* mgck_wnd, double width, double height, Value* imagemagick_col_str) asm("_ZN4HPHP16f_magicknewimageERKNS_6ObjectEddRKNS_6StringE");

TypedValue * fg1_magicknewimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicknewimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicknewimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicknewimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicknewimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicknewimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicknewimage", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicknextimage(HPHP::Object const&)
_ZN4HPHP17f_magicknextimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magicknextimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magicknextimageERKNS_6ObjectE");

TypedValue * fg1_magicknextimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicknextimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magicknextimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicknextimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicknextimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicknextimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicknextimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicknormalizeimage(HPHP::Object const&)
_ZN4HPHP22f_magicknormalizeimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magicknormalizeimage(Value* mgck_wnd) asm("_ZN4HPHP22f_magicknormalizeimageERKNS_6ObjectE");

TypedValue * fg1_magicknormalizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicknormalizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magicknormalizeimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicknormalizeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicknormalizeimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicknormalizeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicknormalizeimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickoilpaintimage(HPHP::Object const&, double)
_ZN4HPHP21f_magickoilpaintimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
*/

bool fh_magickoilpaintimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP21f_magickoilpaintimageERKNS_6ObjectEd");

TypedValue * fg1_magickoilpaintimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickoilpaintimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickoilpaintimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickoilpaintimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickoilpaintimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickoilpaintimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickoilpaintimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickpaintopaqueimage(HPHP::Object const&, HPHP::Object const&, HPHP::Object const&, double)
_ZN4HPHP24f_magickpaintopaqueimageERKNS_6ObjectES2_S2_d

(return value) => rax
mgck_wnd => rdi
target_pxl_wnd => rsi
fill_pxl_wnd => rdx
fuzz => xmm0
*/

bool fh_magickpaintopaqueimage(Value* mgck_wnd, Value* target_pxl_wnd, Value* fill_pxl_wnd, double fuzz) asm("_ZN4HPHP24f_magickpaintopaqueimageERKNS_6ObjectES2_S2_d");

TypedValue * fg1_magickpaintopaqueimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickpaintopaqueimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickpaintopaqueimage((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickpaintopaqueimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfDouble) && (args-2)->m_type == KindOfObject && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickpaintopaqueimage((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickpaintopaqueimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickpaintopaqueimage", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickpainttransparentimage(HPHP::Object const&, HPHP::Object const&, double, double)
_ZN4HPHP29f_magickpainttransparentimageERKNS_6ObjectES2_dd

(return value) => rax
mgck_wnd => rdi
target => rsi
opacity => xmm0
fuzz => xmm1
*/

bool fh_magickpainttransparentimage(Value* mgck_wnd, Value* target, double opacity, double fuzz) asm("_ZN4HPHP29f_magickpainttransparentimageERKNS_6ObjectES2_dd");

TypedValue * fg1_magickpainttransparentimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickpainttransparentimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickpainttransparentimage((Value*)(args-0), (Value*)(args-1), (count > 2) ? (args[-2].m_data.dbl) : (double)(k_MW_TransparentOpacity), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickpainttransparentimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfDouble) && (count <= 2 || (args-2)->m_type == KindOfDouble) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickpainttransparentimage((Value*)(args-0), (Value*)(args-1), (count > 2) ? (args[-2].m_data.dbl) : (double)(k_MW_TransparentOpacity), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickpainttransparentimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickpainttransparentimage", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickpingimage(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_magickpingimageERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
filename => rsi
*/

bool fh_magickpingimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP17f_magickpingimageERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magickpingimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickpingimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickpingimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickpingimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickpingimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickpingimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickpingimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickposterizeimage(HPHP::Object const&, double, bool)
_ZN4HPHP22f_magickposterizeimageERKNS_6ObjectEdb

(return value) => rax
mgck_wnd => rdi
levels => xmm0
dither => rsi
*/

bool fh_magickposterizeimage(Value* mgck_wnd, double levels, bool dither) asm("_ZN4HPHP22f_magickposterizeimageERKNS_6ObjectEdb");

TypedValue * fg1_magickposterizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickposterizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickposterizeimage((Value*)(args-0), (args[-1].m_data.dbl), (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickposterizeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfBoolean && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickposterizeimage((Value*)(args-0), (args[-1].m_data.dbl), (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickposterizeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickposterizeimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magickpreviewimages(HPHP::Object const&, int)
_ZN4HPHP21f_magickpreviewimagesERKNS_6ObjectEi

(return value) => rax
_rv => rdi
mgck_wnd => rsi
preview => rdx
*/

Value* fh_magickpreviewimages(Value* _rv, Value* mgck_wnd, int preview) asm("_ZN4HPHP21f_magickpreviewimagesERKNS_6ObjectEi");

TypedValue * fg1_magickpreviewimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickpreviewimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickpreviewimages((Value*)(rv), (Value*)(args-0), (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickpreviewimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magickpreviewimages((Value*)(&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickpreviewimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickpreviewimages", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickpreviousimage(HPHP::Object const&)
_ZN4HPHP21f_magickpreviousimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickpreviousimage(Value* mgck_wnd) asm("_ZN4HPHP21f_magickpreviousimageERKNS_6ObjectE");

TypedValue * fg1_magickpreviousimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickpreviousimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickpreviousimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickpreviousimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickpreviousimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickpreviousimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickpreviousimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickprofileimage(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_magickprofileimageERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
mgck_wnd => rdi
name => rsi
profile => rdx
*/

bool fh_magickprofileimage(Value* mgck_wnd, Value* name, Value* profile) asm("_ZN4HPHP20f_magickprofileimageERKNS_6ObjectERKNS_6StringES5_");

TypedValue * fg1_magickprofileimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickprofileimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickprofileimage((Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickprofileimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickprofileimage((Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickprofileimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickprofileimage", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickquantizeimage(HPHP::Object const&, double, int, double, bool, bool)
_ZN4HPHP21f_magickquantizeimageERKNS_6ObjectEdidbb

(return value) => rax
mgck_wnd => rdi
number_colors => xmm0
colorspace_type => rsi
treedepth => xmm1
dither => rdx
measure_error => rcx
*/

bool fh_magickquantizeimage(Value* mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) asm("_ZN4HPHP21f_magickquantizeimageERKNS_6ObjectEdidbb");

TypedValue * fg1_magickquantizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickquantizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickquantizeimage((Value*)(args-0), (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickquantizeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfBoolean && (args-4)->m_type == KindOfBoolean && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickquantizeimage((Value*)(args-0), (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickquantizeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickquantizeimage", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickquantizeimages(HPHP::Object const&, double, int, double, bool, bool)
_ZN4HPHP22f_magickquantizeimagesERKNS_6ObjectEdidbb

(return value) => rax
mgck_wnd => rdi
number_colors => xmm0
colorspace_type => rsi
treedepth => xmm1
dither => rdx
measure_error => rcx
*/

bool fh_magickquantizeimages(Value* mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) asm("_ZN4HPHP22f_magickquantizeimagesERKNS_6ObjectEdidbb");

TypedValue * fg1_magickquantizeimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickquantizeimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickquantizeimages((Value*)(args-0), (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickquantizeimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfBoolean && (args-4)->m_type == KindOfBoolean && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickquantizeimages((Value*)(args-0), (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickquantizeimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickquantizeimages", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_magickqueryfontmetrics(HPHP::Object const&, HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP24f_magickqueryfontmetricsERKNS_6ObjectES2_RKNS_6StringEb

(return value) => rax
_rv => rdi
mgck_wnd => rsi
drw_wnd => rdx
txt => rcx
multiline => r8
*/

Value* fh_magickqueryfontmetrics(Value* _rv, Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP24f_magickqueryfontmetricsERKNS_6ObjectES2_RKNS_6StringEb");

TypedValue * fg1_magickqueryfontmetrics(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickqueryfontmetrics(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickqueryfontmetrics((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickqueryfontmetrics(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_magickqueryfontmetrics((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickqueryfontmetrics(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickqueryfontmetrics", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickradialblurimage(HPHP::Object const&, double)
_ZN4HPHP23f_magickradialblurimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
angle => xmm0
*/

bool fh_magickradialblurimage(Value* mgck_wnd, double angle) asm("_ZN4HPHP23f_magickradialblurimageERKNS_6ObjectEd");

TypedValue * fg1_magickradialblurimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickradialblurimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickradialblurimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickradialblurimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickradialblurimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickradialblurimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickradialblurimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickraiseimage(HPHP::Object const&, double, double, int, int, bool)
_ZN4HPHP18f_magickraiseimageERKNS_6ObjectEddiib

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
x => rsi
y => rdx
raise => rcx
*/

bool fh_magickraiseimage(Value* mgck_wnd, double width, double height, int x, int y, bool raise) asm("_ZN4HPHP18f_magickraiseimageERKNS_6ObjectEddiib");

TypedValue * fg1_magickraiseimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickraiseimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickraiseimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickraiseimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfBoolean && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickraiseimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickraiseimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickraiseimage", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickreadimage(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_magickreadimageERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
filename => rsi
*/

bool fh_magickreadimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP17f_magickreadimageERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magickreadimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickreadimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickreadimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickreadimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickreadimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickreadimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickreadimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickreadimageblob(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21f_magickreadimageblobERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
blob => rsi
*/

bool fh_magickreadimageblob(Value* mgck_wnd, Value* blob) asm("_ZN4HPHP21f_magickreadimageblobERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magickreadimageblob(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickreadimageblob(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickreadimageblob((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickreadimageblob(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickreadimageblob((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickreadimageblob(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickreadimageblob", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickreadimagefile(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21f_magickreadimagefileERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
handle => rsi
*/

bool fh_magickreadimagefile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP21f_magickreadimagefileERKNS_6ObjectES2_");

TypedValue * fg1_magickreadimagefile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickreadimagefile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickreadimagefile((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickreadimagefile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickreadimagefile((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickreadimagefile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickreadimagefile", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickreadimages(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP18f_magickreadimagesERKNS_6ObjectERKNS_5ArrayE

(return value) => rax
mgck_wnd => rdi
img_filenames_array => rsi
*/

bool fh_magickreadimages(Value* mgck_wnd, Value* img_filenames_array) asm("_ZN4HPHP18f_magickreadimagesERKNS_6ObjectERKNS_5ArrayE");

TypedValue * fg1_magickreadimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickreadimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickreadimages((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickreadimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickreadimages((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickreadimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickreadimages", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickreducenoiseimage(HPHP::Object const&, double)
_ZN4HPHP24f_magickreducenoiseimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
*/

bool fh_magickreducenoiseimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP24f_magickreducenoiseimageERKNS_6ObjectEd");

TypedValue * fg1_magickreducenoiseimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickreducenoiseimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickreducenoiseimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickreducenoiseimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickreducenoiseimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickreducenoiseimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickreducenoiseimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickremoveimage(HPHP::Object const&)
_ZN4HPHP19f_magickremoveimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickremoveimage(Value* mgck_wnd) asm("_ZN4HPHP19f_magickremoveimageERKNS_6ObjectE");

TypedValue * fg1_magickremoveimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickremoveimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickremoveimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickremoveimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickremoveimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickremoveimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickremoveimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_magickremoveimageprofile(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP26f_magickremoveimageprofileERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
mgck_wnd => rsi
name => rdx
*/

Value* fh_magickremoveimageprofile(Value* _rv, Value* mgck_wnd, Value* name) asm("_ZN4HPHP26f_magickremoveimageprofileERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magickremoveimageprofile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickremoveimageprofile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magickremoveimageprofile((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magickremoveimageprofile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_magickremoveimageprofile((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickremoveimageprofile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickremoveimageprofile", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickremoveimageprofiles(HPHP::Object const&)
_ZN4HPHP27f_magickremoveimageprofilesERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickremoveimageprofiles(Value* mgck_wnd) asm("_ZN4HPHP27f_magickremoveimageprofilesERKNS_6ObjectE");

TypedValue * fg1_magickremoveimageprofiles(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickremoveimageprofiles(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickremoveimageprofiles((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickremoveimageprofiles(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickremoveimageprofiles((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickremoveimageprofiles(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickremoveimageprofiles", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickresampleimage(HPHP::Object const&, double, double, int, double)
_ZN4HPHP21f_magickresampleimageERKNS_6ObjectEddid

(return value) => rax
mgck_wnd => rdi
x_resolution => xmm0
y_resolution => xmm1
filter_type => rsi
blur => xmm2
*/

bool fh_magickresampleimage(Value* mgck_wnd, double x_resolution, double y_resolution, int filter_type, double blur) asm("_ZN4HPHP21f_magickresampleimageERKNS_6ObjectEddid");

TypedValue * fg1_magickresampleimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickresampleimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickresampleimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (args[-4].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickresampleimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickresampleimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (args[-4].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickresampleimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickresampleimage", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_magickresetiterator(HPHP::Object const&)
_ZN4HPHP21f_magickresetiteratorERKNS_6ObjectE

mgck_wnd => rdi
*/

void fh_magickresetiterator(Value* mgck_wnd) asm("_ZN4HPHP21f_magickresetiteratorERKNS_6ObjectE");

TypedValue * fg1_magickresetiterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickresetiterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_magickresetiterator((Value*)(args-0));
  return rv;
}

TypedValue* fg_magickresetiterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_magickresetiterator((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickresetiterator(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickresetiterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickresizeimage(HPHP::Object const&, double, double, int, double)
_ZN4HPHP19f_magickresizeimageERKNS_6ObjectEddid

(return value) => rax
mgck_wnd => rdi
columns => xmm0
rows => xmm1
filter_type => rsi
blur => xmm2
*/

bool fh_magickresizeimage(Value* mgck_wnd, double columns, double rows, int filter_type, double blur) asm("_ZN4HPHP19f_magickresizeimageERKNS_6ObjectEddid");

TypedValue * fg1_magickresizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickresizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickresizeimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (args[-4].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickresizeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickresizeimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (args[-4].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickresizeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickresizeimage", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickrollimage(HPHP::Object const&, int, int)
_ZN4HPHP17f_magickrollimageERKNS_6ObjectEii

(return value) => rax
mgck_wnd => rdi
x_offset => rsi
y_offset => rdx
*/

bool fh_magickrollimage(Value* mgck_wnd, int x_offset, int y_offset) asm("_ZN4HPHP17f_magickrollimageERKNS_6ObjectEii");

TypedValue * fg1_magickrollimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickrollimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickrollimage((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickrollimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickrollimage((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickrollimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickrollimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickrotateimage(HPHP::Object const&, HPHP::Object const&, double)
_ZN4HPHP19f_magickrotateimageERKNS_6ObjectES2_d

(return value) => rax
mgck_wnd => rdi
background => rsi
degrees => xmm0
*/

bool fh_magickrotateimage(Value* mgck_wnd, Value* background, double degrees) asm("_ZN4HPHP19f_magickrotateimageERKNS_6ObjectES2_d");

TypedValue * fg1_magickrotateimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickrotateimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickrotateimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickrotateimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickrotateimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickrotateimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickrotateimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksampleimage(HPHP::Object const&, double, double)
_ZN4HPHP19f_magicksampleimageERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
columns => xmm0
rows => xmm1
*/

bool fh_magicksampleimage(Value* mgck_wnd, double columns, double rows) asm("_ZN4HPHP19f_magicksampleimageERKNS_6ObjectEdd");

TypedValue * fg1_magicksampleimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksampleimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksampleimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksampleimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksampleimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksampleimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksampleimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickscaleimage(HPHP::Object const&, double, double)
_ZN4HPHP18f_magickscaleimageERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
columns => xmm0
rows => xmm1
*/

bool fh_magickscaleimage(Value* mgck_wnd, double columns, double rows) asm("_ZN4HPHP18f_magickscaleimageERKNS_6ObjectEdd");

TypedValue * fg1_magickscaleimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickscaleimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickscaleimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickscaleimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickscaleimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickscaleimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickscaleimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickseparateimagechannel(HPHP::Object const&, int)
_ZN4HPHP28f_magickseparateimagechannelERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
channel_type => rsi
*/

bool fh_magickseparateimagechannel(Value* mgck_wnd, int channel_type) asm("_ZN4HPHP28f_magickseparateimagechannelERKNS_6ObjectEi");

TypedValue * fg1_magickseparateimagechannel(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickseparateimagechannel(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickseparateimagechannel((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickseparateimagechannel(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickseparateimagechannel((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickseparateimagechannel(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickseparateimagechannel", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetcompressionquality(HPHP::Object const&, double)
_ZN4HPHP29f_magicksetcompressionqualityERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
quality => xmm0
*/

bool fh_magicksetcompressionquality(Value* mgck_wnd, double quality) asm("_ZN4HPHP29f_magicksetcompressionqualityERKNS_6ObjectEd");

TypedValue * fg1_magicksetcompressionquality(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetcompressionquality(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetcompressionquality((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetcompressionquality(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetcompressionquality((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetcompressionquality(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetcompressionquality", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetfilename(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP19f_magicksetfilenameERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
filename => rsi
*/

bool fh_magicksetfilename(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP19f_magicksetfilenameERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magicksetfilename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetfilename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetfilename((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetfilename(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetfilename((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetfilename(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetfilename", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_magicksetfirstiterator(HPHP::Object const&)
_ZN4HPHP24f_magicksetfirstiteratorERKNS_6ObjectE

mgck_wnd => rdi
*/

void fh_magicksetfirstiterator(Value* mgck_wnd) asm("_ZN4HPHP24f_magicksetfirstiteratorERKNS_6ObjectE");

TypedValue * fg1_magicksetfirstiterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetfirstiterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_magicksetfirstiterator((Value*)(args-0));
  return rv;
}

TypedValue* fg_magicksetfirstiterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_magicksetfirstiterator((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetfirstiterator(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetfirstiterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetformat(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_magicksetformatERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
format => rsi
*/

bool fh_magicksetformat(Value* mgck_wnd, Value* format) asm("_ZN4HPHP17f_magicksetformatERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magicksetformat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetformat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetformat((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetformat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetformat((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetformat(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetformat", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP16f_magicksetimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
replace_wand => rsi
*/

bool fh_magicksetimage(Value* mgck_wnd, Value* replace_wand) asm("_ZN4HPHP16f_magicksetimageERKNS_6ObjectES2_");

TypedValue * fg1_magicksetimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagebackgroundcolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP31f_magicksetimagebackgroundcolorERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
background_pxl_wnd => rsi
*/

bool fh_magicksetimagebackgroundcolor(Value* mgck_wnd, Value* background_pxl_wnd) asm("_ZN4HPHP31f_magicksetimagebackgroundcolorERKNS_6ObjectES2_");

TypedValue * fg1_magicksetimagebackgroundcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagebackgroundcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagebackgroundcolor((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagebackgroundcolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagebackgroundcolor((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagebackgroundcolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagebackgroundcolor", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagebias(HPHP::Object const&, double)
_ZN4HPHP20f_magicksetimagebiasERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
bias => xmm0
*/

bool fh_magicksetimagebias(Value* mgck_wnd, double bias) asm("_ZN4HPHP20f_magicksetimagebiasERKNS_6ObjectEd");

TypedValue * fg1_magicksetimagebias(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagebias(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagebias((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagebias(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagebias((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagebias(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagebias", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimageblueprimary(HPHP::Object const&, double, double)
_ZN4HPHP27f_magicksetimageblueprimaryERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x => xmm0
y => xmm1
*/

bool fh_magicksetimageblueprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP27f_magicksetimageblueprimaryERKNS_6ObjectEdd");

TypedValue * fg1_magicksetimageblueprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimageblueprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimageblueprimary((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimageblueprimary(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimageblueprimary((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimageblueprimary(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimageblueprimary", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagebordercolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP27f_magicksetimagebordercolorERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
border_pxl_wnd => rsi
*/

bool fh_magicksetimagebordercolor(Value* mgck_wnd, Value* border_pxl_wnd) asm("_ZN4HPHP27f_magicksetimagebordercolorERKNS_6ObjectES2_");

TypedValue * fg1_magicksetimagebordercolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagebordercolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagebordercolor((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagebordercolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagebordercolor((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagebordercolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagebordercolor", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagecolormapcolor(HPHP::Object const&, double, HPHP::Object const&)
_ZN4HPHP29f_magicksetimagecolormapcolorERKNS_6ObjectEdS2_

(return value) => rax
mgck_wnd => rdi
index => xmm0
mapcolor_pxl_wnd => rsi
*/

bool fh_magicksetimagecolormapcolor(Value* mgck_wnd, double index, Value* mapcolor_pxl_wnd) asm("_ZN4HPHP29f_magicksetimagecolormapcolorERKNS_6ObjectEdS2_");

TypedValue * fg1_magicksetimagecolormapcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagecolormapcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagecolormapcolor((Value*)(args-0), (args[-1].m_data.dbl), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagecolormapcolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfObject && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagecolormapcolor((Value*)(args-0), (args[-1].m_data.dbl), (Value*)(args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagecolormapcolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagecolormapcolor", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagecolorspace(HPHP::Object const&, int)
_ZN4HPHP26f_magicksetimagecolorspaceERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
colorspace_type => rsi
*/

bool fh_magicksetimagecolorspace(Value* mgck_wnd, int colorspace_type) asm("_ZN4HPHP26f_magicksetimagecolorspaceERKNS_6ObjectEi");

TypedValue * fg1_magicksetimagecolorspace(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagecolorspace(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagecolorspace((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagecolorspace(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagecolorspace((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagecolorspace(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagecolorspace", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagecompose(HPHP::Object const&, int)
_ZN4HPHP23f_magicksetimagecomposeERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
composite_operator => rsi
*/

bool fh_magicksetimagecompose(Value* mgck_wnd, int composite_operator) asm("_ZN4HPHP23f_magicksetimagecomposeERKNS_6ObjectEi");

TypedValue * fg1_magicksetimagecompose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagecompose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagecompose((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagecompose(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagecompose((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagecompose(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagecompose", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagecompression(HPHP::Object const&, int)
_ZN4HPHP27f_magicksetimagecompressionERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
compression_type => rsi
*/

bool fh_magicksetimagecompression(Value* mgck_wnd, int compression_type) asm("_ZN4HPHP27f_magicksetimagecompressionERKNS_6ObjectEi");

TypedValue * fg1_magicksetimagecompression(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagecompression(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagecompression((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagecompression(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagecompression((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagecompression(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagecompression", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagecompressionquality(HPHP::Object const&, double)
_ZN4HPHP34f_magicksetimagecompressionqualityERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
quality => xmm0
*/

bool fh_magicksetimagecompressionquality(Value* mgck_wnd, double quality) asm("_ZN4HPHP34f_magicksetimagecompressionqualityERKNS_6ObjectEd");

TypedValue * fg1_magicksetimagecompressionquality(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagecompressionquality(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagecompressionquality((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagecompressionquality(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagecompressionquality((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagecompressionquality(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagecompressionquality", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagedelay(HPHP::Object const&, double)
_ZN4HPHP21f_magicksetimagedelayERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
delay => xmm0
*/

bool fh_magicksetimagedelay(Value* mgck_wnd, double delay) asm("_ZN4HPHP21f_magicksetimagedelayERKNS_6ObjectEd");

TypedValue * fg1_magicksetimagedelay(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagedelay(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagedelay((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagedelay(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagedelay((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagedelay(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagedelay", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagedepth(HPHP::Object const&, int, int)
_ZN4HPHP21f_magicksetimagedepthERKNS_6ObjectEii

(return value) => rax
mgck_wnd => rdi
depth => rsi
channel_type => rdx
*/

bool fh_magicksetimagedepth(Value* mgck_wnd, int depth, int channel_type) asm("_ZN4HPHP21f_magicksetimagedepthERKNS_6ObjectEii");

TypedValue * fg1_magicksetimagedepth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagedepth(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagedepth((Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagedepth(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagedepth((Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagedepth(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagedepth", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagedispose(HPHP::Object const&, int)
_ZN4HPHP23f_magicksetimagedisposeERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
dispose_type => rsi
*/

bool fh_magicksetimagedispose(Value* mgck_wnd, int dispose_type) asm("_ZN4HPHP23f_magicksetimagedisposeERKNS_6ObjectEi");

TypedValue * fg1_magicksetimagedispose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagedispose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagedispose((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagedispose(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagedispose((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagedispose(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagedispose", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagefilename(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP24f_magicksetimagefilenameERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
filename => rsi
*/

bool fh_magicksetimagefilename(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP24f_magicksetimagefilenameERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magicksetimagefilename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagefilename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagefilename((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagefilename(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagefilename((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagefilename(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagefilename", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimageformat(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP22f_magicksetimageformatERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
format => rsi
*/

bool fh_magicksetimageformat(Value* mgck_wnd, Value* format) asm("_ZN4HPHP22f_magicksetimageformatERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magicksetimageformat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimageformat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimageformat((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimageformat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimageformat((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimageformat(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimageformat", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagegamma(HPHP::Object const&, double)
_ZN4HPHP21f_magicksetimagegammaERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
gamma => xmm0
*/

bool fh_magicksetimagegamma(Value* mgck_wnd, double gamma) asm("_ZN4HPHP21f_magicksetimagegammaERKNS_6ObjectEd");

TypedValue * fg1_magicksetimagegamma(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagegamma(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagegamma((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagegamma(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagegamma((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagegamma(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagegamma", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagegreenprimary(HPHP::Object const&, double, double)
_ZN4HPHP28f_magicksetimagegreenprimaryERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x => xmm0
y => xmm1
*/

bool fh_magicksetimagegreenprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP28f_magicksetimagegreenprimaryERKNS_6ObjectEdd");

TypedValue * fg1_magicksetimagegreenprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagegreenprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagegreenprimary((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagegreenprimary(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagegreenprimary((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagegreenprimary(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagegreenprimary", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimageindex(HPHP::Object const&, int)
_ZN4HPHP21f_magicksetimageindexERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
index => rsi
*/

bool fh_magicksetimageindex(Value* mgck_wnd, int index) asm("_ZN4HPHP21f_magicksetimageindexERKNS_6ObjectEi");

TypedValue * fg1_magicksetimageindex(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimageindex(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimageindex((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimageindex(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimageindex((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimageindex(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimageindex", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimageinterlacescheme(HPHP::Object const&, int)
_ZN4HPHP31f_magicksetimageinterlaceschemeERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
interlace_type => rsi
*/

bool fh_magicksetimageinterlacescheme(Value* mgck_wnd, int interlace_type) asm("_ZN4HPHP31f_magicksetimageinterlaceschemeERKNS_6ObjectEi");

TypedValue * fg1_magicksetimageinterlacescheme(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimageinterlacescheme(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimageinterlacescheme((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimageinterlacescheme(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimageinterlacescheme((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimageinterlacescheme(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimageinterlacescheme", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimageiterations(HPHP::Object const&, double)
_ZN4HPHP26f_magicksetimageiterationsERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
iterations => xmm0
*/

bool fh_magicksetimageiterations(Value* mgck_wnd, double iterations) asm("_ZN4HPHP26f_magicksetimageiterationsERKNS_6ObjectEd");

TypedValue * fg1_magicksetimageiterations(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimageiterations(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimageiterations((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimageiterations(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimageiterations((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimageiterations(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimageiterations", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagemattecolor(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP26f_magicksetimagemattecolorERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
matte_pxl_wnd => rsi
*/

bool fh_magicksetimagemattecolor(Value* mgck_wnd, Value* matte_pxl_wnd) asm("_ZN4HPHP26f_magicksetimagemattecolorERKNS_6ObjectES2_");

TypedValue * fg1_magicksetimagemattecolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagemattecolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagemattecolor((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagemattecolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagemattecolor((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagemattecolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagemattecolor", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimageoption(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP22f_magicksetimageoptionERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
mgck_wnd => rdi
format => rsi
key => rdx
value => rcx
*/

bool fh_magicksetimageoption(Value* mgck_wnd, Value* format, Value* key, Value* value) asm("_ZN4HPHP22f_magicksetimageoptionERKNS_6ObjectERKNS_6StringES5_S5_");

TypedValue * fg1_magicksetimageoption(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimageoption(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimageoption((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimageoption(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimageoption((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimageoption(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimageoption", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagepixels(HPHP::Object const&, int, int, double, double, HPHP::String const&, int, HPHP::Array const&)
_ZN4HPHP22f_magicksetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEiRKNS_5ArrayE

(return value) => rax
mgck_wnd => rdi
x_offset => rsi
y_offset => rdx
columns => xmm0
rows => xmm1
smap => rcx
storage_type => r8
pixel_array => r9
*/

bool fh_magicksetimagepixels(Value* mgck_wnd, int x_offset, int y_offset, double columns, double rows, Value* smap, int storage_type, Value* pixel_array) asm("_ZN4HPHP22f_magicksetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEiRKNS_5ArrayE");

TypedValue * fg1_magicksetimagepixels(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagepixels(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-7)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-7);
  }
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if (!IS_STRING_TYPE((args-5)->m_type)) {
    tvCastToStringInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagepixels((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (Value*)(args-5), (int)(args[-6].m_data.num), (Value*)(args-7))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagepixels(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 8LL) {
      if ((args-7)->m_type == KindOfArray && (args-6)->m_type == KindOfInt64 && IS_STRING_TYPE((args-5)->m_type) && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagepixels((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (Value*)(args-5), (int)(args[-6].m_data.num), (Value*)(args-7))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagepixels(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagepixels", count, 8, 8, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 8);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimageprofile(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP23f_magicksetimageprofileERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
mgck_wnd => rdi
name => rsi
profile => rdx
*/

bool fh_magicksetimageprofile(Value* mgck_wnd, Value* name, Value* profile) asm("_ZN4HPHP23f_magicksetimageprofileERKNS_6ObjectERKNS_6StringES5_");

TypedValue * fg1_magicksetimageprofile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimageprofile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimageprofile((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimageprofile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimageprofile((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimageprofile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimageprofile", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimageredprimary(HPHP::Object const&, double, double)
_ZN4HPHP26f_magicksetimageredprimaryERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x => xmm0
y => xmm1
*/

bool fh_magicksetimageredprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP26f_magicksetimageredprimaryERKNS_6ObjectEdd");

TypedValue * fg1_magicksetimageredprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimageredprimary(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimageredprimary((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimageredprimary(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimageredprimary((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimageredprimary(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimageredprimary", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagerenderingintent(HPHP::Object const&, int)
_ZN4HPHP31f_magicksetimagerenderingintentERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
rendering_intent => rsi
*/

bool fh_magicksetimagerenderingintent(Value* mgck_wnd, int rendering_intent) asm("_ZN4HPHP31f_magicksetimagerenderingintentERKNS_6ObjectEi");

TypedValue * fg1_magicksetimagerenderingintent(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagerenderingintent(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagerenderingintent((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagerenderingintent(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagerenderingintent((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagerenderingintent(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagerenderingintent", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimageresolution(HPHP::Object const&, double, double)
_ZN4HPHP26f_magicksetimageresolutionERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x_resolution => xmm0
y_resolution => xmm1
*/

bool fh_magicksetimageresolution(Value* mgck_wnd, double x_resolution, double y_resolution) asm("_ZN4HPHP26f_magicksetimageresolutionERKNS_6ObjectEdd");

TypedValue * fg1_magicksetimageresolution(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimageresolution(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimageresolution((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimageresolution(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimageresolution((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimageresolution(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimageresolution", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagescene(HPHP::Object const&, double)
_ZN4HPHP21f_magicksetimagesceneERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
scene => xmm0
*/

bool fh_magicksetimagescene(Value* mgck_wnd, double scene) asm("_ZN4HPHP21f_magicksetimagesceneERKNS_6ObjectEd");

TypedValue * fg1_magicksetimagescene(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagescene(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagescene((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagescene(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagescene((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagescene(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagescene", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagetype(HPHP::Object const&, int)
_ZN4HPHP20f_magicksetimagetypeERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
image_type => rsi
*/

bool fh_magicksetimagetype(Value* mgck_wnd, int image_type) asm("_ZN4HPHP20f_magicksetimagetypeERKNS_6ObjectEi");

TypedValue * fg1_magicksetimagetype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagetype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagetype((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagetype(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagetype((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagetype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagetype", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimageunits(HPHP::Object const&, int)
_ZN4HPHP21f_magicksetimageunitsERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
resolution_type => rsi
*/

bool fh_magicksetimageunits(Value* mgck_wnd, int resolution_type) asm("_ZN4HPHP21f_magicksetimageunitsERKNS_6ObjectEi");

TypedValue * fg1_magicksetimageunits(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimageunits(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimageunits((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimageunits(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimageunits((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimageunits(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimageunits", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagevirtualpixelmethod(HPHP::Object const&, int)
_ZN4HPHP34f_magicksetimagevirtualpixelmethodERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
virtual_pixel_method => rsi
*/

bool fh_magicksetimagevirtualpixelmethod(Value* mgck_wnd, int virtual_pixel_method) asm("_ZN4HPHP34f_magicksetimagevirtualpixelmethodERKNS_6ObjectEi");

TypedValue * fg1_magicksetimagevirtualpixelmethod(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagevirtualpixelmethod(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagevirtualpixelmethod((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagevirtualpixelmethod(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagevirtualpixelmethod((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagevirtualpixelmethod(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagevirtualpixelmethod", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetimagewhitepoint(HPHP::Object const&, double, double)
_ZN4HPHP26f_magicksetimagewhitepointERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x => xmm0
y => xmm1
*/

bool fh_magicksetimagewhitepoint(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP26f_magicksetimagewhitepointERKNS_6ObjectEdd");

TypedValue * fg1_magicksetimagewhitepoint(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetimagewhitepoint(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetimagewhitepoint((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetimagewhitepoint(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetimagewhitepoint((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetimagewhitepoint(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetimagewhitepoint", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetinterlacescheme(HPHP::Object const&, int)
_ZN4HPHP26f_magicksetinterlaceschemeERKNS_6ObjectEi

(return value) => rax
mgck_wnd => rdi
interlace_type => rsi
*/

bool fh_magicksetinterlacescheme(Value* mgck_wnd, int interlace_type) asm("_ZN4HPHP26f_magicksetinterlaceschemeERKNS_6ObjectEi");

TypedValue * fg1_magicksetinterlacescheme(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetinterlacescheme(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetinterlacescheme((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetinterlacescheme(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetinterlacescheme((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetinterlacescheme(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetinterlacescheme", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_magicksetlastiterator(HPHP::Object const&)
_ZN4HPHP23f_magicksetlastiteratorERKNS_6ObjectE

mgck_wnd => rdi
*/

void fh_magicksetlastiterator(Value* mgck_wnd) asm("_ZN4HPHP23f_magicksetlastiteratorERKNS_6ObjectE");

TypedValue * fg1_magicksetlastiterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetlastiterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_magicksetlastiterator((Value*)(args-0));
  return rv;
}

TypedValue* fg_magicksetlastiterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_magicksetlastiterator((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetlastiterator(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetlastiterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetpassphrase(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21f_magicksetpassphraseERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
passphrase => rsi
*/

bool fh_magicksetpassphrase(Value* mgck_wnd, Value* passphrase) asm("_ZN4HPHP21f_magicksetpassphraseERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magicksetpassphrase(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetpassphrase(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetpassphrase((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetpassphrase(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetpassphrase((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetpassphrase(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetpassphrase", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetresolution(HPHP::Object const&, double, double)
_ZN4HPHP21f_magicksetresolutionERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
x_resolution => xmm0
y_resolution => xmm1
*/

bool fh_magicksetresolution(Value* mgck_wnd, double x_resolution, double y_resolution) asm("_ZN4HPHP21f_magicksetresolutionERKNS_6ObjectEdd");

TypedValue * fg1_magicksetresolution(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetresolution(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetresolution((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetresolution(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetresolution((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetresolution(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetresolution", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetsamplingfactors(HPHP::Object const&, double, HPHP::Array const&)
_ZN4HPHP26f_magicksetsamplingfactorsERKNS_6ObjectEdRKNS_5ArrayE

(return value) => rax
mgck_wnd => rdi
number_factors => xmm0
sampling_factors => rsi
*/

bool fh_magicksetsamplingfactors(Value* mgck_wnd, double number_factors, Value* sampling_factors) asm("_ZN4HPHP26f_magicksetsamplingfactorsERKNS_6ObjectEdRKNS_5ArrayE");

TypedValue * fg1_magicksetsamplingfactors(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetsamplingfactors(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetsamplingfactors((Value*)(args-0), (args[-1].m_data.dbl), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetsamplingfactors(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfArray && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetsamplingfactors((Value*)(args-0), (args[-1].m_data.dbl), (Value*)(args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetsamplingfactors(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetsamplingfactors", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetsize(HPHP::Object const&, int, int)
_ZN4HPHP15f_magicksetsizeERKNS_6ObjectEii

(return value) => rax
mgck_wnd => rdi
columns => rsi
rows => rdx
*/

bool fh_magicksetsize(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP15f_magicksetsizeERKNS_6ObjectEii");

TypedValue * fg1_magicksetsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetsize((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetsize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetsize((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetsize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetsize", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksetwandsize(HPHP::Object const&, int, int)
_ZN4HPHP19f_magicksetwandsizeERKNS_6ObjectEii

(return value) => rax
mgck_wnd => rdi
columns => rsi
rows => rdx
*/

bool fh_magicksetwandsize(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP19f_magicksetwandsizeERKNS_6ObjectEii");

TypedValue * fg1_magicksetwandsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksetwandsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksetwandsize((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksetwandsize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksetwandsize((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksetwandsize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksetwandsize", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksharpenimage(HPHP::Object const&, double, double, int)
_ZN4HPHP20f_magicksharpenimageERKNS_6ObjectEddi

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
channel_type => rsi
*/

bool fh_magicksharpenimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP20f_magicksharpenimageERKNS_6ObjectEddi");

TypedValue * fg1_magicksharpenimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksharpenimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksharpenimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksharpenimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksharpenimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksharpenimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksharpenimage", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickshaveimage(HPHP::Object const&, int, int)
_ZN4HPHP18f_magickshaveimageERKNS_6ObjectEii

(return value) => rax
mgck_wnd => rdi
columns => rsi
rows => rdx
*/

bool fh_magickshaveimage(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP18f_magickshaveimageERKNS_6ObjectEii");

TypedValue * fg1_magickshaveimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickshaveimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickshaveimage((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickshaveimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickshaveimage((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickshaveimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickshaveimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickshearimage(HPHP::Object const&, HPHP::Object const&, double, double)
_ZN4HPHP18f_magickshearimageERKNS_6ObjectES2_dd

(return value) => rax
mgck_wnd => rdi
background => rsi
x_shear => xmm0
y_shear => xmm1
*/

bool fh_magickshearimage(Value* mgck_wnd, Value* background, double x_shear, double y_shear) asm("_ZN4HPHP18f_magickshearimageERKNS_6ObjectES2_dd");

TypedValue * fg1_magickshearimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickshearimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickshearimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickshearimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickshearimage((Value*)(args-0), (Value*)(args-1), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickshearimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickshearimage", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicksolarizeimage(HPHP::Object const&, double)
_ZN4HPHP21f_magicksolarizeimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
threshold => xmm0
*/

bool fh_magicksolarizeimage(Value* mgck_wnd, double threshold) asm("_ZN4HPHP21f_magicksolarizeimageERKNS_6ObjectEd");

TypedValue * fg1_magicksolarizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksolarizeimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicksolarizeimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicksolarizeimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicksolarizeimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksolarizeimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksolarizeimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickspliceimage(HPHP::Object const&, double, double, int, int)
_ZN4HPHP19f_magickspliceimageERKNS_6ObjectEddii

(return value) => rax
mgck_wnd => rdi
width => xmm0
height => xmm1
x => rsi
y => rdx
*/

bool fh_magickspliceimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP19f_magickspliceimageERKNS_6ObjectEddii");

TypedValue * fg1_magickspliceimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickspliceimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickspliceimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickspliceimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickspliceimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickspliceimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickspliceimage", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickspreadimage(HPHP::Object const&, double)
_ZN4HPHP19f_magickspreadimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
radius => xmm0
*/

bool fh_magickspreadimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP19f_magickspreadimageERKNS_6ObjectEd");

TypedValue * fg1_magickspreadimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickspreadimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickspreadimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickspreadimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickspreadimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickspreadimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickspreadimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magicksteganoimage(HPHP::Object const&, HPHP::Object const&, int)
_ZN4HPHP20f_magicksteganoimageERKNS_6ObjectES2_i

(return value) => rax
_rv => rdi
mgck_wnd => rsi
watermark_wand => rdx
offset => rcx
*/

Value* fh_magicksteganoimage(Value* _rv, Value* mgck_wnd, Value* watermark_wand, int offset) asm("_ZN4HPHP20f_magicksteganoimageERKNS_6ObjectES2_i");

TypedValue * fg1_magicksteganoimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicksteganoimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magicksteganoimage((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magicksteganoimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magicksteganoimage((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicksteganoimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicksteganoimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickstereoimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP19f_magickstereoimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
offset_wand => rsi
*/

bool fh_magickstereoimage(Value* mgck_wnd, Value* offset_wand) asm("_ZN4HPHP19f_magickstereoimageERKNS_6ObjectES2_");

TypedValue * fg1_magickstereoimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickstereoimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickstereoimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickstereoimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickstereoimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickstereoimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickstereoimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickstripimage(HPHP::Object const&)
_ZN4HPHP18f_magickstripimageERKNS_6ObjectE

(return value) => rax
mgck_wnd => rdi
*/

bool fh_magickstripimage(Value* mgck_wnd) asm("_ZN4HPHP18f_magickstripimageERKNS_6ObjectE");

TypedValue * fg1_magickstripimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickstripimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_magickstripimage((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickstripimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickstripimage((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickstripimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickstripimage", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickswirlimage(HPHP::Object const&, double)
_ZN4HPHP18f_magickswirlimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
degrees => xmm0
*/

bool fh_magickswirlimage(Value* mgck_wnd, double degrees) asm("_ZN4HPHP18f_magickswirlimageERKNS_6ObjectEd");

TypedValue * fg1_magickswirlimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickswirlimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickswirlimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickswirlimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickswirlimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickswirlimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickswirlimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magicktextureimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP20f_magicktextureimageERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
mgck_wnd => rsi
texture_wand => rdx
*/

Value* fh_magicktextureimage(Value* _rv, Value* mgck_wnd, Value* texture_wand) asm("_ZN4HPHP20f_magicktextureimageERKNS_6ObjectES2_");

TypedValue * fg1_magicktextureimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicktextureimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magicktextureimage((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magicktextureimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magicktextureimage((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicktextureimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicktextureimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickthresholdimage(HPHP::Object const&, double, int)
_ZN4HPHP22f_magickthresholdimageERKNS_6ObjectEdi

(return value) => rax
mgck_wnd => rdi
threshold => xmm0
channel_type => rsi
*/

bool fh_magickthresholdimage(Value* mgck_wnd, double threshold, int channel_type) asm("_ZN4HPHP22f_magickthresholdimageERKNS_6ObjectEdi");

TypedValue * fg1_magickthresholdimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickthresholdimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickthresholdimage((Value*)(args-0), (args[-1].m_data.dbl), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickthresholdimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickthresholdimage((Value*)(args-0), (args[-1].m_data.dbl), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickthresholdimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickthresholdimage", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicktintimage(HPHP::Object const&, int, HPHP::Object const&)
_ZN4HPHP17f_magicktintimageERKNS_6ObjectEiS2_

(return value) => rax
mgck_wnd => rdi
tint_pxl_wnd => rsi
opacity_pxl_wnd => rdx
*/

bool fh_magicktintimage(Value* mgck_wnd, int tint_pxl_wnd, Value* opacity_pxl_wnd) asm("_ZN4HPHP17f_magicktintimageERKNS_6ObjectEiS2_");

TypedValue * fg1_magicktintimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicktintimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicktintimage((Value*)(args-0), (int)(args[-1].m_data.num), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicktintimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfObject && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicktintimage((Value*)(args-0), (int)(args[-1].m_data.num), (Value*)(args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicktintimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicktintimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_magicktransformimage(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP22f_magicktransformimageERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
_rv => rdi
mgck_wnd => rsi
crop => rdx
geometry => rcx
*/

Value* fh_magicktransformimage(Value* _rv, Value* mgck_wnd, Value* crop, Value* geometry) asm("_ZN4HPHP22f_magicktransformimageERKNS_6ObjectERKNS_6StringES5_");

TypedValue * fg1_magicktransformimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicktransformimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_magicktransformimage((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_magicktransformimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_magicktransformimage((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicktransformimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicktransformimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magicktrimimage(HPHP::Object const&, double)
_ZN4HPHP17f_magicktrimimageERKNS_6ObjectEd

(return value) => rax
mgck_wnd => rdi
fuzz => xmm0
*/

bool fh_magicktrimimage(Value* mgck_wnd, double fuzz) asm("_ZN4HPHP17f_magicktrimimageERKNS_6ObjectEd");

TypedValue * fg1_magicktrimimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magicktrimimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magicktrimimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magicktrimimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magicktrimimage((Value*)(args-0), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magicktrimimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magicktrimimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickunsharpmaskimage(HPHP::Object const&, double, double, double, double, int)
_ZN4HPHP24f_magickunsharpmaskimageERKNS_6ObjectEddddi

(return value) => rax
mgck_wnd => rdi
radius => xmm0
sigma => xmm1
amount => xmm2
threshold => xmm3
channel_type => rsi
*/

bool fh_magickunsharpmaskimage(Value* mgck_wnd, double radius, double sigma, double amount, double threshold, int channel_type) asm("_ZN4HPHP24f_magickunsharpmaskimageERKNS_6ObjectEddddi");

TypedValue * fg1_magickunsharpmaskimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickunsharpmaskimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    break;
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickunsharpmaskimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickunsharpmaskimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 5LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfInt64) && (args-4)->m_type == KindOfDouble && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickunsharpmaskimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickunsharpmaskimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickunsharpmaskimage", count, 5, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickwaveimage(HPHP::Object const&, double, double)
_ZN4HPHP17f_magickwaveimageERKNS_6ObjectEdd

(return value) => rax
mgck_wnd => rdi
amplitude => xmm0
wave_length => xmm1
*/

bool fh_magickwaveimage(Value* mgck_wnd, double amplitude, double wave_length) asm("_ZN4HPHP17f_magickwaveimageERKNS_6ObjectEdd");

TypedValue * fg1_magickwaveimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickwaveimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickwaveimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickwaveimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickwaveimage((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickwaveimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickwaveimage", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickwhitethresholdimage(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP27f_magickwhitethresholdimageERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
threshold_pxl_wnd => rsi
*/

bool fh_magickwhitethresholdimage(Value* mgck_wnd, Value* threshold_pxl_wnd) asm("_ZN4HPHP27f_magickwhitethresholdimageERKNS_6ObjectES2_");

TypedValue * fg1_magickwhitethresholdimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickwhitethresholdimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickwhitethresholdimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickwhitethresholdimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickwhitethresholdimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickwhitethresholdimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickwhitethresholdimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickwriteimage(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP18f_magickwriteimageERKNS_6ObjectERKNS_6StringE

(return value) => rax
mgck_wnd => rdi
filename => rsi
*/

bool fh_magickwriteimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP18f_magickwriteimageERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_magickwriteimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickwriteimage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickwriteimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickwriteimage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickwriteimage((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickwriteimage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickwriteimage", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickwriteimagefile(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP22f_magickwriteimagefileERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
handle => rsi
*/

bool fh_magickwriteimagefile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP22f_magickwriteimagefileERKNS_6ObjectES2_");

TypedValue * fg1_magickwriteimagefile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickwriteimagefile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickwriteimagefile((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickwriteimagefile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickwriteimagefile((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickwriteimagefile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickwriteimagefile", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickwriteimages(HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP19f_magickwriteimagesERKNS_6ObjectERKNS_6StringEb

(return value) => rax
mgck_wnd => rdi
filename => rsi
join_images => rdx
*/

bool fh_magickwriteimages(Value* mgck_wnd, Value* filename, bool join_images) asm("_ZN4HPHP19f_magickwriteimagesERKNS_6ObjectERKNS_6StringEb");

TypedValue * fg1_magickwriteimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickwriteimages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickwriteimages((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&empty_string), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickwriteimages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickwriteimages((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&empty_string), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickwriteimages(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickwriteimages", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_magickwriteimagesfile(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP23f_magickwriteimagesfileERKNS_6ObjectES2_

(return value) => rax
mgck_wnd => rdi
handle => rsi
*/

bool fh_magickwriteimagesfile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP23f_magickwriteimagesfileERKNS_6ObjectES2_");

TypedValue * fg1_magickwriteimagesfile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_magickwriteimagesfile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_magickwriteimagesfile((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_magickwriteimagesfile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_magickwriteimagesfile((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_magickwriteimagesfile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("magickwriteimagesfile", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetalpha(HPHP::Object const&)
_ZN4HPHP15f_pixelgetalphaERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetalpha(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetalphaERKNS_6ObjectE");

TypedValue * fg1_pixelgetalpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetalpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetalpha((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetalpha(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetalpha((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetalpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetalpha", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetalphaquantum(HPHP::Object const&)
_ZN4HPHP22f_pixelgetalphaquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetalphaquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetalphaquantumERKNS_6ObjectE");

TypedValue * fg1_pixelgetalphaquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetalphaquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetalphaquantum((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetalphaquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetalphaquantum((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetalphaquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetalphaquantum", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetblack(HPHP::Object const&)
_ZN4HPHP15f_pixelgetblackERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetblack(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetblackERKNS_6ObjectE");

TypedValue * fg1_pixelgetblack(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetblack(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetblack((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetblack(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetblack((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetblack(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetblack", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetblackquantum(HPHP::Object const&)
_ZN4HPHP22f_pixelgetblackquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetblackquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetblackquantumERKNS_6ObjectE");

TypedValue * fg1_pixelgetblackquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetblackquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetblackquantum((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetblackquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetblackquantum((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetblackquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetblackquantum", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetblue(HPHP::Object const&)
_ZN4HPHP14f_pixelgetblueERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetblue(Value* pxl_wnd) asm("_ZN4HPHP14f_pixelgetblueERKNS_6ObjectE");

TypedValue * fg1_pixelgetblue(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetblue(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetblue((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetblue(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetblue((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetblue(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetblue", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetbluequantum(HPHP::Object const&)
_ZN4HPHP21f_pixelgetbluequantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetbluequantum(Value* pxl_wnd) asm("_ZN4HPHP21f_pixelgetbluequantumERKNS_6ObjectE");

TypedValue * fg1_pixelgetbluequantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetbluequantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetbluequantum((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetbluequantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetbluequantum((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetbluequantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetbluequantum", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_pixelgetcolorasstring(HPHP::Object const&)
_ZN4HPHP23f_pixelgetcolorasstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_wnd => rsi
*/

Value* fh_pixelgetcolorasstring(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetcolorasstringERKNS_6ObjectE");

TypedValue * fg1_pixelgetcolorasstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetcolorasstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_pixelgetcolorasstring((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_pixelgetcolorasstring(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_pixelgetcolorasstring((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetcolorasstring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetcolorasstring", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetcolorcount(HPHP::Object const&)
_ZN4HPHP20f_pixelgetcolorcountERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetcolorcount(Value* pxl_wnd) asm("_ZN4HPHP20f_pixelgetcolorcountERKNS_6ObjectE");

TypedValue * fg1_pixelgetcolorcount(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetcolorcount(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetcolorcount((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetcolorcount(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetcolorcount((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetcolorcount(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetcolorcount", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetcyan(HPHP::Object const&)
_ZN4HPHP14f_pixelgetcyanERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetcyan(Value* pxl_wnd) asm("_ZN4HPHP14f_pixelgetcyanERKNS_6ObjectE");

TypedValue * fg1_pixelgetcyan(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetcyan(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetcyan((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetcyan(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetcyan((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetcyan(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetcyan", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetcyanquantum(HPHP::Object const&)
_ZN4HPHP21f_pixelgetcyanquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetcyanquantum(Value* pxl_wnd) asm("_ZN4HPHP21f_pixelgetcyanquantumERKNS_6ObjectE");

TypedValue * fg1_pixelgetcyanquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetcyanquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetcyanquantum((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetcyanquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetcyanquantum((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetcyanquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetcyanquantum", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_pixelgetexception(HPHP::Object const&)
_ZN4HPHP19f_pixelgetexceptionERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_wnd => rsi
*/

Value* fh_pixelgetexception(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP19f_pixelgetexceptionERKNS_6ObjectE");

TypedValue * fg1_pixelgetexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_pixelgetexception((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_pixelgetexception(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_pixelgetexception((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetexception(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetexception", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_pixelgetexceptionstring(HPHP::Object const&)
_ZN4HPHP25f_pixelgetexceptionstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_wnd => rsi
*/

Value* fh_pixelgetexceptionstring(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP25f_pixelgetexceptionstringERKNS_6ObjectE");

TypedValue * fg1_pixelgetexceptionstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetexceptionstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_pixelgetexceptionstring((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_pixelgetexceptionstring(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_pixelgetexceptionstring((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetexceptionstring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetexceptionstring", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_pixelgetexceptiontype(HPHP::Object const&)
_ZN4HPHP23f_pixelgetexceptiontypeERKNS_6ObjectE

(return value) => rax
pxl_wnd => rdi
*/

long fh_pixelgetexceptiontype(Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetexceptiontypeERKNS_6ObjectE");

TypedValue * fg1_pixelgetexceptiontype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetexceptiontype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_pixelgetexceptiontype((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetexceptiontype(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_pixelgetexceptiontype((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetexceptiontype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetexceptiontype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetgreen(HPHP::Object const&)
_ZN4HPHP15f_pixelgetgreenERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetgreen(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetgreenERKNS_6ObjectE");

TypedValue * fg1_pixelgetgreen(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetgreen(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetgreen((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetgreen(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetgreen((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetgreen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetgreen", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetgreenquantum(HPHP::Object const&)
_ZN4HPHP22f_pixelgetgreenquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetgreenquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetgreenquantumERKNS_6ObjectE");

TypedValue * fg1_pixelgetgreenquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetgreenquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetgreenquantum((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetgreenquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetgreenquantum((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetgreenquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetgreenquantum", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetindex(HPHP::Object const&)
_ZN4HPHP15f_pixelgetindexERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetindex(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetindexERKNS_6ObjectE");

TypedValue * fg1_pixelgetindex(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetindex(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetindex((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetindex(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetindex((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetindex(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetindex", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetmagenta(HPHP::Object const&)
_ZN4HPHP17f_pixelgetmagentaERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetmagenta(Value* pxl_wnd) asm("_ZN4HPHP17f_pixelgetmagentaERKNS_6ObjectE");

TypedValue * fg1_pixelgetmagenta(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetmagenta(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetmagenta((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetmagenta(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetmagenta((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetmagenta(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetmagenta", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetmagentaquantum(HPHP::Object const&)
_ZN4HPHP24f_pixelgetmagentaquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetmagentaquantum(Value* pxl_wnd) asm("_ZN4HPHP24f_pixelgetmagentaquantumERKNS_6ObjectE");

TypedValue * fg1_pixelgetmagentaquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetmagentaquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetmagentaquantum((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetmagentaquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetmagentaquantum((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetmagentaquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetmagentaquantum", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetopacity(HPHP::Object const&)
_ZN4HPHP17f_pixelgetopacityERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetopacity(Value* pxl_wnd) asm("_ZN4HPHP17f_pixelgetopacityERKNS_6ObjectE");

TypedValue * fg1_pixelgetopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetopacity((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetopacity(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetopacity((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetopacity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetopacity", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetopacityquantum(HPHP::Object const&)
_ZN4HPHP24f_pixelgetopacityquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetopacityquantum(Value* pxl_wnd) asm("_ZN4HPHP24f_pixelgetopacityquantumERKNS_6ObjectE");

TypedValue * fg1_pixelgetopacityquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetopacityquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetopacityquantum((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetopacityquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetopacityquantum((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetopacityquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetopacityquantum", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_pixelgetquantumcolor(HPHP::Object const&)
_ZN4HPHP22f_pixelgetquantumcolorERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_wnd => rsi
*/

Value* fh_pixelgetquantumcolor(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetquantumcolorERKNS_6ObjectE");

TypedValue * fg1_pixelgetquantumcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetquantumcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_pixelgetquantumcolor((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_pixelgetquantumcolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_pixelgetquantumcolor((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetquantumcolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetquantumcolor", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetred(HPHP::Object const&)
_ZN4HPHP13f_pixelgetredERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetred(Value* pxl_wnd) asm("_ZN4HPHP13f_pixelgetredERKNS_6ObjectE");

TypedValue * fg1_pixelgetred(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetred(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetred((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetred(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetred((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetred(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetred", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetredquantum(HPHP::Object const&)
_ZN4HPHP20f_pixelgetredquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetredquantum(Value* pxl_wnd) asm("_ZN4HPHP20f_pixelgetredquantumERKNS_6ObjectE");

TypedValue * fg1_pixelgetredquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetredquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetredquantum((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetredquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetredquantum((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetredquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetredquantum", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetyellow(HPHP::Object const&)
_ZN4HPHP16f_pixelgetyellowERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetyellow(Value* pxl_wnd) asm("_ZN4HPHP16f_pixelgetyellowERKNS_6ObjectE");

TypedValue * fg1_pixelgetyellow(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetyellow(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetyellow((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetyellow(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetyellow((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetyellow(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetyellow", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
double HPHP::f_pixelgetyellowquantum(HPHP::Object const&)
_ZN4HPHP23f_pixelgetyellowquantumERKNS_6ObjectE

(return value) => xmm0
pxl_wnd => rdi
*/

double fh_pixelgetyellowquantum(Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetyellowquantumERKNS_6ObjectE");

TypedValue * fg1_pixelgetyellowquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetyellowquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfDouble;
  tvCastToObjectInPlace(args-0);
  rv->m_data.dbl = fh_pixelgetyellowquantum((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetyellowquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfDouble;
        rv.m_data.dbl = fh_pixelgetyellowquantum((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetyellowquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetyellowquantum", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetalpha(HPHP::Object const&, double)
_ZN4HPHP15f_pixelsetalphaERKNS_6ObjectEd

pxl_wnd => rdi
alpha => xmm0
*/

void fh_pixelsetalpha(Value* pxl_wnd, double alpha) asm("_ZN4HPHP15f_pixelsetalphaERKNS_6ObjectEd");

TypedValue * fg1_pixelsetalpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetalpha(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetalpha((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetalpha(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetalpha((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetalpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetalpha", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetalphaquantum(HPHP::Object const&, double)
_ZN4HPHP22f_pixelsetalphaquantumERKNS_6ObjectEd

pxl_wnd => rdi
alpha => xmm0
*/

void fh_pixelsetalphaquantum(Value* pxl_wnd, double alpha) asm("_ZN4HPHP22f_pixelsetalphaquantumERKNS_6ObjectEd");

TypedValue * fg1_pixelsetalphaquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetalphaquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetalphaquantum((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetalphaquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetalphaquantum((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetalphaquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetalphaquantum", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetblack(HPHP::Object const&, double)
_ZN4HPHP15f_pixelsetblackERKNS_6ObjectEd

pxl_wnd => rdi
black => xmm0
*/

void fh_pixelsetblack(Value* pxl_wnd, double black) asm("_ZN4HPHP15f_pixelsetblackERKNS_6ObjectEd");

TypedValue * fg1_pixelsetblack(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetblack(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetblack((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetblack(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetblack((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetblack(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetblack", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetblackquantum(HPHP::Object const&, double)
_ZN4HPHP22f_pixelsetblackquantumERKNS_6ObjectEd

pxl_wnd => rdi
black => xmm0
*/

void fh_pixelsetblackquantum(Value* pxl_wnd, double black) asm("_ZN4HPHP22f_pixelsetblackquantumERKNS_6ObjectEd");

TypedValue * fg1_pixelsetblackquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetblackquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetblackquantum((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetblackquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetblackquantum((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetblackquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetblackquantum", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetblue(HPHP::Object const&, double)
_ZN4HPHP14f_pixelsetblueERKNS_6ObjectEd

pxl_wnd => rdi
blue => xmm0
*/

void fh_pixelsetblue(Value* pxl_wnd, double blue) asm("_ZN4HPHP14f_pixelsetblueERKNS_6ObjectEd");

TypedValue * fg1_pixelsetblue(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetblue(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetblue((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetblue(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetblue((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetblue(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetblue", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetbluequantum(HPHP::Object const&, double)
_ZN4HPHP21f_pixelsetbluequantumERKNS_6ObjectEd

pxl_wnd => rdi
blue => xmm0
*/

void fh_pixelsetbluequantum(Value* pxl_wnd, double blue) asm("_ZN4HPHP21f_pixelsetbluequantumERKNS_6ObjectEd");

TypedValue * fg1_pixelsetbluequantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetbluequantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetbluequantum((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetbluequantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetbluequantum((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetbluequantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetbluequantum", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetcolor(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP15f_pixelsetcolorERKNS_6ObjectERKNS_6StringE

pxl_wnd => rdi
imagemagick_col_str => rsi
*/

void fh_pixelsetcolor(Value* pxl_wnd, Value* imagemagick_col_str) asm("_ZN4HPHP15f_pixelsetcolorERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_pixelsetcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetcolor((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_pixelsetcolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetcolor((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetcolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetcolor", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetcolorcount(HPHP::Object const&, int)
_ZN4HPHP20f_pixelsetcolorcountERKNS_6ObjectEi

pxl_wnd => rdi
count => rsi
*/

void fh_pixelsetcolorcount(Value* pxl_wnd, int count) asm("_ZN4HPHP20f_pixelsetcolorcountERKNS_6ObjectEi");

TypedValue * fg1_pixelsetcolorcount(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetcolorcount(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetcolorcount((Value*)(args-0), (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_pixelsetcolorcount(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetcolorcount((Value*)(args-0), (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetcolorcount(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetcolorcount", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetcyan(HPHP::Object const&, double)
_ZN4HPHP14f_pixelsetcyanERKNS_6ObjectEd

pxl_wnd => rdi
cyan => xmm0
*/

void fh_pixelsetcyan(Value* pxl_wnd, double cyan) asm("_ZN4HPHP14f_pixelsetcyanERKNS_6ObjectEd");

TypedValue * fg1_pixelsetcyan(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetcyan(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetcyan((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetcyan(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetcyan((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetcyan(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetcyan", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetcyanquantum(HPHP::Object const&, double)
_ZN4HPHP21f_pixelsetcyanquantumERKNS_6ObjectEd

pxl_wnd => rdi
cyan => xmm0
*/

void fh_pixelsetcyanquantum(Value* pxl_wnd, double cyan) asm("_ZN4HPHP21f_pixelsetcyanquantumERKNS_6ObjectEd");

TypedValue * fg1_pixelsetcyanquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetcyanquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetcyanquantum((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetcyanquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetcyanquantum((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetcyanquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetcyanquantum", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetgreen(HPHP::Object const&, double)
_ZN4HPHP15f_pixelsetgreenERKNS_6ObjectEd

pxl_wnd => rdi
green => xmm0
*/

void fh_pixelsetgreen(Value* pxl_wnd, double green) asm("_ZN4HPHP15f_pixelsetgreenERKNS_6ObjectEd");

TypedValue * fg1_pixelsetgreen(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetgreen(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetgreen((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetgreen(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetgreen((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetgreen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetgreen", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetgreenquantum(HPHP::Object const&, double)
_ZN4HPHP22f_pixelsetgreenquantumERKNS_6ObjectEd

pxl_wnd => rdi
green => xmm0
*/

void fh_pixelsetgreenquantum(Value* pxl_wnd, double green) asm("_ZN4HPHP22f_pixelsetgreenquantumERKNS_6ObjectEd");

TypedValue * fg1_pixelsetgreenquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetgreenquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetgreenquantum((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetgreenquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetgreenquantum((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetgreenquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetgreenquantum", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetindex(HPHP::Object const&, double)
_ZN4HPHP15f_pixelsetindexERKNS_6ObjectEd

pxl_wnd => rdi
index => xmm0
*/

void fh_pixelsetindex(Value* pxl_wnd, double index) asm("_ZN4HPHP15f_pixelsetindexERKNS_6ObjectEd");

TypedValue * fg1_pixelsetindex(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetindex(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetindex((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetindex(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetindex((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetindex(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetindex", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetmagenta(HPHP::Object const&, double)
_ZN4HPHP17f_pixelsetmagentaERKNS_6ObjectEd

pxl_wnd => rdi
magenta => xmm0
*/

void fh_pixelsetmagenta(Value* pxl_wnd, double magenta) asm("_ZN4HPHP17f_pixelsetmagentaERKNS_6ObjectEd");

TypedValue * fg1_pixelsetmagenta(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetmagenta(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetmagenta((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetmagenta(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetmagenta((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetmagenta(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetmagenta", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetmagentaquantum(HPHP::Object const&, double)
_ZN4HPHP24f_pixelsetmagentaquantumERKNS_6ObjectEd

pxl_wnd => rdi
magenta => xmm0
*/

void fh_pixelsetmagentaquantum(Value* pxl_wnd, double magenta) asm("_ZN4HPHP24f_pixelsetmagentaquantumERKNS_6ObjectEd");

TypedValue * fg1_pixelsetmagentaquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetmagentaquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetmagentaquantum((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetmagentaquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetmagentaquantum((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetmagentaquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetmagentaquantum", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetopacity(HPHP::Object const&, double)
_ZN4HPHP17f_pixelsetopacityERKNS_6ObjectEd

pxl_wnd => rdi
opacity => xmm0
*/

void fh_pixelsetopacity(Value* pxl_wnd, double opacity) asm("_ZN4HPHP17f_pixelsetopacityERKNS_6ObjectEd");

TypedValue * fg1_pixelsetopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetopacity(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetopacity((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetopacity(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetopacity((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetopacity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetopacity", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetopacityquantum(HPHP::Object const&, double)
_ZN4HPHP24f_pixelsetopacityquantumERKNS_6ObjectEd

pxl_wnd => rdi
opacity => xmm0
*/

void fh_pixelsetopacityquantum(Value* pxl_wnd, double opacity) asm("_ZN4HPHP24f_pixelsetopacityquantumERKNS_6ObjectEd");

TypedValue * fg1_pixelsetopacityquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetopacityquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetopacityquantum((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetopacityquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetopacityquantum((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetopacityquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetopacityquantum", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetquantumcolor(HPHP::Object const&, double, double, double, double)
_ZN4HPHP22f_pixelsetquantumcolorERKNS_6ObjectEdddd

pxl_wnd => rdi
red => xmm0
green => xmm1
blue => xmm2
opacity => xmm3
*/

void fh_pixelsetquantumcolor(Value* pxl_wnd, double red, double green, double blue, double opacity) asm("_ZN4HPHP22f_pixelsetquantumcolorERKNS_6ObjectEdddd");

TypedValue * fg1_pixelsetquantumcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetquantumcolor(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-4);
    }
  case 4:
    break;
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetquantumcolor((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
  return rv;
}

TypedValue* fg_pixelsetquantumcolor(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfDouble) && (args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetquantumcolor((Value*)(args-0), (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetquantumcolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetquantumcolor", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetred(HPHP::Object const&, double)
_ZN4HPHP13f_pixelsetredERKNS_6ObjectEd

pxl_wnd => rdi
red => xmm0
*/

void fh_pixelsetred(Value* pxl_wnd, double red) asm("_ZN4HPHP13f_pixelsetredERKNS_6ObjectEd");

TypedValue * fg1_pixelsetred(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetred(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetred((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetred(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetred((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetred(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetred", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetredquantum(HPHP::Object const&, double)
_ZN4HPHP20f_pixelsetredquantumERKNS_6ObjectEd

pxl_wnd => rdi
red => xmm0
*/

void fh_pixelsetredquantum(Value* pxl_wnd, double red) asm("_ZN4HPHP20f_pixelsetredquantumERKNS_6ObjectEd");

TypedValue * fg1_pixelsetredquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetredquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetredquantum((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetredquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetredquantum((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetredquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetredquantum", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetyellow(HPHP::Object const&, double)
_ZN4HPHP16f_pixelsetyellowERKNS_6ObjectEd

pxl_wnd => rdi
yellow => xmm0
*/

void fh_pixelsetyellow(Value* pxl_wnd, double yellow) asm("_ZN4HPHP16f_pixelsetyellowERKNS_6ObjectEd");

TypedValue * fg1_pixelsetyellow(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetyellow(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetyellow((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetyellow(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetyellow((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetyellow(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetyellow", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelsetyellowquantum(HPHP::Object const&, double)
_ZN4HPHP23f_pixelsetyellowquantumERKNS_6ObjectEd

pxl_wnd => rdi
yellow => xmm0
*/

void fh_pixelsetyellowquantum(Value* pxl_wnd, double yellow) asm("_ZN4HPHP23f_pixelsetyellowquantumERKNS_6ObjectEd");

TypedValue * fg1_pixelsetyellowquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetyellowquantum(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_pixelsetyellowquantum((Value*)(args-0), (args[-1].m_data.dbl));
  return rv;
}

TypedValue* fg_pixelsetyellowquantum(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelsetyellowquantum((Value*)(args-0), (args[-1].m_data.dbl));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetyellowquantum(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetyellowquantum", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_pixelgetiteratorexception(HPHP::Object const&)
_ZN4HPHP27f_pixelgetiteratorexceptionERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_iter => rsi
*/

Value* fh_pixelgetiteratorexception(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP27f_pixelgetiteratorexceptionERKNS_6ObjectE");

TypedValue * fg1_pixelgetiteratorexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetiteratorexception(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_pixelgetiteratorexception((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_pixelgetiteratorexception(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_pixelgetiteratorexception((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetiteratorexception(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetiteratorexception", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_pixelgetiteratorexceptionstring(HPHP::Object const&)
_ZN4HPHP33f_pixelgetiteratorexceptionstringERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_iter => rsi
*/

Value* fh_pixelgetiteratorexceptionstring(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP33f_pixelgetiteratorexceptionstringERKNS_6ObjectE");

TypedValue * fg1_pixelgetiteratorexceptionstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetiteratorexceptionstring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_pixelgetiteratorexceptionstring((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_pixelgetiteratorexceptionstring(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_pixelgetiteratorexceptionstring((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetiteratorexceptionstring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetiteratorexceptionstring", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_pixelgetiteratorexceptiontype(HPHP::Object const&)
_ZN4HPHP31f_pixelgetiteratorexceptiontypeERKNS_6ObjectE

(return value) => rax
pxl_iter => rdi
*/

long fh_pixelgetiteratorexceptiontype(Value* pxl_iter) asm("_ZN4HPHP31f_pixelgetiteratorexceptiontypeERKNS_6ObjectE");

TypedValue * fg1_pixelgetiteratorexceptiontype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetiteratorexceptiontype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_pixelgetiteratorexceptiontype((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelgetiteratorexceptiontype(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_pixelgetiteratorexceptiontype((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetiteratorexceptiontype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetiteratorexceptiontype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_pixelgetnextiteratorrow(HPHP::Object const&)
_ZN4HPHP25f_pixelgetnextiteratorrowERKNS_6ObjectE

(return value) => rax
_rv => rdi
pxl_iter => rsi
*/

Value* fh_pixelgetnextiteratorrow(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP25f_pixelgetnextiteratorrowERKNS_6ObjectE");

TypedValue * fg1_pixelgetnextiteratorrow(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelgetnextiteratorrow(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_pixelgetnextiteratorrow((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_pixelgetnextiteratorrow(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_pixelgetnextiteratorrow((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelgetnextiteratorrow(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelgetnextiteratorrow", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_pixelresetiterator(HPHP::Object const&)
_ZN4HPHP20f_pixelresetiteratorERKNS_6ObjectE

pxl_iter => rdi
*/

void fh_pixelresetiterator(Value* pxl_iter) asm("_ZN4HPHP20f_pixelresetiteratorERKNS_6ObjectE");

TypedValue * fg1_pixelresetiterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelresetiterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_pixelresetiterator((Value*)(args-0));
  return rv;
}

TypedValue* fg_pixelresetiterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_pixelresetiterator((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelresetiterator(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelresetiterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_pixelsetiteratorrow(HPHP::Object const&, int)
_ZN4HPHP21f_pixelsetiteratorrowERKNS_6ObjectEi

(return value) => rax
pxl_iter => rdi
row => rsi
*/

bool fh_pixelsetiteratorrow(Value* pxl_iter, int row) asm("_ZN4HPHP21f_pixelsetiteratorrowERKNS_6ObjectEi");

TypedValue * fg1_pixelsetiteratorrow(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsetiteratorrow(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_pixelsetiteratorrow((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_pixelsetiteratorrow(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_pixelsetiteratorrow((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsetiteratorrow(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsetiteratorrow", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_pixelsynciterator(HPHP::Object const&)
_ZN4HPHP19f_pixelsynciteratorERKNS_6ObjectE

(return value) => rax
pxl_iter => rdi
*/

bool fh_pixelsynciterator(Value* pxl_iter) asm("_ZN4HPHP19f_pixelsynciteratorERKNS_6ObjectE");

TypedValue * fg1_pixelsynciterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_pixelsynciterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_pixelsynciterator((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_pixelsynciterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_pixelsynciterator((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pixelsynciterator(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pixelsynciterator", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

