#pragma once

#include "hphp/runtime/ext/extension.h"

namespace HPHP {

bool HHVM_FUNCTION(facts_enabled);
Variant HHVM_FUNCTION(facts_db_path, const String& root);
Variant HHVM_FUNCTION(facts_type_to_path, const String& type_name);
Variant HHVM_FUNCTION(facts_function_to_path, const String& function_name);
Variant HHVM_FUNCTION(facts_constant_to_path, const String& constant_name);
Variant HHVM_FUNCTION(facts_type_alias_to_path, const String& type_alias_name);
Array HHVM_FUNCTION(facts_path_to_types, const String& path);
Array HHVM_FUNCTION(facts_path_to_functions, const String& path);
Array HHVM_FUNCTION(facts_path_to_constants, const String& path);
Array HHVM_FUNCTION(facts_path_to_type_aliases, const String& path);
Variant HHVM_FUNCTION(facts_type_name, const String& type);
Variant HHVM_FUNCTION(facts_kind, const String& type);
bool HHVM_FUNCTION(facts_is_abstract, const String& type);
bool HHVM_FUNCTION(facts_is_final, const String& type);
Array HHVM_FUNCTION(
    facts_subtypes, const String& baseType, const Variant& filters);
Array HHVM_FUNCTION(
    facts_transitive_subtypes, const String& baseType, const Variant& filters);
Array HHVM_FUNCTION(
    facts_supertypes, const String& derivedType, const Variant& filters);
Array HHVM_FUNCTION(facts_types_with_attribute, const String& attr);
Array HHVM_FUNCTION(facts_type_aliases_with_attribute, const String& attr);
Array HHVM_FUNCTION(facts_methods_with_attribute, const String& attr);
Array HHVM_FUNCTION(facts_type_attributes, const String& type);
Array HHVM_FUNCTION(facts_type_alias_attributes, const String& type);
Array HHVM_FUNCTION(
    facts_method_attributes, const String& type, const String& method);
Array HHVM_FUNCTION(
    facts_type_attribute_parameters, const String& type, const String& attr);
Array HHVM_FUNCTION(
    facts_type_alias_attribute_parameters,
    const String& typeAlias,
    const String& attr);
Array HHVM_FUNCTION(
    facts_method_attribute_parameters,
    const String& type,
    const String& method,
    const String& attr);

Array HHVM_FUNCTION(facts_all_types);
Array HHVM_FUNCTION(facts_all_functions);
Array HHVM_FUNCTION(facts_all_constants);
Array HHVM_FUNCTION(facts_all_type_aliases);

} // namespace HPHP
