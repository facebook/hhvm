<?hh


<<__EntryPoint>>
function main_negative_offsets_001() :mixed{
$a = dict[ -1 => 0];
array_pop(inout $a);
$a[] = 1;
$a[] = 2;
var_dump($a);
}
