<?hh
/*
* array array_intersect_uassoc ( array $array1, array $array2 [, array $..., callback $key_compare_func] )
* Function is implemented in ext/standard/array.c
*/
function key_compare_func($a, $b) {
    if ($a === $b) {
        return 0;
    }
    return ($a > $b) ? 1 : -1;
}
<<__EntryPoint>> function main(): void {
$array1 = darray["a" => "green", "b" => "brown", "c" => "blue", 0 => "red"];
$array2 = darray["a" => "green", 0 => "yellow", 1 => "red"];
$result = array_intersect_uassoc($array1, $array2, fun("key_compare_func"));
var_dump($result);
}
