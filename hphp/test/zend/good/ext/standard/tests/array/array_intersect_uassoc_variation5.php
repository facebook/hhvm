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
$arr_default_int = varray[1, 2 ];
$arr_float = darray[0 => 1.00, 1 => 2.00, 2 => 3.00];
$arr_string = varray['1', '2', '3'];
$arr_string_float = varray['1.00', '2.00'];

echo "\n-- Result of integers and floating point intersection --\n";
var_dump( array_intersect_uassoc($arr_default_int, $arr_float, fun("key_compare_func")) );

echo "\n-- Result of integers and strings containing integers intersection --\n";
var_dump( array_intersect_uassoc($arr_default_int, $arr_string, fun("key_compare_func")) );

echo "\n-- Result of integers and strings containing floating points intersection --\n";
var_dump( array_intersect_uassoc($arr_default_int, $arr_string_float, fun("key_compare_func")) );
echo "===DONE===\n";
}
