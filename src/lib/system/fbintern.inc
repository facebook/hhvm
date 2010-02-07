#if EXT_TYPE == 0
"fb_set_opcode", T(Void), S(0), "opcode", T(Int32), NULL, S(0), "callback", T(String), NULL, S(0), NULL, S(0), 
"fb_reset_opcode", T(Void), S(0), "opcode", T(Int32), NULL, S(0), NULL, S(0), 
"fb_config_coredump", T(Void), S(0), "enabled", T(Boolean), NULL, S(0), "limit", T(Int32), NULL, S(0), NULL, S(0), 
"fb_debug_rlog", T(Boolean), S(0), "err", T(Variant), NULL, S(0), "include_flags", T(Int32), "-1", S(0), "type", T(Int32), "-1", S(0), "headers", T(Variant), "null_variant", S(0), NULL, S(0), 
"fb_backtrace", T(String), S(0), "exception", T(Variant), "null_variant", S(0), NULL, S(0), 
"fb_render_wrapped", T(String), S(0), "text", T(String), NULL, S(0), "linelen", T(Int32), "26", S(0), "head", T(String), "\"<span>\"", S(0), "tail", T(String), "\"</span><wbr></wbr><span class=\\\"word_break\\\"></span>\"", S(0), NULL, S(0), 
"fb_add_included_file", T(Void), S(0), "filepath", T(String), NULL, S(0), NULL, S(0), 
"fb_request_timers", T(Variant), S(0), NULL, S(0), 
"fb_get_ape_version", T(String), S(0), NULL, S(0), 
"fb_get_derived_classes", T(Array), S(0), "filename", T(String), NULL, S(0), "basecls", T(String), NULL, S(0), NULL, S(0), 
"hotprofiler_enable", T(Void), S(0), "level", T(Int32), NULL, S(0), NULL, S(0), 
"hotprofiler_disable", T(Variant), S(0), NULL, S(0), 
"phprof_enable", T(Void), S(0), "flags", T(Int32), "0", S(0), NULL, S(0), 
"phprof_disable", T(Variant), S(0), NULL, S(0), 
"fql_set_static_data_10", T(Void), S(0), "tables", T(Array), NULL, S(0), "functions", T(Array), NULL, S(0), NULL, S(0), 
"fql_static_data_set_10", T(Boolean), S(0), NULL, S(0), 
"fql_parse_10", T(Array), S(0), "query", T(String), NULL, S(0), NULL, S(0), 
"fql_multiparse_10", T(Array), S(0), "query", T(Array), NULL, S(0), NULL, S(0), 
"xhp_preprocess_code", T(Array), S(0), "code", T(String), NULL, S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
