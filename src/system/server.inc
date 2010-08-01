#if EXT_TYPE == 0
"dangling_server_proxy_old_request", T(Boolean), S(0), NULL, S(64), 
"dangling_server_proxy_new_request", T(Boolean), S(0), "host", T(String), NULL, S(0), NULL, S(64), 
"pagelet_server_is_enabled", T(Boolean), S(0), NULL, S(64), 
"pagelet_server_task_start", T(Object), S(0), "url", T(String), NULL, S(0), "headers", T(Array), "null_array", S(0), "post_data", T(String), "null_string", S(0), NULL, S(64), 
"pagelet_server_task_status", T(Boolean), S(0), "task", T(Object), NULL, S(0), NULL, S(64), 
"pagelet_server_task_result", T(String), S(0), "task", T(Object), NULL, S(0), "headers", T(Variant), NULL, S(1), "code", T(Variant), NULL, S(1), NULL, S(64), 
"xbox_send_message", T(Boolean), S(0), "msg", T(String), NULL, S(0), "ret", T(Variant), NULL, S(1), "timeout_ms", T(Int64), NULL, S(0), "host", T(String), "\"localhost\"", S(0), NULL, S(64), 
"xbox_post_message", T(Boolean), S(0), "msg", T(String), NULL, S(0), "host", T(String), "\"localhost\"", S(0), NULL, S(64), 
"xbox_task_start", T(Object), S(0), "message", T(String), NULL, S(0), NULL, S(64), 
"xbox_task_status", T(Boolean), S(0), "task", T(Object), NULL, S(0), NULL, S(64), 
"xbox_task_result", T(Int64), S(0), "task", T(Object), NULL, S(0), "timeout_ms", T(Int64), NULL, S(0), "ret", T(Variant), NULL, S(1), NULL, S(64), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
