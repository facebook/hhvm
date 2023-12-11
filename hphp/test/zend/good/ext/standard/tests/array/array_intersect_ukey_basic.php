<?hh
/*
* proto array array_intersect_ukey ( array $array1, array $array2 [, array $ ..., callback $key_compare_func] )
* Function is implemented in ext/standard/array.c
*/
function key_compare_func($key1, $key2) :mixed{
    if ($key1 == $key2) return 0;
    else if ($key1 > $key2) return 1;
    else return -1;
}
<<__EntryPoint>> function main(): void {
$array1 = dict['blue' => 1, 'red' => 2, 'green' => 3, 'purple' => 4];
$array2 = dict['green' => 5, 'blue' => 6, 'yellow' => 7, 'cyan' => 8];
var_dump(array_intersect_ukey($array1, $array2, key_compare_func<>));
}
