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
HPHP::Variant HPHP::f_xmlwriter_open_memory()
_ZN4HPHP23f_xmlwriter_open_memoryEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_xmlwriter_open_memory(TypedValue* _rv) asm("_ZN4HPHP23f_xmlwriter_open_memoryEv");

TypedValue* fg_xmlwriter_open_memory(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_xmlwriter_open_memory((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("xmlwriter_open_memory", 0, 1);
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
HPHP::Object HPHP::f_xmlwriter_open_uri(HPHP::String const&)
_ZN4HPHP20f_xmlwriter_open_uriERKNS_6StringE

(return value) => rax
_rv => rdi
uri => rsi
*/

Value* fh_xmlwriter_open_uri(Value* _rv, Value* uri) asm("_ZN4HPHP20f_xmlwriter_open_uriERKNS_6StringE");

TypedValue * fg1_xmlwriter_open_uri(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_open_uri(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  fh_xmlwriter_open_uri((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_xmlwriter_open_uri(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_xmlwriter_open_uri((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_open_uri(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_open_uri", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_set_indent_string(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP29f_xmlwriter_set_indent_stringERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
indentstring => rsi
*/

bool fh_xmlwriter_set_indent_string(Value* xmlwriter, Value* indentstring) asm("_ZN4HPHP29f_xmlwriter_set_indent_stringERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_xmlwriter_set_indent_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_set_indent_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_set_indent_string((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_set_indent_string(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_set_indent_string((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_set_indent_string(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_set_indent_string", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_set_indent(HPHP::Object const&, bool)
_ZN4HPHP22f_xmlwriter_set_indentERKNS_6ObjectEb

(return value) => rax
xmlwriter => rdi
indent => rsi
*/

bool fh_xmlwriter_set_indent(Value* xmlwriter, bool indent) asm("_ZN4HPHP22f_xmlwriter_set_indentERKNS_6ObjectEb");

TypedValue * fg1_xmlwriter_set_indent(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_set_indent(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_set_indent((Value*)(args-0), (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_set_indent(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_set_indent((Value*)(args-0), (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_set_indent(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_set_indent", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_start_document(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP26f_xmlwriter_start_documentERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
xmlwriter => rdi
version => rsi
encoding => rdx
standalone => rcx
*/

bool fh_xmlwriter_start_document(Value* xmlwriter, Value* version, Value* encoding, Value* standalone) asm("_ZN4HPHP26f_xmlwriter_start_documentERKNS_6ObjectERKNS_6StringES5_S5_");

TypedValue * fg1_xmlwriter_start_document(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_document(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  String defVal1 = "1.0";
  rv->m_data.num = (fh_xmlwriter_start_document((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_document(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        String defVal1 = "1.0";
        rv.m_data.num = (fh_xmlwriter_start_document((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_document(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_document", count, 1, 4, 1);
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
bool HPHP::f_xmlwriter_start_element(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP25f_xmlwriter_start_elementERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
name => rsi
*/

bool fh_xmlwriter_start_element(Value* xmlwriter, Value* name) asm("_ZN4HPHP25f_xmlwriter_start_elementERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_xmlwriter_start_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_start_element((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_element(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_element((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_element(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_element", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_start_element_ns(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP28f_xmlwriter_start_element_nsERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
xmlwriter => rdi
prefix => rsi
name => rdx
uri => rcx
*/

bool fh_xmlwriter_start_element_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri) asm("_ZN4HPHP28f_xmlwriter_start_element_nsERKNS_6ObjectERKNS_6StringES5_S5_");

TypedValue * fg1_xmlwriter_start_element_ns(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_element_ns(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  rv->m_data.num = (fh_xmlwriter_start_element_ns((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_element_ns(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_element_ns((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_element_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_element_ns", count, 4, 4, 1);
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
bool HPHP::f_xmlwriter_write_element_ns(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP28f_xmlwriter_write_element_nsERKNS_6ObjectERKNS_6StringES5_S5_S5_

(return value) => rax
xmlwriter => rdi
prefix => rsi
name => rdx
uri => rcx
content => r8
*/

bool fh_xmlwriter_write_element_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri, Value* content) asm("_ZN4HPHP28f_xmlwriter_write_element_nsERKNS_6ObjectERKNS_6StringES5_S5_S5_");

TypedValue * fg1_xmlwriter_write_element_ns(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_element_ns(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    break;
  }
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
  rv->m_data.num = (fh_xmlwriter_write_element_ns((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_element_ns(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_element_ns((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_element_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_element_ns", count, 4, 5, 1);
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
bool HPHP::f_xmlwriter_write_element(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP25f_xmlwriter_write_elementERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
xmlwriter => rdi
name => rsi
content => rdx
*/

bool fh_xmlwriter_write_element(Value* xmlwriter, Value* name, Value* content) asm("_ZN4HPHP25f_xmlwriter_write_elementERKNS_6ObjectERKNS_6StringES5_");

TypedValue * fg1_xmlwriter_write_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  rv->m_data.num = (fh_xmlwriter_write_element((Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_element(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_element((Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_element(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_element", count, 2, 3, 1);
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
bool HPHP::f_xmlwriter_end_element(HPHP::Object const&)
_ZN4HPHP23f_xmlwriter_end_elementERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_element(Value* xmlwriter) asm("_ZN4HPHP23f_xmlwriter_end_elementERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_end_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_end_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_end_element((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_end_element(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_end_element((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_end_element(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_end_element", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_full_end_element(HPHP::Object const&)
_ZN4HPHP28f_xmlwriter_full_end_elementERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_full_end_element(Value* xmlwriter) asm("_ZN4HPHP28f_xmlwriter_full_end_elementERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_full_end_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_full_end_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_full_end_element((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_full_end_element(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_full_end_element((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_full_end_element(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_full_end_element", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_start_attribute_ns(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP30f_xmlwriter_start_attribute_nsERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
xmlwriter => rdi
prefix => rsi
name => rdx
uri => rcx
*/

bool fh_xmlwriter_start_attribute_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri) asm("_ZN4HPHP30f_xmlwriter_start_attribute_nsERKNS_6ObjectERKNS_6StringES5_S5_");

TypedValue * fg1_xmlwriter_start_attribute_ns(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_attribute_ns(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  rv->m_data.num = (fh_xmlwriter_start_attribute_ns((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_attribute_ns(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_attribute_ns((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_attribute_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_attribute_ns", count, 4, 4, 1);
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
bool HPHP::f_xmlwriter_start_attribute(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP27f_xmlwriter_start_attributeERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
name => rsi
*/

bool fh_xmlwriter_start_attribute(Value* xmlwriter, Value* name) asm("_ZN4HPHP27f_xmlwriter_start_attributeERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_xmlwriter_start_attribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_attribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_start_attribute((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_attribute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_attribute((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_attribute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_attribute", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_write_attribute_ns(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP30f_xmlwriter_write_attribute_nsERKNS_6ObjectERKNS_6StringES5_S5_S5_

(return value) => rax
xmlwriter => rdi
prefix => rsi
name => rdx
uri => rcx
content => r8
*/

bool fh_xmlwriter_write_attribute_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri, Value* content) asm("_ZN4HPHP30f_xmlwriter_write_attribute_nsERKNS_6ObjectERKNS_6StringES5_S5_S5_");

TypedValue * fg1_xmlwriter_write_attribute_ns(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_attribute_ns(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-4)->m_type)) {
    tvCastToStringInPlace(args-4);
  }
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
  rv->m_data.num = (fh_xmlwriter_write_attribute_ns((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3), (Value*)(args-4))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_attribute_ns(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if (IS_STRING_TYPE((args-4)->m_type) && IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_attribute_ns((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3), (Value*)(args-4))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_attribute_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_attribute_ns", count, 5, 5, 1);
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
bool HPHP::f_xmlwriter_write_attribute(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP27f_xmlwriter_write_attributeERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
xmlwriter => rdi
name => rsi
value => rdx
*/

bool fh_xmlwriter_write_attribute(Value* xmlwriter, Value* name, Value* value) asm("_ZN4HPHP27f_xmlwriter_write_attributeERKNS_6ObjectERKNS_6StringES5_");

TypedValue * fg1_xmlwriter_write_attribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_attribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  rv->m_data.num = (fh_xmlwriter_write_attribute((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_attribute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_attribute((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_attribute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_attribute", count, 3, 3, 1);
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
bool HPHP::f_xmlwriter_end_attribute(HPHP::Object const&)
_ZN4HPHP25f_xmlwriter_end_attributeERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_attribute(Value* xmlwriter) asm("_ZN4HPHP25f_xmlwriter_end_attributeERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_end_attribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_end_attribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_end_attribute((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_end_attribute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_end_attribute((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_end_attribute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_end_attribute", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_start_cdata(HPHP::Object const&)
_ZN4HPHP23f_xmlwriter_start_cdataERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_start_cdata(Value* xmlwriter) asm("_ZN4HPHP23f_xmlwriter_start_cdataERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_start_cdata(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_cdata(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_start_cdata((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_cdata(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_cdata((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_cdata(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_cdata", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_write_cdata(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP23f_xmlwriter_write_cdataERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
content => rsi
*/

bool fh_xmlwriter_write_cdata(Value* xmlwriter, Value* content) asm("_ZN4HPHP23f_xmlwriter_write_cdataERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_xmlwriter_write_cdata(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_cdata(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_write_cdata((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_cdata(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_cdata((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_cdata(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_cdata", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_end_cdata(HPHP::Object const&)
_ZN4HPHP21f_xmlwriter_end_cdataERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_cdata(Value* xmlwriter) asm("_ZN4HPHP21f_xmlwriter_end_cdataERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_end_cdata(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_end_cdata(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_end_cdata((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_end_cdata(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_end_cdata((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_end_cdata(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_end_cdata", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_start_comment(HPHP::Object const&)
_ZN4HPHP25f_xmlwriter_start_commentERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_start_comment(Value* xmlwriter) asm("_ZN4HPHP25f_xmlwriter_start_commentERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_start_comment(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_comment(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_start_comment((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_comment(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_comment((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_comment(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_comment", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_write_comment(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP25f_xmlwriter_write_commentERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
content => rsi
*/

bool fh_xmlwriter_write_comment(Value* xmlwriter, Value* content) asm("_ZN4HPHP25f_xmlwriter_write_commentERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_xmlwriter_write_comment(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_comment(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_write_comment((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_comment(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_comment((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_comment(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_comment", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_end_comment(HPHP::Object const&)
_ZN4HPHP23f_xmlwriter_end_commentERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_comment(Value* xmlwriter) asm("_ZN4HPHP23f_xmlwriter_end_commentERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_end_comment(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_end_comment(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_end_comment((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_end_comment(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_end_comment((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_end_comment(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_end_comment", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_end_document(HPHP::Object const&)
_ZN4HPHP24f_xmlwriter_end_documentERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_document(Value* xmlwriter) asm("_ZN4HPHP24f_xmlwriter_end_documentERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_end_document(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_end_document(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_end_document((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_end_document(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_end_document((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_end_document(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_end_document", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_start_pi(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP20f_xmlwriter_start_piERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
target => rsi
*/

bool fh_xmlwriter_start_pi(Value* xmlwriter, Value* target) asm("_ZN4HPHP20f_xmlwriter_start_piERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_xmlwriter_start_pi(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_pi(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_start_pi((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_pi(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_pi((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_pi(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_pi", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_write_pi(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_xmlwriter_write_piERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
xmlwriter => rdi
target => rsi
content => rdx
*/

bool fh_xmlwriter_write_pi(Value* xmlwriter, Value* target, Value* content) asm("_ZN4HPHP20f_xmlwriter_write_piERKNS_6ObjectERKNS_6StringES5_");

TypedValue * fg1_xmlwriter_write_pi(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_pi(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  rv->m_data.num = (fh_xmlwriter_write_pi((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_pi(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_pi((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_pi(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_pi", count, 3, 3, 1);
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
bool HPHP::f_xmlwriter_end_pi(HPHP::Object const&)
_ZN4HPHP18f_xmlwriter_end_piERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_pi(Value* xmlwriter) asm("_ZN4HPHP18f_xmlwriter_end_piERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_end_pi(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_end_pi(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_end_pi((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_end_pi(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_end_pi((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_end_pi(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_end_pi", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_text(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP16f_xmlwriter_textERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
content => rsi
*/

bool fh_xmlwriter_text(Value* xmlwriter, Value* content) asm("_ZN4HPHP16f_xmlwriter_textERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_xmlwriter_text(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_text(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_text((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_text(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_text((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_text(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_text", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_write_raw(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21f_xmlwriter_write_rawERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
content => rsi
*/

bool fh_xmlwriter_write_raw(Value* xmlwriter, Value* content) asm("_ZN4HPHP21f_xmlwriter_write_rawERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_xmlwriter_write_raw(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_raw(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_write_raw((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_raw(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_raw((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_raw(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_raw", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_start_dtd(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_xmlwriter_start_dtdERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
xmlwriter => rdi
qualifiedname => rsi
publicid => rdx
systemid => rcx
*/

bool fh_xmlwriter_start_dtd(Value* xmlwriter, Value* qualifiedname, Value* publicid, Value* systemid) asm("_ZN4HPHP21f_xmlwriter_start_dtdERKNS_6ObjectERKNS_6StringES5_S5_");

TypedValue * fg1_xmlwriter_start_dtd(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_dtd(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_start_dtd((Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_dtd(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_dtd((Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_dtd(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_dtd", count, 2, 4, 1);
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
bool HPHP::f_xmlwriter_write_dtd(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_xmlwriter_write_dtdERKNS_6ObjectERKNS_6StringES5_S5_S5_

(return value) => rax
xmlwriter => rdi
name => rsi
publicid => rdx
systemid => rcx
subset => r8
*/

bool fh_xmlwriter_write_dtd(Value* xmlwriter, Value* name, Value* publicid, Value* systemid, Value* subset) asm("_ZN4HPHP21f_xmlwriter_write_dtdERKNS_6ObjectERKNS_6StringES5_S5_S5_");

TypedValue * fg1_xmlwriter_write_dtd(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_dtd(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  rv->m_data.num = (fh_xmlwriter_write_dtd((Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_dtd(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 5LL) {
      if ((count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_dtd((Value*)(args-0), (Value*)(args-1), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_dtd(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_dtd", count, 2, 5, 1);
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
bool HPHP::f_xmlwriter_start_dtd_element(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP29f_xmlwriter_start_dtd_elementERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
qualifiedname => rsi
*/

bool fh_xmlwriter_start_dtd_element(Value* xmlwriter, Value* qualifiedname) asm("_ZN4HPHP29f_xmlwriter_start_dtd_elementERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_xmlwriter_start_dtd_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_dtd_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_start_dtd_element((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_dtd_element(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_dtd_element((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_dtd_element(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_dtd_element", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_write_dtd_element(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP29f_xmlwriter_write_dtd_elementERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
xmlwriter => rdi
name => rsi
content => rdx
*/

bool fh_xmlwriter_write_dtd_element(Value* xmlwriter, Value* name, Value* content) asm("_ZN4HPHP29f_xmlwriter_write_dtd_elementERKNS_6ObjectERKNS_6StringES5_");

TypedValue * fg1_xmlwriter_write_dtd_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_dtd_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  rv->m_data.num = (fh_xmlwriter_write_dtd_element((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_dtd_element(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_dtd_element((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_dtd_element(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_dtd_element", count, 3, 3, 1);
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
bool HPHP::f_xmlwriter_end_dtd_element(HPHP::Object const&)
_ZN4HPHP27f_xmlwriter_end_dtd_elementERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_dtd_element(Value* xmlwriter) asm("_ZN4HPHP27f_xmlwriter_end_dtd_elementERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_end_dtd_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_end_dtd_element(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_end_dtd_element((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_end_dtd_element(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_end_dtd_element((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_end_dtd_element(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_end_dtd_element", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_start_dtd_attlist(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP29f_xmlwriter_start_dtd_attlistERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
name => rsi
*/

bool fh_xmlwriter_start_dtd_attlist(Value* xmlwriter, Value* name) asm("_ZN4HPHP29f_xmlwriter_start_dtd_attlistERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_xmlwriter_start_dtd_attlist(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_dtd_attlist(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_start_dtd_attlist((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_dtd_attlist(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_dtd_attlist((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_dtd_attlist(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_dtd_attlist", count, 2, 2, 1);
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
bool HPHP::f_xmlwriter_write_dtd_attlist(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP29f_xmlwriter_write_dtd_attlistERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
xmlwriter => rdi
name => rsi
content => rdx
*/

bool fh_xmlwriter_write_dtd_attlist(Value* xmlwriter, Value* name, Value* content) asm("_ZN4HPHP29f_xmlwriter_write_dtd_attlistERKNS_6ObjectERKNS_6StringES5_");

TypedValue * fg1_xmlwriter_write_dtd_attlist(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_dtd_attlist(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  rv->m_data.num = (fh_xmlwriter_write_dtd_attlist((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_dtd_attlist(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_dtd_attlist((Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_dtd_attlist(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_dtd_attlist", count, 3, 3, 1);
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
bool HPHP::f_xmlwriter_end_dtd_attlist(HPHP::Object const&)
_ZN4HPHP27f_xmlwriter_end_dtd_attlistERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_dtd_attlist(Value* xmlwriter) asm("_ZN4HPHP27f_xmlwriter_end_dtd_attlistERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_end_dtd_attlist(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_end_dtd_attlist(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_end_dtd_attlist((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_end_dtd_attlist(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_end_dtd_attlist((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_end_dtd_attlist(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_end_dtd_attlist", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_start_dtd_entity(HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP28f_xmlwriter_start_dtd_entityERKNS_6ObjectERKNS_6StringEb

(return value) => rax
xmlwriter => rdi
name => rsi
isparam => rdx
*/

bool fh_xmlwriter_start_dtd_entity(Value* xmlwriter, Value* name, bool isparam) asm("_ZN4HPHP28f_xmlwriter_start_dtd_entityERKNS_6ObjectERKNS_6StringEb");

TypedValue * fg1_xmlwriter_start_dtd_entity(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_start_dtd_entity(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  rv->m_data.num = (fh_xmlwriter_start_dtd_entity((Value*)(args-0), (Value*)(args-1), (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_start_dtd_entity(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfBoolean && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_start_dtd_entity((Value*)(args-0), (Value*)(args-1), (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_start_dtd_entity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_start_dtd_entity", count, 3, 3, 1);
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
bool HPHP::f_xmlwriter_write_dtd_entity(HPHP::Object const&, HPHP::String const&, HPHP::String const&, bool, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP28f_xmlwriter_write_dtd_entityERKNS_6ObjectERKNS_6StringES5_bS5_S5_S5_

(return value) => rax
xmlwriter => rdi
name => rsi
content => rdx
pe => rcx
publicid => r8
systemid => r9
ndataid => st0
*/

bool fh_xmlwriter_write_dtd_entity(Value* xmlwriter, Value* name, Value* content, bool pe, Value* publicid, Value* systemid, Value* ndataid) asm("_ZN4HPHP28f_xmlwriter_write_dtd_entityERKNS_6ObjectERKNS_6StringES5_bS5_S5_S5_");

TypedValue * fg1_xmlwriter_write_dtd_entity(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_write_dtd_entity(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 7
    if (!IS_STRING_TYPE((args-6)->m_type)) {
      tvCastToStringInPlace(args-6);
    }
  case 6:
    if (!IS_STRING_TYPE((args-5)->m_type)) {
      tvCastToStringInPlace(args-5);
    }
  case 5:
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_xmlwriter_write_dtd_entity((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string), (count > 5) ? (Value*)(args-5) : (Value*)(&null_string), (count > 6) ? (Value*)(args-6) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_write_dtd_entity(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 7LL) {
      if ((count <= 6 || IS_STRING_TYPE((args-6)->m_type)) && (count <= 5 || IS_STRING_TYPE((args-5)->m_type)) && (count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && (count <= 3 || (args-3)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_write_dtd_entity((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string), (count > 5) ? (Value*)(args-5) : (Value*)(&null_string), (count > 6) ? (Value*)(args-6) : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_write_dtd_entity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_write_dtd_entity", count, 3, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_xmlwriter_end_dtd_entity(HPHP::Object const&)
_ZN4HPHP26f_xmlwriter_end_dtd_entityERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_dtd_entity(Value* xmlwriter) asm("_ZN4HPHP26f_xmlwriter_end_dtd_entityERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_end_dtd_entity(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_end_dtd_entity(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_end_dtd_entity((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_end_dtd_entity(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_end_dtd_entity((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_end_dtd_entity(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_end_dtd_entity", count, 1, 1, 1);
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
bool HPHP::f_xmlwriter_end_dtd(HPHP::Object const&)
_ZN4HPHP19f_xmlwriter_end_dtdERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_dtd(Value* xmlwriter) asm("_ZN4HPHP19f_xmlwriter_end_dtdERKNS_6ObjectE");

TypedValue * fg1_xmlwriter_end_dtd(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_end_dtd(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_xmlwriter_end_dtd((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_xmlwriter_end_dtd(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_xmlwriter_end_dtd((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_end_dtd(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_end_dtd", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_xmlwriter_flush(HPHP::Object const&, bool)
_ZN4HPHP17f_xmlwriter_flushERKNS_6ObjectEb

(return value) => rax
_rv => rdi
xmlwriter => rsi
empty => rdx
*/

TypedValue* fh_xmlwriter_flush(TypedValue* _rv, Value* xmlwriter, bool empty) asm("_ZN4HPHP17f_xmlwriter_flushERKNS_6ObjectEb");

TypedValue * fg1_xmlwriter_flush(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_flush(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  fh_xmlwriter_flush((rv), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_xmlwriter_flush(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfObject) {
        fh_xmlwriter_flush((&(rv)), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_flush(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_flush", count, 1, 2, 1);
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
HPHP::String HPHP::f_xmlwriter_output_memory(HPHP::Object const&, bool)
_ZN4HPHP25f_xmlwriter_output_memoryERKNS_6ObjectEb

(return value) => rax
_rv => rdi
xmlwriter => rsi
flush => rdx
*/

Value* fh_xmlwriter_output_memory(Value* _rv, Value* xmlwriter, bool flush) asm("_ZN4HPHP25f_xmlwriter_output_memoryERKNS_6ObjectEb");

TypedValue * fg1_xmlwriter_output_memory(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_xmlwriter_output_memory(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
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
  fh_xmlwriter_output_memory((Value*)(rv), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_xmlwriter_output_memory(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_xmlwriter_output_memory((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_xmlwriter_output_memory(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("xmlwriter_output_memory", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



HPHP::VM::Instance* new_XMLWriter_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_XMLWriter) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_XMLWriter(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_XMLWriter::t___construct()
_ZN4HPHP11c_XMLWriter13t___constructEv

this_ => rdi
*/

void th_9XMLWriter___construct(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter13t___constructEv");

TypedValue* tg_9XMLWriter___construct(HPHP::VM::ActRec *ar) {
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
        th_9XMLWriter___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::__construct");
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
bool HPHP::c_XMLWriter::t_openmemory()
_ZN4HPHP11c_XMLWriter12t_openmemoryEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_openMemory(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter12t_openmemoryEv");

TypedValue* tg_9XMLWriter_openMemory(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_openMemory((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::openMemory", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::openMemory");
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
bool HPHP::c_XMLWriter::t_openuri(HPHP::String const&)
_ZN4HPHP11c_XMLWriter9t_openuriERKNS_6StringE

(return value) => rax
this_ => rdi
uri => rsi
*/

bool th_9XMLWriter_openURI(ObjectData* this_, Value* uri) asm("_ZN4HPHP11c_XMLWriter9t_openuriERKNS_6StringE");

TypedValue* tg1_9XMLWriter_openURI(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_openURI(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_openURI((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_openURI(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_openURI((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_openURI(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::openURI", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::openURI");
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
bool HPHP::c_XMLWriter::t_setindentstring(HPHP::String const&)
_ZN4HPHP11c_XMLWriter17t_setindentstringERKNS_6StringE

(return value) => rax
this_ => rdi
indentstring => rsi
*/

bool th_9XMLWriter_setIndentString(ObjectData* this_, Value* indentstring) asm("_ZN4HPHP11c_XMLWriter17t_setindentstringERKNS_6StringE");

TypedValue* tg1_9XMLWriter_setIndentString(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_setIndentString(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_setIndentString((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_setIndentString(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_setIndentString((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_setIndentString(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::setIndentString", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::setIndentString");
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
bool HPHP::c_XMLWriter::t_setindent(bool)
_ZN4HPHP11c_XMLWriter11t_setindentEb

(return value) => rax
this_ => rdi
indent => rsi
*/

bool th_9XMLWriter_setIndent(ObjectData* this_, bool indent) asm("_ZN4HPHP11c_XMLWriter11t_setindentEb");

TypedValue* tg1_9XMLWriter_setIndent(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_setIndent(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToBooleanInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_setIndent((this_), (bool)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_setIndent(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfBoolean) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_setIndent((this_), (bool)(args[-0].m_data.num))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_setIndent(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::setIndent", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::setIndent");
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
bool HPHP::c_XMLWriter::t_startdocument(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter15t_startdocumentERKNS_6StringES3_S3_

(return value) => rax
this_ => rdi
version => rsi
encoding => rdx
standalone => rcx
*/

bool th_9XMLWriter_startDocument(ObjectData* this_, Value* version, Value* encoding, Value* standalone) asm("_ZN4HPHP11c_XMLWriter15t_startdocumentERKNS_6StringES3_S3_");

TypedValue* tg1_9XMLWriter_startDocument(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_startDocument(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  String defVal0 = "1.0";
  rv->m_data.num = (th_9XMLWriter_startDocument((this_), (count > 0) ? (Value*)(args-0) : (Value*)(&defVal0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_startDocument(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 3LL) {
        if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          String defVal0 = "1.0";
          rv.m_data.num = (th_9XMLWriter_startDocument((this_), (count > 0) ? (Value*)(args-0) : (Value*)(&defVal0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_startDocument(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("XMLWriter::startDocument", 3, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startDocument");
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
bool HPHP::c_XMLWriter::t_startelement(HPHP::String const&)
_ZN4HPHP11c_XMLWriter14t_startelementERKNS_6StringE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_9XMLWriter_startElement(ObjectData* this_, Value* name) asm("_ZN4HPHP11c_XMLWriter14t_startelementERKNS_6StringE");

TypedValue* tg1_9XMLWriter_startElement(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_startElement(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_startElement((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_startElement(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_startElement((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_startElement(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::startElement", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startElement");
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
bool HPHP::c_XMLWriter::t_startelementns(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter16t_startelementnsERKNS_6StringES3_S3_

(return value) => rax
this_ => rdi
prefix => rsi
name => rdx
uri => rcx
*/

bool th_9XMLWriter_startElementNS(ObjectData* this_, Value* prefix, Value* name, Value* uri) asm("_ZN4HPHP11c_XMLWriter16t_startelementnsERKNS_6StringES3_S3_");

TypedValue* tg1_9XMLWriter_startElementNS(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_startElementNS(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_startElementNS((this_), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_startElementNS(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 3LL) {
        if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_startElementNS((this_), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_startElementNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::startElementNS", count, 3, 3, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startElementNS");
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
bool HPHP::c_XMLWriter::t_writeelementns(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter16t_writeelementnsERKNS_6StringES3_S3_S3_

(return value) => rax
this_ => rdi
prefix => rsi
name => rdx
uri => rcx
content => r8
*/

bool th_9XMLWriter_writeElementNS(ObjectData* this_, Value* prefix, Value* name, Value* uri, Value* content) asm("_ZN4HPHP11c_XMLWriter16t_writeelementnsERKNS_6StringES3_S3_S3_");

TypedValue* tg1_9XMLWriter_writeElementNS(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeElementNS(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
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
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_writeElementNS((this_), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeElementNS(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 3LL && count <= 4LL) {
        if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_writeElementNS((this_), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeElementNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeElementNS", count, 3, 4, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeElementNS");
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
bool HPHP::c_XMLWriter::t_writeelement(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter14t_writeelementERKNS_6StringES3_

(return value) => rax
this_ => rdi
name => rsi
content => rdx
*/

bool th_9XMLWriter_writeElement(ObjectData* this_, Value* name, Value* content) asm("_ZN4HPHP11c_XMLWriter14t_writeelementERKNS_6StringES3_");

TypedValue* tg1_9XMLWriter_writeElement(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeElement(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_writeElement((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeElement(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_writeElement((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeElement(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeElement", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeElement");
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
bool HPHP::c_XMLWriter::t_endelement()
_ZN4HPHP11c_XMLWriter12t_endelementEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_endElement(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter12t_endelementEv");

TypedValue* tg_9XMLWriter_endElement(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_endElement((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::endElement", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::endElement");
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
bool HPHP::c_XMLWriter::t_fullendelement()
_ZN4HPHP11c_XMLWriter16t_fullendelementEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_fullEndElement(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter16t_fullendelementEv");

TypedValue* tg_9XMLWriter_fullEndElement(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_fullEndElement((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::fullEndElement", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::fullEndElement");
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
bool HPHP::c_XMLWriter::t_startattributens(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter18t_startattributensERKNS_6StringES3_S3_

(return value) => rax
this_ => rdi
prefix => rsi
name => rdx
uri => rcx
*/

bool th_9XMLWriter_startAttributens(ObjectData* this_, Value* prefix, Value* name, Value* uri) asm("_ZN4HPHP11c_XMLWriter18t_startattributensERKNS_6StringES3_S3_");

TypedValue* tg1_9XMLWriter_startAttributens(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_startAttributens(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_startAttributens((this_), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_startAttributens(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 3LL) {
        if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_startAttributens((this_), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_startAttributens(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::startAttributens", count, 3, 3, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startAttributens");
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
bool HPHP::c_XMLWriter::t_startattribute(HPHP::String const&)
_ZN4HPHP11c_XMLWriter16t_startattributeERKNS_6StringE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_9XMLWriter_startAttribute(ObjectData* this_, Value* name) asm("_ZN4HPHP11c_XMLWriter16t_startattributeERKNS_6StringE");

TypedValue* tg1_9XMLWriter_startAttribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_startAttribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_startAttribute((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_startAttribute(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_startAttribute((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_startAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::startAttribute", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startAttribute");
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
bool HPHP::c_XMLWriter::t_writeattributens(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter18t_writeattributensERKNS_6StringES3_S3_S3_

(return value) => rax
this_ => rdi
prefix => rsi
name => rdx
uri => rcx
content => r8
*/

bool th_9XMLWriter_writeAttributeNS(ObjectData* this_, Value* prefix, Value* name, Value* uri, Value* content) asm("_ZN4HPHP11c_XMLWriter18t_writeattributensERKNS_6StringES3_S3_S3_");

TypedValue* tg1_9XMLWriter_writeAttributeNS(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeAttributeNS(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_writeAttributeNS((this_), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeAttributeNS(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 4LL) {
        if (IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_writeAttributeNS((this_), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeAttributeNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeAttributeNS", count, 4, 4, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeAttributeNS");
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
bool HPHP::c_XMLWriter::t_writeattribute(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter16t_writeattributeERKNS_6StringES3_

(return value) => rax
this_ => rdi
name => rsi
value => rdx
*/

bool th_9XMLWriter_writeAttribute(ObjectData* this_, Value* name, Value* value) asm("_ZN4HPHP11c_XMLWriter16t_writeattributeERKNS_6StringES3_");

TypedValue* tg1_9XMLWriter_writeAttribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeAttribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_writeAttribute((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeAttribute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_writeAttribute((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeAttribute", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeAttribute");
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
bool HPHP::c_XMLWriter::t_endattribute()
_ZN4HPHP11c_XMLWriter14t_endattributeEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_endAttribute(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter14t_endattributeEv");

TypedValue* tg_9XMLWriter_endAttribute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_endAttribute((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::endAttribute", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::endAttribute");
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
bool HPHP::c_XMLWriter::t_startcdata()
_ZN4HPHP11c_XMLWriter12t_startcdataEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_startCData(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter12t_startcdataEv");

TypedValue* tg_9XMLWriter_startCData(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_startCData((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::startCData", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startCData");
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
bool HPHP::c_XMLWriter::t_writecdata(HPHP::String const&)
_ZN4HPHP11c_XMLWriter12t_writecdataERKNS_6StringE

(return value) => rax
this_ => rdi
content => rsi
*/

bool th_9XMLWriter_writeCData(ObjectData* this_, Value* content) asm("_ZN4HPHP11c_XMLWriter12t_writecdataERKNS_6StringE");

TypedValue* tg1_9XMLWriter_writeCData(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeCData(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_writeCData((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeCData(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_writeCData((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeCData(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeCData", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeCData");
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
bool HPHP::c_XMLWriter::t_endcdata()
_ZN4HPHP11c_XMLWriter10t_endcdataEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_endCData(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter10t_endcdataEv");

TypedValue* tg_9XMLWriter_endCData(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_endCData((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::endCData", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::endCData");
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
bool HPHP::c_XMLWriter::t_startcomment()
_ZN4HPHP11c_XMLWriter14t_startcommentEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_startComment(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter14t_startcommentEv");

TypedValue* tg_9XMLWriter_startComment(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_startComment((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::startComment", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startComment");
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
bool HPHP::c_XMLWriter::t_writecomment(HPHP::String const&)
_ZN4HPHP11c_XMLWriter14t_writecommentERKNS_6StringE

(return value) => rax
this_ => rdi
content => rsi
*/

bool th_9XMLWriter_writeComment(ObjectData* this_, Value* content) asm("_ZN4HPHP11c_XMLWriter14t_writecommentERKNS_6StringE");

TypedValue* tg1_9XMLWriter_writeComment(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeComment(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_writeComment((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeComment(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_writeComment((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeComment(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeComment", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeComment");
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
bool HPHP::c_XMLWriter::t_endcomment()
_ZN4HPHP11c_XMLWriter12t_endcommentEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_endComment(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter12t_endcommentEv");

TypedValue* tg_9XMLWriter_endComment(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_endComment((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::endComment", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::endComment");
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
bool HPHP::c_XMLWriter::t_enddocument()
_ZN4HPHP11c_XMLWriter13t_enddocumentEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_endDocument(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter13t_enddocumentEv");

TypedValue* tg_9XMLWriter_endDocument(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_endDocument((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::endDocument", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::endDocument");
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
bool HPHP::c_XMLWriter::t_startpi(HPHP::String const&)
_ZN4HPHP11c_XMLWriter9t_startpiERKNS_6StringE

(return value) => rax
this_ => rdi
target => rsi
*/

bool th_9XMLWriter_startPI(ObjectData* this_, Value* target) asm("_ZN4HPHP11c_XMLWriter9t_startpiERKNS_6StringE");

TypedValue* tg1_9XMLWriter_startPI(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_startPI(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_startPI((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_startPI(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_startPI((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_startPI(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::startPI", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startPI");
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
bool HPHP::c_XMLWriter::t_writepi(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter9t_writepiERKNS_6StringES3_

(return value) => rax
this_ => rdi
target => rsi
content => rdx
*/

bool th_9XMLWriter_writePI(ObjectData* this_, Value* target, Value* content) asm("_ZN4HPHP11c_XMLWriter9t_writepiERKNS_6StringES3_");

TypedValue* tg1_9XMLWriter_writePI(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writePI(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_writePI((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writePI(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_writePI((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writePI(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writePI", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writePI");
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
bool HPHP::c_XMLWriter::t_endpi()
_ZN4HPHP11c_XMLWriter7t_endpiEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_endPI(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter7t_endpiEv");

TypedValue* tg_9XMLWriter_endPI(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_endPI((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::endPI", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::endPI");
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
bool HPHP::c_XMLWriter::t_text(HPHP::String const&)
_ZN4HPHP11c_XMLWriter6t_textERKNS_6StringE

(return value) => rax
this_ => rdi
content => rsi
*/

bool th_9XMLWriter_text(ObjectData* this_, Value* content) asm("_ZN4HPHP11c_XMLWriter6t_textERKNS_6StringE");

TypedValue* tg1_9XMLWriter_text(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_text(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_text((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_text(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_text((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_text(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::text", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::text");
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
bool HPHP::c_XMLWriter::t_writeraw(HPHP::String const&)
_ZN4HPHP11c_XMLWriter10t_writerawERKNS_6StringE

(return value) => rax
this_ => rdi
content => rsi
*/

bool th_9XMLWriter_writeRaw(ObjectData* this_, Value* content) asm("_ZN4HPHP11c_XMLWriter10t_writerawERKNS_6StringE");

TypedValue* tg1_9XMLWriter_writeRaw(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeRaw(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_writeRaw((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeRaw(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_writeRaw((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeRaw(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeRaw", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeRaw");
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
bool HPHP::c_XMLWriter::t_startdtd(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter10t_startdtdERKNS_6StringES3_S3_

(return value) => rax
this_ => rdi
qualifiedname => rsi
publicid => rdx
systemid => rcx
*/

bool th_9XMLWriter_startDTD(ObjectData* this_, Value* qualifiedname, Value* publicid, Value* systemid) asm("_ZN4HPHP11c_XMLWriter10t_startdtdERKNS_6StringES3_S3_");

TypedValue* tg1_9XMLWriter_startDTD(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_startDTD(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_startDTD((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_startDTD(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_startDTD((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_startDTD(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::startDTD", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startDTD");
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
bool HPHP::c_XMLWriter::t_writedtd(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter10t_writedtdERKNS_6StringES3_S3_S3_

(return value) => rax
this_ => rdi
name => rsi
publicid => rdx
systemid => rcx
subset => r8
*/

bool th_9XMLWriter_writeDTD(ObjectData* this_, Value* name, Value* publicid, Value* systemid, Value* subset) asm("_ZN4HPHP11c_XMLWriter10t_writedtdERKNS_6StringES3_S3_S3_");

TypedValue* tg1_9XMLWriter_writeDTD(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeDTD(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_writeDTD((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeDTD(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 4LL) {
        if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_writeDTD((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeDTD(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeDTD", count, 1, 4, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeDTD");
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
bool HPHP::c_XMLWriter::t_startdtdelement(HPHP::String const&)
_ZN4HPHP11c_XMLWriter17t_startdtdelementERKNS_6StringE

(return value) => rax
this_ => rdi
qualifiedname => rsi
*/

bool th_9XMLWriter_startDTDElement(ObjectData* this_, Value* qualifiedname) asm("_ZN4HPHP11c_XMLWriter17t_startdtdelementERKNS_6StringE");

TypedValue* tg1_9XMLWriter_startDTDElement(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_startDTDElement(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_startDTDElement((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_startDTDElement(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_startDTDElement((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_startDTDElement(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::startDTDElement", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startDTDElement");
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
bool HPHP::c_XMLWriter::t_writedtdelement(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter17t_writedtdelementERKNS_6StringES3_

(return value) => rax
this_ => rdi
name => rsi
content => rdx
*/

bool th_9XMLWriter_writeDTDElement(ObjectData* this_, Value* name, Value* content) asm("_ZN4HPHP11c_XMLWriter17t_writedtdelementERKNS_6StringES3_");

TypedValue* tg1_9XMLWriter_writeDTDElement(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeDTDElement(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_writeDTDElement((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeDTDElement(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_writeDTDElement((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeDTDElement(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeDTDElement", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeDTDElement");
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
bool HPHP::c_XMLWriter::t_enddtdelement()
_ZN4HPHP11c_XMLWriter15t_enddtdelementEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_endDTDElement(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter15t_enddtdelementEv");

TypedValue* tg_9XMLWriter_endDTDElement(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_endDTDElement((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::endDTDElement", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::endDTDElement");
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
bool HPHP::c_XMLWriter::t_startdtdattlist(HPHP::String const&)
_ZN4HPHP11c_XMLWriter17t_startdtdattlistERKNS_6StringE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_9XMLWriter_startDTDAttlist(ObjectData* this_, Value* name) asm("_ZN4HPHP11c_XMLWriter17t_startdtdattlistERKNS_6StringE");

TypedValue* tg1_9XMLWriter_startDTDAttlist(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_startDTDAttlist(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLWriter_startDTDAttlist((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_startDTDAttlist(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLWriter_startDTDAttlist((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_startDTDAttlist(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::startDTDAttlist", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startDTDAttlist");
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
bool HPHP::c_XMLWriter::t_writedtdattlist(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter17t_writedtdattlistERKNS_6StringES3_

(return value) => rax
this_ => rdi
name => rsi
content => rdx
*/

bool th_9XMLWriter_writeDTDAttlist(ObjectData* this_, Value* name, Value* content) asm("_ZN4HPHP11c_XMLWriter17t_writedtdattlistERKNS_6StringES3_");

TypedValue* tg1_9XMLWriter_writeDTDAttlist(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeDTDAttlist(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_writeDTDAttlist((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeDTDAttlist(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_writeDTDAttlist((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeDTDAttlist(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeDTDAttlist", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeDTDAttlist");
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
bool HPHP::c_XMLWriter::t_enddtdattlist()
_ZN4HPHP11c_XMLWriter15t_enddtdattlistEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_endDTDAttlist(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter15t_enddtdattlistEv");

TypedValue* tg_9XMLWriter_endDTDAttlist(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_endDTDAttlist((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::endDTDAttlist", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::endDTDAttlist");
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
bool HPHP::c_XMLWriter::t_startdtdentity(HPHP::String const&, bool)
_ZN4HPHP11c_XMLWriter16t_startdtdentityERKNS_6StringEb

(return value) => rax
this_ => rdi
name => rsi
isparam => rdx
*/

bool th_9XMLWriter_startDTDEntity(ObjectData* this_, Value* name, bool isparam) asm("_ZN4HPHP11c_XMLWriter16t_startdtdentityERKNS_6StringEb");

TypedValue* tg1_9XMLWriter_startDTDEntity(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_startDTDEntity(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLWriter_startDTDEntity((this_), (Value*)(args-0), (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_startDTDEntity(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-1)->m_type == KindOfBoolean && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_startDTDEntity((this_), (Value*)(args-0), (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_startDTDEntity(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::startDTDEntity", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::startDTDEntity");
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
bool HPHP::c_XMLWriter::t_writedtdentity(HPHP::String const&, HPHP::String const&, bool, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLWriter16t_writedtdentityERKNS_6StringES3_bS3_S3_S3_

(return value) => rax
this_ => rdi
name => rsi
content => rdx
pe => rcx
publicid => r8
systemid => r9
ndataid => st0
*/

bool th_9XMLWriter_writeDTDEntity(ObjectData* this_, Value* name, Value* content, bool pe, Value* publicid, Value* systemid, Value* ndataid) asm("_ZN4HPHP11c_XMLWriter16t_writedtdentityERKNS_6StringES3_bS3_S3_S3_");

TypedValue* tg1_9XMLWriter_writeDTDEntity(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_writeDTDEntity(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 6
    if (!IS_STRING_TYPE((args-5)->m_type)) {
      tvCastToStringInPlace(args-5);
    }
  case 5:
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
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
  rv->m_data.num = (th_9XMLWriter_writeDTDEntity((this_), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string), (count > 5) ? (Value*)(args-5) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLWriter_writeDTDEntity(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 6LL) {
        if ((count <= 5 || IS_STRING_TYPE((args-5)->m_type)) && (count <= 4 || IS_STRING_TYPE((args-4)->m_type)) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || (args-2)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLWriter_writeDTDEntity((this_), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (Value*)(args-4) : (Value*)(&null_string), (count > 5) ? (Value*)(args-5) : (Value*)(&null_string))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 6);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_writeDTDEntity(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 6);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLWriter::writeDTDEntity", count, 2, 6, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::writeDTDEntity");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_XMLWriter::t_enddtdentity()
_ZN4HPHP11c_XMLWriter14t_enddtdentityEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_endDTDEntity(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter14t_enddtdentityEv");

TypedValue* tg_9XMLWriter_endDTDEntity(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_endDTDEntity((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::endDTDEntity", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::endDTDEntity");
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
bool HPHP::c_XMLWriter::t_enddtd()
_ZN4HPHP11c_XMLWriter8t_enddtdEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLWriter_endDTD(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter8t_enddtdEv");

TypedValue* tg_9XMLWriter_endDTD(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLWriter_endDTD((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::endDTD", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::endDTD");
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
HPHP::Variant HPHP::c_XMLWriter::t_flush(bool)
_ZN4HPHP11c_XMLWriter7t_flushEb

(return value) => rax
_rv => rdi
this_ => rsi
empty => rdx
*/

TypedValue* th_9XMLWriter_flush(TypedValue* _rv, ObjectData* this_, bool empty) asm("_ZN4HPHP11c_XMLWriter7t_flushEb");

TypedValue* tg1_9XMLWriter_flush(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_flush(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  th_9XMLWriter_flush((rv), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9XMLWriter_flush(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
          th_9XMLWriter_flush((&(rv)), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_flush(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("XMLWriter::flush", 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::flush");
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
HPHP::String HPHP::c_XMLWriter::t_outputmemory(bool)
_ZN4HPHP11c_XMLWriter14t_outputmemoryEb

(return value) => rax
_rv => rdi
this_ => rsi
flush => rdx
*/

Value* th_9XMLWriter_outputMemory(Value* _rv, ObjectData* this_, bool flush) asm("_ZN4HPHP11c_XMLWriter14t_outputmemoryEb");

TypedValue* tg1_9XMLWriter_outputMemory(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLWriter_outputMemory(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToBooleanInPlace(args-0);
  th_9XMLWriter_outputMemory((Value*)(rv), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9XMLWriter_outputMemory(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
          rv._count = 0;
          rv.m_type = KindOfString;
          th_9XMLWriter_outputMemory((Value*)(&(rv)), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLWriter_outputMemory(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("XMLWriter::outputMemory", 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::outputMemory");
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
HPHP::Variant HPHP::c_XMLWriter::t___destruct()
_ZN4HPHP11c_XMLWriter12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_9XMLWriter___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter12t___destructEv");

TypedValue* tg_9XMLWriter___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_9XMLWriter___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLWriter::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLWriter::__destruct");
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

