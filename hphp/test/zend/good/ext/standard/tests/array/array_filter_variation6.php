<?hh
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback.
 * Source code: ext/standard/array.c
*/

/*
* Passing 'input' array which contains elements as reference to other data
*/

// Callback function
/* Prototype : bool callback(array $input)
 * Parameter : $input - array of which each element need to be checked in function
 * Return Type : returns true or false
 * Description : This function checks each element of an input array if element > 5 then
 * returns true else returns false
 */
function callback($input)
{
  if($input > 5) {
    return true;
  }
  else {
    return false;
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_filter() : usage variations - 'input' containing references ***\n";

// initializing variables
$value1 = varray[1, 2, 8];
$value2 = varray[5, 6, 4];
$input = varray[$value1, 10, $value2, 'value'];

// with 'callback' argument
var_dump( array_filter($input, fun('callback')) );

// with default 'callback' argument
var_dump( array_filter($input) );

echo "Done";
}
