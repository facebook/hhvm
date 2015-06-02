/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_XML_H_
#define incl_HPHP_EXT_XML_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Resource HHVM_FUNCTION(xml_parser_create,
                       const Variant& encoding = null_variant);
bool HHVM_FUNCTION(xml_parser_free,
                   const Resource& parser);
int64_t HHVM_FUNCTION(xml_parse,
                      const Resource& parser,
                      const String& data,
                      bool is_final = true);
int64_t HHVM_FUNCTION(xml_parse_into_struct,
                      const Resource& parser,
                      const String& data,
                      VRefParam values,
                      VRefParam index = uninit_null());
Resource HHVM_FUNCTION(xml_parser_create_ns,
                       const Variant& encoding = null_variant,
                       const Variant& separator = null_variant);
Variant HHVM_FUNCTION(xml_parser_get_option,
                      const Resource& parser,
                      int option);
bool HHVM_FUNCTION(xml_parser_set_option,
                   const Resource& parser,
                   int option,
                   const Variant& value);
bool HHVM_FUNCTION(xml_set_character_data_handler,
                   const Resource& parser,
                   const Variant& handler);
bool HHVM_FUNCTION(xml_set_default_handler,
                   const Resource& parser,
                   const Variant& handler);
bool HHVM_FUNCTION(xml_set_element_handler,
                   const Resource& parser,
                   const Variant& start_element_handler,
                   const Variant& end_element_handler);
bool HHVM_FUNCTION(xml_set_processing_instruction_handler,
                   const Resource& parser,
                   const Variant& handler);
bool HHVM_FUNCTION(xml_set_start_namespace_decl_handler,
                   const Resource& parser,
                   const Variant& handler);
bool HHVM_FUNCTION(xml_set_end_namespace_decl_handler,
                   const Resource& parser,
                   const Variant& handler);
bool HHVM_FUNCTION(xml_set_unparsed_entity_decl_handler,
                   const Resource& parser,
                   const Variant& handler);
bool HHVM_FUNCTION(xml_set_external_entity_ref_handler,
                   const Resource& parser,
                   const Variant& handler);
bool HHVM_FUNCTION(xml_set_notation_decl_handler,
                   const Resource& parser,
                   const Variant& handler);
bool HHVM_FUNCTION(xml_set_object,
                   const Resource& parser,
                   VRefParam object);
int64_t HHVM_FUNCTION(xml_get_current_byte_index,
                      const Resource& parser);
int64_t HHVM_FUNCTION(xml_get_current_column_number,
                      const Resource& parser);
int64_t HHVM_FUNCTION(xml_get_current_line_number,
                      const Resource& parser);
int64_t HHVM_FUNCTION(xml_get_error_code,
                      const Resource& parser);
String HHVM_FUNCTION(xml_error_string,
                     int code);
String HHVM_FUNCTION(utf8_decode,
                     const String& data);
String HHVM_FUNCTION(utf8_encode,
                     const String& data);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_XML_H_
