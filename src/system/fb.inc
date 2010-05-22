#if EXT_TYPE == 0
"fb_thrift_serialize", T(Variant), S(0), "thing", T(Variant), NULL, S(0), NULL, S(0), 
"fb_thrift_unserialize", T(Variant), S(0), "thing", T(Variant), NULL, S(0), "success", T(Variant), NULL, S(1), "errcode", T(Variant), "null_variant", S(1), NULL, S(0), 
"fb_serialize", T(Variant), S(0), "thing", T(Variant), NULL, S(0), NULL, S(0), 
"fb_unserialize", T(Variant), S(0), "thing", T(Variant), NULL, S(0), "success", T(Variant), NULL, S(1), "errcode", T(Variant), "null_variant", S(1), NULL, S(0), 
"fb_rename_function", T(Boolean), S(0), "orig_func_name", T(String), NULL, S(0), "new_func_name", T(String), NULL, S(0), NULL, S(0), 
"fb_utf8ize", T(Boolean), S(0), "input", T(Variant), NULL, S(1), NULL, S(0), 
"fb_call_user_func_safe", T(Array), S(0), "function", T(Variant), NULL, S(0), NULL, S(1), 
"fb_call_user_func_safe_return", T(Variant), S(0), "function", T(Variant), NULL, S(0), "def", T(Variant), NULL, S(0), NULL, S(1), 
"fb_call_user_func_array_safe", T(Array), S(0), "function", T(Variant), NULL, S(0), "params", T(Array), NULL, S(0), NULL, S(0), 
"fb_get_code_coverage", T(Variant), S(0), NULL, S(0), 
"xhprof_enable", T(Void), S(0), "flags", T(Int32), NULL, S(0), "args", T(Array), "null_array", S(0), NULL, S(0), 
"xhprof_disable", T(Variant), S(0), NULL, S(0), 
"xhprof_sample_enable", T(Void), S(0), NULL, S(0), 
"xhprof_sample_disable", T(Variant), S(0), NULL, S(0), 
"fb_load_local_databases", T(Void), S(0), "servers", T(Array), NULL, S(0), NULL, S(0), 
"fb_parallel_query", T(Array), S(0), "sql_map", T(Array), NULL, S(0), "max_thread", T(Int32), "50", S(0), "combine_result", T(Boolean), "true", S(0), "retry_query_on_fail", T(Boolean), "true", S(0), "connect_timeout", T(Int32), "-1", S(0), "read_timeout", T(Int32), "-1", S(0), "timeout_in_ms", T(Boolean), "false", S(0), NULL, S(0), 
"fb_crossall_query", T(Array), S(0), "sql", T(String), NULL, S(0), "max_thread", T(Int32), "50", S(0), "retry_query_on_fail", T(Boolean), "true", S(0), "connect_timeout", T(Int32), "-1", S(0), "read_timeout", T(Int32), "-1", S(0), "timeout_in_ms", T(Boolean), "false", S(0), NULL, S(0), 
#elif EXT_TYPE == 1
"FB_UNSERIALIZE_NONSTRING_VALUE", T(Int64),
"FB_UNSERIALIZE_UNEXPECTED_END", T(Int64),
"FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE", T(Int64),
"FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE", T(Int64),
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
