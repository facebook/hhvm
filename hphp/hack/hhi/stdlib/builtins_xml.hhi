<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const XML_ERROR_ASYNC_ENTITY = 13;
const XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF = 16;
const XML_ERROR_BAD_CHAR_REF = 14;
const XML_ERROR_BINARY_ENTITY_REF = 15;
const XML_ERROR_DUPLICATE_ATTRIBUTE = 8;
const XML_ERROR_EXTERNAL_ENTITY_HANDLING = 21;
const XML_ERROR_INCORRECT_ENCODING = 19;
const XML_ERROR_INVALID_TOKEN = 4;
const XML_ERROR_JUNK_AFTER_DOC_ELEMENT = 9;
const XML_ERROR_MISPLACED_XML_PI = 17;
const XML_ERROR_NONE = 0;
const XML_ERROR_NO_ELEMENTS = 3;
const XML_ERROR_NO_MEMORY = 1;
const XML_ERROR_PARAM_ENTITY_REF = 10;
const XML_ERROR_PARTIAL_CHAR = 6;
const XML_ERROR_RECURSIVE_ENTITY_REF = 12;
const XML_ERROR_SYNTAX = 2;
const XML_ERROR_TAG_MISMATCH = 7;
const XML_ERROR_UNCLOSED_CDATA_SECTION = 20;
const XML_ERROR_UNCLOSED_TOKEN = 5;
const XML_ERROR_UNDEFINED_ENTITY = 11;
const XML_ERROR_UNKNOWN_ENCODING = 18;

const XML_OPTION_CASE_FOLDING = 1;
const XML_OPTION_SKIP_TAGSTART = 3;
const XML_OPTION_SKIP_WHITE = 4;
const XML_OPTION_TARGET_ENCODING = 2;

const XML_SAX_IMPL = "expat";

<<__PHPStdLib>>
function xml_parser_create($encoding = null);
<<__PHPStdLib>>
function xml_parser_free(resource $parser);
<<__PHPStdLib>>
function xml_parse(resource $parser, string $data, bool $is_final = true);
<<__PHPStdLib>>
function xml_parse_into_struct(resource $parser, string $data,
                               inout $values, inout $index);
<<__PHPStdLib>>
function xml_parser_create_ns($encoding = null, $separator = null);
<<__PHPStdLib>>
function xml_parser_get_option(resource $parser, int $option);
<<__PHPStdLib>>
function xml_parser_set_option(resource $parser, int $option, $value);
<<__PHPStdLib>>
function xml_set_character_data_handler(resource $parser, $handler);
<<__PHPStdLib>>
function xml_set_default_handler(resource $parser, $handler);
<<__PHPStdLib>>
function xml_set_element_handler(resource $parser, $start_element_handler, $end_element_handler);
<<__PHPStdLib>>
function xml_set_processing_instruction_handler(resource $parser, $handler);
<<__PHPStdLib>>
function xml_set_start_namespace_decl_handler(resource $parser, $handler);
<<__PHPStdLib>>
function xml_set_end_namespace_decl_handler(resource $parser, $handler);
<<__PHPStdLib>>
function xml_set_unparsed_entity_decl_handler(resource $parser, $handler);
<<__PHPStdLib>>
function xml_set_external_entity_ref_handler(resource $parser, $handler);
<<__PHPStdLib>>
function xml_set_notation_decl_handler(resource $parser, $handler);
<<__PHPStdLib>>
function xml_set_object(resource $parser, $object);
<<__PHPStdLib>>
function xml_get_current_byte_index(resource $parser);
<<__PHPStdLib>>
function xml_get_current_column_number(resource $parser);
<<__PHPStdLib>>
function xml_get_current_line_number(resource $parser);
<<__PHPStdLib>>
function xml_get_error_code(resource $parser);
<<__PHPStdLib>>
function xml_error_string(int $code);
<<__PHPStdLib, __Rx>>
function utf8_decode(string $data);
<<__PHPStdLib, __Rx>>
function utf8_encode(string $data);
