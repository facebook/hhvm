<?hh


<<__EntryPoint>>
function main_509() :mixed{
$a = vec[1, 2, 3];
var_dump($a);
array_pop(inout $a);
var_dump($a);
array_shift(inout $a);
var_dump($a);
}
