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
HPHP::Variant HPHP::f_assert_options(int, HPHP::Variant const&)
_ZN4HPHP16f_assert_optionsEiRKNS_7VariantE

(return value) => rax
_rv => rdi
what => rsi
value => rdx
*/

TypedValue* fh_assert_options(TypedValue* _rv, int what, TypedValue* value) asm("_ZN4HPHP16f_assert_optionsEiRKNS_7VariantE");

TypedValue * fg1_assert_options(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_assert_options(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_assert_options((rv), (int)(args[-0].m_data.num), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_assert_options(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((args-0)->m_type == KindOfInt64) {
        fh_assert_options((&(rv)), (int)(args[-0].m_data.num), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_assert_options(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("assert_options", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_assert(HPHP::Variant const&)
_ZN4HPHP8f_assertERKNS_7VariantE

(return value) => rax
_rv => rdi
assertion => rsi
*/

TypedValue* fh_assert(TypedValue* _rv, TypedValue* assertion) asm("_ZN4HPHP8f_assertERKNS_7VariantE");

TypedValue* fg_assert(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_assert((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("assert", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_dl(HPHP::String const&)
_ZN4HPHP4f_dlERKNS_6StringE

(return value) => rax
library => rdi
*/

long fh_dl(Value* library) asm("_ZN4HPHP4f_dlERKNS_6StringE");

TypedValue * fg1_dl(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dl(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (int64_t)fh_dl((Value*)(args-0));
  return rv;
}

TypedValue* fg_dl(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_dl((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dl(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dl", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_extension_loaded(HPHP::String const&)
_ZN4HPHP18f_extension_loadedERKNS_6StringE

(return value) => rax
name => rdi
*/

bool fh_extension_loaded(Value* name) asm("_ZN4HPHP18f_extension_loadedERKNS_6StringE");

TypedValue * fg1_extension_loaded(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_extension_loaded(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_extension_loaded((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_extension_loaded(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_extension_loaded((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_extension_loaded(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("extension_loaded", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_get_loaded_extensions(bool)
_ZN4HPHP23f_get_loaded_extensionsEb

(return value) => rax
_rv => rdi
zend_extensions => rsi
*/

Value* fh_get_loaded_extensions(Value* _rv, bool zend_extensions) asm("_ZN4HPHP23f_get_loaded_extensionsEb");

TypedValue * fg1_get_loaded_extensions(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_get_loaded_extensions(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToBooleanInPlace(args-0);
  fh_get_loaded_extensions((Value*)(rv), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_get_loaded_extensions(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_get_loaded_extensions((Value*)(&(rv)), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_get_loaded_extensions(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("get_loaded_extensions", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_get_extension_funcs(HPHP::String const&)
_ZN4HPHP21f_get_extension_funcsERKNS_6StringE

(return value) => rax
_rv => rdi
module_name => rsi
*/

Value* fh_get_extension_funcs(Value* _rv, Value* module_name) asm("_ZN4HPHP21f_get_extension_funcsERKNS_6StringE");

TypedValue * fg1_get_extension_funcs(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_get_extension_funcs(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToStringInPlace(args-0);
  fh_get_extension_funcs((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_get_extension_funcs(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_get_extension_funcs((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_get_extension_funcs(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("get_extension_funcs", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_get_cfg_var(HPHP::String const&)
_ZN4HPHP13f_get_cfg_varERKNS_6StringE

(return value) => rax
_rv => rdi
option => rsi
*/

Value* fh_get_cfg_var(Value* _rv, Value* option) asm("_ZN4HPHP13f_get_cfg_varERKNS_6StringE");

TypedValue * fg1_get_cfg_var(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_get_cfg_var(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_get_cfg_var((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_get_cfg_var(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_get_cfg_var((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_get_cfg_var(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("get_cfg_var", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_get_current_user()
_ZN4HPHP18f_get_current_userEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_current_user(Value* _rv) asm("_ZN4HPHP18f_get_current_userEv");

TypedValue* fg_get_current_user(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_get_current_user((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("get_current_user", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_get_defined_constants(HPHP::Variant const&)
_ZN4HPHP23f_get_defined_constantsERKNS_7VariantE

(return value) => rax
_rv => rdi
categorize => rsi
*/

Value* fh_get_defined_constants(Value* _rv, TypedValue* categorize) asm("_ZN4HPHP23f_get_defined_constantsERKNS_7VariantE");

TypedValue* fg_get_defined_constants(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_get_defined_constants((Value*)(&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&null_variant));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("get_defined_constants", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_get_include_path()
_ZN4HPHP18f_get_include_pathEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_include_path(Value* _rv) asm("_ZN4HPHP18f_get_include_pathEv");

TypedValue* fg_get_include_path(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_get_include_path((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("get_include_path", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_restore_include_path()
_ZN4HPHP22f_restore_include_pathEv

*/

void fh_restore_include_path() asm("_ZN4HPHP22f_restore_include_pathEv");

TypedValue* fg_restore_include_path(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_restore_include_path();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("restore_include_path", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_set_include_path(HPHP::String const&)
_ZN4HPHP18f_set_include_pathERKNS_6StringE

(return value) => rax
_rv => rdi
new_include_path => rsi
*/

Value* fh_set_include_path(Value* _rv, Value* new_include_path) asm("_ZN4HPHP18f_set_include_pathERKNS_6StringE");

TypedValue * fg1_set_include_path(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_set_include_path(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_set_include_path((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_set_include_path(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_set_include_path((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_set_include_path(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("set_include_path", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_get_included_files()
_ZN4HPHP20f_get_included_filesEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_included_files(Value* _rv) asm("_ZN4HPHP20f_get_included_filesEv");

TypedValue* fg_get_included_files(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_get_included_files((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("get_included_files", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_inclued_get_data()
_ZN4HPHP18f_inclued_get_dataEv

(return value) => rax
_rv => rdi
*/

Value* fh_inclued_get_data(Value* _rv) asm("_ZN4HPHP18f_inclued_get_dataEv");

TypedValue* fg_inclued_get_data(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_inclued_get_data((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("inclued_get_data", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_get_magic_quotes_gpc()
_ZN4HPHP22f_get_magic_quotes_gpcEv

(return value) => rax
*/

long fh_get_magic_quotes_gpc() asm("_ZN4HPHP22f_get_magic_quotes_gpcEv");

TypedValue* fg_get_magic_quotes_gpc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_get_magic_quotes_gpc();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("get_magic_quotes_gpc", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_get_magic_quotes_runtime()
_ZN4HPHP26f_get_magic_quotes_runtimeEv

(return value) => rax
*/

long fh_get_magic_quotes_runtime() asm("_ZN4HPHP26f_get_magic_quotes_runtimeEv");

TypedValue* fg_get_magic_quotes_runtime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_get_magic_quotes_runtime();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("get_magic_quotes_runtime", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_get_required_files()
_ZN4HPHP20f_get_required_filesEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_required_files(Value* _rv) asm("_ZN4HPHP20f_get_required_filesEv");

TypedValue* fg_get_required_files(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_get_required_files((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("get_required_files", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_getenv(HPHP::String const&)
_ZN4HPHP8f_getenvERKNS_6StringE

(return value) => rax
_rv => rdi
varname => rsi
*/

TypedValue* fh_getenv(TypedValue* _rv, Value* varname) asm("_ZN4HPHP8f_getenvERKNS_6StringE");

TypedValue * fg1_getenv(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_getenv(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_getenv((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_getenv(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_getenv((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_getenv(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("getenv", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_getlastmod()
_ZN4HPHP12f_getlastmodEv

(return value) => rax
*/

long fh_getlastmod() asm("_ZN4HPHP12f_getlastmodEv");

TypedValue* fg_getlastmod(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_getlastmod();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("getlastmod", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_getmygid()
_ZN4HPHP10f_getmygidEv

(return value) => rax
*/

long fh_getmygid() asm("_ZN4HPHP10f_getmygidEv");

TypedValue* fg_getmygid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_getmygid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("getmygid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_getmyinode()
_ZN4HPHP12f_getmyinodeEv

(return value) => rax
*/

long fh_getmyinode() asm("_ZN4HPHP12f_getmyinodeEv");

TypedValue* fg_getmyinode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_getmyinode();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("getmyinode", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_getmypid()
_ZN4HPHP10f_getmypidEv

(return value) => rax
*/

long fh_getmypid() asm("_ZN4HPHP10f_getmypidEv");

TypedValue* fg_getmypid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_getmypid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("getmypid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_getmyuid()
_ZN4HPHP10f_getmyuidEv

(return value) => rax
*/

long fh_getmyuid() asm("_ZN4HPHP10f_getmyuidEv");

TypedValue* fg_getmyuid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_getmyuid();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("getmyuid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_getopt(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP8f_getoptERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
options => rsi
longopts => rdx
*/

Value* fh_getopt(Value* _rv, Value* options, TypedValue* longopts) asm("_ZN4HPHP8f_getoptERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_getopt(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_getopt(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToStringInPlace(args-0);
  fh_getopt((Value*)(rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_getopt(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_getopt((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_getopt(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("getopt", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_getrusage(int)
_ZN4HPHP11f_getrusageEi

(return value) => rax
_rv => rdi
who => rsi
*/

Value* fh_getrusage(Value* _rv, int who) asm("_ZN4HPHP11f_getrusageEi");

TypedValue * fg1_getrusage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_getrusage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToInt64InPlace(args-0);
  fh_getrusage((Value*)(rv), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_getrusage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_getrusage((Value*)(&(rv)), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_getrusage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("getrusage", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_clock_getres(int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_clock_getresEiRKNS_14VRefParamValueES2_

(return value) => rax
clk_id => rdi
sec => rsi
nsec => rdx
*/

bool fh_clock_getres(int clk_id, TypedValue* sec, TypedValue* nsec) asm("_ZN4HPHP14f_clock_getresEiRKNS_14VRefParamValueES2_");

TypedValue * fg1_clock_getres(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_clock_getres(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_clock_getres((int)(args[-0].m_data.num), (args-1), (args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_clock_getres(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_clock_getres((int)(args[-0].m_data.num), (args-1), (args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_clock_getres(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("clock_getres", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_clock_gettime(int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP15f_clock_gettimeEiRKNS_14VRefParamValueES2_

(return value) => rax
clk_id => rdi
sec => rsi
nsec => rdx
*/

bool fh_clock_gettime(int clk_id, TypedValue* sec, TypedValue* nsec) asm("_ZN4HPHP15f_clock_gettimeEiRKNS_14VRefParamValueES2_");

TypedValue * fg1_clock_gettime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_clock_gettime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_clock_gettime((int)(args[-0].m_data.num), (args-1), (args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_clock_gettime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_clock_gettime((int)(args[-0].m_data.num), (args-1), (args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_clock_gettime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("clock_gettime", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_clock_settime(int, long, long)
_ZN4HPHP15f_clock_settimeEill

(return value) => rax
clk_id => rdi
sec => rsi
nsec => rdx
*/

bool fh_clock_settime(int clk_id, long sec, long nsec) asm("_ZN4HPHP15f_clock_settimeEill");

TypedValue * fg1_clock_settime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_clock_settime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_clock_settime((int)(args[-0].m_data.num), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_clock_settime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_clock_settime((int)(args[-0].m_data.num), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_clock_settime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("clock_settime", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_ini_alter(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_ini_alterERKNS_6StringES2_

(return value) => rax
_rv => rdi
varname => rsi
newvalue => rdx
*/

Value* fh_ini_alter(Value* _rv, Value* varname, Value* newvalue) asm("_ZN4HPHP11f_ini_alterERKNS_6StringES2_");

TypedValue * fg1_ini_alter(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_ini_alter(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_ini_alter((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_ini_alter(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_ini_alter((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ini_alter(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ini_alter", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_ini_get_all(HPHP::String const&)
_ZN4HPHP13f_ini_get_allERKNS_6StringE

(return value) => rax
_rv => rdi
extension => rsi
*/

Value* fh_ini_get_all(Value* _rv, Value* extension) asm("_ZN4HPHP13f_ini_get_allERKNS_6StringE");

TypedValue * fg1_ini_get_all(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_ini_get_all(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToStringInPlace(args-0);
  fh_ini_get_all((Value*)(rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_ini_get_all(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_ini_get_all((Value*)(&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ini_get_all(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("ini_get_all", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_ini_get(HPHP::String const&)
_ZN4HPHP9f_ini_getERKNS_6StringE

(return value) => rax
_rv => rdi
varname => rsi
*/

Value* fh_ini_get(Value* _rv, Value* varname) asm("_ZN4HPHP9f_ini_getERKNS_6StringE");

TypedValue * fg1_ini_get(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_ini_get(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_ini_get((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_ini_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_ini_get((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ini_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ini_get", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_ini_restore(HPHP::String const&)
_ZN4HPHP13f_ini_restoreERKNS_6StringE

varname => rdi
*/

void fh_ini_restore(Value* varname) asm("_ZN4HPHP13f_ini_restoreERKNS_6StringE");

TypedValue * fg1_ini_restore(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_ini_restore(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  fh_ini_restore((Value*)(args-0));
  return rv;
}

TypedValue* fg_ini_restore(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_ini_restore((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ini_restore(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ini_restore", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_ini_set(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9f_ini_setERKNS_6StringES2_

(return value) => rax
_rv => rdi
varname => rsi
newvalue => rdx
*/

Value* fh_ini_set(Value* _rv, Value* varname, Value* newvalue) asm("_ZN4HPHP9f_ini_setERKNS_6StringES2_");

TypedValue * fg1_ini_set(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_ini_set(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_ini_set((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_ini_set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_ini_set((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ini_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ini_set", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_memory_get_allocation()
_ZN4HPHP23f_memory_get_allocationEv

(return value) => rax
*/

long fh_memory_get_allocation() asm("_ZN4HPHP23f_memory_get_allocationEv");

TypedValue* fg_memory_get_allocation(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_memory_get_allocation();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("memory_get_allocation", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_memory_get_peak_usage(bool)
_ZN4HPHP23f_memory_get_peak_usageEb

(return value) => rax
real_usage => rdi
*/

long fh_memory_get_peak_usage(bool real_usage) asm("_ZN4HPHP23f_memory_get_peak_usageEb");

TypedValue * fg1_memory_get_peak_usage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memory_get_peak_usage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToBooleanInPlace(args-0);
  rv->m_data.num = (int64_t)fh_memory_get_peak_usage((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_memory_get_peak_usage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_memory_get_peak_usage((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memory_get_peak_usage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("memory_get_peak_usage", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_memory_get_usage(bool)
_ZN4HPHP18f_memory_get_usageEb

(return value) => rax
real_usage => rdi
*/

long fh_memory_get_usage(bool real_usage) asm("_ZN4HPHP18f_memory_get_usageEb");

TypedValue * fg1_memory_get_usage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memory_get_usage(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToBooleanInPlace(args-0);
  rv->m_data.num = (int64_t)fh_memory_get_usage((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* fg_memory_get_usage(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_memory_get_usage((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memory_get_usage(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("memory_get_usage", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_php_ini_scanned_files()
_ZN4HPHP23f_php_ini_scanned_filesEv

(return value) => rax
_rv => rdi
*/

Value* fh_php_ini_scanned_files(Value* _rv) asm("_ZN4HPHP23f_php_ini_scanned_filesEv");

TypedValue* fg_php_ini_scanned_files(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_php_ini_scanned_files((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("php_ini_scanned_files", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_php_logo_guid()
_ZN4HPHP15f_php_logo_guidEv

(return value) => rax
_rv => rdi
*/

Value* fh_php_logo_guid(Value* _rv) asm("_ZN4HPHP15f_php_logo_guidEv");

TypedValue* fg_php_logo_guid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_php_logo_guid((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("php_logo_guid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_php_sapi_name()
_ZN4HPHP15f_php_sapi_nameEv

(return value) => rax
_rv => rdi
*/

Value* fh_php_sapi_name(Value* _rv) asm("_ZN4HPHP15f_php_sapi_nameEv");

TypedValue* fg_php_sapi_name(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_php_sapi_name((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("php_sapi_name", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_php_uname(HPHP::String const&)
_ZN4HPHP11f_php_unameERKNS_6StringE

(return value) => rax
_rv => rdi
mode => rsi
*/

Value* fh_php_uname(Value* _rv, Value* mode) asm("_ZN4HPHP11f_php_unameERKNS_6StringE");

TypedValue * fg1_php_uname(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_php_uname(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_php_uname((Value*)(rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_php_uname(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_php_uname((Value*)(&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_php_uname(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("php_uname", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_phpcredits(int)
_ZN4HPHP12f_phpcreditsEi

(return value) => rax
flag => rdi
*/

bool fh_phpcredits(int flag) asm("_ZN4HPHP12f_phpcreditsEi");

TypedValue * fg1_phpcredits(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_phpcredits(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_phpcredits((count > 0) ? (int)(args[-0].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_phpcredits(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_phpcredits((count > 0) ? (int)(args[-0].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_phpcredits(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("phpcredits", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_phpinfo(int)
_ZN4HPHP9f_phpinfoEi

(return value) => rax
what => rdi
*/

bool fh_phpinfo(int what) asm("_ZN4HPHP9f_phpinfoEi");

TypedValue * fg1_phpinfo(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_phpinfo(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_phpinfo((count > 0) ? (int)(args[-0].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_phpinfo(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_phpinfo((count > 0) ? (int)(args[-0].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_phpinfo(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("phpinfo", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_phpversion(HPHP::String const&)
_ZN4HPHP12f_phpversionERKNS_6StringE

(return value) => rax
_rv => rdi
extension => rsi
*/

Value* fh_phpversion(Value* _rv, Value* extension) asm("_ZN4HPHP12f_phpversionERKNS_6StringE");

TypedValue * fg1_phpversion(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_phpversion(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_phpversion((Value*)(rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_phpversion(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_phpversion((Value*)(&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_phpversion(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("phpversion", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_putenv(HPHP::String const&)
_ZN4HPHP8f_putenvERKNS_6StringE

(return value) => rax
setting => rdi
*/

bool fh_putenv(Value* setting) asm("_ZN4HPHP8f_putenvERKNS_6StringE");

TypedValue * fg1_putenv(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_putenv(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_putenv((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_putenv(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_putenv((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_putenv(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("putenv", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_set_magic_quotes_runtime(bool)
_ZN4HPHP26f_set_magic_quotes_runtimeEb

(return value) => rax
new_setting => rdi
*/

bool fh_set_magic_quotes_runtime(bool new_setting) asm("_ZN4HPHP26f_set_magic_quotes_runtimeEb");

TypedValue * fg1_set_magic_quotes_runtime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_set_magic_quotes_runtime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToBooleanInPlace(args-0);
  rv->m_data.num = (fh_set_magic_quotes_runtime((bool)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_set_magic_quotes_runtime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfBoolean) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_set_magic_quotes_runtime((bool)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_set_magic_quotes_runtime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("set_magic_quotes_runtime", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_set_time_limit(int)
_ZN4HPHP16f_set_time_limitEi

seconds => rdi
*/

void fh_set_time_limit(int seconds) asm("_ZN4HPHP16f_set_time_limitEi");

TypedValue * fg1_set_time_limit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_set_time_limit(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToInt64InPlace(args-0);
  fh_set_time_limit((int)(args[-0].m_data.num));
  return rv;
}

TypedValue* fg_set_time_limit(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_set_time_limit((int)(args[-0].m_data.num));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_set_time_limit(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("set_time_limit", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_sys_get_temp_dir()
_ZN4HPHP18f_sys_get_temp_dirEv

(return value) => rax
_rv => rdi
*/

Value* fh_sys_get_temp_dir(Value* _rv) asm("_ZN4HPHP18f_sys_get_temp_dirEv");

TypedValue* fg_sys_get_temp_dir(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_sys_get_temp_dir((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("sys_get_temp_dir", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_version_compare(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP17f_version_compareERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
version1 => rsi
version2 => rdx
sop => rcx
*/

TypedValue* fh_version_compare(TypedValue* _rv, Value* version1, Value* version2, Value* sop) asm("_ZN4HPHP17f_version_compareERKNS_6StringES2_S2_");

TypedValue * fg1_version_compare(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_version_compare(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_version_compare((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_version_compare(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_version_compare((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_version_compare(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("version_compare", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_gc_enabled()
_ZN4HPHP12f_gc_enabledEv

(return value) => rax
*/

bool fh_gc_enabled() asm("_ZN4HPHP12f_gc_enabledEv");

TypedValue* fg_gc_enabled(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_gc_enabled()) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("gc_enabled", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_gc_enable()
_ZN4HPHP11f_gc_enableEv

*/

void fh_gc_enable() asm("_ZN4HPHP11f_gc_enableEv");

TypedValue* fg_gc_enable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_gc_enable();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("gc_enable", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_gc_disable()
_ZN4HPHP12f_gc_disableEv

*/

void fh_gc_disable() asm("_ZN4HPHP12f_gc_disableEv");

TypedValue* fg_gc_disable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_gc_disable();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("gc_disable", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_gc_collect_cycles()
_ZN4HPHP19f_gc_collect_cyclesEv

(return value) => rax
*/

long fh_gc_collect_cycles() asm("_ZN4HPHP19f_gc_collect_cyclesEv");

TypedValue* fg_gc_collect_cycles(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_gc_collect_cycles();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("gc_collect_cycles", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_zend_logo_guid()
_ZN4HPHP16f_zend_logo_guidEv

(return value) => rax
_rv => rdi
*/

Value* fh_zend_logo_guid(Value* _rv) asm("_ZN4HPHP16f_zend_logo_guidEv");

TypedValue* fg_zend_logo_guid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_zend_logo_guid((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("zend_logo_guid", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_zend_thread_id()
_ZN4HPHP16f_zend_thread_idEv

(return value) => rax
*/

long fh_zend_thread_id() asm("_ZN4HPHP16f_zend_thread_idEv");

TypedValue* fg_zend_thread_id(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_zend_thread_id();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("zend_thread_id", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_zend_version()
_ZN4HPHP14f_zend_versionEv

(return value) => rax
_rv => rdi
*/

Value* fh_zend_version(Value* _rv) asm("_ZN4HPHP14f_zend_versionEv");

TypedValue* fg_zend_version(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_zend_version((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("zend_version", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

