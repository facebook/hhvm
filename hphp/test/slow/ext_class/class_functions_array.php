<?hh


<<__EntryPoint>>
function main_class_functions_array() :mixed{
var_dump(method_exists(darray["foo" => 42], "foo"));
var_dump(get_class_methods(varray["bar"]));
var_dump(is_a(varray["bar"], "foo"));
var_dump(is_subclass_of(varray["bar"], "foo"));
}
