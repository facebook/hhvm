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
HPHP::String HPHP::f_implode(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP9f_implodeERKNS_7VariantES2_

(return value) => rax
_rv => rdi
arg1 => rsi
arg2 => rdx
*/

Value* fh_implode(Value* _rv, TypedValue* arg1, TypedValue* arg2) asm("_ZN4HPHP9f_implodeERKNS_7VariantES2_");

TypedValue* fg_implode(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      rv._count = 0;
      rv.m_type = KindOfString;
      fh_implode((Value*)(&(rv)), (args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("implode", count, 1, 2, 1);
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
HPHP::Variant HPHP::f_strtok(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP8f_strtokERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
str => rsi
token => rdx
*/

TypedValue* fh_strtok(TypedValue* _rv, Value* str, TypedValue* token) asm("_ZN4HPHP8f_strtokERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_strtok(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strtok(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_strtok((rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strtok(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_strtok((&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&null_variant));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strtok(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strtok", count, 1, 2, 1);
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
HPHP::Variant HPHP::f_str_replace(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP13f_str_replaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
search => rsi
replace => rdx
subject => rcx
count => r8
*/

TypedValue* fh_str_replace(TypedValue* _rv, TypedValue* search, TypedValue* replace, TypedValue* subject, TypedValue* count) asm("_ZN4HPHP13f_str_replaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE");

TypedValue* fg_str_replace(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      VRefParamValue defVal3 = null;
      fh_str_replace((&(rv)), (args-0), (args-1), (args-2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 4);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("str_replace", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_str_ireplace(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_str_ireplaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
search => rsi
replace => rdx
subject => rcx
count => r8
*/

TypedValue* fh_str_ireplace(TypedValue* _rv, TypedValue* search, TypedValue* replace, TypedValue* subject, TypedValue* count) asm("_ZN4HPHP14f_str_ireplaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE");

TypedValue* fg_str_ireplace(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      VRefParamValue defVal3 = null;
      fh_str_ireplace((&(rv)), (args-0), (args-1), (args-2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 4);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("str_ireplace", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_substr_replace(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP16f_substr_replaceERKNS_7VariantES2_S2_S2_

(return value) => rax
_rv => rdi
str => rsi
replacement => rdx
start => rcx
length => r8
*/

TypedValue* fh_substr_replace(TypedValue* _rv, TypedValue* str, TypedValue* replacement, TypedValue* start, TypedValue* length) asm("_ZN4HPHP16f_substr_replaceERKNS_7VariantES2_S2_S2_");

TypedValue* fg_substr_replace(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      Variant defVal3 = 0x7FFFFFFF;
      fh_substr_replace((&(rv)), (args-0), (args-1), (args-2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 4);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("substr_replace", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_strtr(HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP7f_strtrERKNS_6StringERKNS_7VariantES5_

(return value) => rax
_rv => rdi
str => rsi
from => rdx
to => rcx
*/

TypedValue* fh_strtr(TypedValue* _rv, Value* str, TypedValue* from, TypedValue* to) asm("_ZN4HPHP7f_strtrERKNS_6StringERKNS_7VariantES5_");

TypedValue * fg1_strtr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strtr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_strtr((rv), (Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strtr(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_strtr((&(rv)), (Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strtr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strtr", count, 2, 3, 1);
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
HPHP::String HPHP::f_convert_cyr_string(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_convert_cyr_stringERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
str => rsi
from => rdx
to => rcx
*/

Value* fh_convert_cyr_string(Value* _rv, Value* str, Value* from, Value* to) asm("_ZN4HPHP20f_convert_cyr_stringERKNS_6StringES2_S2_");

TypedValue * fg1_convert_cyr_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_convert_cyr_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_convert_cyr_string((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_convert_cyr_string(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_convert_cyr_string((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_convert_cyr_string(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("convert_cyr_string", count, 3, 3, 1);
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
HPHP::String HPHP::f_hebrev(HPHP::String const&, int)
_ZN4HPHP8f_hebrevERKNS_6StringEi

(return value) => rax
_rv => rdi
hebrew_text => rsi
max_chars_per_line => rdx
*/

Value* fh_hebrev(Value* _rv, Value* hebrew_text, int max_chars_per_line) asm("_ZN4HPHP8f_hebrevERKNS_6StringEi");

TypedValue * fg1_hebrev(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hebrev(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_hebrev((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hebrev(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hebrev((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hebrev(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hebrev", count, 1, 2, 1);
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
HPHP::String HPHP::f_hebrevc(HPHP::String const&, int)
_ZN4HPHP9f_hebrevcERKNS_6StringEi

(return value) => rax
_rv => rdi
hebrew_text => rsi
max_chars_per_line => rdx
*/

Value* fh_hebrevc(Value* _rv, Value* hebrew_text, int max_chars_per_line) asm("_ZN4HPHP9f_hebrevcERKNS_6StringEi");

TypedValue * fg1_hebrevc(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_hebrevc(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_hebrevc((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hebrevc(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hebrevc((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hebrevc(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hebrevc", count, 1, 2, 1);
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
HPHP::Variant HPHP::f_setlocale(int, int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP11f_setlocaleEiiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
category => rdx
locale => rcx
_argv => r8
*/

TypedValue* fh_setlocale(TypedValue* _rv, long long _argc, int category, TypedValue* locale, Value* _argv) asm("_ZN4HPHP11f_setlocaleEiiRKNS_7VariantERKNS_5ArrayE");

TypedValue * fg1_setlocale(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_setlocale(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-2, false);
    for (long long i = 2; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-2);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-2, tvAsVariant(extraArg));
      } else {
        ai.set(i-2, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  fh_setlocale((rv), (count), (int)(args[-0].m_data.num), (args-1), (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_setlocale(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      if ((args-0)->m_type == KindOfInt64) {
        Array extraArgs;
        {
          ArrayInit ai(count-2, false);
          for (long long i = 2; i < count; ++i) {
            TypedValue* extraArg = ar->getExtraArg(i-2);
            if (tvIsStronglyBound(extraArg)) {
              ai.setRef(i-2, tvAsVariant(extraArg));
            } else {
              ai.set(i-2, tvAsVariant(extraArg));
            }
          }
          extraArgs = ai.create();
        }
        fh_setlocale((&(rv)), (count), (int)(args[-0].m_data.num), (args-1), (Value*)(&extraArgs));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_setlocale(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_missing_arguments_nr("setlocale", count+1, 1);
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
HPHP::Array HPHP::f_localeconv()
_ZN4HPHP12f_localeconvEv

(return value) => rax
_rv => rdi
*/

Value* fh_localeconv(Value* _rv) asm("_ZN4HPHP12f_localeconvEv");

TypedValue* fg_localeconv(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_localeconv((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("localeconv", 0, 1);
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
HPHP::Variant HPHP::f_sscanf(int, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP8f_sscanfEiRKNS_6StringES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
str => rdx
format => rcx
_argv => r8
*/

TypedValue* fh_sscanf(TypedValue* _rv, long long _argc, Value* str, Value* format, Value* _argv) asm("_ZN4HPHP8f_sscanfEiRKNS_6StringES2_RKNS_5ArrayE");

TypedValue * fg1_sscanf(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sscanf(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Array extraArgs;
  {
    ArrayInit ai(count-2, false);
    for (long long i = 2; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-2);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-2, tvAsVariant(extraArg));
      } else {
        ai.set(i-2, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  fh_sscanf((rv), (count), (Value*)(args-0), (Value*)(args-1), (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_sscanf(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        Array extraArgs;
        {
          ArrayInit ai(count-2, false);
          for (long long i = 2; i < count; ++i) {
            TypedValue* extraArg = ar->getExtraArg(i-2);
            if (tvIsStronglyBound(extraArg)) {
              ai.setRef(i-2, tvAsVariant(extraArg));
            } else {
              ai.set(i-2, tvAsVariant(extraArg));
            }
          }
          extraArgs = ai.create();
        }
        fh_sscanf((&(rv)), (count), (Value*)(args-0), (Value*)(args-1), (Value*)(&extraArgs));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sscanf(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_missing_arguments_nr("sscanf", count+1, 1);
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
HPHP::String HPHP::f_number_format(double, int, HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_number_formatEdiRKNS_6StringES2_

(return value) => rax
_rv => rdi
number => xmm0
decimals => rsi
dec_point => rdx
thousands_sep => rcx
*/

Value* fh_number_format(Value* _rv, double number, int decimals, Value* dec_point, Value* thousands_sep) asm("_ZN4HPHP15f_number_formatEdiRKNS_6StringES2_");

TypedValue * fg1_number_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_number_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
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
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  String defVal2 = ".";
  String defVal3 = ",";
  fh_number_format((Value*)(rv), (args[-0].m_data.dbl), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (Value*)(args-3) : (Value*)(&defVal3));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_number_format(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfDouble) {
        rv._count = 0;
        rv.m_type = KindOfString;
        String defVal2 = ".";
        String defVal3 = ",";
        fh_number_format((Value*)(&(rv)), (args[-0].m_data.dbl), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2), (count > 3) ? (Value*)(args-3) : (Value*)(&defVal3));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_number_format(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("number_format", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_substr_compare(HPHP::String const&, HPHP::String const&, int, int, bool)
_ZN4HPHP16f_substr_compareERKNS_6StringES2_iib

(return value) => rax
_rv => rdi
main_str => rsi
str => rdx
offset => rcx
length => r8
case_insensitivity => r9
*/

TypedValue* fh_substr_compare(TypedValue* _rv, Value* main_str, Value* str, int offset, int length, bool case_insensitivity) asm("_ZN4HPHP16f_substr_compareERKNS_6StringES2_iib");

TypedValue * fg1_substr_compare(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_substr_compare(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
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
  fh_substr_compare((rv), (Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(INT_MAX), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_substr_compare(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfBoolean) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfInt64 && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_substr_compare((&(rv)), (Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(INT_MAX), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_substr_compare(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("substr_compare", count, 3, 5, 1);
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
HPHP::Variant HPHP::f_strrchr(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP9f_strrchrERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
*/

TypedValue* fh_strrchr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP9f_strrchrERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_strrchr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strrchr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_strrchr((rv), (Value*)(args-0), (args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strrchr(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_strrchr((&(rv)), (Value*)(args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strrchr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strrchr", count, 2, 2, 1);
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
HPHP::Variant HPHP::f_strstr(HPHP::String const&, HPHP::Variant const&, bool)
_ZN4HPHP8f_strstrERKNS_6StringERKNS_7VariantEb

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
before_needle => rcx
*/

TypedValue* fh_strstr(TypedValue* _rv, Value* haystack, TypedValue* needle, bool before_needle) asm("_ZN4HPHP8f_strstrERKNS_6StringERKNS_7VariantEb");

TypedValue * fg1_strstr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strstr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_strstr((rv), (Value*)(args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strstr(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strstr((&(rv)), (Value*)(args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strstr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strstr", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_stristr(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP9f_stristrERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
*/

TypedValue* fh_stristr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP9f_stristrERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_stristr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stristr(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_stristr((rv), (Value*)(args-0), (args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stristr(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_stristr((&(rv)), (Value*)(args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stristr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stristr", count, 2, 2, 1);
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
HPHP::Variant HPHP::f_strpbrk(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9f_strpbrkERKNS_6StringES2_

(return value) => rax
_rv => rdi
haystack => rsi
char_list => rdx
*/

TypedValue* fh_strpbrk(TypedValue* _rv, Value* haystack, Value* char_list) asm("_ZN4HPHP9f_strpbrkERKNS_6StringES2_");

TypedValue * fg1_strpbrk(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strpbrk(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_strpbrk((rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strpbrk(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strpbrk((&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strpbrk(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strpbrk", count, 2, 2, 1);
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
HPHP::Variant HPHP::f_strpos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP8f_strposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_strpos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP8f_strposERKNS_6StringERKNS_7VariantEi");

TypedValue * fg1_strpos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strpos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_strpos((rv), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strpos(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strpos((&(rv)), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strpos(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strpos", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_stripos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP9f_striposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_stripos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP9f_striposERKNS_6StringERKNS_7VariantEi");

TypedValue * fg1_stripos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_stripos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_stripos((rv), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stripos(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_stripos((&(rv)), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stripos(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stripos", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_strrpos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP9f_strrposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_strrpos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP9f_strrposERKNS_6StringERKNS_7VariantEi");

TypedValue * fg1_strrpos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strrpos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_strrpos((rv), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strrpos(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strrpos((&(rv)), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strrpos(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strrpos", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_strripos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP10f_strriposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_strripos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP10f_strriposERKNS_6StringERKNS_7VariantEi");

TypedValue * fg1_strripos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strripos(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_strripos((rv), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strripos(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strripos((&(rv)), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strripos(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strripos", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_substr_count(HPHP::String const&, HPHP::String const&, int, int)
_ZN4HPHP14f_substr_countERKNS_6StringES2_ii

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
length => r8
*/

TypedValue* fh_substr_count(TypedValue* _rv, Value* haystack, Value* needle, int offset, int length) asm("_ZN4HPHP14f_substr_countERKNS_6StringES2_ii");

TypedValue * fg1_substr_count(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_substr_count(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_substr_count((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0x7FFFFFFF));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_substr_count(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_substr_count((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0x7FFFFFFF));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_substr_count(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("substr_count", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_strspn(HPHP::String const&, HPHP::String const&, int, int)
_ZN4HPHP8f_strspnERKNS_6StringES2_ii

(return value) => rax
_rv => rdi
str1 => rsi
str2 => rdx
start => rcx
length => r8
*/

TypedValue* fh_strspn(TypedValue* _rv, Value* str1, Value* str2, int start, int length) asm("_ZN4HPHP8f_strspnERKNS_6StringES2_ii");

TypedValue * fg1_strspn(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strspn(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_strspn((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0x7FFFFFFF));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strspn(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strspn((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0x7FFFFFFF));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strspn(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strspn", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_strcspn(HPHP::String const&, HPHP::String const&, int, int)
_ZN4HPHP9f_strcspnERKNS_6StringES2_ii

(return value) => rax
_rv => rdi
str1 => rsi
str2 => rdx
start => rcx
length => r8
*/

TypedValue* fh_strcspn(TypedValue* _rv, Value* str1, Value* str2, int start, int length) asm("_ZN4HPHP9f_strcspnERKNS_6StringES2_ii");

TypedValue * fg1_strcspn(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_strcspn(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_strcspn((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0x7FFFFFFF));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_strcspn(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_strcspn((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0x7FFFFFFF));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_strcspn(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("strcspn", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_count_chars(HPHP::String const&, long long)
_ZN4HPHP13f_count_charsERKNS_6StringEx

(return value) => rax
_rv => rdi
str => rsi
mode => rdx
*/

TypedValue* fh_count_chars(TypedValue* _rv, Value* str, long long mode) asm("_ZN4HPHP13f_count_charsERKNS_6StringEx");

TypedValue * fg1_count_chars(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_count_chars(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  fh_count_chars((rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_count_chars(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_count_chars((&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_count_chars(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("count_chars", count, 1, 2, 1);
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
HPHP::Variant HPHP::f_str_word_count(HPHP::String const&, long long, HPHP::String const&)
_ZN4HPHP16f_str_word_countERKNS_6StringExS2_

(return value) => rax
_rv => rdi
str => rsi
format => rdx
charlist => rcx
*/

TypedValue* fh_str_word_count(TypedValue* _rv, Value* str, long long format, Value* charlist) asm("_ZN4HPHP16f_str_word_countERKNS_6StringExS2_");

TypedValue * fg1_str_word_count(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_str_word_count(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_str_word_count((rv), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0), (count > 2) ? (Value*)(args-2) : (Value*)(&empty_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_str_word_count(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_str_word_count((&(rv)), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0), (count > 2) ? (Value*)(args-2) : (Value*)(&empty_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_str_word_count(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("str_word_count", count, 1, 3, 1);
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
void HPHP::f_parse_str(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP11f_parse_strERKNS_6StringERKNS_14VRefParamValueE

str => rdi
arr => rsi
*/

void fh_parse_str(Value* str, TypedValue* arr) asm("_ZN4HPHP11f_parse_strERKNS_6StringERKNS_14VRefParamValueE");

TypedValue * fg1_parse_str(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_parse_str(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal1 = null;
  fh_parse_str((Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  return rv;
}

TypedValue* fg_parse_str(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        VRefParamValue defVal1 = null;
        fh_parse_str((Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_parse_str(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("parse_str", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}




} // !HPHP

