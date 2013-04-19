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

Value* fh_get_declared_classes(Value* _rv) asm("_ZN4HPHP22f_get_declared_classesEv");

TypedValue* fg_get_declared_classes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_get_declared_classes(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("get_declared_classes", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_get_declared_interfaces(Value* _rv) asm("_ZN4HPHP25f_get_declared_interfacesEv");

TypedValue* fg_get_declared_interfaces(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_get_declared_interfaces(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("get_declared_interfaces", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_get_declared_traits(Value* _rv) asm("_ZN4HPHP21f_get_declared_traitsEv");

TypedValue* fg_get_declared_traits(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_get_declared_traits(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("get_declared_traits", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_class_exists(Value* class_name, bool autoload) asm("_ZN4HPHP14f_class_existsERKNS_6StringEb");

void fg1_class_exists(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_class_exists(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_class_exists(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_class_exists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_class_exists(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_class_exists(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("class_exists", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_interface_exists(Value* interface_name, bool autoload) asm("_ZN4HPHP18f_interface_existsERKNS_6StringEb");

void fg1_interface_exists(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_interface_exists(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_interface_exists(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_interface_exists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_interface_exists(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_interface_exists(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("interface_exists", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_trait_exists(Value* trait_name, bool autoload) asm("_ZN4HPHP14f_trait_existsERKNS_6StringEb");

void fg1_trait_exists(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_trait_exists(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_trait_exists(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_trait_exists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_trait_exists(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_trait_exists(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("trait_exists", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_get_class_methods(Value* _rv, TypedValue* class_or_object) asm("_ZN4HPHP19f_get_class_methodsERKNS_7VariantE");

TypedValue* fg_get_class_methods(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfArray;
    fh_get_class_methods(&(rv->m_data), (args-0));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("get_class_methods", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_get_class_constants(Value* _rv, Value* class_name) asm("_ZN4HPHP21f_get_class_constantsERKNS_6StringE");

void fg1_get_class_constants(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_get_class_constants(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_get_class_constants(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_get_class_constants(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      fh_get_class_constants(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_get_class_constants(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("get_class_constants", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_get_class_vars(Value* _rv, Value* class_name) asm("_ZN4HPHP16f_get_class_varsERKNS_6StringE");

void fg1_get_class_vars(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_get_class_vars(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_get_class_vars(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_get_class_vars(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      fh_get_class_vars(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_get_class_vars(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("get_class_vars", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_get_class(TypedValue* _rv, TypedValue* object) asm("_ZN4HPHP11f_get_classERKNS_7VariantE");

TypedValue* fg_get_class(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    fh_get_class(rv, (count > 0) ? (args-0) : (TypedValue*)(&null_variant));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("get_class", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_get_parent_class(TypedValue* _rv, TypedValue* object) asm("_ZN4HPHP18f_get_parent_classERKNS_7VariantE");

TypedValue* fg_get_parent_class(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    fh_get_parent_class(rv, (count > 0) ? (args-0) : (TypedValue*)(&null_variant));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("get_parent_class", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_a(TypedValue* class_or_object, Value* class_name, bool allow_string) asm("_ZN4HPHP6f_is_aERKNS_7VariantERKNS_6StringEb");

void fg1_is_a(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_a(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_a((args-0), &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_is_a(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_a((args-0), &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_is_a(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_a", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_subclass_of(TypedValue* class_or_object, Value* class_name, bool allow_string) asm("_ZN4HPHP16f_is_subclass_ofERKNS_7VariantERKNS_6StringEb");

void fg1_is_subclass_of(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_subclass_of(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_subclass_of((args-0), &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_is_subclass_of(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_subclass_of((args-0), &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_is_subclass_of(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_subclass_of", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_method_exists(TypedValue* class_or_object, Value* method_name) asm("_ZN4HPHP15f_method_existsERKNS_7VariantERKNS_6StringE");

void fg1_method_exists(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_method_exists(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_method_exists((args-0), &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_method_exists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_method_exists((args-0), &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_method_exists(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("method_exists", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_property_exists(TypedValue* _rv, TypedValue* class_or_object, Value* property) asm("_ZN4HPHP17f_property_existsERKNS_7VariantERKNS_6StringE");

void fg1_property_exists(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_property_exists(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_property_exists(rv, (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_property_exists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type)) {
      fh_property_exists(rv, (args-0), &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_property_exists(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("property_exists", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_get_object_vars(TypedValue* _rv, TypedValue* object) asm("_ZN4HPHP17f_get_object_varsERKNS_7VariantE");

TypedValue* fg_get_object_vars(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_get_object_vars(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("get_object_vars", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_call_user_method_array(TypedValue* _rv, Value* method_name, TypedValue* obj, Value* paramarr) asm("_ZN4HPHP24f_call_user_method_arrayERKNS_6StringERKNS_14VRefParamValueERKNS_5ArrayE");

void fg1_call_user_method_array(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_call_user_method_array(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_call_user_method_array(rv, &args[-0].m_data, (args-1), &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_call_user_method_array(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfArray &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_call_user_method_array(rv, &args[-0].m_data, (args-1), &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_call_user_method_array(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("call_user_method_array", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_call_user_method(TypedValue* _rv, int64_t _argc, Value* method_name, TypedValue* obj, Value* _argv) asm("_ZN4HPHP18f_call_user_methodEiRKNS_6StringERKNS_14VRefParamValueERKNS_5ArrayE");

void fg1_call_user_method(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_call_user_method(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);

  Array extraArgs;
  {
    ArrayInit ai(count-2);
    for (int32_t i = 2; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-2);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-2, tvAsVariant(extraArg));
      } else {
        ai.set(i-2, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  fh_call_user_method(rv, count, &args[-0].m_data, (args-1), (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_call_user_method(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {

      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int32_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_call_user_method(rv, count, &args[-0].m_data, (args-1), (Value*)(&extraArgs));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_call_user_method(rv, ar, count);
    }
  } else {
    throw_missing_arguments_nr("call_user_method", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
