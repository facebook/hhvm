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

/*
bool HPHP::f_mail(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP6f_mailERKNS_6StringES2_S2_S2_S2_

(return value) => rax
to => rdi
subject => rsi
message => rdx
additional_headers => rcx
additional_parameters => r8
*/

bool fh_mail(Value* to, Value* subject, Value* message, Value* additional_headers, Value* additional_parameters) asm("_ZN4HPHP6f_mailERKNS_6StringES2_S2_S2_S2_");

TypedValue * fg1_mail(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mail(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_mail((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mail(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 5LL) {
      if ((count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mail((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mail(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mail", count, 3, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
long long HPHP::f_ezmlm_hash(HPHP::String const&)
_ZN4HPHP12f_ezmlm_hashERKNS_6StringE

(return value) => rax
addr => rdi
*/

long long fh_ezmlm_hash(Value* addr) asm("_ZN4HPHP12f_ezmlm_hashERKNS_6StringE");

TypedValue * fg1_ezmlm_hash(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ezmlm_hash(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (long long)fh_ezmlm_hash((Value*)(args-0));
  return rv;
}

TypedValue* fg_ezmlm_hash(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_ezmlm_hash((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ezmlm_hash(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ezmlm_hash", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Object HPHP::f_mailparse_msg_create()
_ZN4HPHP22f_mailparse_msg_createEv

(return value) => rax
_rv => rdi
*/

Value* fh_mailparse_msg_create(Value* _rv) asm("_ZN4HPHP22f_mailparse_msg_createEv");

TypedValue* fg_mailparse_msg_create(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfObject;
      fh_mailparse_msg_create((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mailparse_msg_create", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_mailparse_msg_free(HPHP::Object const&)
_ZN4HPHP20f_mailparse_msg_freeERKNS_6ObjectE

(return value) => rax
mimemail => rdi
*/

bool fh_mailparse_msg_free(Value* mimemail) asm("_ZN4HPHP20f_mailparse_msg_freeERKNS_6ObjectE");

TypedValue * fg1_mailparse_msg_free(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_msg_free(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_mailparse_msg_free((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mailparse_msg_free(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mailparse_msg_free((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_msg_free(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_msg_free", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mailparse_msg_parse_file(HPHP::String const&)
_ZN4HPHP26f_mailparse_msg_parse_fileERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_mailparse_msg_parse_file(TypedValue* _rv, Value* filename) asm("_ZN4HPHP26f_mailparse_msg_parse_fileERKNS_6StringE");

TypedValue * fg1_mailparse_msg_parse_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_msg_parse_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mailparse_msg_parse_file((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mailparse_msg_parse_file(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_mailparse_msg_parse_file((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_msg_parse_file(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_msg_parse_file", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_mailparse_msg_parse(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21f_mailparse_msg_parseERKNS_6ObjectERKNS_6StringE

(return value) => rax
mimemail => rdi
data => rsi
*/

bool fh_mailparse_msg_parse(Value* mimemail, Value* data) asm("_ZN4HPHP21f_mailparse_msg_parseERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_mailparse_msg_parse(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_msg_parse(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_mailparse_msg_parse((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mailparse_msg_parse(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mailparse_msg_parse((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_msg_parse(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_msg_parse", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mailparse_msg_extract_part_file(HPHP::Object const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP33f_mailparse_msg_extract_part_fileERKNS_6ObjectERKNS_7VariantES5_

(return value) => rax
_rv => rdi
mimemail => rsi
filename => rdx
callbackfunc => rcx
*/

TypedValue* fh_mailparse_msg_extract_part_file(TypedValue* _rv, Value* mimemail, TypedValue* filename, TypedValue* callbackfunc) asm("_ZN4HPHP33f_mailparse_msg_extract_part_fileERKNS_6ObjectERKNS_7VariantES5_");

TypedValue * fg1_mailparse_msg_extract_part_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_msg_extract_part_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  Variant defVal2 = empty_string;
  fh_mailparse_msg_extract_part_file((rv), (Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mailparse_msg_extract_part_file(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((args-0)->m_type == KindOfObject) {
        Variant defVal2 = empty_string;
        fh_mailparse_msg_extract_part_file((&(rv)), (Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_msg_extract_part_file(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_msg_extract_part_file", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mailparse_msg_extract_whole_part_file(HPHP::Object const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP39f_mailparse_msg_extract_whole_part_fileERKNS_6ObjectERKNS_7VariantES5_

(return value) => rax
_rv => rdi
mimemail => rsi
filename => rdx
callbackfunc => rcx
*/

TypedValue* fh_mailparse_msg_extract_whole_part_file(TypedValue* _rv, Value* mimemail, TypedValue* filename, TypedValue* callbackfunc) asm("_ZN4HPHP39f_mailparse_msg_extract_whole_part_fileERKNS_6ObjectERKNS_7VariantES5_");

TypedValue * fg1_mailparse_msg_extract_whole_part_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_msg_extract_whole_part_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  Variant defVal2 = empty_string;
  fh_mailparse_msg_extract_whole_part_file((rv), (Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mailparse_msg_extract_whole_part_file(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((args-0)->m_type == KindOfObject) {
        Variant defVal2 = empty_string;
        fh_mailparse_msg_extract_whole_part_file((&(rv)), (Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_msg_extract_whole_part_file(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_msg_extract_whole_part_file", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mailparse_msg_extract_part(HPHP::Object const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP28f_mailparse_msg_extract_partERKNS_6ObjectERKNS_7VariantES5_

(return value) => rax
_rv => rdi
mimemail => rsi
msgbody => rdx
callbackfunc => rcx
*/

TypedValue* fh_mailparse_msg_extract_part(TypedValue* _rv, Value* mimemail, TypedValue* msgbody, TypedValue* callbackfunc) asm("_ZN4HPHP28f_mailparse_msg_extract_partERKNS_6ObjectERKNS_7VariantES5_");

TypedValue * fg1_mailparse_msg_extract_part(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_msg_extract_part(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  Variant defVal2 = empty_string;
  fh_mailparse_msg_extract_part((rv), (Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mailparse_msg_extract_part(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((args-0)->m_type == KindOfObject) {
        Variant defVal2 = empty_string;
        fh_mailparse_msg_extract_part((&(rv)), (Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_msg_extract_part(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_msg_extract_part", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Array HPHP::f_mailparse_msg_get_part_data(HPHP::Object const&)
_ZN4HPHP29f_mailparse_msg_get_part_dataERKNS_6ObjectE

(return value) => rax
_rv => rdi
mimemail => rsi
*/

Value* fh_mailparse_msg_get_part_data(Value* _rv, Value* mimemail) asm("_ZN4HPHP29f_mailparse_msg_get_part_dataERKNS_6ObjectE");

TypedValue * fg1_mailparse_msg_get_part_data(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_msg_get_part_data(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_mailparse_msg_get_part_data((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mailparse_msg_get_part_data(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_mailparse_msg_get_part_data((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_msg_get_part_data(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_msg_get_part_data", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mailparse_msg_get_part(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP24f_mailparse_msg_get_partERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
mimemail => rsi
mimesection => rdx
*/

TypedValue* fh_mailparse_msg_get_part(TypedValue* _rv, Value* mimemail, Value* mimesection) asm("_ZN4HPHP24f_mailparse_msg_get_partERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_mailparse_msg_get_part(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_msg_get_part(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_mailparse_msg_get_part((rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mailparse_msg_get_part(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_mailparse_msg_get_part((&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_msg_get_part(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_msg_get_part", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Array HPHP::f_mailparse_msg_get_structure(HPHP::Object const&)
_ZN4HPHP29f_mailparse_msg_get_structureERKNS_6ObjectE

(return value) => rax
_rv => rdi
mimemail => rsi
*/

Value* fh_mailparse_msg_get_structure(Value* _rv, Value* mimemail) asm("_ZN4HPHP29f_mailparse_msg_get_structureERKNS_6ObjectE");

TypedValue * fg1_mailparse_msg_get_structure(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_msg_get_structure(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_mailparse_msg_get_structure((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mailparse_msg_get_structure(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_mailparse_msg_get_structure((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_msg_get_structure(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_msg_get_structure", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Array HPHP::f_mailparse_rfc822_parse_addresses(HPHP::String const&)
_ZN4HPHP34f_mailparse_rfc822_parse_addressesERKNS_6StringE

(return value) => rax
_rv => rdi
addresses => rsi
*/

Value* fh_mailparse_rfc822_parse_addresses(Value* _rv, Value* addresses) asm("_ZN4HPHP34f_mailparse_rfc822_parse_addressesERKNS_6StringE");

TypedValue * fg1_mailparse_rfc822_parse_addresses(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_rfc822_parse_addresses(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToStringInPlace(args-0);
  fh_mailparse_rfc822_parse_addresses((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mailparse_rfc822_parse_addresses(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_mailparse_rfc822_parse_addresses((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_rfc822_parse_addresses(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_rfc822_parse_addresses", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_mailparse_stream_encode(HPHP::Object const&, HPHP::Object const&, HPHP::String const&)
_ZN4HPHP25f_mailparse_stream_encodeERKNS_6ObjectES2_RKNS_6StringE

(return value) => rax
sourcefp => rdi
destfp => rsi
encoding => rdx
*/

bool fh_mailparse_stream_encode(Value* sourcefp, Value* destfp, Value* encoding) asm("_ZN4HPHP25f_mailparse_stream_encodeERKNS_6ObjectES2_RKNS_6StringE");

TypedValue * fg1_mailparse_stream_encode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_stream_encode(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_mailparse_stream_encode((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mailparse_stream_encode(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mailparse_stream_encode((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_stream_encode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_stream_encode", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mailparse_uudecode_all(HPHP::Object const&)
_ZN4HPHP24f_mailparse_uudecode_allERKNS_6ObjectE

(return value) => rax
_rv => rdi
fp => rsi
*/

TypedValue* fh_mailparse_uudecode_all(TypedValue* _rv, Value* fp) asm("_ZN4HPHP24f_mailparse_uudecode_allERKNS_6ObjectE");

TypedValue * fg1_mailparse_uudecode_all(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_uudecode_all(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_mailparse_uudecode_all((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mailparse_uudecode_all(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_mailparse_uudecode_all((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_uudecode_all(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_uudecode_all", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mailparse_determine_best_xfer_encoding(HPHP::Object const&)
_ZN4HPHP40f_mailparse_determine_best_xfer_encodingERKNS_6ObjectE

(return value) => rax
_rv => rdi
fp => rsi
*/

TypedValue* fh_mailparse_determine_best_xfer_encoding(TypedValue* _rv, Value* fp) asm("_ZN4HPHP40f_mailparse_determine_best_xfer_encodingERKNS_6ObjectE");

TypedValue * fg1_mailparse_determine_best_xfer_encoding(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mailparse_determine_best_xfer_encoding(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_mailparse_determine_best_xfer_encoding((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mailparse_determine_best_xfer_encoding(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_mailparse_determine_best_xfer_encoding((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mailparse_determine_best_xfer_encoding(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mailparse_determine_best_xfer_encoding", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}




} // !HPHP

