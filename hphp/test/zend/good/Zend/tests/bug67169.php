<?hh
<<__EntryPoint>> function main(): void {
$array = varray['a', 'b'];
array_splice(inout $array, 0, 2);
$array[] = 'c';
var_dump($array);

$array = varray['a', 'b'];
array_shift(inout $array);
array_shift(inout $array);
$array[] = 'c';
var_dump($array);
}
