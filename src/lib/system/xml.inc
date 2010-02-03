#if EXT_TYPE == 0
"xml_parser_create", T(Object), S(0), "encoding", T(String), "null_string", S(0), NULL, S(0), 
"xml_parser_free", T(Boolean), S(0), "parser", T(Object), NULL, S(0), NULL, S(0), 
"xml_parse", T(Int32), S(0), "parser", T(Object), NULL, S(0), "data", T(String), NULL, S(0), "is_final", T(Boolean), "true", S(0), NULL, S(0), 
"xml_parse_into_struct", T(Int32), S(0), "parser", T(Object), NULL, S(0), "data", T(String), NULL, S(0), "values", T(Variant), NULL, S(1), "index", T(Variant), "null", S(1), NULL, S(0), 
"xml_parser_create_ns", T(Object), S(0), "encoding", T(String), "null_string", S(0), "separator", T(String), "null_string", S(0), NULL, S(0), 
"xml_parser_get_option", T(Variant), S(0), "parser", T(Object), NULL, S(0), "option", T(Int32), NULL, S(0), NULL, S(0), 
"xml_parser_set_option", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "option", T(Int32), NULL, S(0), "value", T(Variant), NULL, S(0), NULL, S(0), 
"xml_set_character_data_handler", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "handler", T(String), NULL, S(0), NULL, S(0), 
"xml_set_default_handler", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "handler", T(String), NULL, S(0), NULL, S(0), 
"xml_set_element_handler", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "start_element_handler", T(String), NULL, S(0), "end_element_handler", T(String), NULL, S(0), NULL, S(0), 
"xml_set_processing_instruction_handler", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "handler", T(String), NULL, S(0), NULL, S(0), 
"xml_set_start_namespace_decl_handler", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "handler", T(String), NULL, S(0), NULL, S(0), 
"xml_set_end_namespace_decl_handler", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "handler", T(String), NULL, S(0), NULL, S(0), 
"xml_set_unparsed_entity_decl_handler", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "handler", T(String), NULL, S(0), NULL, S(0), 
"xml_set_external_entity_ref_handler", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "handler", T(String), NULL, S(0), NULL, S(0), 
"xml_set_notation_decl_handler", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "handler", T(String), NULL, S(0), NULL, S(0), 
"xml_set_object", T(Boolean), S(0), "parser", T(Object), NULL, S(0), "object", T(Variant), NULL, S(1), NULL, S(0), 
"xml_get_current_byte_index", T(Int32), S(0), "parser", T(Object), NULL, S(0), NULL, S(0), 
"xml_get_current_column_number", T(Int32), S(0), "parser", T(Object), NULL, S(0), NULL, S(0), 
"xml_get_current_line_number", T(Int32), S(0), "parser", T(Object), NULL, S(0), NULL, S(0), 
"xml_get_error_code", T(Int32), S(0), "parser", T(Object), NULL, S(0), NULL, S(0), 
"xml_error_string", T(String), S(0), "code", T(Int32), NULL, S(0), NULL, S(0), 
"utf8_decode", T(String), S(0), "data", T(String), NULL, S(0), NULL, S(0), 
"utf8_encode", T(String), S(0), "data", T(String), NULL, S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
