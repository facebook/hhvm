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

Value* fh_hphp_get_extension_info(Value* _rv, Value* name) asm("_ZN4HPHP25f_hphp_get_extension_infoERKNS_6StringE");

Value* fh_hphp_get_method_info(Value* _rv, TypedValue* cname, TypedValue* name) asm("_ZN4HPHP22f_hphp_get_method_infoERKNS_7VariantES2_");

Value* fh_hphp_get_closure_info(Value* _rv, TypedValue* closure) asm("_ZN4HPHP23f_hphp_get_closure_infoERKNS_7VariantE");

TypedValue* fh_hphp_get_class_constant(TypedValue* _rv, TypedValue* cls, TypedValue* name) asm("_ZN4HPHP25f_hphp_get_class_constantERKNS_7VariantES2_");

Value* fh_hphp_get_class_info(Value* _rv, TypedValue* name) asm("_ZN4HPHP21f_hphp_get_class_infoERKNS_7VariantE");

Value* fh_hphp_get_function_info(Value* _rv, Value* name) asm("_ZN4HPHP24f_hphp_get_function_infoERKNS_6StringE");

TypedValue* fh_hphp_invoke(TypedValue* _rv, Value* name, Value* params) asm("_ZN4HPHP13f_hphp_invokeERKNS_6StringERKNS_5ArrayE");

TypedValue* fh_hphp_invoke_method(TypedValue* _rv, TypedValue* obj, Value* cls, Value* name, Value* params) asm("_ZN4HPHP20f_hphp_invoke_methodERKNS_7VariantERKNS_6StringES5_RKNS_5ArrayE");

bool fh_hphp_instanceof(Value* obj, Value* name) asm("_ZN4HPHP17f_hphp_instanceofERKNS_6ObjectERKNS_6StringE");

Value* fh_hphp_create_object(Value* _rv, Value* name, Value* params) asm("_ZN4HPHP20f_hphp_create_objectERKNS_6StringERKNS_5ArrayE");

Value* fh_hphp_create_object_without_constructor(Value* _rv, Value* name) asm("_ZN4HPHP40f_hphp_create_object_without_constructorERKNS_6StringE");

TypedValue* fh_hphp_get_property(TypedValue* _rv, Value* obj, Value* cls, Value* prop) asm("_ZN4HPHP19f_hphp_get_propertyERKNS_6ObjectERKNS_6StringES5_");

void fh_hphp_set_property(Value* obj, Value* cls, Value* prop, TypedValue* value) asm("_ZN4HPHP19f_hphp_set_propertyERKNS_6ObjectERKNS_6StringES5_RKNS_7VariantE");

TypedValue* fh_hphp_get_static_property(TypedValue* _rv, Value* cls, Value* prop) asm("_ZN4HPHP26f_hphp_get_static_propertyERKNS_6StringES2_");

void fh_hphp_set_static_property(Value* cls, Value* prop, TypedValue* value) asm("_ZN4HPHP26f_hphp_set_static_propertyERKNS_6StringES2_RKNS_7VariantE");

Value* fh_hphp_get_original_class_name(Value* _rv, Value* name) asm("_ZN4HPHP30f_hphp_get_original_class_nameERKNS_6StringE");

bool fh_hphp_scalar_typehints_enabled() asm("_ZN4HPHP31f_hphp_scalar_typehints_enabledEv");

} // namespace HPHP
