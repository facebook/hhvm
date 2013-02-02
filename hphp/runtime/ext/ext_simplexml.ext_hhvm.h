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
HPHP::Variant HPHP::f_simplexml_load_string(HPHP::String const&, HPHP::String const&, long long, HPHP::String const&, bool)
_ZN4HPHP23f_simplexml_load_stringERKNS_6StringES2_xS2_b

(return value) => rax
_rv => rdi
data => rsi
class_name => rdx
options => rcx
ns => r8
is_prefix => r9
*/

TypedValue* fh_simplexml_load_string(TypedValue* _rv, Value* data, Value* class_name, long long options, Value* ns, bool is_prefix) asm("_ZN4HPHP23f_simplexml_load_stringERKNS_6StringES2_xS2_b");

/*
HPHP::Variant HPHP::f_simplexml_load_file(HPHP::String const&, HPHP::String const&, long long, HPHP::String const&, bool)
_ZN4HPHP21f_simplexml_load_fileERKNS_6StringES2_xS2_b

(return value) => rax
_rv => rdi
filename => rsi
class_name => rdx
options => rcx
ns => r8
is_prefix => r9
*/

TypedValue* fh_simplexml_load_file(TypedValue* _rv, Value* filename, Value* class_name, long long options, Value* ns, bool is_prefix) asm("_ZN4HPHP21f_simplexml_load_fileERKNS_6StringES2_xS2_b");

/*
HPHP::Variant HPHP::f_libxml_get_errors()
_ZN4HPHP19f_libxml_get_errorsEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_libxml_get_errors(TypedValue* _rv) asm("_ZN4HPHP19f_libxml_get_errorsEv");

/*
HPHP::Variant HPHP::f_libxml_get_last_error()
_ZN4HPHP23f_libxml_get_last_errorEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_libxml_get_last_error(TypedValue* _rv) asm("_ZN4HPHP23f_libxml_get_last_errorEv");

/*
void HPHP::f_libxml_clear_errors()
_ZN4HPHP21f_libxml_clear_errorsEv

*/

void fh_libxml_clear_errors() asm("_ZN4HPHP21f_libxml_clear_errorsEv");

/*
bool HPHP::f_libxml_use_internal_errors(HPHP::Variant const&)
_ZN4HPHP28f_libxml_use_internal_errorsERKNS_7VariantE

(return value) => rax
use_errors => rdi
*/

bool fh_libxml_use_internal_errors(TypedValue* use_errors) asm("_ZN4HPHP28f_libxml_use_internal_errorsERKNS_7VariantE");

/*
void HPHP::f_libxml_set_streams_context(HPHP::Object const&)
_ZN4HPHP28f_libxml_set_streams_contextERKNS_6ObjectE

streams_context => rdi
*/

void fh_libxml_set_streams_context(Value* streams_context) asm("_ZN4HPHP28f_libxml_set_streams_contextERKNS_6ObjectE");

/*
bool HPHP::f_libxml_disable_entity_loader(bool)
_ZN4HPHP30f_libxml_disable_entity_loaderEb

(return value) => rax
disable => rdi
*/

bool fh_libxml_disable_entity_loader(bool disable) asm("_ZN4HPHP30f_libxml_disable_entity_loaderEb");


} // !HPHP

