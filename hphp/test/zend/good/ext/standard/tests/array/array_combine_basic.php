<?hh
/* Prototype  : array array_combine(array $keys, array $values)
 * Description: Creates an array by using the elements of the first parameter as keys
 *              and the elements of the second as the corresponding values
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_combine() : basic functionality ***\n";

/* Different arrays for $keys and $values arguments */

// array with default keys for $keys and $values arguments
$keys_array = vec[1, 2];
$values_array = vec[3,4];
var_dump( array_combine($keys_array, $values_array) );

// associative arrays for $keys and $values arguments
$keys_array = dict[1 => "a", 2 => 'b'];
$values_array = dict[3 => 'c', 4 => "d"];
var_dump( array_combine($keys_array, $values_array) );

// mixed array for $keys and $values arguments
$keys_array = dict[0 => 1, 2 => "b"];
$values_array = dict[3 => 'c', 4 => 4];
var_dump( array_combine($keys_array, $values_array) );

echo "Done";
}
