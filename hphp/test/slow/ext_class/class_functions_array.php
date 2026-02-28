<?hh


<<__EntryPoint>>
function main_class_functions_array() :mixed{
var_dump(method_exists(dict["foo" => 42], "foo"));
var_dump(get_class_methods(vec["bar"]));
var_dump(is_a(vec["bar"], "foo"));
var_dump(is_subclass_of(vec["bar"], "foo"));
}
