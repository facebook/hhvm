<?hh

<<__EntryPoint>>
function main_parentheses_deref() :mixed{
$array = varray[varray[varray[varray[0, 1]]], 1];
var_dump(($array)[1]);
var_dump((($array[0][0])[0])[1]);
$obj = new stdClass(); $obj->a = 0; $obj->b = varray[var_dump<>, 1];
(clone $obj)->b[0](1);
}
