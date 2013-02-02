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
HPHP::Object HPHP::f_xml_parser_create(HPHP::String const&)
_ZN4HPHP19f_xml_parser_createERKNS_6StringE

(return value) => rax
_rv => rdi
encoding => rsi
*/

Value* fh_xml_parser_create(Value* _rv, Value* encoding) asm("_ZN4HPHP19f_xml_parser_createERKNS_6StringE");

/*
bool HPHP::f_xml_parser_free(HPHP::Object const&)
_ZN4HPHP17f_xml_parser_freeERKNS_6ObjectE

(return value) => rax
parser => rdi
*/

bool fh_xml_parser_free(Value* parser) asm("_ZN4HPHP17f_xml_parser_freeERKNS_6ObjectE");

/*
long long HPHP::f_xml_parse(HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP11f_xml_parseERKNS_6ObjectERKNS_6StringEb

(return value) => rax
parser => rdi
data => rsi
is_final => rdx
*/

long long fh_xml_parse(Value* parser, Value* data, bool is_final) asm("_ZN4HPHP11f_xml_parseERKNS_6ObjectERKNS_6StringEb");

/*
long long HPHP::f_xml_parse_into_struct(HPHP::Object const&, HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP23f_xml_parse_into_structERKNS_6ObjectERKNS_6StringERKNS_14VRefParamValueES8_

(return value) => rax
parser => rdi
data => rsi
values => rdx
index => rcx
*/

long long fh_xml_parse_into_struct(Value* parser, Value* data, TypedValue* values, TypedValue* index) asm("_ZN4HPHP23f_xml_parse_into_structERKNS_6ObjectERKNS_6StringERKNS_14VRefParamValueES8_");

/*
HPHP::Object HPHP::f_xml_parser_create_ns(HPHP::String const&, HPHP::String const&)
_ZN4HPHP22f_xml_parser_create_nsERKNS_6StringES2_

(return value) => rax
_rv => rdi
encoding => rsi
separator => rdx
*/

Value* fh_xml_parser_create_ns(Value* _rv, Value* encoding, Value* separator) asm("_ZN4HPHP22f_xml_parser_create_nsERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_xml_parser_get_option(HPHP::Object const&, int)
_ZN4HPHP23f_xml_parser_get_optionERKNS_6ObjectEi

(return value) => rax
_rv => rdi
parser => rsi
option => rdx
*/

TypedValue* fh_xml_parser_get_option(TypedValue* _rv, Value* parser, int option) asm("_ZN4HPHP23f_xml_parser_get_optionERKNS_6ObjectEi");

/*
bool HPHP::f_xml_parser_set_option(HPHP::Object const&, int, HPHP::Variant const&)
_ZN4HPHP23f_xml_parser_set_optionERKNS_6ObjectEiRKNS_7VariantE

(return value) => rax
parser => rdi
option => rsi
value => rdx
*/

bool fh_xml_parser_set_option(Value* parser, int option, TypedValue* value) asm("_ZN4HPHP23f_xml_parser_set_optionERKNS_6ObjectEiRKNS_7VariantE");

/*
bool HPHP::f_xml_set_character_data_handler(HPHP::Object const&, HPHP::Variant const&)
_ZN4HPHP32f_xml_set_character_data_handlerERKNS_6ObjectERKNS_7VariantE

(return value) => rax
parser => rdi
handler => rsi
*/

bool fh_xml_set_character_data_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP32f_xml_set_character_data_handlerERKNS_6ObjectERKNS_7VariantE");

/*
bool HPHP::f_xml_set_default_handler(HPHP::Object const&, HPHP::Variant const&)
_ZN4HPHP25f_xml_set_default_handlerERKNS_6ObjectERKNS_7VariantE

(return value) => rax
parser => rdi
handler => rsi
*/

bool fh_xml_set_default_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP25f_xml_set_default_handlerERKNS_6ObjectERKNS_7VariantE");

/*
bool HPHP::f_xml_set_element_handler(HPHP::Object const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP25f_xml_set_element_handlerERKNS_6ObjectERKNS_7VariantES5_

(return value) => rax
parser => rdi
start_element_handler => rsi
end_element_handler => rdx
*/

bool fh_xml_set_element_handler(Value* parser, TypedValue* start_element_handler, TypedValue* end_element_handler) asm("_ZN4HPHP25f_xml_set_element_handlerERKNS_6ObjectERKNS_7VariantES5_");

/*
bool HPHP::f_xml_set_processing_instruction_handler(HPHP::Object const&, HPHP::Variant const&)
_ZN4HPHP40f_xml_set_processing_instruction_handlerERKNS_6ObjectERKNS_7VariantE

(return value) => rax
parser => rdi
handler => rsi
*/

bool fh_xml_set_processing_instruction_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP40f_xml_set_processing_instruction_handlerERKNS_6ObjectERKNS_7VariantE");

/*
bool HPHP::f_xml_set_start_namespace_decl_handler(HPHP::Object const&, HPHP::Variant const&)
_ZN4HPHP38f_xml_set_start_namespace_decl_handlerERKNS_6ObjectERKNS_7VariantE

(return value) => rax
parser => rdi
handler => rsi
*/

bool fh_xml_set_start_namespace_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP38f_xml_set_start_namespace_decl_handlerERKNS_6ObjectERKNS_7VariantE");

/*
bool HPHP::f_xml_set_end_namespace_decl_handler(HPHP::Object const&, HPHP::Variant const&)
_ZN4HPHP36f_xml_set_end_namespace_decl_handlerERKNS_6ObjectERKNS_7VariantE

(return value) => rax
parser => rdi
handler => rsi
*/

bool fh_xml_set_end_namespace_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP36f_xml_set_end_namespace_decl_handlerERKNS_6ObjectERKNS_7VariantE");

/*
bool HPHP::f_xml_set_unparsed_entity_decl_handler(HPHP::Object const&, HPHP::Variant const&)
_ZN4HPHP38f_xml_set_unparsed_entity_decl_handlerERKNS_6ObjectERKNS_7VariantE

(return value) => rax
parser => rdi
handler => rsi
*/

bool fh_xml_set_unparsed_entity_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP38f_xml_set_unparsed_entity_decl_handlerERKNS_6ObjectERKNS_7VariantE");

/*
bool HPHP::f_xml_set_external_entity_ref_handler(HPHP::Object const&, HPHP::Variant const&)
_ZN4HPHP37f_xml_set_external_entity_ref_handlerERKNS_6ObjectERKNS_7VariantE

(return value) => rax
parser => rdi
handler => rsi
*/

bool fh_xml_set_external_entity_ref_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP37f_xml_set_external_entity_ref_handlerERKNS_6ObjectERKNS_7VariantE");

/*
bool HPHP::f_xml_set_notation_decl_handler(HPHP::Object const&, HPHP::Variant const&)
_ZN4HPHP31f_xml_set_notation_decl_handlerERKNS_6ObjectERKNS_7VariantE

(return value) => rax
parser => rdi
handler => rsi
*/

bool fh_xml_set_notation_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP31f_xml_set_notation_decl_handlerERKNS_6ObjectERKNS_7VariantE");

/*
bool HPHP::f_xml_set_object(HPHP::Object const&, HPHP::VRefParamValue const&)
_ZN4HPHP16f_xml_set_objectERKNS_6ObjectERKNS_14VRefParamValueE

(return value) => rax
parser => rdi
object => rsi
*/

bool fh_xml_set_object(Value* parser, TypedValue* object) asm("_ZN4HPHP16f_xml_set_objectERKNS_6ObjectERKNS_14VRefParamValueE");

/*
long long HPHP::f_xml_get_current_byte_index(HPHP::Object const&)
_ZN4HPHP28f_xml_get_current_byte_indexERKNS_6ObjectE

(return value) => rax
parser => rdi
*/

long long fh_xml_get_current_byte_index(Value* parser) asm("_ZN4HPHP28f_xml_get_current_byte_indexERKNS_6ObjectE");

/*
long long HPHP::f_xml_get_current_column_number(HPHP::Object const&)
_ZN4HPHP31f_xml_get_current_column_numberERKNS_6ObjectE

(return value) => rax
parser => rdi
*/

long long fh_xml_get_current_column_number(Value* parser) asm("_ZN4HPHP31f_xml_get_current_column_numberERKNS_6ObjectE");

/*
long long HPHP::f_xml_get_current_line_number(HPHP::Object const&)
_ZN4HPHP29f_xml_get_current_line_numberERKNS_6ObjectE

(return value) => rax
parser => rdi
*/

long long fh_xml_get_current_line_number(Value* parser) asm("_ZN4HPHP29f_xml_get_current_line_numberERKNS_6ObjectE");

/*
long long HPHP::f_xml_get_error_code(HPHP::Object const&)
_ZN4HPHP20f_xml_get_error_codeERKNS_6ObjectE

(return value) => rax
parser => rdi
*/

long long fh_xml_get_error_code(Value* parser) asm("_ZN4HPHP20f_xml_get_error_codeERKNS_6ObjectE");

/*
HPHP::String HPHP::f_xml_error_string(int)
_ZN4HPHP18f_xml_error_stringEi

(return value) => rax
_rv => rdi
code => rsi
*/

Value* fh_xml_error_string(Value* _rv, int code) asm("_ZN4HPHP18f_xml_error_stringEi");

/*
HPHP::String HPHP::f_utf8_decode(HPHP::String const&)
_ZN4HPHP13f_utf8_decodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

Value* fh_utf8_decode(Value* _rv, Value* data) asm("_ZN4HPHP13f_utf8_decodeERKNS_6StringE");

/*
HPHP::String HPHP::f_utf8_encode(HPHP::String const&)
_ZN4HPHP13f_utf8_encodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

Value* fh_utf8_encode(Value* _rv, Value* data) asm("_ZN4HPHP13f_utf8_encodeERKNS_6StringE");


} // !HPHP

