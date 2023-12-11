<?hh
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback.
 * Source code: ext/standard/array.c
*/
/*  callback function
 *  Prototype : bool odd(array $input)
 *  Parameters : $input - array for which each elements should be checked into the function
 *  Return Type : bool - true if element is odd and returns false otherwise
 *  Description : Function takes array as input and checks for its each elements.
*/
function odd($input) :mixed{
  return ($input % 2 != 0);
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_filter() : error conditions ***\n";

$input = vec[0, 1, 2, 3, 5];

$extra_arg = 10;

// with incorrect callback function
echo "-- Testing array_filter() function with incorrect callback --";
var_dump( array_filter($input, "even") );

echo "Done";
}
