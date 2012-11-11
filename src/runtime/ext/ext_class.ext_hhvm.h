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
HPHP::Array HPHP::f_get_declared_classes()
_ZN4HPHP22f_get_declared_classesEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_declared_classes(Value* _rv) asm("_ZN4HPHP22f_get_declared_classesEv");

/*
HPHP::Array HPHP::f_get_declared_interfaces()
_ZN4HPHP25f_get_declared_interfacesEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_declared_interfaces(Value* _rv) asm("_ZN4HPHP25f_get_declared_interfacesEv");

/*
HPHP::Array HPHP::f_get_declared_traits()
_ZN4HPHP21f_get_declared_traitsEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_declared_traits(Value* _rv) asm("_ZN4HPHP21f_get_declared_traitsEv");

/*
bool HPHP::f_class_exists(HPHP::String const&, bool)
_ZN4HPHP14f_class_existsERKNS_6StringEb

(return value) => rax
class_name => rdi
autoload => rsi
*/

bool fh_class_exists(Value* class_name, bool autoload) asm("_ZN4HPHP14f_class_existsERKNS_6StringEb");

/*
bool HPHP::f_interface_exists(HPHP::String const&, bool)
_ZN4HPHP18f_interface_existsERKNS_6StringEb

(return value) => rax
interface_name => rdi
autoload => rsi
*/

bool fh_interface_exists(Value* interface_name, bool autoload) asm("_ZN4HPHP18f_interface_existsERKNS_6StringEb");

/*
bool HPHP::f_trait_exists(HPHP::String const&, bool)
_ZN4HPHP14f_trait_existsERKNS_6StringEb

(return value) => rax
trait_name => rdi
autoload => rsi
*/

bool fh_trait_exists(Value* trait_name, bool autoload) asm("_ZN4HPHP14f_trait_existsERKNS_6StringEb");

/*
HPHP::Array HPHP::f_get_class_methods(HPHP::Variant const&)
_ZN4HPHP19f_get_class_methodsERKNS_7VariantE

(return value) => rax
_rv => rdi
class_or_object => rsi
*/

Value* fh_get_class_methods(Value* _rv, TypedValue* class_or_object) asm("_ZN4HPHP19f_get_class_methodsERKNS_7VariantE");

/*
HPHP::Array HPHP::f_get_class_vars(HPHP::String const&)
_ZN4HPHP16f_get_class_varsERKNS_6StringE

(return value) => rax
_rv => rdi
class_name => rsi
*/

Value* fh_get_class_vars(Value* _rv, Value* class_name) asm("_ZN4HPHP16f_get_class_varsERKNS_6StringE");

/*
HPHP::Array HPHP::f_get_class_constants(HPHP::String const&)
_ZN4HPHP21f_get_class_constantsERKNS_6StringE

(return value) => rax
_rv => rdi
class_name => rsi
*/

Value* fh_get_class_constants(Value* _rv, Value* class_name) asm("_ZN4HPHP21f_get_class_constantsERKNS_6StringE");

/*
HPHP::Variant HPHP::f_get_class(HPHP::Variant const&)
_ZN4HPHP11f_get_classERKNS_7VariantE

(return value) => rax
_rv => rdi
object => rsi
*/

TypedValue* fh_get_class(TypedValue* _rv, TypedValue* object) asm("_ZN4HPHP11f_get_classERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_get_parent_class(HPHP::Variant const&)
_ZN4HPHP18f_get_parent_classERKNS_7VariantE

(return value) => rax
_rv => rdi
object => rsi
*/

TypedValue* fh_get_parent_class(TypedValue* _rv, TypedValue* object) asm("_ZN4HPHP18f_get_parent_classERKNS_7VariantE");

/*
bool HPHP::f_is_a(HPHP::Variant const&, HPHP::String const&, bool)
_ZN4HPHP6f_is_aERKNS_7VariantERKNS_6StringEb

(return value) => rax
class_or_object => rdi
class_name => rsi
allow_string => rdx
*/

bool fh_is_a(TypedValue* class_or_object, Value* class_name, bool allow_string) asm("_ZN4HPHP6f_is_aERKNS_7VariantERKNS_6StringEb");

/*
bool HPHP::f_is_subclass_of(HPHP::Variant const&, HPHP::String const&, bool)
_ZN4HPHP16f_is_subclass_ofERKNS_7VariantERKNS_6StringEb

(return value) => rax
class_or_object => rdi
class_name => rsi
allow_string => rdx
*/

bool fh_is_subclass_of(TypedValue* class_or_object, Value* class_name, bool allow_string) asm("_ZN4HPHP16f_is_subclass_ofERKNS_7VariantERKNS_6StringEb");

/*
bool HPHP::f_method_exists(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP15f_method_existsERKNS_7VariantERKNS_6StringE

(return value) => rax
class_or_object => rdi
method_name => rsi
*/

bool fh_method_exists(TypedValue* class_or_object, Value* method_name) asm("_ZN4HPHP15f_method_existsERKNS_7VariantERKNS_6StringE");

/*
bool HPHP::f_property_exists(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP17f_property_existsERKNS_7VariantERKNS_6StringE

(return value) => rax
class_or_object => rdi
property => rsi
*/

bool fh_property_exists(TypedValue* class_or_object, Value* property) asm("_ZN4HPHP17f_property_existsERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_get_object_vars(HPHP::Variant const&)
_ZN4HPHP17f_get_object_varsERKNS_7VariantE

(return value) => rax
_rv => rdi
object => rsi
*/

TypedValue* fh_get_object_vars(TypedValue* _rv, TypedValue* object) asm("_ZN4HPHP17f_get_object_varsERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_call_user_method_array(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::Array const&)
_ZN4HPHP24f_call_user_method_arrayERKNS_6StringERKNS_14VRefParamValueERKNS_5ArrayE

(return value) => rax
_rv => rdi
method_name => rsi
obj => rdx
paramarr => rcx
*/

TypedValue* fh_call_user_method_array(TypedValue* _rv, Value* method_name, TypedValue* obj, Value* paramarr) asm("_ZN4HPHP24f_call_user_method_arrayERKNS_6StringERKNS_14VRefParamValueERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_call_user_method(int, HPHP::String const&, HPHP::VRefParamValue const&, HPHP::Array const&)
_ZN4HPHP18f_call_user_methodEiRKNS_6StringERKNS_14VRefParamValueERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
method_name => rdx
obj => rcx
_argv => r8
*/

TypedValue* fh_call_user_method(TypedValue* _rv, long long _argc, Value* method_name, TypedValue* obj, Value* _argv) asm("_ZN4HPHP18f_call_user_methodEiRKNS_6StringERKNS_14VRefParamValueERKNS_5ArrayE");


} // !HPHP

