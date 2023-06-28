<?hh


<<__EntryPoint>>
function main_1761() :mixed{
var_dump(strtr("", "ll", "a"));
var_dump(strtr("hello", "", "a"));
var_dump(strtr("hello", "ll", "a"));
var_dump(strtr("hello", darray["" => "a"]));
var_dump(strtr("hello", darray["ll" => "a"]));
var_dump(strtr("012", varray["foo", "bar", "baz"]));
}
