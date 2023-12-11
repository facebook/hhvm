<?hh


<<__EntryPoint>>
function main_1761() :mixed{
var_dump(strtr("", "ll", "a"));
var_dump(strtr("hello", "", "a"));
var_dump(strtr("hello", "ll", "a"));
var_dump(strtr("hello", dict["" => "a"]));
var_dump(strtr("hello", dict["ll" => "a"]));
var_dump(strtr("012", vec["foo", "bar", "baz"]));
}
