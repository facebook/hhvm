<?hh
/* Prototype  : array array_intersect_ukey(array arr1, array arr2 [, array ...], callback key_compare_func)
 * Description: Computes the intersection of arrays using a callback function on the keys for comparison.
 * Source code: ext/standard/array.c
 */

//Call back function
function key_compare_func($key1, $key2)
:mixed{
    if (HH\Lib\Legacy_FIXME\eq($key1, $key2))
        return 0;
    else
        return ($key1 > $key2)? 1:-1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_ukey() : usage variation ***\n";

//Initialize variables
$arr1_string_int = dict['0' => '1', '1' => '2'];
$arr2_string_int = dict['0' => '1', '1' => '3'];
$arr1_string_float = dict['0.00' => '1.00', '1.00' => '2.00'];
$arr2_string_float = dict['0.00' => '1.00', '1.00' => '3.00'];

echo "\n-- Result of strings containing integers intersection --\n";
var_dump( array_intersect_ukey($arr1_string_int, $arr2_string_int, key_compare_func<>) );

echo "\n-- Result of strings containing floating points intersection --\n";
var_dump( array_intersect_ukey($arr1_string_float, $arr2_string_float, key_compare_func<>) );

echo "\n-- Result of strings containing integers and strings containing floating points intersection --\n";
var_dump( array_intersect_ukey($arr1_string_int, $arr2_string_float, key_compare_func<>) );
echo "===DONE===\n";
}
