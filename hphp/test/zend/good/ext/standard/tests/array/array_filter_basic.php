<?hh
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback.
 * Source code: ext/standard/array.c
*/


/* Callback function
 * Prototype : bool even(array $input)
 * Parameters : $input - input array each element of which will be checked in function even()
 * Return type : boolean - true if element is even and false otherwise
 * Description : This function takes array as parameter and checks for each element of array.
 *              It returns true if the element is even number else returns false
 */
function even($input)
{
  return ($input % 2 == 0);
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_filter() : basic functionality ***\n";


// Initialise all required variables
$input = varray[1, 2, 3, 0, -1];  // 0 will be considered as FALSE and removed in default callback

// with all possible arguments
var_dump( array_filter($input,fun("even")) );

// with default arguments
var_dump( array_filter($input) );

echo "Done";
}
