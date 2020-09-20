<?hh
/* Prototype  : array array_intersect_uassoc(array arr1, array arr2 [, array ...], callback key_compare_func)
 * Description: Computes the intersection of arrays with additional index check, compares indexes by a callback function
 * Source code: ext/standard/array.c
 */

function key_compare_func($a, $b)
{
    if ($a === $b) {
        return 0;
    }
    return ($a > $b)? 1:-1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_uassoc() : usage variation ***\n";

//Initialize variables
$arr1_string_int = varray['1', '2'];
$arr2_string_int = varray['1', '3'];
$arr1_string_float = varray['1.00', '2.00'];
$arr2_string_float = varray['1.00', '3.00'];

echo "\n-- Result of strings containing integers intersection --\n";
var_dump( array_intersect_uassoc($arr1_string_int, $arr2_string_int, fun("key_compare_func")) );

echo "\n-- Result of strings containing floating points intersection --\n";
var_dump( array_intersect_uassoc($arr1_string_float, $arr2_string_float, fun("key_compare_func")) );

echo "\n-- Result of strings containing integers and strings containing floating points intersection --\n";
var_dump( array_intersect_uassoc($arr1_string_int, $arr2_string_float, fun("key_compare_func")) );
echo "===DONE===\n";
}
