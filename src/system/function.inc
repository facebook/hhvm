#if EXT_TYPE == 0
"get_defined_functions", T(Array), S(0), NULL, S(0), 
"function_exists", T(Boolean), S(0), "function_name", T(String), NULL, S(0), NULL, S(0), 
"is_callable", T(Boolean), S(0), "v", T(Some), NULL, S(0), "syntax", T(Boolean), "false", S(0), "name", T(Variant), "null", S(1), NULL, S(0), 
"call_user_func_array", T(Variant), S(0), "function", T(Variant), NULL, S(0), "params", T(Array), NULL, S(0), NULL, S(8), 
"call_user_func", T(Variant), S(0), "function", T(Variant), NULL, S(0), NULL, S(9), 
"create_function", T(String), S(0), "args", T(String), NULL, S(0), "code", T(String), NULL, S(0), NULL, S(0), 
"func_get_arg", T(Variant), S(0), "arg_num", T(Int32), NULL, S(0), NULL, S(0), 
"func_get_args", T(Array), S(0), NULL, S(0), 
"func_num_args", T(Int32), S(0), NULL, S(0), 
"register_postsend_function", T(Void), S(0), "function", T(Variant), NULL, S(0), NULL, S(1), 
"register_shutdown_function", T(Void), S(0), "function", T(Variant), NULL, S(0), NULL, S(1), 
"register_cleanup_function", T(Void), S(0), "function", T(Variant), NULL, S(0), NULL, S(1), 
"register_tick_function", T(Boolean), S(0), "function", T(Variant), NULL, S(0), NULL, S(1), 
"unregister_tick_function", T(Void), S(0), "function_name", T(Variant), NULL, S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
