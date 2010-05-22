#if EXT_TYPE == 0
"apc_add", T(Boolean), S(0), "key", T(String), NULL, S(0), "var", T(Variant), NULL, S(0), "ttl", T(Int64), "0", S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_store", T(Boolean), S(0), "key", T(String), NULL, S(0), "var", T(Variant), NULL, S(0), "ttl", T(Int64), "0", S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_fetch", T(Variant), S(0), "key", T(Variant), NULL, S(0), "success", T(Variant), "null", S(1), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_delete", T(Variant), S(0), "key", T(Variant), NULL, S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_compile_file", T(Boolean), S(0), "filename", T(String), NULL, S(0), "atomic", T(Boolean), "true", S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_cache_info", T(Variant), S(0), "cache_id", T(Int64), "0", S(0), "limited", T(Boolean), "false", S(0), NULL, S(0), 
"apc_clear_cache", T(Boolean), S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_define_constants", T(Boolean), S(0), "key", T(String), NULL, S(0), "constants", T(String), NULL, S(0), "case_sensitive", T(Boolean), "true", S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_load_constants", T(Boolean), S(0), "key", T(String), NULL, S(0), "case_sensitive", T(Boolean), "true", S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_sma_info", T(Array), S(0), "limited", T(Boolean), "false", S(0), NULL, S(0), 
"apc_filehits", T(Array), S(0), NULL, S(0), 
"apc_delete_file", T(Variant), S(0), "keys", T(Variant), NULL, S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_inc", T(Variant), S(0), "key", T(String), NULL, S(0), "step", T(Int64), "1", S(0), "success", T(Variant), "null", S(1), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_dec", T(Variant), S(0), "key", T(String), NULL, S(0), "step", T(Int64), "1", S(0), "success", T(Variant), "null", S(1), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_cas", T(Boolean), S(0), "key", T(String), NULL, S(0), "old_cas", T(Int64), NULL, S(0), "new_cas", T(Int64), NULL, S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_bin_dump", T(Variant), S(0), "cache_id", T(Int64), "0", S(0), "filter", T(Variant), "null_variant", S(0), NULL, S(0), 
"apc_bin_load", T(Boolean), S(0), "data", T(String), NULL, S(0), "flags", T(Int64), "0", S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
"apc_bin_dumpfile", T(Variant), S(0), "cache_id", T(Int64), NULL, S(0), "filter", T(Variant), NULL, S(0), "filename", T(String), NULL, S(0), "flags", T(Int64), "0", S(0), "context", T(Object), "null", S(0), NULL, S(0), 
"apc_bin_loadfile", T(Boolean), S(0), "filename", T(String), NULL, S(0), "context", T(Object), "null", S(0), "flags", T(Int64), "0", S(0), "cache_id", T(Int64), "0", S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
