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

Value* fh_spl_classes(Value* _rv) asm("_ZN4HPHP13f_spl_classesEv");

Value* fh_spl_object_hash(Value* _rv, Value* obj) asm("_ZN4HPHP17f_spl_object_hashERKNS_6ObjectE");

long fh_hphp_object_pointer(Value* obj) asm("_ZN4HPHP21f_hphp_object_pointerERKNS_6ObjectE");

TypedValue* fh_hphp_get_this(TypedValue* _rv) asm("_ZN4HPHP15f_hphp_get_thisEv");

TypedValue* fh_class_implements(TypedValue* _rv, TypedValue* obj, bool autoload) asm("_ZN4HPHP18f_class_implementsERKNS_7VariantEb");

TypedValue* fh_class_parents(TypedValue* _rv, TypedValue* obj, bool autoload) asm("_ZN4HPHP15f_class_parentsERKNS_7VariantEb");

TypedValue* fh_class_uses(TypedValue* _rv, TypedValue* obj, bool autoload) asm("_ZN4HPHP12f_class_usesERKNS_7VariantEb");

TypedValue* fh_iterator_apply(TypedValue* _rv, TypedValue* obj, TypedValue* func, Value* params) asm("_ZN4HPHP16f_iterator_applyERKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fh_iterator_count(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP16f_iterator_countERKNS_7VariantE");

TypedValue* fh_iterator_to_array(TypedValue* _rv, TypedValue* obj, bool use_keys) asm("_ZN4HPHP19f_iterator_to_arrayERKNS_7VariantEb");

bool fh_spl_autoload_register(TypedValue* autoload_function, bool throws, bool prepend) asm("_ZN4HPHP23f_spl_autoload_registerERKNS_7VariantEbb");

bool fh_spl_autoload_unregister(TypedValue* autoload_function) asm("_ZN4HPHP25f_spl_autoload_unregisterERKNS_7VariantE");

TypedValue* fh_spl_autoload_functions(TypedValue* _rv) asm("_ZN4HPHP24f_spl_autoload_functionsEv");

void fh_spl_autoload_call(Value* class_name) asm("_ZN4HPHP19f_spl_autoload_callERKNS_6StringE");

Value* fh_spl_autoload_extensions(Value* _rv, Value* file_extensions) asm("_ZN4HPHP25f_spl_autoload_extensionsERKNS_6StringE");

void fh_spl_autoload(Value* class_name, Value* file_extensions) asm("_ZN4HPHP14f_spl_autoloadERKNS_6StringES2_");

} // namespace HPHP
