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

Value* fh_xml_parser_create(Value* _rv, Value* encoding) asm("_ZN4HPHP19f_xml_parser_createERKNS_6StringE");

void fg1_xml_parser_create(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_parser_create(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_xml_parser_create(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_xml_parser_create(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfObject;
      fh_xml_parser_create(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_xml_parser_create(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("xml_parser_create", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_xml_parser_create_ns(Value* _rv, Value* encoding, Value* separator) asm("_ZN4HPHP22f_xml_parser_create_nsERKNS_6StringES2_");

void fg1_xml_parser_create_ns(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_parser_create_ns(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
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
  rv->m_type = KindOfObject;
  fh_xml_parser_create_ns(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_xml_parser_create_ns(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfObject;
      fh_xml_parser_create_ns(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_xml_parser_create_ns(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("xml_parser_create_ns", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_parser_free(Value* parser) asm("_ZN4HPHP17f_xml_parser_freeERKNS_6ObjectE");

void fg1_xml_parser_free(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_parser_free(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_parser_free(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xml_parser_free(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_parser_free(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xml_parser_free(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_parser_free", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_xml_parse(Value* parser, Value* data, bool is_final) asm("_ZN4HPHP11f_xml_parseERKNS_6ObjectERKNS_6StringEb");

void fg1_xml_parse(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_parse(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
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
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_xml_parse(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true));
}

TypedValue* fg_xml_parse(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_xml_parse(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true));
    } else {
      fg1_xml_parse(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_parse", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_xml_parse_into_struct(Value* parser, Value* data, TypedValue* values, TypedValue* index) asm("_ZN4HPHP23f_xml_parse_into_structERKNS_6ObjectERKNS_6StringERKNS_14VRefParamValueES8_");

void fg1_xml_parse_into_struct(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_parse_into_struct(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfInt64;
  VRefParamValue defVal3 = uninit_null();
  rv->m_data.num = (int64_t)fh_xml_parse_into_struct(&args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
}

TypedValue* fg_xml_parse_into_struct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      VRefParamValue defVal3 = uninit_null();
      rv->m_data.num = (int64_t)fh_xml_parse_into_struct(&args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
    } else {
      fg1_xml_parse_into_struct(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_parse_into_struct", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_xml_parser_get_option(TypedValue* _rv, Value* parser, int option) asm("_ZN4HPHP23f_xml_parser_get_optionERKNS_6ObjectEi");

void fg1_xml_parser_get_option(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_parser_get_option(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_xml_parser_get_option(rv, &args[-0].m_data, (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_xml_parser_get_option(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_xml_parser_get_option(rv, &args[-0].m_data, (int)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_xml_parser_get_option(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_parser_get_option", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_parser_set_option(Value* parser, int option, TypedValue* value) asm("_ZN4HPHP23f_xml_parser_set_optionERKNS_6ObjectEiRKNS_7VariantE");

void fg1_xml_parser_set_option(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_parser_set_option(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_parser_set_option(&args[-0].m_data, (int)(args[-1].m_data.num), (args-2))) ? 1LL : 0LL;
}

TypedValue* fg_xml_parser_set_option(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_parser_set_option(&args[-0].m_data, (int)(args[-1].m_data.num), (args-2))) ? 1LL : 0LL;
    } else {
      fg1_xml_parser_set_option(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_parser_set_option", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_set_character_data_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP32f_xml_set_character_data_handlerERKNS_6ObjectERKNS_7VariantE");

void fg1_xml_set_character_data_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_set_character_data_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_set_character_data_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_xml_set_character_data_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_set_character_data_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_xml_set_character_data_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_set_character_data_handler", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_set_default_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP25f_xml_set_default_handlerERKNS_6ObjectERKNS_7VariantE");

void fg1_xml_set_default_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_set_default_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_set_default_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_xml_set_default_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_set_default_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_xml_set_default_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_set_default_handler", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_set_element_handler(Value* parser, TypedValue* start_element_handler, TypedValue* end_element_handler) asm("_ZN4HPHP25f_xml_set_element_handlerERKNS_6ObjectERKNS_7VariantES5_");

void fg1_xml_set_element_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_set_element_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_set_element_handler(&args[-0].m_data, (args-1), (args-2))) ? 1LL : 0LL;
}

TypedValue* fg_xml_set_element_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_set_element_handler(&args[-0].m_data, (args-1), (args-2))) ? 1LL : 0LL;
    } else {
      fg1_xml_set_element_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_set_element_handler", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_set_processing_instruction_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP40f_xml_set_processing_instruction_handlerERKNS_6ObjectERKNS_7VariantE");

void fg1_xml_set_processing_instruction_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_set_processing_instruction_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_set_processing_instruction_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_xml_set_processing_instruction_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_set_processing_instruction_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_xml_set_processing_instruction_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_set_processing_instruction_handler", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_set_start_namespace_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP38f_xml_set_start_namespace_decl_handlerERKNS_6ObjectERKNS_7VariantE");

void fg1_xml_set_start_namespace_decl_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_set_start_namespace_decl_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_set_start_namespace_decl_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_xml_set_start_namespace_decl_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_set_start_namespace_decl_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_xml_set_start_namespace_decl_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_set_start_namespace_decl_handler", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_set_end_namespace_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP36f_xml_set_end_namespace_decl_handlerERKNS_6ObjectERKNS_7VariantE");

void fg1_xml_set_end_namespace_decl_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_set_end_namespace_decl_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_set_end_namespace_decl_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_xml_set_end_namespace_decl_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_set_end_namespace_decl_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_xml_set_end_namespace_decl_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_set_end_namespace_decl_handler", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_set_unparsed_entity_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP38f_xml_set_unparsed_entity_decl_handlerERKNS_6ObjectERKNS_7VariantE");

void fg1_xml_set_unparsed_entity_decl_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_set_unparsed_entity_decl_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_set_unparsed_entity_decl_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_xml_set_unparsed_entity_decl_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_set_unparsed_entity_decl_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_xml_set_unparsed_entity_decl_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_set_unparsed_entity_decl_handler", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_set_external_entity_ref_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP37f_xml_set_external_entity_ref_handlerERKNS_6ObjectERKNS_7VariantE");

void fg1_xml_set_external_entity_ref_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_set_external_entity_ref_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_set_external_entity_ref_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_xml_set_external_entity_ref_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_set_external_entity_ref_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_xml_set_external_entity_ref_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_set_external_entity_ref_handler", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_set_notation_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP31f_xml_set_notation_decl_handlerERKNS_6ObjectERKNS_7VariantE");

void fg1_xml_set_notation_decl_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_set_notation_decl_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_set_notation_decl_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_xml_set_notation_decl_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_set_notation_decl_handler(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_xml_set_notation_decl_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_set_notation_decl_handler", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xml_set_object(Value* parser, TypedValue* object) asm("_ZN4HPHP16f_xml_set_objectERKNS_6ObjectERKNS_14VRefParamValueE");

void fg1_xml_set_object(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_set_object(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xml_set_object(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_xml_set_object(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xml_set_object(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_xml_set_object(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_set_object", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_xml_get_current_byte_index(Value* parser) asm("_ZN4HPHP28f_xml_get_current_byte_indexERKNS_6ObjectE");

void fg1_xml_get_current_byte_index(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_get_current_byte_index(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_xml_get_current_byte_index(&args[-0].m_data);
}

TypedValue* fg_xml_get_current_byte_index(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_xml_get_current_byte_index(&args[-0].m_data);
    } else {
      fg1_xml_get_current_byte_index(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_get_current_byte_index", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_xml_get_current_column_number(Value* parser) asm("_ZN4HPHP31f_xml_get_current_column_numberERKNS_6ObjectE");

void fg1_xml_get_current_column_number(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_get_current_column_number(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_xml_get_current_column_number(&args[-0].m_data);
}

TypedValue* fg_xml_get_current_column_number(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_xml_get_current_column_number(&args[-0].m_data);
    } else {
      fg1_xml_get_current_column_number(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_get_current_column_number", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_xml_get_current_line_number(Value* parser) asm("_ZN4HPHP29f_xml_get_current_line_numberERKNS_6ObjectE");

void fg1_xml_get_current_line_number(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_get_current_line_number(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_xml_get_current_line_number(&args[-0].m_data);
}

TypedValue* fg_xml_get_current_line_number(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_xml_get_current_line_number(&args[-0].m_data);
    } else {
      fg1_xml_get_current_line_number(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_get_current_line_number", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_xml_get_error_code(Value* parser) asm("_ZN4HPHP20f_xml_get_error_codeERKNS_6ObjectE");

void fg1_xml_get_error_code(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_get_error_code(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_xml_get_error_code(&args[-0].m_data);
}

TypedValue* fg_xml_get_error_code(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_xml_get_error_code(&args[-0].m_data);
    } else {
      fg1_xml_get_error_code(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_get_error_code", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_xml_error_string(Value* _rv, int code) asm("_ZN4HPHP18f_xml_error_stringEi");

void fg1_xml_error_string(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xml_error_string(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  fh_xml_error_string(&(rv->m_data), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_xml_error_string(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      fh_xml_error_string(&(rv->m_data), (int)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_xml_error_string(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xml_error_string", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_utf8_decode(Value* _rv, Value* data) asm("_ZN4HPHP13f_utf8_decodeERKNS_6StringE");

void fg1_utf8_decode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_utf8_decode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_utf8_decode(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_utf8_decode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_utf8_decode(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_utf8_decode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("utf8_decode", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_utf8_encode(Value* _rv, Value* data) asm("_ZN4HPHP13f_utf8_encodeERKNS_6StringE");

void fg1_utf8_encode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_utf8_encode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_utf8_encode(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_utf8_encode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_utf8_encode(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_utf8_encode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("utf8_encode", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
