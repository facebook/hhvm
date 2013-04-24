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
HPHP::Variant HPHP::f_dom_document_create_element(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP29f_dom_document_create_elementERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
value => rcx
*/

TypedValue* fh_dom_document_create_element(TypedValue* _rv, TypedValue* obj, Value* name, Value* value) asm("_ZN4HPHP29f_dom_document_create_elementERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_document_create_element(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_create_element(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_dom_document_create_element((rv), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_create_element(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_create_element((&(rv)), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_create_element(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_create_element", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_create_document_fragment(HPHP::Variant const&)
_ZN4HPHP39f_dom_document_create_document_fragmentERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_document_create_document_fragment(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP39f_dom_document_create_document_fragmentERKNS_7VariantE");

TypedValue* fg_dom_document_create_document_fragment(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_dom_document_create_document_fragment((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("dom_document_create_document_fragment", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_create_text_node(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP31f_dom_document_create_text_nodeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
data => rdx
*/

TypedValue* fh_dom_document_create_text_node(TypedValue* _rv, TypedValue* obj, Value* data) asm("_ZN4HPHP31f_dom_document_create_text_nodeERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_create_text_node(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_create_text_node(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_create_text_node((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_create_text_node(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_create_text_node((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_create_text_node(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_create_text_node", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_create_comment(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP29f_dom_document_create_commentERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
data => rdx
*/

TypedValue* fh_dom_document_create_comment(TypedValue* _rv, TypedValue* obj, Value* data) asm("_ZN4HPHP29f_dom_document_create_commentERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_create_comment(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_create_comment(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_create_comment((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_create_comment(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_create_comment((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_create_comment(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_create_comment", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_create_cdatasection(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP34f_dom_document_create_cdatasectionERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
data => rdx
*/

TypedValue* fh_dom_document_create_cdatasection(TypedValue* _rv, TypedValue* obj, Value* data) asm("_ZN4HPHP34f_dom_document_create_cdatasectionERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_create_cdatasection(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_create_cdatasection(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_create_cdatasection((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_create_cdatasection(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_create_cdatasection((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_create_cdatasection(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_create_cdatasection", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_create_processing_instruction(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP44f_dom_document_create_processing_instructionERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
target => rdx
data => rcx
*/

TypedValue* fh_dom_document_create_processing_instruction(TypedValue* _rv, TypedValue* obj, Value* target, Value* data) asm("_ZN4HPHP44f_dom_document_create_processing_instructionERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_document_create_processing_instruction(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_create_processing_instruction(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_dom_document_create_processing_instruction((rv), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_create_processing_instruction(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_create_processing_instruction((&(rv)), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_create_processing_instruction(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_create_processing_instruction", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_create_attribute(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP31f_dom_document_create_attributeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_document_create_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP31f_dom_document_create_attributeERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_create_attribute(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_create_attribute(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_create_attribute((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_create_attribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_create_attribute((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_create_attribute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_create_attribute", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_create_entity_reference(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP38f_dom_document_create_entity_referenceERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_document_create_entity_reference(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP38f_dom_document_create_entity_referenceERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_create_entity_reference(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_create_entity_reference(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_create_entity_reference((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_create_entity_reference(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_create_entity_reference((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_create_entity_reference(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_create_entity_reference", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_get_elements_by_tag_name(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP39f_dom_document_get_elements_by_tag_nameERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_document_get_elements_by_tag_name(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP39f_dom_document_get_elements_by_tag_nameERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_get_elements_by_tag_name(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_get_elements_by_tag_name(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_get_elements_by_tag_name((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_get_elements_by_tag_name(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_get_elements_by_tag_name((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_get_elements_by_tag_name(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_get_elements_by_tag_name", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_import_node(HPHP::Variant const&, HPHP::Object const&, bool)
_ZN4HPHP26f_dom_document_import_nodeERKNS_7VariantERKNS_6ObjectEb

(return value) => rax
_rv => rdi
obj => rsi
importednode => rdx
deep => rcx
*/

TypedValue* fh_dom_document_import_node(TypedValue* _rv, TypedValue* obj, Value* importednode, bool deep) asm("_ZN4HPHP26f_dom_document_import_nodeERKNS_7VariantERKNS_6ObjectEb");

TypedValue * fg1_dom_document_import_node(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_import_node(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  fh_dom_document_import_node((rv), (args-0), &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_import_node(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (args-1)->m_type == KindOfObject) {
        fh_dom_document_import_node((&(rv)), (args-0), &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_import_node(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_import_node", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_create_element_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP32f_dom_document_create_element_nsERKNS_7VariantERKNS_6StringES5_S5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
qualifiedname => rcx
value => r8
*/

TypedValue* fh_dom_document_create_element_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* qualifiedname, Value* value) asm("_ZN4HPHP32f_dom_document_create_element_nsERKNS_7VariantERKNS_6StringES5_S5_");

TypedValue * fg1_dom_document_create_element_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_create_element_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  fh_dom_document_create_element_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_create_element_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_create_element_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_create_element_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_create_element_ns", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_create_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP34f_dom_document_create_attribute_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
qualifiedname => rcx
*/

TypedValue* fh_dom_document_create_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* qualifiedname) asm("_ZN4HPHP34f_dom_document_create_attribute_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_document_create_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_create_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_document_create_attribute_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_create_attribute_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_create_attribute_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_create_attribute_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_create_attribute_ns", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_get_elements_by_tag_name_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP42f_dom_document_get_elements_by_tag_name_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_document_get_elements_by_tag_name_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP42f_dom_document_get_elements_by_tag_name_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_document_get_elements_by_tag_name_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_get_elements_by_tag_name_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_document_get_elements_by_tag_name_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_get_elements_by_tag_name_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_get_elements_by_tag_name_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_get_elements_by_tag_name_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_get_elements_by_tag_name_ns", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_get_element_by_id(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP32f_dom_document_get_element_by_idERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
elementid => rdx
*/

TypedValue* fh_dom_document_get_element_by_id(TypedValue* _rv, TypedValue* obj, Value* elementid) asm("_ZN4HPHP32f_dom_document_get_element_by_idERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_get_element_by_id(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_get_element_by_id(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_get_element_by_id((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_get_element_by_id(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_get_element_by_id((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_get_element_by_id(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_get_element_by_id", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_normalize_document(HPHP::Variant const&)
_ZN4HPHP33f_dom_document_normalize_documentERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_document_normalize_document(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP33f_dom_document_normalize_documentERKNS_7VariantE");

TypedValue* fg_dom_document_normalize_document(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_dom_document_normalize_document((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("dom_document_normalize_document", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_save(HPHP::Variant const&, HPHP::String const&, long)
_ZN4HPHP19f_dom_document_saveERKNS_7VariantERKNS_6StringEl

(return value) => rax
_rv => rdi
obj => rsi
file => rdx
options => rcx
*/

TypedValue* fh_dom_document_save(TypedValue* _rv, TypedValue* obj, Value* file, long options) asm("_ZN4HPHP19f_dom_document_saveERKNS_7VariantERKNS_6StringEl");

TypedValue * fg1_dom_document_save(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_save(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  fh_dom_document_save((rv), (args-0), &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_save(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_save((&(rv)), (args-0), &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_save(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_save", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_savexml(HPHP::Variant const&, HPHP::Object const&, long)
_ZN4HPHP22f_dom_document_savexmlERKNS_7VariantERKNS_6ObjectEl

(return value) => rax
_rv => rdi
obj => rsi
node => rdx
options => rcx
*/

TypedValue* fh_dom_document_savexml(TypedValue* _rv, TypedValue* obj, Value* node, long options) asm("_ZN4HPHP22f_dom_document_savexmlERKNS_7VariantERKNS_6ObjectEl");

TypedValue * fg1_dom_document_savexml(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_savexml(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-1);
    }
  case 1:
    break;
  }
  fh_dom_document_savexml((rv), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_object), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_savexml(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfObject)) {
        fh_dom_document_savexml((&(rv)), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_object), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_savexml(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_savexml", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_validate(HPHP::Variant const&)
_ZN4HPHP23f_dom_document_validateERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_document_validate(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP23f_dom_document_validateERKNS_7VariantE");

TypedValue* fg_dom_document_validate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_dom_document_validate((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("dom_document_validate", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_xinclude(HPHP::Variant const&, long)
_ZN4HPHP23f_dom_document_xincludeERKNS_7VariantEl

(return value) => rax
_rv => rdi
obj => rsi
options => rdx
*/

TypedValue* fh_dom_document_xinclude(TypedValue* _rv, TypedValue* obj, long options) asm("_ZN4HPHP23f_dom_document_xincludeERKNS_7VariantEl");

TypedValue * fg1_dom_document_xinclude(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_xinclude(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_dom_document_xinclude((rv), (args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_xinclude(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_dom_document_xinclude((&(rv)), (args-0), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_xinclude(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_xinclude", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_save_html(HPHP::Variant const&)
_ZN4HPHP24f_dom_document_save_htmlERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_document_save_html(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP24f_dom_document_save_htmlERKNS_7VariantE");

TypedValue* fg_dom_document_save_html(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_dom_document_save_html((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("dom_document_save_html", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_save_html_file(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP29f_dom_document_save_html_fileERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
file => rdx
*/

TypedValue* fh_dom_document_save_html_file(TypedValue* _rv, TypedValue* obj, Value* file) asm("_ZN4HPHP29f_dom_document_save_html_fileERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_save_html_file(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_save_html_file(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_save_html_file((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_save_html_file(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_save_html_file((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_save_html_file(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_save_html_file", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_schema_validate_file(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP35f_dom_document_schema_validate_fileERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
filename => rdx
*/

TypedValue* fh_dom_document_schema_validate_file(TypedValue* _rv, TypedValue* obj, Value* filename) asm("_ZN4HPHP35f_dom_document_schema_validate_fileERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_schema_validate_file(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_schema_validate_file(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_schema_validate_file((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_schema_validate_file(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_schema_validate_file((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_schema_validate_file(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_schema_validate_file", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_schema_validate_xml(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP34f_dom_document_schema_validate_xmlERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
source => rdx
*/

TypedValue* fh_dom_document_schema_validate_xml(TypedValue* _rv, TypedValue* obj, Value* source) asm("_ZN4HPHP34f_dom_document_schema_validate_xmlERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_schema_validate_xml(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_schema_validate_xml(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_schema_validate_xml((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_schema_validate_xml(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_schema_validate_xml((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_schema_validate_xml(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_schema_validate_xml", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_relaxng_validate_file(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP36f_dom_document_relaxng_validate_fileERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
filename => rdx
*/

TypedValue* fh_dom_document_relaxng_validate_file(TypedValue* _rv, TypedValue* obj, Value* filename) asm("_ZN4HPHP36f_dom_document_relaxng_validate_fileERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_relaxng_validate_file(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_relaxng_validate_file(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_relaxng_validate_file((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_relaxng_validate_file(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_relaxng_validate_file((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_relaxng_validate_file(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_relaxng_validate_file", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_document_relaxng_validate_xml(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP35f_dom_document_relaxng_validate_xmlERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
source => rdx
*/

TypedValue* fh_dom_document_relaxng_validate_xml(TypedValue* _rv, TypedValue* obj, Value* source) asm("_ZN4HPHP35f_dom_document_relaxng_validate_xmlERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_document_relaxng_validate_xml(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_document_relaxng_validate_xml(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_document_relaxng_validate_xml((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_document_relaxng_validate_xml(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_document_relaxng_validate_xml((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_document_relaxng_validate_xml(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_document_relaxng_validate_xml", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_insert_before(HPHP::Variant const&, HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP24f_dom_node_insert_beforeERKNS_7VariantERKNS_6ObjectES5_

(return value) => rax
_rv => rdi
obj => rsi
newnode => rdx
refnode => rcx
*/

TypedValue* fh_dom_node_insert_before(TypedValue* _rv, TypedValue* obj, Value* newnode, Value* refnode) asm("_ZN4HPHP24f_dom_node_insert_beforeERKNS_7VariantERKNS_6ObjectES5_");

TypedValue * fg1_dom_node_insert_before(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_node_insert_before(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  Object defVal2 = uninit_null();
  fh_dom_node_insert_before((rv), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_node_insert_before(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfObject) && (args-1)->m_type == KindOfObject) {
        Object defVal2 = uninit_null();
        fh_dom_node_insert_before((&(rv)), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_node_insert_before(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_node_insert_before", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_replace_child(HPHP::Variant const&, HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP24f_dom_node_replace_childERKNS_7VariantERKNS_6ObjectES5_

(return value) => rax
_rv => rdi
obj => rsi
newchildobj => rdx
oldchildobj => rcx
*/

TypedValue* fh_dom_node_replace_child(TypedValue* _rv, TypedValue* obj, Value* newchildobj, Value* oldchildobj) asm("_ZN4HPHP24f_dom_node_replace_childERKNS_7VariantERKNS_6ObjectES5_");

TypedValue * fg1_dom_node_replace_child(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_node_replace_child(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  fh_dom_node_replace_child((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_node_replace_child(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfObject && (args-1)->m_type == KindOfObject) {
        fh_dom_node_replace_child((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_node_replace_child(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_node_replace_child", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_remove_child(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP23f_dom_node_remove_childERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
node => rdx
*/

TypedValue* fh_dom_node_remove_child(TypedValue* _rv, TypedValue* obj, Value* node) asm("_ZN4HPHP23f_dom_node_remove_childERKNS_7VariantERKNS_6ObjectE");

TypedValue * fg1_dom_node_remove_child(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_node_remove_child(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-1);
  fh_dom_node_remove_child((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_node_remove_child(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject) {
        fh_dom_node_remove_child((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_node_remove_child(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_node_remove_child", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_append_child(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP23f_dom_node_append_childERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
newnode => rdx
*/

TypedValue* fh_dom_node_append_child(TypedValue* _rv, TypedValue* obj, Value* newnode) asm("_ZN4HPHP23f_dom_node_append_childERKNS_7VariantERKNS_6ObjectE");

TypedValue * fg1_dom_node_append_child(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_node_append_child(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-1);
  fh_dom_node_append_child((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_node_append_child(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject) {
        fh_dom_node_append_child((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_node_append_child(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_node_append_child", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_has_child_nodes(HPHP::Variant const&)
_ZN4HPHP26f_dom_node_has_child_nodesERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_node_has_child_nodes(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP26f_dom_node_has_child_nodesERKNS_7VariantE");

TypedValue* fg_dom_node_has_child_nodes(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_dom_node_has_child_nodes((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("dom_node_has_child_nodes", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_clone_node(HPHP::Variant const&, bool)
_ZN4HPHP21f_dom_node_clone_nodeERKNS_7VariantEb

(return value) => rax
_rv => rdi
obj => rsi
deep => rdx
*/

TypedValue* fh_dom_node_clone_node(TypedValue* _rv, TypedValue* obj, bool deep) asm("_ZN4HPHP21f_dom_node_clone_nodeERKNS_7VariantEb");

TypedValue * fg1_dom_node_clone_node(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_node_clone_node(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_dom_node_clone_node((rv), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_node_clone_node(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean)) {
        fh_dom_node_clone_node((&(rv)), (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_node_clone_node(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_node_clone_node", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_normalize(HPHP::Variant const&)
_ZN4HPHP20f_dom_node_normalizeERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_node_normalize(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP20f_dom_node_normalizeERKNS_7VariantE");

TypedValue* fg_dom_node_normalize(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_dom_node_normalize((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("dom_node_normalize", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_is_supported(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP23f_dom_node_is_supportedERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
feature => rdx
version => rcx
*/

TypedValue* fh_dom_node_is_supported(TypedValue* _rv, TypedValue* obj, Value* feature, Value* version) asm("_ZN4HPHP23f_dom_node_is_supportedERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_node_is_supported(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_node_is_supported(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_node_is_supported((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_node_is_supported(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_node_is_supported((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_node_is_supported(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_node_is_supported", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_has_attributes(HPHP::Variant const&)
_ZN4HPHP25f_dom_node_has_attributesERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_node_has_attributes(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP25f_dom_node_has_attributesERKNS_7VariantE");

TypedValue* fg_dom_node_has_attributes(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_dom_node_has_attributes((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("dom_node_has_attributes", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_is_same_node(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP23f_dom_node_is_same_nodeERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
node => rdx
*/

TypedValue* fh_dom_node_is_same_node(TypedValue* _rv, TypedValue* obj, Value* node) asm("_ZN4HPHP23f_dom_node_is_same_nodeERKNS_7VariantERKNS_6ObjectE");

TypedValue * fg1_dom_node_is_same_node(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_node_is_same_node(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-1);
  fh_dom_node_is_same_node((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_node_is_same_node(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject) {
        fh_dom_node_is_same_node((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_node_is_same_node(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_node_is_same_node", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_lookup_prefix(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP24f_dom_node_lookup_prefixERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
prefix => rdx
*/

TypedValue* fh_dom_node_lookup_prefix(TypedValue* _rv, TypedValue* obj, Value* prefix) asm("_ZN4HPHP24f_dom_node_lookup_prefixERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_node_lookup_prefix(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_node_lookup_prefix(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_node_lookup_prefix((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_node_lookup_prefix(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_node_lookup_prefix((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_node_lookup_prefix(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_node_lookup_prefix", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_is_default_namespace(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP31f_dom_node_is_default_namespaceERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
*/

TypedValue* fh_dom_node_is_default_namespace(TypedValue* _rv, TypedValue* obj, Value* namespaceuri) asm("_ZN4HPHP31f_dom_node_is_default_namespaceERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_node_is_default_namespace(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_node_is_default_namespace(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_node_is_default_namespace((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_node_is_default_namespace(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_node_is_default_namespace((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_node_is_default_namespace(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_node_is_default_namespace", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_node_lookup_namespace_uri(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP31f_dom_node_lookup_namespace_uriERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
*/

TypedValue* fh_dom_node_lookup_namespace_uri(TypedValue* _rv, TypedValue* obj, Value* namespaceuri) asm("_ZN4HPHP31f_dom_node_lookup_namespace_uriERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_node_lookup_namespace_uri(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_node_lookup_namespace_uri(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_node_lookup_namespace_uri((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_node_lookup_namespace_uri(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_node_lookup_namespace_uri((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_node_lookup_namespace_uri(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_node_lookup_namespace_uri", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_nodelist_item(HPHP::Variant const&, long)
_ZN4HPHP19f_dom_nodelist_itemERKNS_7VariantEl

(return value) => rax
_rv => rdi
obj => rsi
index => rdx
*/

TypedValue* fh_dom_nodelist_item(TypedValue* _rv, TypedValue* obj, long index) asm("_ZN4HPHP19f_dom_nodelist_itemERKNS_7VariantEl");

TypedValue * fg1_dom_nodelist_item(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_nodelist_item(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_dom_nodelist_item((rv), (args-0), (long)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_nodelist_item(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64) {
        fh_dom_nodelist_item((&(rv)), (args-0), (long)(args[-1].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_nodelist_item(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_nodelist_item", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_namednodemap_get_named_item(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP33f_dom_namednodemap_get_named_itemERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_namednodemap_get_named_item(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP33f_dom_namednodemap_get_named_itemERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_namednodemap_get_named_item(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_namednodemap_get_named_item(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_namednodemap_get_named_item((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_namednodemap_get_named_item(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_namednodemap_get_named_item((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_namednodemap_get_named_item(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_namednodemap_get_named_item", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_namednodemap_item(HPHP::Variant const&, long)
_ZN4HPHP23f_dom_namednodemap_itemERKNS_7VariantEl

(return value) => rax
_rv => rdi
obj => rsi
index => rdx
*/

TypedValue* fh_dom_namednodemap_item(TypedValue* _rv, TypedValue* obj, long index) asm("_ZN4HPHP23f_dom_namednodemap_itemERKNS_7VariantEl");

TypedValue * fg1_dom_namednodemap_item(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_namednodemap_item(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_dom_namednodemap_item((rv), (args-0), (long)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_namednodemap_item(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64) {
        fh_dom_namednodemap_item((&(rv)), (args-0), (long)(args[-1].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_namednodemap_item(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_namednodemap_item", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_namednodemap_get_named_item_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP36f_dom_namednodemap_get_named_item_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_namednodemap_get_named_item_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP36f_dom_namednodemap_get_named_item_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_namednodemap_get_named_item_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_namednodemap_get_named_item_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_namednodemap_get_named_item_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_namednodemap_get_named_item_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_namednodemap_get_named_item_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_namednodemap_get_named_item_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_namednodemap_get_named_item_ns", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_characterdata_substring_data(HPHP::Variant const&, long, long)
_ZN4HPHP34f_dom_characterdata_substring_dataERKNS_7VariantEll

(return value) => rax
_rv => rdi
obj => rsi
offset => rdx
count => rcx
*/

TypedValue* fh_dom_characterdata_substring_data(TypedValue* _rv, TypedValue* obj, long offset, long count) asm("_ZN4HPHP34f_dom_characterdata_substring_dataERKNS_7VariantEll");

TypedValue * fg1_dom_characterdata_substring_data(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_characterdata_substring_data(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  fh_dom_characterdata_substring_data((rv), (args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_characterdata_substring_data(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64) {
        fh_dom_characterdata_substring_data((&(rv)), (args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_characterdata_substring_data(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_characterdata_substring_data", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_characterdata_append_data(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP31f_dom_characterdata_append_dataERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
arg => rdx
*/

TypedValue* fh_dom_characterdata_append_data(TypedValue* _rv, TypedValue* obj, Value* arg) asm("_ZN4HPHP31f_dom_characterdata_append_dataERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_characterdata_append_data(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_characterdata_append_data(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_characterdata_append_data((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_characterdata_append_data(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_characterdata_append_data((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_characterdata_append_data(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_characterdata_append_data", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_characterdata_insert_data(HPHP::Variant const&, long, HPHP::String const&)
_ZN4HPHP31f_dom_characterdata_insert_dataERKNS_7VariantElRKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
offset => rdx
data => rcx
*/

TypedValue* fh_dom_characterdata_insert_data(TypedValue* _rv, TypedValue* obj, long offset, Value* data) asm("_ZN4HPHP31f_dom_characterdata_insert_dataERKNS_7VariantElRKNS_6StringE");

TypedValue * fg1_dom_characterdata_insert_data(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_characterdata_insert_data(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  fh_dom_characterdata_insert_data((rv), (args-0), (long)(args[-1].m_data.num), &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_characterdata_insert_data(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfInt64) {
        fh_dom_characterdata_insert_data((&(rv)), (args-0), (long)(args[-1].m_data.num), &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_characterdata_insert_data(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_characterdata_insert_data", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_characterdata_delete_data(HPHP::Variant const&, long, long)
_ZN4HPHP31f_dom_characterdata_delete_dataERKNS_7VariantEll

(return value) => rax
_rv => rdi
obj => rsi
offset => rdx
count => rcx
*/

TypedValue* fh_dom_characterdata_delete_data(TypedValue* _rv, TypedValue* obj, long offset, long count) asm("_ZN4HPHP31f_dom_characterdata_delete_dataERKNS_7VariantEll");

TypedValue * fg1_dom_characterdata_delete_data(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_characterdata_delete_data(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  fh_dom_characterdata_delete_data((rv), (args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_characterdata_delete_data(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64) {
        fh_dom_characterdata_delete_data((&(rv)), (args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_characterdata_delete_data(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_characterdata_delete_data", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_characterdata_replace_data(HPHP::Variant const&, long, long, HPHP::String const&)
_ZN4HPHP32f_dom_characterdata_replace_dataERKNS_7VariantEllRKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
offset => rdx
count => rcx
data => r8
*/

TypedValue* fh_dom_characterdata_replace_data(TypedValue* _rv, TypedValue* obj, long offset, long count, Value* data) asm("_ZN4HPHP32f_dom_characterdata_replace_dataERKNS_7VariantEllRKNS_6StringE");

TypedValue * fg1_dom_characterdata_replace_data(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_characterdata_replace_data(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  fh_dom_characterdata_replace_data((rv), (args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num), &args[-3].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_characterdata_replace_data(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-3)->m_type) && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64) {
        fh_dom_characterdata_replace_data((&(rv)), (args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num), &args[-3].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_characterdata_replace_data(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_characterdata_replace_data", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_attr_is_id(HPHP::Variant const&)
_ZN4HPHP16f_dom_attr_is_idERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_attr_is_id(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP16f_dom_attr_is_idERKNS_7VariantE");

TypedValue* fg_dom_attr_is_id(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_dom_attr_is_id((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("dom_attr_is_id", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_get_attribute(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP27f_dom_element_get_attributeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_element_get_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP27f_dom_element_get_attributeERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_element_get_attribute(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_get_attribute(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_element_get_attribute((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_get_attribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_get_attribute((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_get_attribute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_get_attribute", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_set_attribute(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP27f_dom_element_set_attributeERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
value => rcx
*/

TypedValue* fh_dom_element_set_attribute(TypedValue* _rv, TypedValue* obj, Value* name, Value* value) asm("_ZN4HPHP27f_dom_element_set_attributeERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_element_set_attribute(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_set_attribute(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_element_set_attribute((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_set_attribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_set_attribute((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_set_attribute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_set_attribute", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_remove_attribute(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP30f_dom_element_remove_attributeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_element_remove_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP30f_dom_element_remove_attributeERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_element_remove_attribute(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_remove_attribute(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_element_remove_attribute((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_remove_attribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_remove_attribute((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_remove_attribute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_remove_attribute", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_get_attribute_node(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP32f_dom_element_get_attribute_nodeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_element_get_attribute_node(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP32f_dom_element_get_attribute_nodeERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_element_get_attribute_node(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_get_attribute_node(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_element_get_attribute_node((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_get_attribute_node(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_get_attribute_node((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_get_attribute_node(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_get_attribute_node", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_set_attribute_node(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP32f_dom_element_set_attribute_nodeERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
newattr => rdx
*/

TypedValue* fh_dom_element_set_attribute_node(TypedValue* _rv, TypedValue* obj, Value* newattr) asm("_ZN4HPHP32f_dom_element_set_attribute_nodeERKNS_7VariantERKNS_6ObjectE");

TypedValue * fg1_dom_element_set_attribute_node(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_set_attribute_node(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-1);
  fh_dom_element_set_attribute_node((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_set_attribute_node(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject) {
        fh_dom_element_set_attribute_node((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_set_attribute_node(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_set_attribute_node", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_remove_attribute_node(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP35f_dom_element_remove_attribute_nodeERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
oldattr => rdx
*/

TypedValue* fh_dom_element_remove_attribute_node(TypedValue* _rv, TypedValue* obj, Value* oldattr) asm("_ZN4HPHP35f_dom_element_remove_attribute_nodeERKNS_7VariantERKNS_6ObjectE");

TypedValue * fg1_dom_element_remove_attribute_node(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_remove_attribute_node(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-1);
  fh_dom_element_remove_attribute_node((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_remove_attribute_node(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject) {
        fh_dom_element_remove_attribute_node((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_remove_attribute_node(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_remove_attribute_node", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_get_elements_by_tag_name(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP38f_dom_element_get_elements_by_tag_nameERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_element_get_elements_by_tag_name(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP38f_dom_element_get_elements_by_tag_nameERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_element_get_elements_by_tag_name(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_get_elements_by_tag_name(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_element_get_elements_by_tag_name((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_get_elements_by_tag_name(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_get_elements_by_tag_name((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_get_elements_by_tag_name(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_get_elements_by_tag_name", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_get_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP30f_dom_element_get_attribute_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_element_get_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP30f_dom_element_get_attribute_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_element_get_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_get_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_element_get_attribute_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_get_attribute_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_get_attribute_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_get_attribute_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_get_attribute_ns", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_set_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP30f_dom_element_set_attribute_nsERKNS_7VariantERKNS_6StringES5_S5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
name => rcx
value => r8
*/

TypedValue* fh_dom_element_set_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* name, Value* value) asm("_ZN4HPHP30f_dom_element_set_attribute_nsERKNS_7VariantERKNS_6StringES5_S5_");

TypedValue * fg1_dom_element_set_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_set_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_element_set_attribute_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_set_attribute_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_set_attribute_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_set_attribute_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_set_attribute_ns", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_remove_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP33f_dom_element_remove_attribute_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_element_remove_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP33f_dom_element_remove_attribute_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_element_remove_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_remove_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_element_remove_attribute_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_remove_attribute_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_remove_attribute_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_remove_attribute_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_remove_attribute_ns", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_get_attribute_node_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP35f_dom_element_get_attribute_node_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_element_get_attribute_node_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP35f_dom_element_get_attribute_node_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_element_get_attribute_node_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_get_attribute_node_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_element_get_attribute_node_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_get_attribute_node_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_get_attribute_node_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_get_attribute_node_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_get_attribute_node_ns", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_set_attribute_node_ns(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP35f_dom_element_set_attribute_node_nsERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
newattr => rdx
*/

TypedValue* fh_dom_element_set_attribute_node_ns(TypedValue* _rv, TypedValue* obj, Value* newattr) asm("_ZN4HPHP35f_dom_element_set_attribute_node_nsERKNS_7VariantERKNS_6ObjectE");

TypedValue * fg1_dom_element_set_attribute_node_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_set_attribute_node_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-1);
  fh_dom_element_set_attribute_node_ns((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_set_attribute_node_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject) {
        fh_dom_element_set_attribute_node_ns((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_set_attribute_node_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_set_attribute_node_ns", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_get_elements_by_tag_name_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP41f_dom_element_get_elements_by_tag_name_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_element_get_elements_by_tag_name_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP41f_dom_element_get_elements_by_tag_name_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_element_get_elements_by_tag_name_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_get_elements_by_tag_name_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_element_get_elements_by_tag_name_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_get_elements_by_tag_name_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_get_elements_by_tag_name_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_get_elements_by_tag_name_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_get_elements_by_tag_name_ns", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_has_attribute(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP27f_dom_element_has_attributeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_element_has_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP27f_dom_element_has_attributeERKNS_7VariantERKNS_6StringE");

TypedValue * fg1_dom_element_has_attribute(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_has_attribute(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_dom_element_has_attribute((rv), (args-0), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_has_attribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_has_attribute((&(rv)), (args-0), &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_has_attribute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_has_attribute", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_has_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP30f_dom_element_has_attribute_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_element_has_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP30f_dom_element_has_attribute_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_element_has_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_has_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_element_has_attribute_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_has_attribute_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_has_attribute_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_has_attribute_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_has_attribute_ns", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_set_id_attribute(HPHP::Variant const&, HPHP::String const&, bool)
_ZN4HPHP30f_dom_element_set_id_attributeERKNS_7VariantERKNS_6StringEb

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
isid => rcx
*/

TypedValue* fh_dom_element_set_id_attribute(TypedValue* _rv, TypedValue* obj, Value* name, bool isid) asm("_ZN4HPHP30f_dom_element_set_id_attributeERKNS_7VariantERKNS_6StringEb");

TypedValue * fg1_dom_element_set_id_attribute(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_set_id_attribute(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_element_set_id_attribute((rv), (args-0), &args[-1].m_data, (bool)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_set_id_attribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfBoolean && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_set_id_attribute((&(rv)), (args-0), &args[-1].m_data, (bool)(args[-2].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_set_id_attribute(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_set_id_attribute", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_set_id_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, bool)
_ZN4HPHP33f_dom_element_set_id_attribute_nsERKNS_7VariantERKNS_6StringES5_b

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
isid => r8
*/

TypedValue* fh_dom_element_set_id_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname, bool isid) asm("_ZN4HPHP33f_dom_element_set_id_attribute_nsERKNS_7VariantERKNS_6StringES5_b");

TypedValue * fg1_dom_element_set_id_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_set_id_attribute_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_element_set_id_attribute_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data, (bool)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_set_id_attribute_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfBoolean && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_element_set_id_attribute_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data, (bool)(args[-3].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_set_id_attribute_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_set_id_attribute_ns", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_element_set_id_attribute_node(HPHP::Variant const&, HPHP::Object const&, bool)
_ZN4HPHP35f_dom_element_set_id_attribute_nodeERKNS_7VariantERKNS_6ObjectEb

(return value) => rax
_rv => rdi
obj => rsi
idattr => rdx
isid => rcx
*/

TypedValue* fh_dom_element_set_id_attribute_node(TypedValue* _rv, TypedValue* obj, Value* idattr, bool isid) asm("_ZN4HPHP35f_dom_element_set_id_attribute_nodeERKNS_7VariantERKNS_6ObjectEb");

TypedValue * fg1_dom_element_set_id_attribute_node(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_element_set_id_attribute_node(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  fh_dom_element_set_id_attribute_node((rv), (args-0), &args[-1].m_data, (bool)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_element_set_id_attribute_node(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfBoolean && (args-1)->m_type == KindOfObject) {
        fh_dom_element_set_id_attribute_node((&(rv)), (args-0), &args[-1].m_data, (bool)(args[-2].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_element_set_id_attribute_node(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_element_set_id_attribute_node", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_text_split_text(HPHP::Variant const&, long)
_ZN4HPHP21f_dom_text_split_textERKNS_7VariantEl

(return value) => rax
_rv => rdi
obj => rsi
offset => rdx
*/

TypedValue* fh_dom_text_split_text(TypedValue* _rv, TypedValue* obj, long offset) asm("_ZN4HPHP21f_dom_text_split_textERKNS_7VariantEl");

TypedValue * fg1_dom_text_split_text(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_text_split_text(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_dom_text_split_text((rv), (args-0), (long)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_text_split_text(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64) {
        fh_dom_text_split_text((&(rv)), (args-0), (long)(args[-1].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_text_split_text(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_text_split_text", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_text_is_whitespace_in_element_content(HPHP::Variant const&)
_ZN4HPHP43f_dom_text_is_whitespace_in_element_contentERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_text_is_whitespace_in_element_content(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP43f_dom_text_is_whitespace_in_element_contentERKNS_7VariantE");

TypedValue* fg_dom_text_is_whitespace_in_element_content(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_dom_text_is_whitespace_in_element_content((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("dom_text_is_whitespace_in_element_content", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_xpath_register_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP23f_dom_xpath_register_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
prefix => rdx
uri => rcx
*/

TypedValue* fh_dom_xpath_register_ns(TypedValue* _rv, TypedValue* obj, Value* prefix, Value* uri) asm("_ZN4HPHP23f_dom_xpath_register_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_dom_xpath_register_ns(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_xpath_register_ns(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_xpath_register_ns((rv), (args-0), &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_xpath_register_ns(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_xpath_register_ns((&(rv)), (args-0), &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_xpath_register_ns(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_xpath_register_ns", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_xpath_query(HPHP::Variant const&, HPHP::String const&, HPHP::Object const&)
_ZN4HPHP17f_dom_xpath_queryERKNS_7VariantERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
expr => rdx
context => rcx
*/

TypedValue* fh_dom_xpath_query(TypedValue* _rv, TypedValue* obj, Value* expr, Value* context) asm("_ZN4HPHP17f_dom_xpath_queryERKNS_7VariantERKNS_6StringERKNS_6ObjectE");

TypedValue * fg1_dom_xpath_query(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_xpath_query(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_xpath_query((rv), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_object));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_xpath_query(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfObject) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_xpath_query((&(rv)), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_object));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_xpath_query(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_xpath_query", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_xpath_evaluate(HPHP::Variant const&, HPHP::String const&, HPHP::Object const&)
_ZN4HPHP20f_dom_xpath_evaluateERKNS_7VariantERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
expr => rdx
context => rcx
*/

TypedValue* fh_dom_xpath_evaluate(TypedValue* _rv, TypedValue* obj, Value* expr, Value* context) asm("_ZN4HPHP20f_dom_xpath_evaluateERKNS_7VariantERKNS_6StringERKNS_6ObjectE");

TypedValue * fg1_dom_xpath_evaluate(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_dom_xpath_evaluate(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  fh_dom_xpath_evaluate((rv), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_object));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_dom_xpath_evaluate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfObject) && IS_STRING_TYPE((args-1)->m_type)) {
        fh_dom_xpath_evaluate((&(rv)), (args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_object));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_dom_xpath_evaluate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("dom_xpath_evaluate", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_dom_xpath_register_php_functions(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP34f_dom_xpath_register_php_functionsERKNS_7VariantES2_

(return value) => rax
_rv => rdi
obj => rsi
funcs => rdx
*/

TypedValue* fh_dom_xpath_register_php_functions(TypedValue* _rv, TypedValue* obj, TypedValue* funcs) asm("_ZN4HPHP34f_dom_xpath_register_php_functionsERKNS_7VariantES2_");

TypedValue* fg_dom_xpath_register_php_functions(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      Variant defVal1;
      fh_dom_xpath_register_php_functions((&(rv)), (args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 2);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("dom_xpath_register_php_functions", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



HPHP::VM::Instance* new_DOMNode_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMNode) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMNode(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMNode);
/*
void HPHP::c_DOMNode::t___construct()
_ZN4HPHP9c_DOMNode13t___constructEv

this_ => rdi
*/

void th_7DOMNode___construct(ObjectData* this_) asm("_ZN4HPHP9c_DOMNode13t___constructEv");

TypedValue* tg_7DOMNode___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_7DOMNode___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNode::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t_appendchild(HPHP::Object const&)
_ZN4HPHP9c_DOMNode13t_appendchildERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
newnode => rdx
*/

TypedValue* th_7DOMNode_appendChild(TypedValue* _rv, ObjectData* this_, Value* newnode) asm("_ZN4HPHP9c_DOMNode13t_appendchildERKNS_6ObjectE");

TypedValue* tg1_7DOMNode_appendChild(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_appendChild(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_7DOMNode_appendChild((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7DOMNode_appendChild(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          th_7DOMNode_appendChild((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_appendChild(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNode::appendChild", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::appendChild");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t_clonenode(bool)
_ZN4HPHP9c_DOMNode11t_clonenodeEb

(return value) => rax
_rv => rdi
this_ => rsi
deep => rdx
*/

TypedValue* th_7DOMNode_cloneNode(TypedValue* _rv, ObjectData* this_, bool deep) asm("_ZN4HPHP9c_DOMNode11t_clonenodeEb");

TypedValue* tg1_7DOMNode_cloneNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_cloneNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  th_7DOMNode_cloneNode((rv), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7DOMNode_cloneNode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
          th_7DOMNode_cloneNode((&(rv)), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_cloneNode(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DOMNode::cloneNode", 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::cloneNode");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_DOMNode::t_getlineno()
_ZN4HPHP9c_DOMNode11t_getlinenoEv

(return value) => rax
this_ => rdi
*/

long th_7DOMNode_getLineNo(ObjectData* this_) asm("_ZN4HPHP9c_DOMNode11t_getlinenoEv");

TypedValue* tg_7DOMNode_getLineNo(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_7DOMNode_getLineNo((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNode::getLineNo", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::getLineNo");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMNode::t_hasattributes()
_ZN4HPHP9c_DOMNode15t_hasattributesEv

(return value) => rax
this_ => rdi
*/

bool th_7DOMNode_hasAttributes(ObjectData* this_) asm("_ZN4HPHP9c_DOMNode15t_hasattributesEv");

TypedValue* tg_7DOMNode_hasAttributes(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_7DOMNode_hasAttributes((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNode::hasAttributes", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::hasAttributes");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMNode::t_haschildnodes()
_ZN4HPHP9c_DOMNode15t_haschildnodesEv

(return value) => rax
this_ => rdi
*/

bool th_7DOMNode_hasChildNodes(ObjectData* this_) asm("_ZN4HPHP9c_DOMNode15t_haschildnodesEv");

TypedValue* tg_7DOMNode_hasChildNodes(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_7DOMNode_hasChildNodes((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNode::hasChildNodes", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::hasChildNodes");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t_insertbefore(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP9c_DOMNode14t_insertbeforeERKNS_6ObjectES3_

(return value) => rax
_rv => rdi
this_ => rsi
newnode => rdx
refnode => rcx
*/

TypedValue* th_7DOMNode_insertBefore(TypedValue* _rv, ObjectData* this_, Value* newnode, Value* refnode) asm("_ZN4HPHP9c_DOMNode14t_insertbeforeERKNS_6ObjectES3_");

TypedValue* tg1_7DOMNode_insertBefore(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_insertBefore(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  Object defVal1 = uninit_null();
  th_7DOMNode_insertBefore((rv), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7DOMNode_insertBefore(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfObject) && (args-0)->m_type == KindOfObject) {
          Object defVal1 = uninit_null();
          th_7DOMNode_insertBefore((&(rv)), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_insertBefore(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNode::insertBefore", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::insertBefore");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMNode::t_isdefaultnamespace(HPHP::String const&)
_ZN4HPHP9c_DOMNode20t_isdefaultnamespaceERKNS_6StringE

(return value) => rax
this_ => rdi
namespaceuri => rsi
*/

bool th_7DOMNode_isDefaultNamespace(ObjectData* this_, Value* namespaceuri) asm("_ZN4HPHP9c_DOMNode20t_isdefaultnamespaceERKNS_6StringE");

TypedValue* tg1_7DOMNode_isDefaultNamespace(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_isDefaultNamespace(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_7DOMNode_isDefaultNamespace((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_7DOMNode_isDefaultNamespace(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_7DOMNode_isDefaultNamespace((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_isDefaultNamespace(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNode::isDefaultNamespace", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::isDefaultNamespace");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMNode::t_issamenode(HPHP::Object const&)
_ZN4HPHP9c_DOMNode12t_issamenodeERKNS_6ObjectE

(return value) => rax
this_ => rdi
node => rsi
*/

bool th_7DOMNode_isSameNode(ObjectData* this_, Value* node) asm("_ZN4HPHP9c_DOMNode12t_issamenodeERKNS_6ObjectE");

TypedValue* tg1_7DOMNode_isSameNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_isSameNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (th_7DOMNode_isSameNode((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_7DOMNode_isSameNode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_7DOMNode_isSameNode((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_isSameNode(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNode::isSameNode", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::isSameNode");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMNode::t_issupported(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9c_DOMNode13t_issupportedERKNS_6StringES3_

(return value) => rax
this_ => rdi
feature => rsi
version => rdx
*/

bool th_7DOMNode_isSupported(ObjectData* this_, Value* feature, Value* version) asm("_ZN4HPHP9c_DOMNode13t_issupportedERKNS_6StringES3_");

TypedValue* tg1_7DOMNode_isSupported(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_isSupported(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_7DOMNode_isSupported((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_7DOMNode_isSupported(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_7DOMNode_isSupported((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_isSupported(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNode::isSupported", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::isSupported");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t_lookupnamespaceuri(HPHP::String const&)
_ZN4HPHP9c_DOMNode20t_lookupnamespaceuriERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
*/

TypedValue* th_7DOMNode_lookupNamespaceUri(TypedValue* _rv, ObjectData* this_, Value* namespaceuri) asm("_ZN4HPHP9c_DOMNode20t_lookupnamespaceuriERKNS_6StringE");

TypedValue* tg1_7DOMNode_lookupNamespaceUri(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_lookupNamespaceUri(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_7DOMNode_lookupNamespaceUri((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7DOMNode_lookupNamespaceUri(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_7DOMNode_lookupNamespaceUri((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_lookupNamespaceUri(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNode::lookupNamespaceUri", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::lookupNamespaceUri");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t_lookupprefix(HPHP::String const&)
_ZN4HPHP9c_DOMNode14t_lookupprefixERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
prefix => rdx
*/

TypedValue* th_7DOMNode_lookupPrefix(TypedValue* _rv, ObjectData* this_, Value* prefix) asm("_ZN4HPHP9c_DOMNode14t_lookupprefixERKNS_6StringE");

TypedValue* tg1_7DOMNode_lookupPrefix(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_lookupPrefix(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_7DOMNode_lookupPrefix((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7DOMNode_lookupPrefix(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_7DOMNode_lookupPrefix((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_lookupPrefix(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNode::lookupPrefix", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::lookupPrefix");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DOMNode::t_normalize()
_ZN4HPHP9c_DOMNode11t_normalizeEv

this_ => rdi
*/

void th_7DOMNode_normalize(ObjectData* this_) asm("_ZN4HPHP9c_DOMNode11t_normalizeEv");

TypedValue* tg_7DOMNode_normalize(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_7DOMNode_normalize((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNode::normalize", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::normalize");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t_removechild(HPHP::Object const&)
_ZN4HPHP9c_DOMNode13t_removechildERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
node => rdx
*/

TypedValue* th_7DOMNode_removeChild(TypedValue* _rv, ObjectData* this_, Value* node) asm("_ZN4HPHP9c_DOMNode13t_removechildERKNS_6ObjectE");

TypedValue* tg1_7DOMNode_removeChild(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_removeChild(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_7DOMNode_removeChild((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7DOMNode_removeChild(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          th_7DOMNode_removeChild((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_removeChild(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNode::removeChild", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::removeChild");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t_replacechild(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP9c_DOMNode14t_replacechildERKNS_6ObjectES3_

(return value) => rax
_rv => rdi
this_ => rsi
newchildobj => rdx
oldchildobj => rcx
*/

TypedValue* th_7DOMNode_replaceChild(TypedValue* _rv, ObjectData* this_, Value* newchildobj, Value* oldchildobj) asm("_ZN4HPHP9c_DOMNode14t_replacechildERKNS_6ObjectES3_");

TypedValue* tg1_7DOMNode_replaceChild(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_replaceChild(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  th_7DOMNode_replaceChild((rv), (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7DOMNode_replaceChild(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
          th_7DOMNode_replaceChild((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_replaceChild(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNode::replaceChild", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::replaceChild");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t_c14n(bool, bool, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP9c_DOMNode6t_c14nEbbRKNS_7VariantES3_

(return value) => rax
_rv => rdi
this_ => rsi
exclusive => rdx
with_comments => rcx
xpath => r8
ns_prefixes => r9
*/

TypedValue* th_7DOMNode_c14n(TypedValue* _rv, ObjectData* this_, bool exclusive, bool with_comments, TypedValue* xpath, TypedValue* ns_prefixes) asm("_ZN4HPHP9c_DOMNode6t_c14nEbbRKNS_7VariantES3_");

TypedValue* tg1_7DOMNode_c14n(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_c14n(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    if ((args-0)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-0);
    }
  case 0:
    break;
  }
  Variant defVal2;
  Variant defVal3;
  th_7DOMNode_c14n((rv), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7DOMNode_c14n(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 4LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (count <= 0 || (args-0)->m_type == KindOfBoolean)) {
          Variant defVal2;
          Variant defVal3;
          th_7DOMNode_c14n((&(rv)), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_c14n(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DOMNode::c14n", 4, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::c14n");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t_c14nfile(HPHP::String const&, bool, bool, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP9c_DOMNode10t_c14nfileERKNS_6StringEbbRKNS_7VariantES6_

(return value) => rax
_rv => rdi
this_ => rsi
uri => rdx
exclusive => rcx
with_comments => r8
xpath => r9
ns_prefixes => st0
*/

TypedValue* th_7DOMNode_c14nfile(TypedValue* _rv, ObjectData* this_, Value* uri, bool exclusive, bool with_comments, TypedValue* xpath, TypedValue* ns_prefixes) asm("_ZN4HPHP9c_DOMNode10t_c14nfileERKNS_6StringEbbRKNS_7VariantES6_");

TypedValue* tg1_7DOMNode_c14nfile(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMNode_c14nfile(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
  case 4:
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal3;
  Variant defVal4;
  th_7DOMNode_c14nfile((rv), (this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7DOMNode_c14nfile(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 5LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfBoolean) && IS_STRING_TYPE((args-0)->m_type)) {
          Variant defVal3;
          Variant defVal4;
          th_7DOMNode_c14nfile((&(rv)), (this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args-4) : (TypedValue*)(&defVal4));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMNode_c14nfile(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNode::c14nfile", count, 1, 5, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::c14nfile");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t_getnodepath()
_ZN4HPHP9c_DOMNode13t_getnodepathEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_7DOMNode_getNodePath(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP9c_DOMNode13t_getnodepathEv");

TypedValue* tg_7DOMNode_getNodePath(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_7DOMNode_getNodePath((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNode::getNodePath", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::getNodePath");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t___get(HPHP::Variant)
_ZN4HPHP9c_DOMNode7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_7DOMNode___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP9c_DOMNode7t___getENS_7VariantE");

TypedValue* tg_7DOMNode___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_7DOMNode___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNode::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNode::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP9c_DOMNode7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_7DOMNode___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP9c_DOMNode7t___setENS_7VariantES1_");

TypedValue* tg_7DOMNode___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_7DOMNode___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNode::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMNode::t___isset(HPHP::Variant)
_ZN4HPHP9c_DOMNode9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_7DOMNode___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP9c_DOMNode9t___issetENS_7VariantE");

TypedValue* tg_7DOMNode___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_7DOMNode___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNode::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNode::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMAttr_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMAttr) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMAttr(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMAttr);
/*
void HPHP::c_DOMAttr::t___construct(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9c_DOMAttr13t___constructERKNS_6StringES3_

this_ => rdi
name => rsi
value => rdx
*/

void th_7DOMAttr___construct(ObjectData* this_, Value* name, Value* value) asm("_ZN4HPHP9c_DOMAttr13t___constructERKNS_6StringES3_");

TypedValue* tg1_7DOMAttr___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMAttr___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
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
  th_7DOMAttr___construct((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_7DOMAttr___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_7DOMAttr___construct((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMAttr___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMAttr::__construct", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMAttr::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMAttr::t_isid()
_ZN4HPHP9c_DOMAttr6t_isidEv

(return value) => rax
this_ => rdi
*/

bool th_7DOMAttr_isId(ObjectData* this_) asm("_ZN4HPHP9c_DOMAttr6t_isidEv");

TypedValue* tg_7DOMAttr_isId(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_7DOMAttr_isId((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMAttr::isId", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMAttr::isId");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMAttr::t___get(HPHP::Variant)
_ZN4HPHP9c_DOMAttr7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_7DOMAttr___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP9c_DOMAttr7t___getENS_7VariantE");

TypedValue* tg_7DOMAttr___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_7DOMAttr___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMAttr::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMAttr::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMAttr::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP9c_DOMAttr7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_7DOMAttr___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP9c_DOMAttr7t___setENS_7VariantES1_");

TypedValue* tg_7DOMAttr___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_7DOMAttr___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMAttr::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMAttr::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMAttr::t___isset(HPHP::Variant)
_ZN4HPHP9c_DOMAttr9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_7DOMAttr___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP9c_DOMAttr9t___issetENS_7VariantE");

TypedValue* tg_7DOMAttr___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_7DOMAttr___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMAttr::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMAttr::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMCharacterData_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMCharacterData) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMCharacterData(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMCharacterData);
/*
void HPHP::c_DOMCharacterData::t___construct()
_ZN4HPHP18c_DOMCharacterData13t___constructEv

this_ => rdi
*/

void th_16DOMCharacterData___construct(ObjectData* this_) asm("_ZN4HPHP18c_DOMCharacterData13t___constructEv");

TypedValue* tg_16DOMCharacterData___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_16DOMCharacterData___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMCharacterData::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMCharacterData::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMCharacterData::t_appenddata(HPHP::String const&)
_ZN4HPHP18c_DOMCharacterData12t_appenddataERKNS_6StringE

(return value) => rax
this_ => rdi
arg => rsi
*/

bool th_16DOMCharacterData_appendData(ObjectData* this_, Value* arg) asm("_ZN4HPHP18c_DOMCharacterData12t_appenddataERKNS_6StringE");

TypedValue* tg1_16DOMCharacterData_appendData(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16DOMCharacterData_appendData(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_16DOMCharacterData_appendData((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_16DOMCharacterData_appendData(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_16DOMCharacterData_appendData((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16DOMCharacterData_appendData(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMCharacterData::appendData", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMCharacterData::appendData");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMCharacterData::t_deletedata(long, long)
_ZN4HPHP18c_DOMCharacterData12t_deletedataEll

(return value) => rax
this_ => rdi
offset => rsi
count => rdx
*/

bool th_16DOMCharacterData_deleteData(ObjectData* this_, long offset, long count) asm("_ZN4HPHP18c_DOMCharacterData12t_deletedataEll");

TypedValue* tg1_16DOMCharacterData_deleteData(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16DOMCharacterData_deleteData(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (th_16DOMCharacterData_deleteData((this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_16DOMCharacterData_deleteData(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_16DOMCharacterData_deleteData((this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16DOMCharacterData_deleteData(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMCharacterData::deleteData", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMCharacterData::deleteData");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMCharacterData::t_insertdata(long, HPHP::String const&)
_ZN4HPHP18c_DOMCharacterData12t_insertdataElRKNS_6StringE

(return value) => rax
this_ => rdi
offset => rsi
data => rdx
*/

bool th_16DOMCharacterData_insertData(ObjectData* this_, long offset, Value* data) asm("_ZN4HPHP18c_DOMCharacterData12t_insertdataElRKNS_6StringE");

TypedValue* tg1_16DOMCharacterData_insertData(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16DOMCharacterData_insertData(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (th_16DOMCharacterData_insertData((this_), (long)(args[-0].m_data.num), &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_16DOMCharacterData_insertData(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_16DOMCharacterData_insertData((this_), (long)(args[-0].m_data.num), &args[-1].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16DOMCharacterData_insertData(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMCharacterData::insertData", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMCharacterData::insertData");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMCharacterData::t_replacedata(long, long, HPHP::String const&)
_ZN4HPHP18c_DOMCharacterData13t_replacedataEllRKNS_6StringE

(return value) => rax
this_ => rdi
offset => rsi
count => rdx
data => rcx
*/

bool th_16DOMCharacterData_replaceData(ObjectData* this_, long offset, long count, Value* data) asm("_ZN4HPHP18c_DOMCharacterData13t_replacedataEllRKNS_6StringE");

TypedValue* tg1_16DOMCharacterData_replaceData(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16DOMCharacterData_replaceData(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (th_16DOMCharacterData_replaceData((this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), &args[-2].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_16DOMCharacterData_replaceData(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 3LL) {
        if (IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_16DOMCharacterData_replaceData((this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), &args[-2].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16DOMCharacterData_replaceData(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMCharacterData::replaceData", count, 3, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DOMCharacterData::replaceData");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_DOMCharacterData::t_substringdata(long, long)
_ZN4HPHP18c_DOMCharacterData15t_substringdataEll

(return value) => rax
_rv => rdi
this_ => rsi
offset => rdx
count => rcx
*/

Value* th_16DOMCharacterData_substringData(Value* _rv, ObjectData* this_, long offset, long count) asm("_ZN4HPHP18c_DOMCharacterData15t_substringdataEll");

TypedValue* tg1_16DOMCharacterData_substringData(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16DOMCharacterData_substringData(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_16DOMCharacterData_substringData((&rv->m_data), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_16DOMCharacterData_substringData(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfString;
          th_16DOMCharacterData_substringData((&rv.m_data), (this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16DOMCharacterData_substringData(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMCharacterData::substringData", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMCharacterData::substringData");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMCharacterData::t___get(HPHP::Variant)
_ZN4HPHP18c_DOMCharacterData7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_16DOMCharacterData___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP18c_DOMCharacterData7t___getENS_7VariantE");

TypedValue* tg_16DOMCharacterData___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_16DOMCharacterData___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMCharacterData::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMCharacterData::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMCharacterData::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP18c_DOMCharacterData7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_16DOMCharacterData___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP18c_DOMCharacterData7t___setENS_7VariantES1_");

TypedValue* tg_16DOMCharacterData___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_16DOMCharacterData___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMCharacterData::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMCharacterData::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMCharacterData::t___isset(HPHP::Variant)
_ZN4HPHP18c_DOMCharacterData9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_16DOMCharacterData___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP18c_DOMCharacterData9t___issetENS_7VariantE");

TypedValue* tg_16DOMCharacterData___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_16DOMCharacterData___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMCharacterData::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMCharacterData::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMComment_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMComment) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMComment(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMComment);
/*
void HPHP::c_DOMComment::t___construct(HPHP::String const&)
_ZN4HPHP12c_DOMComment13t___constructERKNS_6StringE

this_ => rdi
value => rsi
*/

void th_10DOMComment___construct(ObjectData* this_, Value* value) asm("_ZN4HPHP12c_DOMComment13t___constructERKNS_6StringE");

TypedValue* tg1_10DOMComment___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMComment___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_10DOMComment___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_10DOMComment___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_10DOMComment___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMComment___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DOMComment::__construct", 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMComment::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMText_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMText) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMText(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMText);
/*
void HPHP::c_DOMText::t___construct(HPHP::String const&)
_ZN4HPHP9c_DOMText13t___constructERKNS_6StringE

this_ => rdi
value => rsi
*/

void th_7DOMText___construct(ObjectData* this_, Value* value) asm("_ZN4HPHP9c_DOMText13t___constructERKNS_6StringE");

TypedValue* tg1_7DOMText___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMText___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_7DOMText___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_7DOMText___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_7DOMText___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMText___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DOMText::__construct", 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMText::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMText::t_iswhitespaceinelementcontent()
_ZN4HPHP9c_DOMText30t_iswhitespaceinelementcontentEv

(return value) => rax
this_ => rdi
*/

bool th_7DOMText_isWhitespaceInElementContent(ObjectData* this_) asm("_ZN4HPHP9c_DOMText30t_iswhitespaceinelementcontentEv");

TypedValue* tg_7DOMText_isWhitespaceInElementContent(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_7DOMText_isWhitespaceInElementContent((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMText::isWhitespaceInElementContent", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMText::isWhitespaceInElementContent");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMText::t_splittext(long)
_ZN4HPHP9c_DOMText11t_splittextEl

(return value) => rax
_rv => rdi
this_ => rsi
offset => rdx
*/

TypedValue* th_7DOMText_splitText(TypedValue* _rv, ObjectData* this_, long offset) asm("_ZN4HPHP9c_DOMText11t_splittextEl");

TypedValue* tg1_7DOMText_splitText(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_7DOMText_splitText(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_7DOMText_splitText((rv), (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_7DOMText_splitText(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          th_7DOMText_splitText((&(rv)), (this_), (long)(args[-0].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_7DOMText_splitText(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMText::splitText", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMText::splitText");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMText::t___get(HPHP::Variant)
_ZN4HPHP9c_DOMText7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_7DOMText___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP9c_DOMText7t___getENS_7VariantE");

TypedValue* tg_7DOMText___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_7DOMText___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMText::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMText::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMText::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP9c_DOMText7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_7DOMText___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP9c_DOMText7t___setENS_7VariantES1_");

TypedValue* tg_7DOMText___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_7DOMText___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMText::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMText::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMText::t___isset(HPHP::Variant)
_ZN4HPHP9c_DOMText9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_7DOMText___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP9c_DOMText9t___issetENS_7VariantE");

TypedValue* tg_7DOMText___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_7DOMText___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMText::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMText::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMCDATASection_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMCDATASection) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMCDATASection(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMCDATASection);
/*
void HPHP::c_DOMCDATASection::t___construct(HPHP::String const&)
_ZN4HPHP17c_DOMCDATASection13t___constructERKNS_6StringE

this_ => rdi
value => rsi
*/

void th_15DOMCDATASection___construct(ObjectData* this_, Value* value) asm("_ZN4HPHP17c_DOMCDATASection13t___constructERKNS_6StringE");

TypedValue* tg1_15DOMCDATASection___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_15DOMCDATASection___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_15DOMCDATASection___construct((this_), &args[-0].m_data);
  return rv;
}

TypedValue* tg_15DOMCDATASection___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_15DOMCDATASection___construct((this_), &args[-0].m_data);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_15DOMCDATASection___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMCDATASection::__construct", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMCDATASection::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMDocument_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMDocument) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMDocument(cls);
  return inst;
}

IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(DOMDocument);
/*
void HPHP::c_DOMDocument::t___construct(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13c_DOMDocument13t___constructERKNS_6StringES3_

this_ => rdi
version => rsi
encoding => rdx
*/

void th_11DOMDocument___construct(ObjectData* this_, Value* version, Value* encoding) asm("_ZN4HPHP13c_DOMDocument13t___constructERKNS_6StringES3_");

TypedValue* tg1_11DOMDocument___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
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
  th_11DOMDocument___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_11DOMDocument___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 2LL) {
        if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_11DOMDocument___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DOMDocument::__construct", 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_createattribute(HPHP::String const&)
_ZN4HPHP13c_DOMDocument17t_createattributeERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_11DOMDocument_createAttribute(TypedValue* _rv, ObjectData* this_, Value* name) asm("_ZN4HPHP13c_DOMDocument17t_createattributeERKNS_6StringE");

TypedValue* tg1_11DOMDocument_createAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_createAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_11DOMDocument_createAttribute((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_createAttribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_createAttribute((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_createAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::createAttribute", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::createAttribute");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_createattributens(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13c_DOMDocument19t_createattributensERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
qualifiedname => rcx
*/

TypedValue* th_11DOMDocument_createAttributens(TypedValue* _rv, ObjectData* this_, Value* namespaceuri, Value* qualifiedname) asm("_ZN4HPHP13c_DOMDocument19t_createattributensERKNS_6StringES3_");

TypedValue* tg1_11DOMDocument_createAttributens(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_createAttributens(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_11DOMDocument_createAttributens((rv), (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_createAttributens(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_createAttributens((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_createAttributens(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::createAttributens", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::createAttributens");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_createcdatasection(HPHP::String const&)
_ZN4HPHP13c_DOMDocument20t_createcdatasectionERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
data => rdx
*/

TypedValue* th_11DOMDocument_createCDATASection(TypedValue* _rv, ObjectData* this_, Value* data) asm("_ZN4HPHP13c_DOMDocument20t_createcdatasectionERKNS_6StringE");

TypedValue* tg1_11DOMDocument_createCDATASection(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_createCDATASection(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_11DOMDocument_createCDATASection((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_createCDATASection(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_createCDATASection((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_createCDATASection(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::createCDATASection", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::createCDATASection");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_createcomment(HPHP::String const&)
_ZN4HPHP13c_DOMDocument15t_createcommentERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
data => rdx
*/

TypedValue* th_11DOMDocument_createComment(TypedValue* _rv, ObjectData* this_, Value* data) asm("_ZN4HPHP13c_DOMDocument15t_createcommentERKNS_6StringE");

TypedValue* tg1_11DOMDocument_createComment(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_createComment(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_11DOMDocument_createComment((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_createComment(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_createComment((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_createComment(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::createComment", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::createComment");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_createdocumentfragment()
_ZN4HPHP13c_DOMDocument24t_createdocumentfragmentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_11DOMDocument_createDocumentFragment(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP13c_DOMDocument24t_createdocumentfragmentEv");

TypedValue* tg_11DOMDocument_createDocumentFragment(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_11DOMDocument_createDocumentFragment((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMDocument::createDocumentFragment", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::createDocumentFragment");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_createelement(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13c_DOMDocument15t_createelementERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_11DOMDocument_createElement(TypedValue* _rv, ObjectData* this_, Value* name, Value* value) asm("_ZN4HPHP13c_DOMDocument15t_createelementERKNS_6StringES3_");

TypedValue* tg1_11DOMDocument_createElement(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_createElement(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  th_11DOMDocument_createElement((rv), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_createElement(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_createElement((&(rv)), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_createElement(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::createElement", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::createElement");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_createelementns(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP13c_DOMDocument17t_createelementnsERKNS_6StringES3_S3_

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
qualifiedname => rcx
value => r8
*/

TypedValue* th_11DOMDocument_createElementNS(TypedValue* _rv, ObjectData* this_, Value* namespaceuri, Value* qualifiedname, Value* value) asm("_ZN4HPHP13c_DOMDocument17t_createelementnsERKNS_6StringES3_S3_");

TypedValue* tg1_11DOMDocument_createElementNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_createElementNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  th_11DOMDocument_createElementNS((rv), (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_createElementNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_createElementNS((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_createElementNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::createElementNS", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::createElementNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_createentityreference(HPHP::String const&)
_ZN4HPHP13c_DOMDocument23t_createentityreferenceERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_11DOMDocument_createEntityReference(TypedValue* _rv, ObjectData* this_, Value* name) asm("_ZN4HPHP13c_DOMDocument23t_createentityreferenceERKNS_6StringE");

TypedValue* tg1_11DOMDocument_createEntityReference(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_createEntityReference(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_11DOMDocument_createEntityReference((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_createEntityReference(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_createEntityReference((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_createEntityReference(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::createEntityReference", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::createEntityReference");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_createprocessinginstruction(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13c_DOMDocument29t_createprocessinginstructionERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
target => rdx
data => rcx
*/

TypedValue* th_11DOMDocument_createProcessingInstruction(TypedValue* _rv, ObjectData* this_, Value* target, Value* data) asm("_ZN4HPHP13c_DOMDocument29t_createprocessinginstructionERKNS_6StringES3_");

TypedValue* tg1_11DOMDocument_createProcessingInstruction(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_createProcessingInstruction(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  th_11DOMDocument_createProcessingInstruction((rv), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_createProcessingInstruction(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_createProcessingInstruction((&(rv)), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_createProcessingInstruction(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::createProcessingInstruction", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::createProcessingInstruction");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_createtextnode(HPHP::String const&)
_ZN4HPHP13c_DOMDocument16t_createtextnodeERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
data => rdx
*/

TypedValue* th_11DOMDocument_createTextNode(TypedValue* _rv, ObjectData* this_, Value* data) asm("_ZN4HPHP13c_DOMDocument16t_createtextnodeERKNS_6StringE");

TypedValue* tg1_11DOMDocument_createTextNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_createTextNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_11DOMDocument_createTextNode((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_createTextNode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_createTextNode((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_createTextNode(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::createTextNode", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::createTextNode");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_getelementbyid(HPHP::String const&)
_ZN4HPHP13c_DOMDocument16t_getelementbyidERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
elementid => rdx
*/

TypedValue* th_11DOMDocument_getElementById(TypedValue* _rv, ObjectData* this_, Value* elementid) asm("_ZN4HPHP13c_DOMDocument16t_getelementbyidERKNS_6StringE");

TypedValue* tg1_11DOMDocument_getElementById(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_getElementById(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_11DOMDocument_getElementById((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_getElementById(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_getElementById((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_getElementById(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::getElementById", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::getElementById");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_getelementsbytagname(HPHP::String const&)
_ZN4HPHP13c_DOMDocument22t_getelementsbytagnameERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_11DOMDocument_getElementsByTagName(TypedValue* _rv, ObjectData* this_, Value* name) asm("_ZN4HPHP13c_DOMDocument22t_getelementsbytagnameERKNS_6StringE");

TypedValue* tg1_11DOMDocument_getElementsByTagName(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_getElementsByTagName(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_11DOMDocument_getElementsByTagName((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_getElementsByTagName(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_getElementsByTagName((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_getElementsByTagName(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::getElementsByTagName", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::getElementsByTagName");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_getelementsbytagnamens(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13c_DOMDocument24t_getelementsbytagnamensERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* th_11DOMDocument_getElementsByTagNameNS(TypedValue* _rv, ObjectData* this_, Value* namespaceuri, Value* localname) asm("_ZN4HPHP13c_DOMDocument24t_getelementsbytagnamensERKNS_6StringES3_");

TypedValue* tg1_11DOMDocument_getElementsByTagNameNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_getElementsByTagNameNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_11DOMDocument_getElementsByTagNameNS((rv), (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_getElementsByTagNameNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_getElementsByTagNameNS((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_getElementsByTagNameNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::getElementsByTagNameNS", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::getElementsByTagNameNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_importnode(HPHP::Object const&, bool)
_ZN4HPHP13c_DOMDocument12t_importnodeERKNS_6ObjectEb

(return value) => rax
_rv => rdi
this_ => rsi
importednode => rdx
deep => rcx
*/

TypedValue* th_11DOMDocument_importNode(TypedValue* _rv, ObjectData* this_, Value* importednode, bool deep) asm("_ZN4HPHP13c_DOMDocument12t_importnodeERKNS_6ObjectEb");

TypedValue* tg1_11DOMDocument_importNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_importNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  th_11DOMDocument_importNode((rv), (this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_importNode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfObject) {
          th_11DOMDocument_importNode((&(rv)), (this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_importNode(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::importNode", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::importNode");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_load(HPHP::String const&, long)
_ZN4HPHP13c_DOMDocument6t_loadERKNS_6StringEl

(return value) => rax
_rv => rdi
this_ => rsi
filename => rdx
options => rcx
*/

TypedValue* th_11DOMDocument_load(TypedValue* _rv, ObjectData* this_, Value* filename, long options) asm("_ZN4HPHP13c_DOMDocument6t_loadERKNS_6StringEl");

TypedValue* tg1_11DOMDocument_load(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_load(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  th_11DOMDocument_load((rv), (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_load(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_load((&(rv)), (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_load(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::load", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::load");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_loadhtml(HPHP::String const&)
_ZN4HPHP13c_DOMDocument10t_loadhtmlERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
source => rdx
*/

TypedValue* th_11DOMDocument_loadHTML(TypedValue* _rv, ObjectData* this_, Value* source) asm("_ZN4HPHP13c_DOMDocument10t_loadhtmlERKNS_6StringE");

TypedValue* tg1_11DOMDocument_loadHTML(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_loadHTML(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_11DOMDocument_loadHTML((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_loadHTML(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_loadHTML((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_loadHTML(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::loadHTML", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::loadHTML");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_loadhtmlfile(HPHP::String const&)
_ZN4HPHP13c_DOMDocument14t_loadhtmlfileERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
filename => rdx
*/

TypedValue* th_11DOMDocument_loadHTMLFile(TypedValue* _rv, ObjectData* this_, Value* filename) asm("_ZN4HPHP13c_DOMDocument14t_loadhtmlfileERKNS_6StringE");

TypedValue* tg1_11DOMDocument_loadHTMLFile(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_loadHTMLFile(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_11DOMDocument_loadHTMLFile((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_loadHTMLFile(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_loadHTMLFile((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_loadHTMLFile(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::loadHTMLFile", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::loadHTMLFile");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_loadxml(HPHP::String const&, long)
_ZN4HPHP13c_DOMDocument9t_loadxmlERKNS_6StringEl

(return value) => rax
_rv => rdi
this_ => rsi
source => rdx
options => rcx
*/

TypedValue* th_11DOMDocument_loadXML(TypedValue* _rv, ObjectData* this_, Value* source, long options) asm("_ZN4HPHP13c_DOMDocument9t_loadxmlERKNS_6StringEl");

TypedValue* tg1_11DOMDocument_loadXML(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_loadXML(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  th_11DOMDocument_loadXML((rv), (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_loadXML(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_loadXML((&(rv)), (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_loadXML(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::loadXML", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::loadXML");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DOMDocument::t_normalizedocument()
_ZN4HPHP13c_DOMDocument19t_normalizedocumentEv

this_ => rdi
*/

void th_11DOMDocument_normalizeDocument(ObjectData* this_) asm("_ZN4HPHP13c_DOMDocument19t_normalizedocumentEv");

TypedValue* tg_11DOMDocument_normalizeDocument(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_11DOMDocument_normalizeDocument((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMDocument::normalizeDocument", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::normalizeDocument");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMDocument::t_registernodeclass(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13c_DOMDocument19t_registernodeclassERKNS_6StringES3_

(return value) => rax
this_ => rdi
baseclass => rsi
extendedclass => rdx
*/

bool th_11DOMDocument_registerNodeClass(ObjectData* this_, Value* baseclass, Value* extendedclass) asm("_ZN4HPHP13c_DOMDocument19t_registernodeclassERKNS_6StringES3_");

TypedValue* tg1_11DOMDocument_registerNodeClass(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_registerNodeClass(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_11DOMDocument_registerNodeClass((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_11DOMDocument_registerNodeClass(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_11DOMDocument_registerNodeClass((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_registerNodeClass(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::registerNodeClass", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::registerNodeClass");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMDocument::t_relaxngvalidate(HPHP::String const&)
_ZN4HPHP13c_DOMDocument17t_relaxngvalidateERKNS_6StringE

(return value) => rax
this_ => rdi
filename => rsi
*/

bool th_11DOMDocument_relaxNGValidate(ObjectData* this_, Value* filename) asm("_ZN4HPHP13c_DOMDocument17t_relaxngvalidateERKNS_6StringE");

TypedValue* tg1_11DOMDocument_relaxNGValidate(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_relaxNGValidate(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_11DOMDocument_relaxNGValidate((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_11DOMDocument_relaxNGValidate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_11DOMDocument_relaxNGValidate((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_relaxNGValidate(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::relaxNGValidate", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::relaxNGValidate");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMDocument::t_relaxngvalidatesource(HPHP::String const&)
_ZN4HPHP13c_DOMDocument23t_relaxngvalidatesourceERKNS_6StringE

(return value) => rax
this_ => rdi
source => rsi
*/

bool th_11DOMDocument_relaxNGValidateSource(ObjectData* this_, Value* source) asm("_ZN4HPHP13c_DOMDocument23t_relaxngvalidatesourceERKNS_6StringE");

TypedValue* tg1_11DOMDocument_relaxNGValidateSource(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_relaxNGValidateSource(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_11DOMDocument_relaxNGValidateSource((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_11DOMDocument_relaxNGValidateSource(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_11DOMDocument_relaxNGValidateSource((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_relaxNGValidateSource(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::relaxNGValidateSource", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::relaxNGValidateSource");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_save(HPHP::String const&, long)
_ZN4HPHP13c_DOMDocument6t_saveERKNS_6StringEl

(return value) => rax
_rv => rdi
this_ => rsi
file => rdx
options => rcx
*/

TypedValue* th_11DOMDocument_save(TypedValue* _rv, ObjectData* this_, Value* file, long options) asm("_ZN4HPHP13c_DOMDocument6t_saveERKNS_6StringEl");

TypedValue* tg1_11DOMDocument_save(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_save(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  th_11DOMDocument_save((rv), (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_save(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_save((&(rv)), (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_save(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::save", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::save");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_savehtml()
_ZN4HPHP13c_DOMDocument10t_savehtmlEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_11DOMDocument_saveHTML(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP13c_DOMDocument10t_savehtmlEv");

TypedValue* tg_11DOMDocument_saveHTML(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_11DOMDocument_saveHTML((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMDocument::saveHTML", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::saveHTML");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_savehtmlfile(HPHP::String const&)
_ZN4HPHP13c_DOMDocument14t_savehtmlfileERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
file => rdx
*/

TypedValue* th_11DOMDocument_saveHTMLFile(TypedValue* _rv, ObjectData* this_, Value* file) asm("_ZN4HPHP13c_DOMDocument14t_savehtmlfileERKNS_6StringE");

TypedValue* tg1_11DOMDocument_saveHTMLFile(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_saveHTMLFile(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_11DOMDocument_saveHTMLFile((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_saveHTMLFile(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_11DOMDocument_saveHTMLFile((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_saveHTMLFile(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::saveHTMLFile", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::saveHTMLFile");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_savexml(HPHP::Object const&, long)
_ZN4HPHP13c_DOMDocument9t_savexmlERKNS_6ObjectEl

(return value) => rax
_rv => rdi
this_ => rsi
node => rdx
options => rcx
*/

TypedValue* th_11DOMDocument_saveXML(TypedValue* _rv, ObjectData* this_, Value* node, long options) asm("_ZN4HPHP13c_DOMDocument9t_savexmlERKNS_6ObjectEl");

TypedValue* tg1_11DOMDocument_saveXML(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_saveXML(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    if ((args-0)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-0);
    }
  case 0:
    break;
  }
  th_11DOMDocument_saveXML((rv), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_object), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_saveXML(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfObject)) {
          th_11DOMDocument_saveXML((&(rv)), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_object), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_saveXML(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DOMDocument::saveXML", 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::saveXML");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMDocument::t_schemavalidate(HPHP::String const&)
_ZN4HPHP13c_DOMDocument16t_schemavalidateERKNS_6StringE

(return value) => rax
this_ => rdi
filename => rsi
*/

bool th_11DOMDocument_schemaValidate(ObjectData* this_, Value* filename) asm("_ZN4HPHP13c_DOMDocument16t_schemavalidateERKNS_6StringE");

TypedValue* tg1_11DOMDocument_schemaValidate(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_schemaValidate(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_11DOMDocument_schemaValidate((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_11DOMDocument_schemaValidate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_11DOMDocument_schemaValidate((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_schemaValidate(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::schemaValidate", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::schemaValidate");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMDocument::t_schemavalidatesource(HPHP::String const&)
_ZN4HPHP13c_DOMDocument22t_schemavalidatesourceERKNS_6StringE

(return value) => rax
this_ => rdi
source => rsi
*/

bool th_11DOMDocument_schemaValidateSource(ObjectData* this_, Value* source) asm("_ZN4HPHP13c_DOMDocument22t_schemavalidatesourceERKNS_6StringE");

TypedValue* tg1_11DOMDocument_schemaValidateSource(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_schemaValidateSource(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_11DOMDocument_schemaValidateSource((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_11DOMDocument_schemaValidateSource(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_11DOMDocument_schemaValidateSource((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_schemaValidateSource(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocument::schemaValidateSource", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::schemaValidateSource");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMDocument::t_validate()
_ZN4HPHP13c_DOMDocument10t_validateEv

(return value) => rax
this_ => rdi
*/

bool th_11DOMDocument_validate(ObjectData* this_) asm("_ZN4HPHP13c_DOMDocument10t_validateEv");

TypedValue* tg_11DOMDocument_validate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_11DOMDocument_validate((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMDocument::validate", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::validate");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t_xinclude(long)
_ZN4HPHP13c_DOMDocument10t_xincludeEl

(return value) => rax
_rv => rdi
this_ => rsi
options => rdx
*/

TypedValue* th_11DOMDocument_xinclude(TypedValue* _rv, ObjectData* this_, long options) asm("_ZN4HPHP13c_DOMDocument10t_xincludeEl");

TypedValue* tg1_11DOMDocument_xinclude(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMDocument_xinclude(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_11DOMDocument_xinclude((rv), (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMDocument_xinclude(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
          th_11DOMDocument_xinclude((&(rv)), (this_), (count > 0) ? (long)(args[-0].m_data.num) : (long)(0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMDocument_xinclude(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DOMDocument::xinclude", 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::xinclude");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t___get(HPHP::Variant)
_ZN4HPHP13c_DOMDocument7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_11DOMDocument___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP13c_DOMDocument7t___getENS_7VariantE");

TypedValue* tg_11DOMDocument___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_11DOMDocument___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMDocument::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocument::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP13c_DOMDocument7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_11DOMDocument___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP13c_DOMDocument7t___setENS_7VariantES1_");

TypedValue* tg_11DOMDocument___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_11DOMDocument___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMDocument::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMDocument::t___isset(HPHP::Variant)
_ZN4HPHP13c_DOMDocument9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_11DOMDocument___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP13c_DOMDocument9t___issetENS_7VariantE");

TypedValue* tg_11DOMDocument___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_11DOMDocument___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMDocument::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocument::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMDocumentFragment_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMDocumentFragment) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMDocumentFragment(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMDocumentFragment);
/*
void HPHP::c_DOMDocumentFragment::t___construct()
_ZN4HPHP21c_DOMDocumentFragment13t___constructEv

this_ => rdi
*/

void th_19DOMDocumentFragment___construct(ObjectData* this_) asm("_ZN4HPHP21c_DOMDocumentFragment13t___constructEv");

TypedValue* tg_19DOMDocumentFragment___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_19DOMDocumentFragment___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMDocumentFragment::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocumentFragment::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMDocumentFragment::t_appendxml(HPHP::String const&)
_ZN4HPHP21c_DOMDocumentFragment11t_appendxmlERKNS_6StringE

(return value) => rax
this_ => rdi
data => rsi
*/

bool th_19DOMDocumentFragment_appendXML(ObjectData* this_, Value* data) asm("_ZN4HPHP21c_DOMDocumentFragment11t_appendxmlERKNS_6StringE");

TypedValue* tg1_19DOMDocumentFragment_appendXML(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_19DOMDocumentFragment_appendXML(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_19DOMDocumentFragment_appendXML((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_19DOMDocumentFragment_appendXML(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_19DOMDocumentFragment_appendXML((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_19DOMDocumentFragment_appendXML(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMDocumentFragment::appendXML", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocumentFragment::appendXML");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMDocumentType_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMDocumentType) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMDocumentType(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMDocumentType);
/*
void HPHP::c_DOMDocumentType::t___construct()
_ZN4HPHP17c_DOMDocumentType13t___constructEv

this_ => rdi
*/

void th_15DOMDocumentType___construct(ObjectData* this_) asm("_ZN4HPHP17c_DOMDocumentType13t___constructEv");

TypedValue* tg_15DOMDocumentType___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_15DOMDocumentType___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMDocumentType::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocumentType::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocumentType::t___get(HPHP::Variant)
_ZN4HPHP17c_DOMDocumentType7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_15DOMDocumentType___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP17c_DOMDocumentType7t___getENS_7VariantE");

TypedValue* tg_15DOMDocumentType___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_15DOMDocumentType___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMDocumentType::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocumentType::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMDocumentType::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP17c_DOMDocumentType7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_15DOMDocumentType___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP17c_DOMDocumentType7t___setENS_7VariantES1_");

TypedValue* tg_15DOMDocumentType___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_15DOMDocumentType___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMDocumentType::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocumentType::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMDocumentType::t___isset(HPHP::Variant)
_ZN4HPHP17c_DOMDocumentType9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_15DOMDocumentType___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP17c_DOMDocumentType9t___issetENS_7VariantE");

TypedValue* tg_15DOMDocumentType___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_15DOMDocumentType___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMDocumentType::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMDocumentType::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMElement_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMElement) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMElement(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMElement);
/*
void HPHP::c_DOMElement::t___construct(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP12c_DOMElement13t___constructERKNS_6StringES3_S3_

this_ => rdi
name => rsi
value => rdx
namespaceuri => rcx
*/

void th_10DOMElement___construct(ObjectData* this_, Value* name, Value* value, Value* namespaceuri) asm("_ZN4HPHP12c_DOMElement13t___constructERKNS_6StringES3_S3_");

TypedValue* tg1_10DOMElement___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
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
  th_10DOMElement___construct((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_10DOMElement___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_10DOMElement___construct((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::__construct", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_DOMElement::t_getattribute(HPHP::String const&)
_ZN4HPHP12c_DOMElement14t_getattributeERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

Value* th_10DOMElement_getAttribute(Value* _rv, ObjectData* this_, Value* name) asm("_ZN4HPHP12c_DOMElement14t_getattributeERKNS_6StringE");

TypedValue* tg1_10DOMElement_getAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_getAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  th_10DOMElement_getAttribute((&rv->m_data), (this_), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_getAttribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfString;
          th_10DOMElement_getAttribute((&rv.m_data), (this_), &args[-0].m_data);
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_getAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::getAttribute", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::getAttribute");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t_getattributenode(HPHP::String const&)
_ZN4HPHP12c_DOMElement18t_getattributenodeERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_10DOMElement_getAttributeNode(TypedValue* _rv, ObjectData* this_, Value* name) asm("_ZN4HPHP12c_DOMElement18t_getattributenodeERKNS_6StringE");

TypedValue* tg1_10DOMElement_getAttributeNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_getAttributeNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_10DOMElement_getAttributeNode((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_getAttributeNode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_10DOMElement_getAttributeNode((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_getAttributeNode(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::getAttributeNode", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::getAttributeNode");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_DOMElement::t_getattributenodens(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12c_DOMElement20t_getattributenodensERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
localname => rcx
*/

Value* th_10DOMElement_getAttributeNodeNS(Value* _rv, ObjectData* this_, Value* namespaceuri, Value* localname) asm("_ZN4HPHP12c_DOMElement20t_getattributenodensERKNS_6StringES3_");

TypedValue* tg1_10DOMElement_getAttributeNodeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_getAttributeNodeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10DOMElement_getAttributeNodeNS((&rv->m_data), (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_getAttributeNodeNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfObject;
          th_10DOMElement_getAttributeNodeNS((&rv.m_data), (this_), &args[-0].m_data, &args[-1].m_data);
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_getAttributeNodeNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::getAttributeNodeNS", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::getAttributeNodeNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_DOMElement::t_getattributens(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12c_DOMElement16t_getattributensERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
localname => rcx
*/

Value* th_10DOMElement_getAttributeNS(Value* _rv, ObjectData* this_, Value* namespaceuri, Value* localname) asm("_ZN4HPHP12c_DOMElement16t_getattributensERKNS_6StringES3_");

TypedValue* tg1_10DOMElement_getAttributeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_getAttributeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10DOMElement_getAttributeNS((&rv->m_data), (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_getAttributeNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfString;
          th_10DOMElement_getAttributeNS((&rv.m_data), (this_), &args[-0].m_data, &args[-1].m_data);
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_getAttributeNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::getAttributeNS", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::getAttributeNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_DOMElement::t_getelementsbytagname(HPHP::String const&)
_ZN4HPHP12c_DOMElement22t_getelementsbytagnameERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

Value* th_10DOMElement_getElementsByTagName(Value* _rv, ObjectData* this_, Value* name) asm("_ZN4HPHP12c_DOMElement22t_getelementsbytagnameERKNS_6StringE");

TypedValue* tg1_10DOMElement_getElementsByTagName(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_getElementsByTagName(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  th_10DOMElement_getElementsByTagName((&rv->m_data), (this_), &args[-0].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_getElementsByTagName(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfObject;
          th_10DOMElement_getElementsByTagName((&rv.m_data), (this_), &args[-0].m_data);
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_getElementsByTagName(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::getElementsByTagName", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::getElementsByTagName");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_DOMElement::t_getelementsbytagnamens(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12c_DOMElement24t_getelementsbytagnamensERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
localname => rcx
*/

Value* th_10DOMElement_getElementsByTagNameNS(Value* _rv, ObjectData* this_, Value* namespaceuri, Value* localname) asm("_ZN4HPHP12c_DOMElement24t_getelementsbytagnamensERKNS_6StringES3_");

TypedValue* tg1_10DOMElement_getElementsByTagNameNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_getElementsByTagNameNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10DOMElement_getElementsByTagNameNS((&rv->m_data), (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_getElementsByTagNameNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfObject;
          th_10DOMElement_getElementsByTagNameNS((&rv.m_data), (this_), &args[-0].m_data, &args[-1].m_data);
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_getElementsByTagNameNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::getElementsByTagNameNS", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::getElementsByTagNameNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMElement::t_hasattribute(HPHP::String const&)
_ZN4HPHP12c_DOMElement14t_hasattributeERKNS_6StringE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_10DOMElement_hasAttribute(ObjectData* this_, Value* name) asm("_ZN4HPHP12c_DOMElement14t_hasattributeERKNS_6StringE");

TypedValue* tg1_10DOMElement_hasAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_hasAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_10DOMElement_hasAttribute((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_10DOMElement_hasAttribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_10DOMElement_hasAttribute((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_hasAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::hasAttribute", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::hasAttribute");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMElement::t_hasattributens(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12c_DOMElement16t_hasattributensERKNS_6StringES3_

(return value) => rax
this_ => rdi
namespaceuri => rsi
localname => rdx
*/

bool th_10DOMElement_hasAttributeNS(ObjectData* this_, Value* namespaceuri, Value* localname) asm("_ZN4HPHP12c_DOMElement16t_hasattributensERKNS_6StringES3_");

TypedValue* tg1_10DOMElement_hasAttributeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_hasAttributeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_10DOMElement_hasAttributeNS((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_10DOMElement_hasAttributeNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_10DOMElement_hasAttributeNS((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_hasAttributeNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::hasAttributeNS", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::hasAttributeNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMElement::t_removeattribute(HPHP::String const&)
_ZN4HPHP12c_DOMElement17t_removeattributeERKNS_6StringE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_10DOMElement_removeAttribute(ObjectData* this_, Value* name) asm("_ZN4HPHP12c_DOMElement17t_removeattributeERKNS_6StringE");

TypedValue* tg1_10DOMElement_removeAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_removeAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_10DOMElement_removeAttribute((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_10DOMElement_removeAttribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_10DOMElement_removeAttribute((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_removeAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::removeAttribute", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::removeAttribute");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t_removeattributenode(HPHP::Object const&)
_ZN4HPHP12c_DOMElement21t_removeattributenodeERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
oldattr => rdx
*/

TypedValue* th_10DOMElement_removeAttributeNode(TypedValue* _rv, ObjectData* this_, Value* oldattr) asm("_ZN4HPHP12c_DOMElement21t_removeattributenodeERKNS_6ObjectE");

TypedValue* tg1_10DOMElement_removeAttributeNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_removeAttributeNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_10DOMElement_removeAttributeNode((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_removeAttributeNode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          th_10DOMElement_removeAttributeNode((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_removeAttributeNode(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::removeAttributeNode", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::removeAttributeNode");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t_removeattributens(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12c_DOMElement19t_removeattributensERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* th_10DOMElement_removeAttributeNS(TypedValue* _rv, ObjectData* this_, Value* namespaceuri, Value* localname) asm("_ZN4HPHP12c_DOMElement19t_removeattributensERKNS_6StringES3_");

TypedValue* tg1_10DOMElement_removeAttributeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_removeAttributeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10DOMElement_removeAttributeNS((rv), (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_removeAttributeNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          th_10DOMElement_removeAttributeNS((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_removeAttributeNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::removeAttributeNS", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::removeAttributeNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t_setattribute(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12c_DOMElement14t_setattributeERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_10DOMElement_setAttribute(TypedValue* _rv, ObjectData* this_, Value* name, Value* value) asm("_ZN4HPHP12c_DOMElement14t_setattributeERKNS_6StringES3_");

TypedValue* tg1_10DOMElement_setAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_setAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10DOMElement_setAttribute((rv), (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_setAttribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          th_10DOMElement_setAttribute((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_setAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::setAttribute", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::setAttribute");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t_setattributenode(HPHP::Object const&)
_ZN4HPHP12c_DOMElement18t_setattributenodeERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
newattr => rdx
*/

TypedValue* th_10DOMElement_setAttributeNode(TypedValue* _rv, ObjectData* this_, Value* newattr) asm("_ZN4HPHP12c_DOMElement18t_setattributenodeERKNS_6ObjectE");

TypedValue* tg1_10DOMElement_setAttributeNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_setAttributeNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_10DOMElement_setAttributeNode((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_setAttributeNode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          th_10DOMElement_setAttributeNode((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_setAttributeNode(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::setAttributeNode", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::setAttributeNode");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t_setattributenodens(HPHP::Object const&)
_ZN4HPHP12c_DOMElement20t_setattributenodensERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
newattr => rdx
*/

TypedValue* th_10DOMElement_setAttributeNodeNS(TypedValue* _rv, ObjectData* this_, Value* newattr) asm("_ZN4HPHP12c_DOMElement20t_setattributenodensERKNS_6ObjectE");

TypedValue* tg1_10DOMElement_setAttributeNodeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_setAttributeNodeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_10DOMElement_setAttributeNodeNS((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_setAttributeNodeNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          th_10DOMElement_setAttributeNodeNS((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_setAttributeNodeNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::setAttributeNodeNS", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::setAttributeNodeNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t_setattributens(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP12c_DOMElement16t_setattributensERKNS_6StringES3_S3_

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
name => rcx
value => r8
*/

TypedValue* th_10DOMElement_setAttributeNS(TypedValue* _rv, ObjectData* this_, Value* namespaceuri, Value* name, Value* value) asm("_ZN4HPHP12c_DOMElement16t_setattributensERKNS_6StringES3_S3_");

TypedValue* tg1_10DOMElement_setAttributeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_setAttributeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10DOMElement_setAttributeNS((rv), (this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_setAttributeNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 3LL) {
        if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          th_10DOMElement_setAttributeNS((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_setAttributeNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::setAttributeNS", count, 3, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::setAttributeNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t_setidattribute(HPHP::String const&, bool)
_ZN4HPHP12c_DOMElement16t_setidattributeERKNS_6StringEb

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
isid => rcx
*/

TypedValue* th_10DOMElement_setIDAttribute(TypedValue* _rv, ObjectData* this_, Value* name, bool isid) asm("_ZN4HPHP12c_DOMElement16t_setidattributeERKNS_6StringEb");

TypedValue* tg1_10DOMElement_setIDAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_setIDAttribute(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10DOMElement_setIDAttribute((rv), (this_), &args[-0].m_data, (bool)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_setIDAttribute(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-1)->m_type == KindOfBoolean && IS_STRING_TYPE((args-0)->m_type)) {
          th_10DOMElement_setIDAttribute((&(rv)), (this_), &args[-0].m_data, (bool)(args[-1].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_setIDAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::setIDAttribute", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::setIDAttribute");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t_setidattributenode(HPHP::Object const&, bool)
_ZN4HPHP12c_DOMElement20t_setidattributenodeERKNS_6ObjectEb

(return value) => rax
_rv => rdi
this_ => rsi
idattr => rdx
isid => rcx
*/

TypedValue* th_10DOMElement_setIDAttributeNode(TypedValue* _rv, ObjectData* this_, Value* idattr, bool isid) asm("_ZN4HPHP12c_DOMElement20t_setidattributenodeERKNS_6ObjectEb");

TypedValue* tg1_10DOMElement_setIDAttributeNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_setIDAttributeNode(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  th_10DOMElement_setIDAttributeNode((rv), (this_), &args[-0].m_data, (bool)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_setIDAttributeNode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfObject) {
          th_10DOMElement_setIDAttributeNode((&(rv)), (this_), &args[-0].m_data, (bool)(args[-1].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_setIDAttributeNode(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::setIDAttributeNode", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::setIDAttributeNode");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t_setidattributens(HPHP::String const&, HPHP::String const&, bool)
_ZN4HPHP12c_DOMElement18t_setidattributensERKNS_6StringES3_b

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
localname => rcx
isid => r8
*/

TypedValue* th_10DOMElement_setIDAttributeNS(TypedValue* _rv, ObjectData* this_, Value* namespaceuri, Value* localname, bool isid) asm("_ZN4HPHP12c_DOMElement18t_setidattributensERKNS_6StringES3_b");

TypedValue* tg1_10DOMElement_setIDAttributeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_10DOMElement_setIDAttributeNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10DOMElement_setIDAttributeNS((rv), (this_), &args[-0].m_data, &args[-1].m_data, (bool)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_10DOMElement_setIDAttributeNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 3LL) {
        if ((args-2)->m_type == KindOfBoolean && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          th_10DOMElement_setIDAttributeNS((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data, (bool)(args[-2].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_10DOMElement_setIDAttributeNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMElement::setIDAttributeNS", count, 3, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::setIDAttributeNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t___get(HPHP::Variant)
_ZN4HPHP12c_DOMElement7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_10DOMElement___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP12c_DOMElement7t___getENS_7VariantE");

TypedValue* tg_10DOMElement___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_10DOMElement___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMElement::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMElement::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP12c_DOMElement7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_10DOMElement___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP12c_DOMElement7t___setENS_7VariantES1_");

TypedValue* tg_10DOMElement___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_10DOMElement___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMElement::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMElement::t___isset(HPHP::Variant)
_ZN4HPHP12c_DOMElement9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_10DOMElement___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP12c_DOMElement9t___issetENS_7VariantE");

TypedValue* tg_10DOMElement___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_10DOMElement___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMElement::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMElement::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMEntity_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMEntity) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMEntity(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMEntity);
/*
void HPHP::c_DOMEntity::t___construct()
_ZN4HPHP11c_DOMEntity13t___constructEv

this_ => rdi
*/

void th_9DOMEntity___construct(ObjectData* this_) asm("_ZN4HPHP11c_DOMEntity13t___constructEv");

TypedValue* tg_9DOMEntity___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_9DOMEntity___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMEntity::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMEntity::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMEntity::t___get(HPHP::Variant)
_ZN4HPHP11c_DOMEntity7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_9DOMEntity___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP11c_DOMEntity7t___getENS_7VariantE");

TypedValue* tg_9DOMEntity___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_9DOMEntity___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMEntity::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMEntity::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMEntity::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP11c_DOMEntity7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_9DOMEntity___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP11c_DOMEntity7t___setENS_7VariantES1_");

TypedValue* tg_9DOMEntity___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_9DOMEntity___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMEntity::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMEntity::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMEntity::t___isset(HPHP::Variant)
_ZN4HPHP11c_DOMEntity9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_9DOMEntity___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP11c_DOMEntity9t___issetENS_7VariantE");

TypedValue* tg_9DOMEntity___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9DOMEntity___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMEntity::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMEntity::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMEntityReference_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMEntityReference) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMEntityReference(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMEntityReference);
/*
void HPHP::c_DOMEntityReference::t___construct(HPHP::String const&)
_ZN4HPHP20c_DOMEntityReference13t___constructERKNS_6StringE

this_ => rdi
name => rsi
*/

void th_18DOMEntityReference___construct(ObjectData* this_, Value* name) asm("_ZN4HPHP20c_DOMEntityReference13t___constructERKNS_6StringE");

TypedValue* tg1_18DOMEntityReference___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_18DOMEntityReference___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_18DOMEntityReference___construct((this_), &args[-0].m_data);
  return rv;
}

TypedValue* tg_18DOMEntityReference___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_18DOMEntityReference___construct((this_), &args[-0].m_data);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_18DOMEntityReference___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMEntityReference::__construct", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMEntityReference::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMNotation_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMNotation) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMNotation(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMNotation);
/*
void HPHP::c_DOMNotation::t___construct()
_ZN4HPHP13c_DOMNotation13t___constructEv

this_ => rdi
*/

void th_11DOMNotation___construct(ObjectData* this_) asm("_ZN4HPHP13c_DOMNotation13t___constructEv");

TypedValue* tg_11DOMNotation___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_11DOMNotation___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNotation::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNotation::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNotation::t___get(HPHP::Variant)
_ZN4HPHP13c_DOMNotation7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_11DOMNotation___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP13c_DOMNotation7t___getENS_7VariantE");

TypedValue* tg_11DOMNotation___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_11DOMNotation___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNotation::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNotation::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNotation::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP13c_DOMNotation7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_11DOMNotation___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP13c_DOMNotation7t___setENS_7VariantES1_");

TypedValue* tg_11DOMNotation___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_11DOMNotation___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNotation::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNotation::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMNotation::t___isset(HPHP::Variant)
_ZN4HPHP13c_DOMNotation9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_11DOMNotation___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP13c_DOMNotation9t___issetENS_7VariantE");

TypedValue* tg_11DOMNotation___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_11DOMNotation___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNotation::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNotation::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMProcessingInstruction_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMProcessingInstruction) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMProcessingInstruction(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMProcessingInstruction);
/*
void HPHP::c_DOMProcessingInstruction::t___construct(HPHP::String const&, HPHP::String const&)
_ZN4HPHP26c_DOMProcessingInstruction13t___constructERKNS_6StringES3_

this_ => rdi
name => rsi
value => rdx
*/

void th_24DOMProcessingInstruction___construct(ObjectData* this_, Value* name, Value* value) asm("_ZN4HPHP26c_DOMProcessingInstruction13t___constructERKNS_6StringES3_");

TypedValue* tg1_24DOMProcessingInstruction___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_24DOMProcessingInstruction___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
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
  th_24DOMProcessingInstruction___construct((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_24DOMProcessingInstruction___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_24DOMProcessingInstruction___construct((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_24DOMProcessingInstruction___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMProcessingInstruction::__construct", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMProcessingInstruction::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMProcessingInstruction::t___get(HPHP::Variant)
_ZN4HPHP26c_DOMProcessingInstruction7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_24DOMProcessingInstruction___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP26c_DOMProcessingInstruction7t___getENS_7VariantE");

TypedValue* tg_24DOMProcessingInstruction___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_24DOMProcessingInstruction___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMProcessingInstruction::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMProcessingInstruction::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMProcessingInstruction::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP26c_DOMProcessingInstruction7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_24DOMProcessingInstruction___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP26c_DOMProcessingInstruction7t___setENS_7VariantES1_");

TypedValue* tg_24DOMProcessingInstruction___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_24DOMProcessingInstruction___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMProcessingInstruction::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMProcessingInstruction::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMProcessingInstruction::t___isset(HPHP::Variant)
_ZN4HPHP26c_DOMProcessingInstruction9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_24DOMProcessingInstruction___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP26c_DOMProcessingInstruction9t___issetENS_7VariantE");

TypedValue* tg_24DOMProcessingInstruction___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_24DOMProcessingInstruction___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMProcessingInstruction::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMProcessingInstruction::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMNodeIterator_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMNodeIterator) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMNodeIterator(cls);
  return inst;
}

IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(DOMNodeIterator);
/*
void HPHP::c_DOMNodeIterator::t___construct()
_ZN4HPHP17c_DOMNodeIterator13t___constructEv

this_ => rdi
*/

void th_15DOMNodeIterator___construct(ObjectData* this_) asm("_ZN4HPHP17c_DOMNodeIterator13t___constructEv");

TypedValue* tg_15DOMNodeIterator___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_15DOMNodeIterator___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNodeIterator::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeIterator::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNodeIterator::t_current()
_ZN4HPHP17c_DOMNodeIterator9t_currentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_15DOMNodeIterator_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP17c_DOMNodeIterator9t_currentEv");

TypedValue* tg_15DOMNodeIterator_current(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_15DOMNodeIterator_current((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNodeIterator::current", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeIterator::current");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNodeIterator::t_key()
_ZN4HPHP17c_DOMNodeIterator5t_keyEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_15DOMNodeIterator_key(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP17c_DOMNodeIterator5t_keyEv");

TypedValue* tg_15DOMNodeIterator_key(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_15DOMNodeIterator_key((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNodeIterator::key", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeIterator::key");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNodeIterator::t_next()
_ZN4HPHP17c_DOMNodeIterator6t_nextEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_15DOMNodeIterator_next(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP17c_DOMNodeIterator6t_nextEv");

TypedValue* tg_15DOMNodeIterator_next(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_15DOMNodeIterator_next((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNodeIterator::next", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeIterator::next");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNodeIterator::t_rewind()
_ZN4HPHP17c_DOMNodeIterator8t_rewindEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_15DOMNodeIterator_rewind(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP17c_DOMNodeIterator8t_rewindEv");

TypedValue* tg_15DOMNodeIterator_rewind(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_15DOMNodeIterator_rewind((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNodeIterator::rewind", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeIterator::rewind");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNodeIterator::t_valid()
_ZN4HPHP17c_DOMNodeIterator7t_validEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_15DOMNodeIterator_valid(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP17c_DOMNodeIterator7t_validEv");

TypedValue* tg_15DOMNodeIterator_valid(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_15DOMNodeIterator_valid((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNodeIterator::valid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeIterator::valid");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMNamedNodeMap_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMNamedNodeMap) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMNamedNodeMap(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMNamedNodeMap);
/*
void HPHP::c_DOMNamedNodeMap::t___construct()
_ZN4HPHP17c_DOMNamedNodeMap13t___constructEv

this_ => rdi
*/

void th_15DOMNamedNodeMap___construct(ObjectData* this_) asm("_ZN4HPHP17c_DOMNamedNodeMap13t___constructEv");

TypedValue* tg_15DOMNamedNodeMap___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_15DOMNamedNodeMap___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNamedNodeMap::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNamedNodeMap::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNamedNodeMap::t_getnameditem(HPHP::String const&)
_ZN4HPHP17c_DOMNamedNodeMap14t_getnameditemERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_15DOMNamedNodeMap_getNamedItem(TypedValue* _rv, ObjectData* this_, Value* name) asm("_ZN4HPHP17c_DOMNamedNodeMap14t_getnameditemERKNS_6StringE");

TypedValue* tg1_15DOMNamedNodeMap_getNamedItem(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_15DOMNamedNodeMap_getNamedItem(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_15DOMNamedNodeMap_getNamedItem((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_15DOMNamedNodeMap_getNamedItem(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_15DOMNamedNodeMap_getNamedItem((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_15DOMNamedNodeMap_getNamedItem(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNamedNodeMap::getNamedItem", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNamedNodeMap::getNamedItem");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNamedNodeMap::t_getnameditemns(HPHP::String const&, HPHP::String const&)
_ZN4HPHP17c_DOMNamedNodeMap16t_getnameditemnsERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* th_15DOMNamedNodeMap_getNamedItemNS(TypedValue* _rv, ObjectData* this_, Value* namespaceuri, Value* localname) asm("_ZN4HPHP17c_DOMNamedNodeMap16t_getnameditemnsERKNS_6StringES3_");

TypedValue* tg1_15DOMNamedNodeMap_getNamedItemNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_15DOMNamedNodeMap_getNamedItemNS(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_15DOMNamedNodeMap_getNamedItemNS((rv), (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_15DOMNamedNodeMap_getNamedItemNS(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          th_15DOMNamedNodeMap_getNamedItemNS((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_15DOMNamedNodeMap_getNamedItemNS(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNamedNodeMap::getNamedItemNS", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNamedNodeMap::getNamedItemNS");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNamedNodeMap::t_item(long)
_ZN4HPHP17c_DOMNamedNodeMap6t_itemEl

(return value) => rax
_rv => rdi
this_ => rsi
index => rdx
*/

TypedValue* th_15DOMNamedNodeMap_item(TypedValue* _rv, ObjectData* this_, long index) asm("_ZN4HPHP17c_DOMNamedNodeMap6t_itemEl");

TypedValue* tg1_15DOMNamedNodeMap_item(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_15DOMNamedNodeMap_item(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_15DOMNamedNodeMap_item((rv), (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_15DOMNamedNodeMap_item(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          th_15DOMNamedNodeMap_item((&(rv)), (this_), (long)(args[-0].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_15DOMNamedNodeMap_item(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNamedNodeMap::item", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNamedNodeMap::item");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNamedNodeMap::t___get(HPHP::Variant)
_ZN4HPHP17c_DOMNamedNodeMap7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_15DOMNamedNodeMap___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP17c_DOMNamedNodeMap7t___getENS_7VariantE");

TypedValue* tg_15DOMNamedNodeMap___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_15DOMNamedNodeMap___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNamedNodeMap::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNamedNodeMap::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNamedNodeMap::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP17c_DOMNamedNodeMap7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_15DOMNamedNodeMap___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP17c_DOMNamedNodeMap7t___setENS_7VariantES1_");

TypedValue* tg_15DOMNamedNodeMap___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_15DOMNamedNodeMap___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNamedNodeMap::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNamedNodeMap::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMNamedNodeMap::t___isset(HPHP::Variant)
_ZN4HPHP17c_DOMNamedNodeMap9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_15DOMNamedNodeMap___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP17c_DOMNamedNodeMap9t___issetENS_7VariantE");

TypedValue* tg_15DOMNamedNodeMap___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_15DOMNamedNodeMap___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNamedNodeMap::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNamedNodeMap::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNamedNodeMap::t_getiterator()
_ZN4HPHP17c_DOMNamedNodeMap13t_getiteratorEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_15DOMNamedNodeMap_getIterator(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP17c_DOMNamedNodeMap13t_getiteratorEv");

TypedValue* tg_15DOMNamedNodeMap_getIterator(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_15DOMNamedNodeMap_getIterator((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNamedNodeMap::getIterator", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNamedNodeMap::getIterator");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMNodeList_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMNodeList) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMNodeList(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMNodeList);
/*
void HPHP::c_DOMNodeList::t___construct()
_ZN4HPHP13c_DOMNodeList13t___constructEv

this_ => rdi
*/

void th_11DOMNodeList___construct(ObjectData* this_) asm("_ZN4HPHP13c_DOMNodeList13t___constructEv");

TypedValue* tg_11DOMNodeList___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_11DOMNodeList___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNodeList::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeList::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNodeList::t_item(long)
_ZN4HPHP13c_DOMNodeList6t_itemEl

(return value) => rax
_rv => rdi
this_ => rsi
index => rdx
*/

TypedValue* th_11DOMNodeList_item(TypedValue* _rv, ObjectData* this_, long index) asm("_ZN4HPHP13c_DOMNodeList6t_itemEl");

TypedValue* tg1_11DOMNodeList_item(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11DOMNodeList_item(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_11DOMNodeList_item((rv), (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11DOMNodeList_item(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          th_11DOMNodeList_item((&(rv)), (this_), (long)(args[-0].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11DOMNodeList_item(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMNodeList::item", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeList::item");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNodeList::t___get(HPHP::Variant)
_ZN4HPHP13c_DOMNodeList7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_11DOMNodeList___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP13c_DOMNodeList7t___getENS_7VariantE");

TypedValue* tg_11DOMNodeList___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_11DOMNodeList___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNodeList::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeList::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNodeList::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP13c_DOMNodeList7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_11DOMNodeList___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP13c_DOMNodeList7t___setENS_7VariantES1_");

TypedValue* tg_11DOMNodeList___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_11DOMNodeList___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNodeList::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeList::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMNodeList::t___isset(HPHP::Variant)
_ZN4HPHP13c_DOMNodeList9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_11DOMNodeList___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP13c_DOMNodeList9t___issetENS_7VariantE");

TypedValue* tg_11DOMNodeList___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_11DOMNodeList___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMNodeList::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeList::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMNodeList::t_getiterator()
_ZN4HPHP13c_DOMNodeList13t_getiteratorEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_11DOMNodeList_getIterator(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP13c_DOMNodeList13t_getiteratorEv");

TypedValue* tg_11DOMNodeList_getIterator(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_11DOMNodeList_getIterator((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMNodeList::getIterator", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMNodeList::getIterator");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMImplementation_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMImplementation) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMImplementation(cls);
  return inst;
}

IMPLEMENT_CLASS(DOMImplementation);
/*
void HPHP::c_DOMImplementation::t___construct()
_ZN4HPHP19c_DOMImplementation13t___constructEv

this_ => rdi
*/

void th_17DOMImplementation___construct(ObjectData* this_) asm("_ZN4HPHP19c_DOMImplementation13t___constructEv");

TypedValue* tg_17DOMImplementation___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_17DOMImplementation___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMImplementation::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DOMImplementation::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMImplementation::t_createdocument(HPHP::String const&, HPHP::String const&, HPHP::Object const&)
_ZN4HPHP19c_DOMImplementation16t_createdocumentERKNS_6StringES3_RKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
namespaceuri => rdx
qualifiedname => rcx
doctypeobj => r8
*/

TypedValue* th_17DOMImplementation_createDocument(TypedValue* _rv, ObjectData* this_, Value* namespaceuri, Value* qualifiedname, Value* doctypeobj) asm("_ZN4HPHP19c_DOMImplementation16t_createdocumentERKNS_6StringES3_RKNS_6ObjectE");

TypedValue* tg1_17DOMImplementation_createDocument(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_17DOMImplementation_createDocument(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-2);
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
  th_17DOMImplementation_createDocument((rv), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_object));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_17DOMImplementation_createDocument(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfObject) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          th_17DOMImplementation_createDocument((&(rv)), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_object));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_17DOMImplementation_createDocument(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DOMImplementation::createDocument", 3, 1);
      }
    } else {
      throw_instance_method_fatal("DOMImplementation::createDocument");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMImplementation::t_createdocumenttype(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP19c_DOMImplementation20t_createdocumenttypeERKNS_6StringES3_S3_

(return value) => rax
_rv => rdi
this_ => rsi
qualifiedname => rdx
publicid => rcx
systemid => r8
*/

TypedValue* th_17DOMImplementation_createDocumentType(TypedValue* _rv, ObjectData* this_, Value* qualifiedname, Value* publicid, Value* systemid) asm("_ZN4HPHP19c_DOMImplementation20t_createdocumenttypeERKNS_6StringES3_S3_");

TypedValue* tg1_17DOMImplementation_createDocumentType(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_17DOMImplementation_createDocumentType(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  th_17DOMImplementation_createDocumentType((rv), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_17DOMImplementation_createDocumentType(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 3LL) {
        if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          th_17DOMImplementation_createDocumentType((&(rv)), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_17DOMImplementation_createDocumentType(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DOMImplementation::createDocumentType", 3, 1);
      }
    } else {
      throw_instance_method_fatal("DOMImplementation::createDocumentType");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMImplementation::t_hasfeature(HPHP::String const&, HPHP::String const&)
_ZN4HPHP19c_DOMImplementation12t_hasfeatureERKNS_6StringES3_

(return value) => rax
this_ => rdi
feature => rsi
version => rdx
*/

bool th_17DOMImplementation_hasFeature(ObjectData* this_, Value* feature, Value* version) asm("_ZN4HPHP19c_DOMImplementation12t_hasfeatureERKNS_6StringES3_");

TypedValue* tg1_17DOMImplementation_hasFeature(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_17DOMImplementation_hasFeature(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_17DOMImplementation_hasFeature((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_17DOMImplementation_hasFeature(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_17DOMImplementation_hasFeature((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_17DOMImplementation_hasFeature(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMImplementation::hasFeature", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMImplementation::hasFeature");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DOMXPath_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DOMXPath) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DOMXPath(cls);
  return inst;
}

IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(DOMXPath);
/*
void HPHP::c_DOMXPath::t___construct(HPHP::Variant const&)
_ZN4HPHP10c_DOMXPath13t___constructERKNS_7VariantE

this_ => rdi
doc => rsi
*/

void th_8DOMXPath___construct(ObjectData* this_, TypedValue* doc) asm("_ZN4HPHP10c_DOMXPath13t___constructERKNS_7VariantE");

TypedValue* tg_8DOMXPath___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_8DOMXPath___construct((this_), (args-0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMXPath::__construct", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMXPath::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMXPath::t_evaluate(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP10c_DOMXPath10t_evaluateERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
expr => rdx
context => rcx
*/

TypedValue* th_8DOMXPath_evaluate(TypedValue* _rv, ObjectData* this_, Value* expr, Value* context) asm("_ZN4HPHP10c_DOMXPath10t_evaluateERKNS_6StringERKNS_6ObjectE");

TypedValue* tg1_8DOMXPath_evaluate(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DOMXPath_evaluate(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_8DOMXPath_evaluate((rv), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_object));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DOMXPath_evaluate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfObject) && IS_STRING_TYPE((args-0)->m_type)) {
          th_8DOMXPath_evaluate((&(rv)), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_object));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DOMXPath_evaluate(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMXPath::evaluate", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMXPath::evaluate");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMXPath::t_query(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP10c_DOMXPath7t_queryERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
expr => rdx
context => rcx
*/

TypedValue* th_8DOMXPath_query(TypedValue* _rv, ObjectData* this_, Value* expr, Value* context) asm("_ZN4HPHP10c_DOMXPath7t_queryERKNS_6StringERKNS_6ObjectE");

TypedValue* tg1_8DOMXPath_query(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DOMXPath_query(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_8DOMXPath_query((rv), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_object));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DOMXPath_query(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfObject) && IS_STRING_TYPE((args-0)->m_type)) {
          th_8DOMXPath_query((&(rv)), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_object));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DOMXPath_query(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMXPath::query", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMXPath::query");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMXPath::t_registernamespace(HPHP::String const&, HPHP::String const&)
_ZN4HPHP10c_DOMXPath19t_registernamespaceERKNS_6StringES3_

(return value) => rax
this_ => rdi
prefix => rsi
uri => rdx
*/

bool th_8DOMXPath_registerNamespace(ObjectData* this_, Value* prefix, Value* uri) asm("_ZN4HPHP10c_DOMXPath19t_registernamespaceERKNS_6StringES3_");

TypedValue* tg1_8DOMXPath_registerNamespace(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DOMXPath_registerNamespace(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_8DOMXPath_registerNamespace((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8DOMXPath_registerNamespace(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8DOMXPath_registerNamespace((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DOMXPath_registerNamespace(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DOMXPath::registerNamespace", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMXPath::registerNamespace");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMXPath::t_registerphpfunctions(HPHP::Variant const&)
_ZN4HPHP10c_DOMXPath22t_registerphpfunctionsERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
funcs => rdx
*/

TypedValue* th_8DOMXPath_registerPHPFunctions(TypedValue* _rv, ObjectData* this_, TypedValue* funcs) asm("_ZN4HPHP10c_DOMXPath22t_registerphpfunctionsERKNS_7VariantE");

TypedValue* tg_8DOMXPath_registerPHPFunctions(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        Variant defVal0;
        th_8DOMXPath_registerPHPFunctions((&(rv)), (this_), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DOMXPath::registerPHPFunctions", 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMXPath::registerPHPFunctions");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMXPath::t___get(HPHP::Variant)
_ZN4HPHP10c_DOMXPath7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_8DOMXPath___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP10c_DOMXPath7t___getENS_7VariantE");

TypedValue* tg_8DOMXPath___get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_8DOMXPath___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMXPath::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMXPath::__get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DOMXPath::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP10c_DOMXPath7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_8DOMXPath___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP10c_DOMXPath7t___setENS_7VariantES1_");

TypedValue* tg_8DOMXPath___set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_8DOMXPath___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMXPath::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DOMXPath::__set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DOMXPath::t___isset(HPHP::Variant)
_ZN4HPHP10c_DOMXPath9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_8DOMXPath___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP10c_DOMXPath9t___issetENS_7VariantE");

TypedValue* tg_8DOMXPath___isset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_8DOMXPath___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DOMXPath::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DOMXPath::__isset");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

