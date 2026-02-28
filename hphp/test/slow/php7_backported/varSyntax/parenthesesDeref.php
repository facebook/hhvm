<?hh

<<__EntryPoint>>
function main_parentheses_deref() :mixed{
$array = vec[vec[vec[vec[0, 1]]], 1];
var_dump(($array)[1]);
var_dump((($array[0][0])[0])[1]);
$obj = new stdClass(); $obj->a = 0; $obj->b = vec[var_dump<>, 1];
(clone $obj)->b[0](1);
}
