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
namespace HPHP {

/*
HPHP::Array HPHP::f_hphp_get_extension_info(HPHP::String const&)
_ZN4HPHP25f_hphp_get_extension_infoERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

Value* fh_hphp_get_extension_info(Value* _rv, Value* name) asm("_ZN4HPHP25f_hphp_get_extension_infoERKNS_6StringE");

/*
HPHP::Array HPHP::f_hphp_get_method_info(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP22f_hphp_get_method_infoERKNS_7VariantES2_

(return value) => rax
_rv => rdi
cname => rsi
name => rdx
*/

Value* fh_hphp_get_method_info(Value* _rv, TypedValue* cname, TypedValue* name) asm("_ZN4HPHP22f_hphp_get_method_infoERKNS_7VariantES2_");

/*
HPHP::Array HPHP::f_hphp_get_closure_info(HPHP::Variant const&)
_ZN4HPHP23f_hphp_get_closure_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
closure => rsi
*/

Value* fh_hphp_get_closure_info(Value* _rv, TypedValue* closure) asm("_ZN4HPHP23f_hphp_get_closure_infoERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_hphp_get_class_constant(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP25f_hphp_get_class_constantERKNS_7VariantES2_

(return value) => rax
_rv => rdi
cls => rsi
name => rdx
*/

TypedValue* fh_hphp_get_class_constant(TypedValue* _rv, TypedValue* cls, TypedValue* name) asm("_ZN4HPHP25f_hphp_get_class_constantERKNS_7VariantES2_");

/*
HPHP::Array HPHP::f_hphp_get_class_info(HPHP::Variant const&)
_ZN4HPHP21f_hphp_get_class_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
name => rsi
*/

Value* fh_hphp_get_class_info(Value* _rv, TypedValue* name) asm("_ZN4HPHP21f_hphp_get_class_infoERKNS_7VariantE");

/*
HPHP::Array HPHP::f_hphp_get_function_info(HPHP::String const&)
_ZN4HPHP24f_hphp_get_function_infoERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

Value* fh_hphp_get_function_info(Value* _rv, Value* name) asm("_ZN4HPHP24f_hphp_get_function_infoERKNS_6StringE");

/*
HPHP::Variant HPHP::f_hphp_invoke(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP13f_hphp_invokeERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
name => rsi
params => rdx
*/

TypedValue* fh_hphp_invoke(TypedValue* _rv, Value* name, Value* params) asm("_ZN4HPHP13f_hphp_invokeERKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_hphp_invoke_method(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP20f_hphp_invoke_methodERKNS_7VariantERKNS_6StringES5_RKNS_5ArrayE

(return value) => rax
_rv => rdi
obj => rsi
cls => rdx
name => rcx
params => r8
*/

TypedValue* fh_hphp_invoke_method(TypedValue* _rv, TypedValue* obj, Value* cls, Value* name, Value* params) asm("_ZN4HPHP20f_hphp_invoke_methodERKNS_7VariantERKNS_6StringES5_RKNS_5ArrayE");

/*
bool HPHP::f_hphp_instanceof(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_hphp_instanceofERKNS_6ObjectERKNS_6StringE

(return value) => rax
obj => rdi
name => rsi
*/

bool fh_hphp_instanceof(Value* obj, Value* name) asm("_ZN4HPHP17f_hphp_instanceofERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Object HPHP::f_hphp_create_object(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP20f_hphp_create_objectERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
name => rsi
params => rdx
*/

Value* fh_hphp_create_object(Value* _rv, Value* name, Value* params) asm("_ZN4HPHP20f_hphp_create_objectERKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_hphp_get_property(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP19f_hphp_get_propertyERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
cls => rdx
prop => rcx
*/

TypedValue* fh_hphp_get_property(TypedValue* _rv, Value* obj, Value* cls, Value* prop) asm("_ZN4HPHP19f_hphp_get_propertyERKNS_6ObjectERKNS_6StringES5_");

/*
void HPHP::f_hphp_set_property(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19f_hphp_set_propertyERKNS_6ObjectERKNS_6StringES5_RKNS_7VariantE

obj => rdi
cls => rsi
prop => rdx
value => rcx
*/

void fh_hphp_set_property(Value* obj, Value* cls, Value* prop, TypedValue* value) asm("_ZN4HPHP19f_hphp_set_propertyERKNS_6ObjectERKNS_6StringES5_RKNS_7VariantE");

/*
HPHP::Variant HPHP::f_hphp_get_static_property(HPHP::String const&, HPHP::String const&)
_ZN4HPHP26f_hphp_get_static_propertyERKNS_6StringES2_

(return value) => rax
_rv => rdi
cls => rsi
prop => rdx
*/

TypedValue* fh_hphp_get_static_property(TypedValue* _rv, Value* cls, Value* prop) asm("_ZN4HPHP26f_hphp_get_static_propertyERKNS_6StringES2_");

/*
void HPHP::f_hphp_set_static_property(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP26f_hphp_set_static_propertyERKNS_6StringES2_RKNS_7VariantE

cls => rdi
prop => rsi
value => rdx
*/

void fh_hphp_set_static_property(Value* cls, Value* prop, TypedValue* value) asm("_ZN4HPHP26f_hphp_set_static_propertyERKNS_6StringES2_RKNS_7VariantE");

/*
HPHP::String HPHP::f_hphp_get_original_class_name(HPHP::String const&)
_ZN4HPHP30f_hphp_get_original_class_nameERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

Value* fh_hphp_get_original_class_name(Value* _rv, Value* name) asm("_ZN4HPHP30f_hphp_get_original_class_nameERKNS_6StringE");

/*
bool HPHP::f_hphp_scalar_typehints_enabled()
_ZN4HPHP31f_hphp_scalar_typehints_enabledEv

(return value) => rax
*/

bool fh_hphp_scalar_typehints_enabled() asm("_ZN4HPHP31f_hphp_scalar_typehints_enabledEv");


} // !HPHP

