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

Value* fh_xml_parser_create(Value* _rv, Value* encoding) asm("_ZN4HPHP19f_xml_parser_createERKNS_6StringE");

Value* fh_xml_parser_create_ns(Value* _rv, Value* encoding, Value* separator) asm("_ZN4HPHP22f_xml_parser_create_nsERKNS_6StringES2_");

bool fh_xml_parser_free(Value* parser) asm("_ZN4HPHP17f_xml_parser_freeERKNS_6ObjectE");

long fh_xml_parse(Value* parser, Value* data, bool is_final) asm("_ZN4HPHP11f_xml_parseERKNS_6ObjectERKNS_6StringEb");

long fh_xml_parse_into_struct(Value* parser, Value* data, TypedValue* values, TypedValue* index) asm("_ZN4HPHP23f_xml_parse_into_structERKNS_6ObjectERKNS_6StringERKNS_14VRefParamValueES8_");

TypedValue* fh_xml_parser_get_option(TypedValue* _rv, Value* parser, int option) asm("_ZN4HPHP23f_xml_parser_get_optionERKNS_6ObjectEi");

bool fh_xml_parser_set_option(Value* parser, int option, TypedValue* value) asm("_ZN4HPHP23f_xml_parser_set_optionERKNS_6ObjectEiRKNS_7VariantE");

bool fh_xml_set_character_data_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP32f_xml_set_character_data_handlerERKNS_6ObjectERKNS_7VariantE");

bool fh_xml_set_default_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP25f_xml_set_default_handlerERKNS_6ObjectERKNS_7VariantE");

bool fh_xml_set_element_handler(Value* parser, TypedValue* start_element_handler, TypedValue* end_element_handler) asm("_ZN4HPHP25f_xml_set_element_handlerERKNS_6ObjectERKNS_7VariantES5_");

bool fh_xml_set_processing_instruction_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP40f_xml_set_processing_instruction_handlerERKNS_6ObjectERKNS_7VariantE");

bool fh_xml_set_start_namespace_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP38f_xml_set_start_namespace_decl_handlerERKNS_6ObjectERKNS_7VariantE");

bool fh_xml_set_end_namespace_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP36f_xml_set_end_namespace_decl_handlerERKNS_6ObjectERKNS_7VariantE");

bool fh_xml_set_unparsed_entity_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP38f_xml_set_unparsed_entity_decl_handlerERKNS_6ObjectERKNS_7VariantE");

bool fh_xml_set_external_entity_ref_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP37f_xml_set_external_entity_ref_handlerERKNS_6ObjectERKNS_7VariantE");

bool fh_xml_set_notation_decl_handler(Value* parser, TypedValue* handler) asm("_ZN4HPHP31f_xml_set_notation_decl_handlerERKNS_6ObjectERKNS_7VariantE");

bool fh_xml_set_object(Value* parser, TypedValue* object) asm("_ZN4HPHP16f_xml_set_objectERKNS_6ObjectERKNS_14VRefParamValueE");

long fh_xml_get_current_byte_index(Value* parser) asm("_ZN4HPHP28f_xml_get_current_byte_indexERKNS_6ObjectE");

long fh_xml_get_current_column_number(Value* parser) asm("_ZN4HPHP31f_xml_get_current_column_numberERKNS_6ObjectE");

long fh_xml_get_current_line_number(Value* parser) asm("_ZN4HPHP29f_xml_get_current_line_numberERKNS_6ObjectE");

long fh_xml_get_error_code(Value* parser) asm("_ZN4HPHP20f_xml_get_error_codeERKNS_6ObjectE");

Value* fh_xml_error_string(Value* _rv, int code) asm("_ZN4HPHP18f_xml_error_stringEi");

Value* fh_utf8_decode(Value* _rv, Value* data) asm("_ZN4HPHP13f_utf8_decodeERKNS_6StringE");

Value* fh_utf8_encode(Value* _rv, Value* data) asm("_ZN4HPHP13f_utf8_encodeERKNS_6StringE");

} // namespace HPHP
