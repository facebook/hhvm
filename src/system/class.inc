#if EXT_TYPE == 0
"get_declared_classes", T(Array), S(0), NULL, S(0),
"get_declared_interfaces", T(Array), S(0), NULL, S(0),
"class_exists", T(Boolean), S(0), "class_name", T(String), NULL, S(0), "autoload", T(Boolean), "false", S(0), NULL, S(0),
"interface_exists", T(Boolean), S(0), "interface_name", T(String), NULL, S(0), "autoload", T(Boolean), "false", S(0), NULL, S(0),
"get_class_methods", T(Array), S(0), "class_or_object", T(Variant), NULL, S(0), NULL, S(0),
"get_class_vars", T(Array), S(0), "class_name", T(String), NULL, S(0), NULL, S(0),
"get_class", T(Variant), S(0), "object", T(Variant), "null_variant", S(0), NULL, S(0),
"get_parent_class", T(Variant), S(0), "object", T(Variant), "null_variant", S(0), NULL, S(0),
"is_a", T(Boolean), S(0), "object", T(Object), NULL, S(0), "class_name", T(String), NULL, S(0), NULL, S(0),
"is_subclass_of", T(Boolean), S(0), "class_or_object", T(Variant), NULL, S(0), "class_name", T(String), NULL, S(0), NULL, S(0),
"method_exists", T(Boolean), S(0), "class_or_object", T(Variant), NULL, S(0), "method_name", T(String), NULL, S(0), NULL, S(0),
"property_exists", T(Boolean), S(0), "class_or_object", T(Variant), NULL, S(0), "property", T(String), NULL, S(0), NULL, S(0),
"get_object_vars", T(Array), S(0), "object", T(Object), NULL, S(0), NULL, S(0),
"call_user_method_array", T(Variant), S(0), "method_name", T(String), NULL, S(0), "obj", T(Variant), NULL, S(1), "paramarr", T(Array), NULL, S(0), NULL, S(0),
"call_user_method", T(Variant), S(0), "method_name", T(String), NULL, S(0), "obj", T(Variant), NULL, S(1), NULL, S(1),
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
