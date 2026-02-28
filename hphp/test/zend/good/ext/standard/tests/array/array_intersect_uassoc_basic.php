<?hh
/*
* array array_intersect_uassoc ( array $array1, array $array2 [, array $..., callback $key_compare_func] )
* Function is implemented in ext/standard/array.c
*/
function key_compare_func($a, $b) :mixed{
    if ($a === $b) {
        return 0;
    }
    return (HH\Lib\Legacy_FIXME\gt($a, $b)) ? 1 : -1;
}
<<__EntryPoint>> function main(): void {
$array1 = dict["a" => "green", "b" => "brown", "c" => "blue", 0 => "red"];
$array2 = dict["a" => "green", 0 => "yellow", 1 => "red"];
$result = array_intersect_uassoc($array1, $array2, key_compare_func<>);
var_dump($result);
}
