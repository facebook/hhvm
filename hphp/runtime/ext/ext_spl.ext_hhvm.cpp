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

Value* fh_spl_classes(Value* _rv) asm("_ZN4HPHP13f_spl_classesEv");

TypedValue* fg_spl_classes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_spl_classes(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("spl_classes", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_spl_object_hash(Value* _rv, Value* obj) asm("_ZN4HPHP17f_spl_object_hashERKNS_6ObjectE");

void fg1_spl_object_hash(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_spl_object_hash(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_spl_object_hash(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_spl_object_hash(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_spl_object_hash(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_spl_object_hash(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("spl_object_hash", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_object_pointer(Value* obj) asm("_ZN4HPHP21f_hphp_object_pointerERKNS_6ObjectE");

void fg1_hphp_object_pointer(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_object_pointer(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_object_pointer(&args[-0].m_data);
}

TypedValue* fg_hphp_object_pointer(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_object_pointer(&args[-0].m_data);
    } else {
      fg1_hphp_object_pointer(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_object_pointer", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_get_this(TypedValue* _rv) asm("_ZN4HPHP15f_hphp_get_thisEv");

TypedValue* fg_hphp_get_this(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_hphp_get_this(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("hphp_get_this", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_class_implements(TypedValue* _rv, TypedValue* obj, bool autoload) asm("_ZN4HPHP18f_class_implementsERKNS_7VariantEb");

void fg1_class_implements(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_class_implements(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_class_implements(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_class_implements(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      fh_class_implements(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_class_implements(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("class_implements", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_class_parents(TypedValue* _rv, TypedValue* obj, bool autoload) asm("_ZN4HPHP15f_class_parentsERKNS_7VariantEb");

void fg1_class_parents(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_class_parents(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_class_parents(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_class_parents(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      fh_class_parents(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_class_parents(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("class_parents", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_class_uses(TypedValue* _rv, TypedValue* obj, bool autoload) asm("_ZN4HPHP12f_class_usesERKNS_7VariantEb");

void fg1_class_uses(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_class_uses(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_class_uses(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_class_uses(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      fh_class_uses(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_class_uses(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("class_uses", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_iterator_apply(TypedValue* _rv, TypedValue* obj, TypedValue* func, Value* params) asm("_ZN4HPHP16f_iterator_applyERKNS_7VariantES2_RKNS_5ArrayE");

void fg1_iterator_apply(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_iterator_apply(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-2);
  fh_iterator_apply(rv, (args-0), (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_array));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_iterator_apply(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfArray)) {
      fh_iterator_apply(rv, (args-0), (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_array));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_iterator_apply(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("iterator_apply", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_iterator_count(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP16f_iterator_countERKNS_7VariantE");

TypedValue* fg_iterator_count(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_iterator_count(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("iterator_count", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_iterator_to_array(TypedValue* _rv, TypedValue* obj, bool use_keys) asm("_ZN4HPHP19f_iterator_to_arrayERKNS_7VariantEb");

void fg1_iterator_to_array(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_iterator_to_array(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_iterator_to_array(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_iterator_to_array(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      fh_iterator_to_array(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_iterator_to_array(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("iterator_to_array", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_spl_autoload_register(TypedValue* autoload_function, bool throws, bool prepend) asm("_ZN4HPHP23f_spl_autoload_registerERKNS_7VariantEbb");

void fg1_spl_autoload_register(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_spl_autoload_register(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
  case 0:
    break;
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_spl_autoload_register((count > 0) ? (args-0) : (TypedValue*)(&null_variant), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_spl_autoload_register(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_spl_autoload_register((count > 0) ? (args-0) : (TypedValue*)(&null_variant), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_spl_autoload_register(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("spl_autoload_register", 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_spl_autoload_unregister(TypedValue* autoload_function) asm("_ZN4HPHP25f_spl_autoload_unregisterERKNS_7VariantE");

TypedValue* fg_spl_autoload_unregister(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_spl_autoload_unregister((args-0))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("spl_autoload_unregister", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_spl_autoload_functions(TypedValue* _rv) asm("_ZN4HPHP24f_spl_autoload_functionsEv");

TypedValue* fg_spl_autoload_functions(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_spl_autoload_functions(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("spl_autoload_functions", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_spl_autoload_call(Value* class_name) asm("_ZN4HPHP19f_spl_autoload_callERKNS_6StringE");

void fg1_spl_autoload_call(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_spl_autoload_call(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_spl_autoload_call(&args[-0].m_data);
}

TypedValue* fg_spl_autoload_call(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfNull;
      fh_spl_autoload_call(&args[-0].m_data);
    } else {
      fg1_spl_autoload_call(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("spl_autoload_call", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_spl_autoload_extensions(Value* _rv, Value* file_extensions) asm("_ZN4HPHP25f_spl_autoload_extensionsERKNS_6StringE");

void fg1_spl_autoload_extensions(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_spl_autoload_extensions(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_spl_autoload_extensions(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_spl_autoload_extensions(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfString;
      fh_spl_autoload_extensions(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_spl_autoload_extensions(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("spl_autoload_extensions", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_spl_autoload(Value* class_name, Value* file_extensions) asm("_ZN4HPHP14f_spl_autoloadERKNS_6StringES2_");

void fg1_spl_autoload(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_spl_autoload(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfNull;
  fh_spl_autoload(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
}

TypedValue* fg_spl_autoload(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfNull;
      fh_spl_autoload(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
    } else {
      fg1_spl_autoload(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("spl_autoload", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
