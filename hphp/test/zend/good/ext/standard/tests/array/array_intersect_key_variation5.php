<?hh
/* Prototype  : array array_intersect_key(array arr1, array arr2 [, array ...])
 * Description: Returns the entries of arr1 that have keys which are present in all the other arguments. 
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_key() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = darray[0 => '0', 10 => '10' , -10 => '-10']; 
$float_indx_array = darray[0.0 => '0.0', 10.5 => '10.5' , -10.5 => '-10.5', 0.5 => '0.5']; 

echo "\n-- Testing array_intersect_key() function with float indexed array --\n";
var_dump( array_intersect_key($input_array, $float_indx_array) );
var_dump( array_intersect_key($float_indx_array,$input_array ) );
echo "===DONE===\n";
}
