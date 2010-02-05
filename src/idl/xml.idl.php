<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('xml_parser_create', Resource,
  array('encoding' => array(String, 'null_string')));

f('xml_parser_free', Boolean,
  array('parser' => Resource));

f('xml_parse', Int32,
  array('parser' => Resource,
        'data' => String,
        'is_final' => array(Boolean, 'true')));

f('xml_parse_into_struct', Int32,
  array('parser' => Resource,
        'data' => String,
        'values' => StringVec | Reference,
        'index' => array(StringVec | Reference, 'null')));

f('xml_parser_create_ns', Resource,
  array('encoding' => array(String, 'null_string'),
        'separator' => array(String, 'null_string')));

f('xml_parser_get_option', Variant,
  array('parser' => Resource,
        'option' => Int32));

f('xml_parser_set_option', Boolean,
  array('parser' => Resource,
        'option' => Int32,
        'value' => Variant));

f('xml_set_character_data_handler', Boolean,
  array('parser' => Resource,
        'handler' => Variant));

f('xml_set_default_handler', Boolean,
  array('parser' => Resource,
        'handler' => Variant));

f('xml_set_element_handler', Boolean,
  array('parser' => Resource,
        'start_element_handler' => Variant,
        'end_element_handler' => Variant));

f('xml_set_processing_instruction_handler', Boolean,
  array('parser' => Resource,
        'handler' => Variant));

f('xml_set_start_namespace_decl_handler', Boolean,
  array('parser' => Resource,
        'handler' => Variant));

f('xml_set_end_namespace_decl_handler', Boolean,
  array('parser' => Resource,
        'handler' => Variant));

f('xml_set_unparsed_entity_decl_handler', Boolean,
  array('parser' => Resource,
        'handler' => Variant));

f('xml_set_external_entity_ref_handler', Boolean,
  array('parser' => Resource,
        'handler' => Variant));

f('xml_set_notation_decl_handler', Boolean,
  array('parser' => Resource,
        'handler' => Variant));

f('xml_set_object', Boolean,
  array('parser' => Resource,
        'object' => Object | Reference));

f('xml_get_current_byte_index', Int32,
  array('parser' => Resource));

f('xml_get_current_column_number', Int32,
  array('parser' => Resource));

f('xml_get_current_line_number', Int32,
  array('parser' => Resource));

f('xml_get_error_code', Int32,
  array('parser' => Resource));

f('xml_error_string', String,
  array('code' => Int32));

///////////////////////////////////////////////////////////////////////////////
// utf8

f('utf8_decode', String,
  array('data' => String));

f('utf8_encode', String,
  array('data' => String));
