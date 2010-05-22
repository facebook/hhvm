#if EXT_TYPE == 0
"hphp_get_extension_info", T(Array), S(0), "name", T(String), NULL, S(0), NULL, S(0), 
"hphp_get_class_info", T(Array), S(0), "name", T(Variant), NULL, S(0), NULL, S(0), 
"hphp_get_function_info", T(Array), S(0), "name", T(String), NULL, S(0), NULL, S(0), 
"hphp_invoke", T(Variant), S(0), "name", T(String), NULL, S(0), "params", T(Array), NULL, S(0), NULL, S(0), 
"hphp_invoke_method", T(Variant), S(0), "obj", T(Variant), NULL, S(0), "cls", T(String), NULL, S(0), "name", T(String), NULL, S(0), "params", T(Array), NULL, S(0), NULL, S(0), 
"hphp_instanceof", T(Boolean), S(0), "obj", T(Object), NULL, S(0), "name", T(String), NULL, S(0), NULL, S(0), 
"hphp_create_object", T(Object), S(0), "name", T(String), NULL, S(0), "params", T(Array), NULL, S(0), NULL, S(0), 
"hphp_get_property", T(Variant), S(0), "obj", T(Object), NULL, S(0), "cls", T(String), NULL, S(0), "prop", T(String), NULL, S(0), NULL, S(0), 
"hphp_set_property", T(Void), S(0), "obj", T(Object), NULL, S(0), "cls", T(String), NULL, S(0), "prop", T(String), NULL, S(0), "value", T(Variant), NULL, S(0), NULL, S(0), 
"hphp_get_static_property", T(Variant), S(0), "cls", T(String), NULL, S(0), "prop", T(String), NULL, S(0), NULL, S(0), 
"hphp_set_static_property", T(Void), S(0), "cls", T(String), NULL, S(0), "prop", T(String), NULL, S(0), "value", T(Variant), NULL, S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
