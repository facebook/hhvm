<?hh

type MyType = int;

<<__EntryPoint>>
function main_type_alias_exists() :mixed{
var_dump(type_alias_exists('I_do_not_exist'));
var_dump(type_alias_exists('MyType'));
}
