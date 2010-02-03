#if EXT_TYPE == 0
#elif EXT_TYPE == 1
"SQLITE3_ASSOC", T(Int64),
"SQLITE3_NUM", T(Int64),
"SQLITE3_BOTH", T(Int64),
"SQLITE3_INTEGER", T(Int64),
"SQLITE3_FLOAT", T(Int64),
"SQLITE3_TEXT", T(Int64),
"SQLITE3_BLOB", T(Int64),
"SQLITE3_NULL", T(Int64),
"SQLITE3_OPEN_READONLY", T(Int64),
"SQLITE3_OPEN_READWRITE", T(Int64),
"SQLITE3_OPEN_CREATE", T(Int64),
#elif EXT_TYPE == 2
"sqlite3", "", NULL, "__construct", T(Void), S(0), NULL, S(0), S(0), S(0), S(0),"open", T(Void), S(0), "filename", T(String), NULL, S(0), "flags", T(Int64), "k_SQLITE3_OPEN_READWRITE|k_SQLITE3_OPEN_CREATE", S(0), "encryption_key", T(String), "null_string", S(0), NULL, S(0), S(0), S(0), S(0),"close", T(Boolean), S(0), NULL, S(0), S(0), S(0), S(0),"exec", T(Boolean), S(0), "sql", T(String), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"version", T(Array), S(0), NULL, S(0), S(0), S(0), S(0),"lastinsertrowid", T(Int64), S(0), NULL, S(0), S(0), S(0), S(0),"lasterrorcode", T(Int64), S(0), NULL, S(0), S(0), S(0), S(0),"lasterrormsg", T(String), S(0), NULL, S(0), S(0), S(0), S(0),"loadextension", T(Boolean), S(0), "extension", T(String), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"changes", T(Int64), S(0), NULL, S(0), S(0), S(0), S(0),"escapestring", T(String), S(0), "sql", T(String), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"prepare", T(Variant), S(0), "sql", T(String), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"query", T(Variant), S(0), "sql", T(String), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"querysingle", T(Variant), S(0), "sql", T(String), NULL, S(0), "entire_row", T(Boolean), "false", S(0), NULL, S(0), S(0), S(0), S(0),"createfunction", T(Boolean), S(0), "name", T(String), NULL, S(0), "callback", T(Variant), NULL, S(0), "argcount", T(Int64), "-1", S(0), NULL, S(0), S(0), S(0), S(0),"createaggregate", T(Boolean), S(0), "name", T(String), NULL, S(0), "step", T(Variant), NULL, S(0), "final", T(Variant), NULL, S(0), "argcount", T(Int64), "-1", S(0), NULL, S(0), S(0), S(0), S(0),"openblob", T(Boolean), S(0), "table", T(String), NULL, S(0), "column", T(String), NULL, S(0), "rowid", T(Int64), NULL, S(0), "dbname", T(String), "null_string", S(0), NULL, S(0), S(0), S(0), S(0),"__destruct", T(Variant), S(0), NULL, S(0), S(0), S(0), S(0),NULL,NULL,
"sqlite3stmt", "", NULL, "__construct", T(Void), S(0), "dbobject", T(Object), NULL, S(0), "statement", T(String), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"paramcount", T(Int64), S(0), NULL, S(0), S(0), S(0), S(0),"close", T(Boolean), S(0), NULL, S(0), S(0), S(0), S(0),"reset", T(Boolean), S(0), NULL, S(0), S(0), S(0), S(0),"clear", T(Boolean), S(0), NULL, S(0), S(0), S(0), S(0),"bindparam", T(Boolean), S(0), "name", T(Variant), NULL, S(0), "parameter", T(Variant), NULL, S(1), "type", T(Int64), "k_SQLITE3_TEXT", S(0), NULL, S(0), S(0), S(0), S(0),"bindvalue", T(Boolean), S(0), "name", T(Variant), NULL, S(0), "parameter", T(Variant), NULL, S(0), "type", T(Int64), "k_SQLITE3_TEXT", S(0), NULL, S(0), S(0), S(0), S(0),"execute", T(Variant), S(0), NULL, S(0), S(0), S(0), S(0),"__destruct", T(Variant), S(0), NULL, S(0), S(0), S(0), S(0),NULL,NULL,
"sqlite3result", "", NULL, "__construct", T(Void), S(0), NULL, S(0), S(0), S(0), S(0),"numcolumns", T(Int64), S(0), NULL, S(0), S(0), S(0), S(0),"columnname", T(String), S(0), "column", T(Int64), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"columntype", T(Int64), S(0), "column", T(Int64), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"fetcharray", T(Variant), S(0), "mode", T(Int64), "k_SQLITE3_BOTH", S(0), NULL, S(0), S(0), S(0), S(0),"reset", T(Boolean), S(0), NULL, S(0), S(0), S(0), S(0),"finalize", T(Boolean), S(0), NULL, S(0), S(0), S(0), S(0),"__destruct", T(Variant), S(0), NULL, S(0), S(0), S(0), S(0),NULL,NULL,
#elif EXT_TYPE == 3

#endif
