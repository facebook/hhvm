<?hh
/* Prototype  : array array_diff_uassoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Computes the difference of arrays with additional index check which is performed by a
 *                 user supplied callback function
 * Source code: ext/standard/array.c
 */

function key_compare_func($key1, $key2)
{
    if ($key1 === $key2) {
        return 0;
    }
    return ($key1 > $key2)? 1:-1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_uassoc() : usage variation ***\n";

//Initialize variables
$arr_float = darray[0 => 1.00, 1 => 2.00];
$arr_string_int = varray['1', '2'];
$arr_string_float = darray['0' => '1.00', '1.00' => '2.00'];

echo "\n-- Result of comparing floating points and strings containing integers --\n";
var_dump( array_diff_uassoc($arr_float, $arr_string_int, fun("key_compare_func")) );
var_dump( array_diff_uassoc($arr_string_int, $arr_float, fun("key_compare_func")) );

echo "\n-- Result of comparing floating points and strings containing floating point --\n";
var_dump( array_diff_uassoc($arr_float, $arr_string_float, fun("key_compare_func")) );
var_dump( array_diff_uassoc($arr_string_float, $arr_float, fun("key_compare_func")) );

echo "===DONE===\n";
}
