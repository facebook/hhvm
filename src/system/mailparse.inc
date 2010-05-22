#if EXT_TYPE == 0
"mail", T(Boolean), S(0), "to", T(String), NULL, S(0), "subject", T(String), NULL, S(0), "message", T(String), NULL, S(0), "additional_headers", T(String), "null_string", S(0), "additional_parameters", T(String), "null_string", S(0), NULL, S(0), 
"ezmlm_hash", T(Int32), S(0), "addr", T(String), NULL, S(0), NULL, S(0), 
"mailparse_msg_create", T(Object), S(0), NULL, S(0), 
"mailparse_msg_free", T(Boolean), S(0), "mimemail", T(Object), NULL, S(0), NULL, S(0), 
"mailparse_msg_parse_file", T(Variant), S(0), "filename", T(String), NULL, S(0), NULL, S(0), 
"mailparse_msg_parse", T(Boolean), S(0), "mimemail", T(Object), NULL, S(0), "data", T(String), NULL, S(0), NULL, S(0), 
"mailparse_msg_extract_part_file", T(Variant), S(0), "mimemail", T(Object), NULL, S(0), "filename", T(Variant), NULL, S(0), "callbackfunc", T(Variant), "\"\"", S(0), NULL, S(0), 
"mailparse_msg_extract_whole_part_file", T(Variant), S(0), "mimemail", T(Object), NULL, S(0), "filename", T(Variant), NULL, S(0), "callbackfunc", T(Variant), "\"\"", S(0), NULL, S(0), 
"mailparse_msg_extract_part", T(Variant), S(0), "mimemail", T(Object), NULL, S(0), "msgbody", T(Variant), NULL, S(0), "callbackfunc", T(Variant), "\"\"", S(0), NULL, S(0), 
"mailparse_msg_get_part_data", T(Array), S(0), "mimemail", T(Object), NULL, S(0), NULL, S(0), 
"mailparse_msg_get_part", T(Variant), S(0), "mimemail", T(Object), NULL, S(0), "mimesection", T(String), NULL, S(0), NULL, S(0), 
"mailparse_msg_get_structure", T(Array), S(0), "mimemail", T(Object), NULL, S(0), NULL, S(0), 
"mailparse_rfc822_parse_addresses", T(Array), S(0), "addresses", T(String), NULL, S(0), NULL, S(0), 
"mailparse_stream_encode", T(Boolean), S(0), "sourcefp", T(Object), NULL, S(0), "destfp", T(Object), NULL, S(0), "encoding", T(String), NULL, S(0), NULL, S(0), 
"mailparse_uudecode_all", T(Variant), S(0), "fp", T(Object), NULL, S(0), NULL, S(0), 
"mailparse_determine_best_xfer_encoding", T(Variant), S(0), "fp", T(Object), NULL, S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
