<?hh
/*
* proto array array_diff_key(array arr1, array arr2 [, array ...])
* Function is implemented in ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
$array1 = darray['blue' => 1, 'red' => 2, 'green' => 3, 'purple' => 4];
$array2 = darray['green' => 5, 'blue' => 6, 'yellow' => 7, 'cyan' => 8];
var_dump(array_diff_key($array1, $array2));
}
