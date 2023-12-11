<?hh
/* Prototype  : array array_diff_uassoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Computes the difference of arrays with additional index check which is performed by a
 *                 user supplied callback function
 * Source code: ext/standard/array.c
 */

function key_compare_func($key1, $key2)
:mixed{
    if ($key1 === $key2) {
        return 0;
    }
    return (HH\Lib\Legacy_FIXME\gt($key1, $key2))? 1:-1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_uassoc() : usage variation ***\n";

//Initialize variables
$arr_default_int = vec[1, 2, 3];
$arr_string_int = vec['1', '2'];
$arr_string_float = dict['0' => '1.00', '1.00' => '2.00'];

echo "\n-- Result of comparing integers and strings containing an integers --\n";
var_dump( array_diff_uassoc($arr_default_int, $arr_string_int, key_compare_func<>) );
var_dump( array_diff_uassoc($arr_string_int, $arr_default_int, key_compare_func<>) );

echo "\n-- Result of comparing integers and strings containing floating points --\n";
var_dump( array_diff_uassoc($arr_default_int, $arr_string_float, key_compare_func<>) );
var_dump( array_diff_uassoc($arr_string_float, $arr_default_int, key_compare_func<>) );

echo "===DONE===\n";
}
