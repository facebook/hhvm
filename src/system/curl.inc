#if EXT_TYPE == 0
"curl_init", T(Variant), S(0), "url", T(String), "null_string", S(0), NULL, S(0), 
"curl_copy_handle", T(Variant), S(0), "ch", T(Object), NULL, S(0), NULL, S(0), 
"curl_version", T(Variant), S(0), "uversion", T(Int32), "CURLVERSION_NOW", S(0), NULL, S(0), 
"curl_setopt", T(Boolean), S(0), "ch", T(Object), NULL, S(0), "option", T(Int32), NULL, S(0), "value", T(Variant), NULL, S(0), NULL, S(0), 
"curl_setopt_array", T(Boolean), S(0), "ch", T(Object), NULL, S(0), "options", T(Array), NULL, S(0), NULL, S(0), 
"curl_exec", T(Variant), S(0), "ch", T(Object), NULL, S(0), NULL, S(0), 
"curl_getinfo", T(Variant), S(0), "ch", T(Object), NULL, S(0), "opt", T(Int32), "0", S(0), NULL, S(0), 
"curl_errno", T(Variant), S(0), "ch", T(Object), NULL, S(0), NULL, S(0), 
"curl_error", T(Variant), S(0), "ch", T(Object), NULL, S(0), NULL, S(0), 
"curl_close", T(Variant), S(0), "ch", T(Object), NULL, S(0), NULL, S(0), 
"curl_multi_init", T(Object), S(0), NULL, S(0), 
"curl_multi_add_handle", T(Variant), S(0), "mh", T(Object), NULL, S(0), "ch", T(Object), NULL, S(0), NULL, S(0), 
"curl_multi_remove_handle", T(Variant), S(0), "mh", T(Object), NULL, S(0), "ch", T(Object), NULL, S(0), NULL, S(0), 
"curl_multi_exec", T(Variant), S(0), "mh", T(Object), NULL, S(0), "still_running", T(Variant), NULL, S(1), NULL, S(0), 
"curl_multi_select", T(Variant), S(0), "mh", T(Object), NULL, S(0), "timeout", T(Double), "1.0", S(0), NULL, S(0), 
"curl_multi_getcontent", T(Variant), S(0), "ch", T(Object), NULL, S(0), NULL, S(0), 
"curl_multi_info_read", T(Variant), S(0), "mh", T(Object), NULL, S(0), "msgs_in_queue", T(Variant), "null", S(1), NULL, S(0), 
"curl_multi_close", T(Variant), S(0), "mh", T(Object), NULL, S(0), NULL, S(0), 
"evhttp_set_cache", T(Void), S(0), "address", T(String), NULL, S(0), "max_conn", T(Int32), NULL, S(0), "port", T(Int32), "80", S(0), NULL, S(0), 
"evhttp_get", T(Variant), S(0), "url", T(String), NULL, S(0), "headers", T(Array), "null_array", S(0), "timeout", T(Int32), "5", S(0), NULL, S(0), 
"evhttp_post", T(Variant), S(0), "url", T(String), NULL, S(0), "data", T(String), NULL, S(0), "headers", T(Array), "null_array", S(0), "timeout", T(Int32), "5", S(0), NULL, S(0), 
"evhttp_async_get", T(Variant), S(0), "url", T(String), NULL, S(0), "headers", T(Array), "null_array", S(0), "timeout", T(Int32), "5", S(0), NULL, S(0), 
"evhttp_async_post", T(Variant), S(0), "url", T(String), NULL, S(0), "data", T(String), NULL, S(0), "headers", T(Array), "null_array", S(0), "timeout", T(Int32), "5", S(0), NULL, S(0), 
"evhttp_recv", T(Variant), S(0), "handle", T(Object), NULL, S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
