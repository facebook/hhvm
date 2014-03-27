<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function xml_parser_create($encoding = null) { }
function xml_parser_free($parser) { }
function xml_parse($parser, $data, $is_final = true) { }
function xml_parse_into_struct($parser, $data, &$values, &$index = null) { }
function xml_parser_create_ns($encoding = null, $separator = null) { }
function xml_parser_get_option($parser, $option) { }
function xml_parser_set_option($parser, $option, $value) { }
function xml_set_character_data_handler($parser, $handler) { }
function xml_set_default_handler($parser, $handler) { }
function xml_set_element_handler($parser, $start_element_handler, $end_element_handler) { }
function xml_set_processing_instruction_handler($parser, $handler) { }
function xml_set_start_namespace_decl_handler($parser, $handler) { }
function xml_set_end_namespace_decl_handler($parser, $handler) { }
function xml_set_unparsed_entity_decl_handler($parser, $handler) { }
function xml_set_external_entity_ref_handler($parser, $handler) { }
function xml_set_notation_decl_handler($parser, $handler) { }
function xml_set_object($parser, &$object) { }
function xml_get_current_byte_index($parser) { }
function xml_get_current_column_number($parser) { }
function xml_get_current_line_number($parser) { }
function xml_get_error_code($parser) { }
function xml_error_string($code) { }
function utf8_decode($data) { }
function utf8_encode($data) { }
