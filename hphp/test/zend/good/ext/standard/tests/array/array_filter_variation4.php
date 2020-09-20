<?hh
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback.
 * Source code: ext/standard/array.c
*/

/*
* Passing different types of callback functions to array_filter()
* with parameters and return
* without parameter and with return
* with parameter and without return
* without parameter and without return
*/

// callback function without parameters and with return value
function callback1()
{
  return 1;
}

// callback function with parameter and without return value
function callback2($input)
{
}

// callback function without parameter and without return value
function callback3()
{
}

// callback function with parameter and with return value
function callback4($input)
{
  if($input > 0 ) {
    return true;
  }
  else {
    return false;
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_filter() : usage variation - different 'callback' functions***\n";

// Initialize variables
$input = darray[0 => 0, 1 => -1, 2 => 2, 3 => 3.4E-3, 4 => 'hello', 5 => "value", "key" => 4, 'null' => NULL];

echo "-- Callback function without parameter and with return --\n";
var_dump( array_filter($input, fun("callback1")) );

echo "-- Callback funciton with parameter and without return --\n";
var_dump( array_filter($input, fun("callback2")) );


echo "-- Callback function without parameter and return --\n";
var_dump( array_filter($input, fun("callback3")) );

echo "-- Callback function with parameter and return --\n";
var_dump( array_filter($input, fun("callback4")) );

echo "Done";
}
