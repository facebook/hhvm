#if EXT_TYPE == 0
"intl_get_error_code", T(Int64), S(0), NULL, S(0), 
"intl_get_error_message", T(String), S(0), NULL, S(0), 
"intl_error_name", T(String), S(0), "error_code", T(Int64), NULL, S(0), NULL, S(0), 
"intl_is_failure", T(Boolean), S(0), "error_code", T(Int64), NULL, S(0), NULL, S(0), 
"collator_asort", T(Variant), S(0), "obj", T(Variant), NULL, S(0), "arr", T(Variant), NULL, S(1), "sort_flag", T(Int64), "0", S(0), NULL, S(0), 
"collator_compare", T(Variant), S(0), "obj", T(Variant), NULL, S(0), "str1", T(String), NULL, S(0), "str2", T(String), NULL, S(0), NULL, S(0), 
"collator_create", T(Variant), S(0), "locale", T(String), NULL, S(0), NULL, S(0), 
"collator_get_attribute", T(Variant), S(0), "obj", T(Variant), NULL, S(0), "attr", T(Int64), NULL, S(0), NULL, S(0), 
"collator_get_error_code", T(Variant), S(0), "obj", T(Variant), NULL, S(0), NULL, S(0), 
"collator_get_error_message", T(Variant), S(0), "obj", T(Variant), NULL, S(0), NULL, S(0), 
"collator_get_locale", T(Variant), S(0), "obj", T(Variant), NULL, S(0), "type", T(Int64), "0", S(0), NULL, S(0), 
"collator_get_strength", T(Variant), S(0), "obj", T(Variant), NULL, S(0), NULL, S(0), 
"collator_set_attribute", T(Variant), S(0), "obj", T(Variant), NULL, S(0), "attr", T(Int64), NULL, S(0), "val", T(Int64), NULL, S(0), NULL, S(0), 
"collator_set_strength", T(Variant), S(0), "obj", T(Variant), NULL, S(0), "strength", T(Int64), NULL, S(0), NULL, S(0), 
"collator_sort_with_sort_keys", T(Variant), S(0), "obj", T(Variant), NULL, S(0), "arr", T(Variant), NULL, S(1), NULL, S(0), 
"collator_sort", T(Variant), S(0), "obj", T(Variant), NULL, S(0), "arr", T(Variant), NULL, S(1), "sort_flag", T(Int64), "0", S(0), NULL, S(0), 
"idn_to_ascii", T(Variant), S(0), "domain", T(String), NULL, S(0), "errorcode", T(Variant), "null", S(1), NULL, S(0), 
"idn_to_unicode", T(Variant), S(0), "domain", T(String), NULL, S(0), "errorcode", T(Variant), "null", S(1), NULL, S(0), 
"idn_to_utf8", T(Variant), S(0), "domain", T(String), NULL, S(0), "errorcode", T(Variant), "null", S(1), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
"collator", "", NULL, "__construct", T(Void), S(0), "locale", T(String), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"asort", T(Boolean), S(0), "arr", T(Variant), NULL, S(1), "sort_flag", T(Int64), "0", S(0), NULL, S(0), S(0), S(0), S(0),"compare", T(Variant), S(0), "str1", T(String), NULL, S(0), "str2", T(String), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"create", T(Variant), S(0), "locale", T(String), NULL, S(0), NULL, S(0), S(0), S(0), S(1),"getattribute", T(Int64), S(0), "attr", T(Int64), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"geterrorcode", T(Int64), S(0), NULL, S(0), S(0), S(0), S(0),"geterrormessage", T(String), S(0), NULL, S(0), S(0), S(0), S(0),"getlocale", T(String), S(0), "type", T(Int64), "0", S(0), NULL, S(0), S(0), S(0), S(0),"getstrength", T(Int64), S(0), NULL, S(0), S(0), S(0), S(0),"setattribute", T(Boolean), S(0), "attr", T(Int64), NULL, S(0), "val", T(Int64), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"setstrength", T(Boolean), S(0), "strength", T(Int64), NULL, S(0), NULL, S(0), S(0), S(0), S(0),"sortwithsortkeys", T(Boolean), S(0), "arr", T(Variant), NULL, S(1), NULL, S(0), S(0), S(0), S(0),"sort", T(Boolean), S(0), "arr", T(Variant), NULL, S(1), "sort_flag", T(Int64), "0", S(0), NULL, S(0), S(0), S(0), S(0),"__destruct", T(Variant), S(0), NULL, S(0), S(0), S(0), S(0),NULL,NULL,"SORT_REGULAR", T(Int64),"SORT_NUMERIC", T(Int64),"SORT_STRING", T(Int64),"FRENCH_COLLATION", T(Int64),"ALTERNATE_HANDLING", T(Int64),"CASE_FIRST", T(Int64),"CASE_LEVEL", T(Int64),"NORMALIZATION_MODE", T(Int64),"STRENGTH", T(Int64),"HIRAGANA_QUATERNARY_MODE", T(Int64),"NUMERIC_COLLATION", T(Int64),"DEFAULT_VALUE", T(Int64),"PRIMARY", T(Int64),"SECONDARY", T(Int64),"TERTIARY", T(Int64),"DEFAULT_STRENGTH", T(Int64),"QUATERNARY", T(Int64),"IDENTICAL", T(Int64),"OFF", T(Int64),"ON", T(Int64),"SHIFTED", T(Int64),"NON_IGNORABLE", T(Int64),"LOWER_FIRST", T(Int64),"UPPER_FIRST", T(Int64),NULL,
"locale", "", NULL, "__construct", T(Void), S(0), NULL, S(0), S(0), S(0), S(0),"__destruct", T(Variant), S(0), NULL, S(0), S(0), S(0), S(0),NULL,NULL,"ACTUAL_LOCALE", T(Int64),"VALID_LOCALE", T(Int64),NULL,
"normalizer", "", NULL, "__construct", T(Void), S(0), NULL, S(0), S(0), S(0), S(0),"isnormalized", T(Variant), S(0), "input", T(String), NULL, S(0), "form", T(Int64), "q_normalizer_FORM_C", S(0), NULL, S(0), S(0), S(0), S(1),"normalize", T(Variant), S(0), "input", T(String), NULL, S(0), "form", T(Int64), "q_normalizer_FORM_C", S(0), NULL, S(0), S(0), S(0), S(1),"__destruct", T(Variant), S(0), NULL, S(0), S(0), S(0), S(0),NULL,NULL,"NONE", T(Int64),"FORM_D", T(Int64),"NFD", T(Int64),"FORM_KD", T(Int64),"NFKD", T(Int64),"FORM_C", T(Int64),"NFC", T(Int64),"FORM_KC", T(Int64),"NFKC", T(Int64),NULL,
#elif EXT_TYPE == 3

#endif
