<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int XML_ERROR_ASYNC_ENTITY;
const int XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF;
const int XML_ERROR_BAD_CHAR_REF;
const int XML_ERROR_BINARY_ENTITY_REF;
const int XML_ERROR_DUPLICATE_ATTRIBUTE;
const int XML_ERROR_EXTERNAL_ENTITY_HANDLING;
const int XML_ERROR_INCORRECT_ENCODING;
const int XML_ERROR_INVALID_TOKEN;
const int XML_ERROR_JUNK_AFTER_DOC_ELEMENT;
const int XML_ERROR_MISPLACED_XML_PI;
const int XML_ERROR_NONE;
const int XML_ERROR_NO_ELEMENTS;
const int XML_ERROR_NO_MEMORY;
const int XML_ERROR_PARAM_ENTITY_REF;
const int XML_ERROR_PARTIAL_CHAR;
const int XML_ERROR_RECURSIVE_ENTITY_REF;
const int XML_ERROR_SYNTAX;
const int XML_ERROR_TAG_MISMATCH;
const int XML_ERROR_UNCLOSED_CDATA_SECTION;
const int XML_ERROR_UNCLOSED_TOKEN;
const int XML_ERROR_UNDEFINED_ENTITY;
const int XML_ERROR_UNKNOWN_ENCODING;

const int XML_OPTION_CASE_FOLDING;
const int XML_OPTION_SKIP_TAGSTART;
const int XML_OPTION_SKIP_WHITE;
const int XML_OPTION_TARGET_ENCODING;

const string XML_SAX_IMPL;

<<__PHPStdLib>>
function xml_parser_create(
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_parser_free(resource $parser): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_parse(
  resource $parser,
  string $data,
  bool $is_final = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_parse_into_struct(
  resource $parser,
  string $data,
  inout HH\FIXME\MISSING_PARAM_TYPE $values,
  inout HH\FIXME\MISSING_PARAM_TYPE $index,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_parser_create_ns(
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
  HH\FIXME\MISSING_PARAM_TYPE $separator = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_parser_get_option(
  resource $parser,
  int $option,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_parser_set_option(
  resource $parser,
  int $option,
  HH\FIXME\MISSING_PARAM_TYPE $value,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_set_character_data_handler(
  resource $parser,
  HH\FIXME\MISSING_PARAM_TYPE $handler,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_set_default_handler(
  resource $parser,
  HH\FIXME\MISSING_PARAM_TYPE $handler,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_set_element_handler(
  resource $parser,
  HH\FIXME\MISSING_PARAM_TYPE $start_element_handler,
  HH\FIXME\MISSING_PARAM_TYPE $end_element_handler,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_set_processing_instruction_handler(
  resource $parser,
  HH\FIXME\MISSING_PARAM_TYPE $handler,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_set_start_namespace_decl_handler(
  resource $parser,
  HH\FIXME\MISSING_PARAM_TYPE $handler,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_set_end_namespace_decl_handler(
  resource $parser,
  HH\FIXME\MISSING_PARAM_TYPE $handler,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_set_unparsed_entity_decl_handler(
  resource $parser,
  HH\FIXME\MISSING_PARAM_TYPE $handler,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_set_external_entity_ref_handler(
  resource $parser,
  HH\FIXME\MISSING_PARAM_TYPE $handler,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_set_notation_decl_handler(
  resource $parser,
  HH\FIXME\MISSING_PARAM_TYPE $handler,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_set_object(
  resource $parser,
  HH\FIXME\MISSING_PARAM_TYPE $object,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_get_current_byte_index(
  resource $parser,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_get_current_column_number(
  resource $parser,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_get_current_line_number(
  resource $parser,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_get_error_code(resource $parser): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xml_error_string(int $code): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function utf8_decode(string $data)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function utf8_encode(string $data)[]: HH\FIXME\MISSING_RETURN_TYPE;
