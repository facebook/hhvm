<?hh

<<__EntryPoint>>
function main_fill_keys() :mixed{
$keys = varray["foo", 5, 10, "bar"];
var_dump(array_fill_keys($keys, "banana"));
}
