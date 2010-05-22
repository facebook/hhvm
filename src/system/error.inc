#if EXT_TYPE == 0
"debug_backtrace", T(Array), S(0), NULL, S(0), 
"debug_print_backtrace", T(Void), S(0), NULL, S(0), 
"error_get_last", T(Array), S(0), NULL, S(0), 
"error_log", T(Boolean), S(0), "message", T(String), NULL, S(0), "message_type", T(Int32), "0", S(0), "destination", T(String), "null_string", S(0), "extra_headers", T(String), "null_string", S(0), NULL, S(0), 
"error_reporting", T(Int32), S(0), "level", T(Variant), "null", S(0), NULL, S(0), 
"restore_error_handler", T(Boolean), S(0), NULL, S(0), 
"restore_exception_handler", T(Boolean), S(0), NULL, S(0), 
"set_error_handler", T(Variant), S(0), "error_handler", T(Variant), NULL, S(0), "error_types", T(Int32), "0", S(0), NULL, S(0), 
"set_exception_handler", T(String), S(0), "exception_handler", T(String), NULL, S(0), NULL, S(0), 
"hphp_set_error_page", T(Void), S(0), "page", T(String), NULL, S(0), NULL, S(0), 
"trigger_error", T(Boolean), S(0), "error_msg", T(String), NULL, S(0), "error_type", T(Int32), "k_E_USER_NOTICE", S(0), NULL, S(0), 
"user_error", T(Boolean), S(0), "error_msg", T(String), NULL, S(0), "error_type", T(Int32), "k_E_USER_NOTICE", S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
