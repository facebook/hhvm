<?hh
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback.
 * Source code: ext/standard/array.c
*/

/*
* Passing different types of array as 'input' argument.
*/

// callback function returning always false
function always_false($input)
:mixed{
  return false;
}

// callback function returning always true
function always_true($input)
:mixed{
  return true;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_filter() : usage variations - different types of array for 'input' argument***\n";

// different types of 'input' array
$input_values = varray[
  varray[0, 1, 2, -1, 034, 0X4A],  // integer values
  varray[0.0, 1.2, 1.2e3, 1.2e-3],  // float values
  varray['value1', "value2", '', " ", ""],  // string values
  varray[true, false, TRUE, FALSE],  // bool values
  varray[null, NULL],  // null values
  darray[1 => 'one', 'zero' => 0, -2 => "value"], //associative array
  darray["one" => 1, '' => 'null', 5 => "float", 1 => 1, "" => 'empty'],  // associative array with different keys
  darray[1 => 'one', 2 => 2, "key" => 'value']  // combinition of associative and non-associative array

];

// loop through each element of 'input' with default callback
for($count = 0; $count < count($input_values); $count++)
{
  echo "-- Iteration ".($count + 1). " --\n";
  var_dump( array_filter($input_values[$count]) );
  var_dump( array_filter($input_values[$count], always_true<>) );
  var_dump( array_filter($input_values[$count], always_false<>) );
}

echo "Done";
}
