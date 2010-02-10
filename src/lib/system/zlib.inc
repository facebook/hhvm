#if EXT_TYPE == 0
"readgzfile", T(Variant), S(0), "filename", T(String), NULL, S(0), "use_include_path", T(Boolean), "false", S(0), NULL, S(0), 
"gzfile", T(Variant), S(0), "filename", T(String), NULL, S(0), "use_include_path", T(Boolean), "false", S(0), NULL, S(0), 
"gzcompress", T(Variant), S(0), "data", T(String), NULL, S(0), "level", T(Int32), "-1", S(0), NULL, S(0), 
"gzuncompress", T(Variant), S(0), "data", T(String), NULL, S(0), "limit", T(Int32), "0", S(0), NULL, S(0), 
"gzdeflate", T(Variant), S(0), "data", T(String), NULL, S(0), "level", T(Int32), "-1", S(0), NULL, S(0), 
"gzinflate", T(Variant), S(0), "data", T(String), NULL, S(0), "limit", T(Int32), "0", S(0), NULL, S(0), 
"gzencode", T(Variant), S(0), "data", T(String), NULL, S(0), "level", T(Int32), "-1", S(0), "encoding_mode", T(Int32), "k_FORCE_GZIP", S(0), NULL, S(0), 
"gzdecode", T(Variant), S(0), "data", T(String), NULL, S(0), NULL, S(0), 
"zlib_get_coding_type", T(String), S(0), NULL, S(0), 
"gzopen", T(Object), S(0), "filename", T(String), NULL, S(0), "mode", T(String), NULL, S(0), "use_include_path", T(Boolean), "false", S(0), NULL, S(0), 
"gzclose", T(Boolean), S(0), "zp", T(Object), NULL, S(0), NULL, S(0), 
"gzrewind", T(Boolean), S(0), "zp", T(Object), NULL, S(0), NULL, S(0), 
"gzeof", T(Boolean), S(0), "zp", T(Object), NULL, S(0), NULL, S(0), 
"gzgetc", T(Variant), S(0), "zp", T(Object), NULL, S(0), NULL, S(0), 
"gzgets", T(Variant), S(0), "zp", T(Object), NULL, S(0), "length", T(Int64), "1024", S(0), NULL, S(0), 
"gzgetss", T(Variant), S(0), "zp", T(Object), NULL, S(0), "length", T(Int64), "0", S(0), "allowable_tags", T(String), "null_string", S(0), NULL, S(0), 
"gzread", T(Variant), S(0), "zp", T(Object), NULL, S(0), "length", T(Int64), "0", S(0), NULL, S(0), 
"gzpassthru", T(Variant), S(0), "zp", T(Object), NULL, S(0), NULL, S(0), 
"gzseek", T(Variant), S(0), "zp", T(Object), NULL, S(0), "offset", T(Int64), NULL, S(0), "whence", T(Int64), "SEEK_SET", S(0), NULL, S(0), 
"gztell", T(Variant), S(0), "zp", T(Object), NULL, S(0), NULL, S(0), 
"gzwrite", T(Variant), S(0), "zp", T(Object), NULL, S(0), "str", T(String), NULL, S(0), "length", T(Int64), "0", S(0), NULL, S(0), 
"gzputs", T(Variant), S(0), "zp", T(Object), NULL, S(0), "str", T(String), NULL, S(0), "length", T(Int64), "0", S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
