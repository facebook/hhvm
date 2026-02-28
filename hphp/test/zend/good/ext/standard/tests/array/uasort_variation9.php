<?hh
/* Prototype  : bool uasort(array $array_arg, string $cmp_function)
 * Description: Sort an array with a user-defined comparison function and maintain index association
 * Source code: ext/standard/array.c
*/

/* Testing uasort() functionality with comparison function having arguments as reference
 */

// comparison function
/* Prototype : int cmp(mixed &$value1, mixed &$value2)
 * Parameters : $value1 and $value2 - values received by reference
 * Return value : 0 - if both values are same
 *                1 - if value1 is greater than value2
 *               -1 - if value1 is less than value2
 * Description : compares value1 and value2
 */
function cmp(inout $value1, inout $value2)
:mixed{
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 > $value2) {
    return 1;
  }
  else
    return -1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing uasort() : 'cmp_function' with reference arguments ***\n";

// Int array with default keys
$int_values = vec[1, 8, 9, 3, 2, 6, 7];
echo "-- Passing integer values to 'cmp_function' --\n";
var_dump( uasort(inout $int_values, cmp<>) );
var_dump($int_values);

// String array with default keys
$string_values = vec["Mango", "Apple", "Orange", "Banana"];
echo "-- Passing string values to 'cmp_function' --\n";
var_dump( uasort(inout $string_values, cmp<>) );
var_dump($string_values);

echo "Done";
}
