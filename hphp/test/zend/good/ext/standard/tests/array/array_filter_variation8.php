<?hh
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback.
 * Source code: ext/standard/array.c
*/

/*
* callback functions are expected to return bool value for array_filter()
* here testing callback functions for return values other than bool
*/

// callback functions
// int as return value
function callback1($input)
:mixed{
  return 5;
}
// float as return value
function callback2($input)
:mixed{
  return 3.4;
}
// string as return value
function callback3($input)
:mixed{
  return 'value';
}
// null as return value
function callback4($input)
:mixed{
  return null;
}
// array as return value
function callback5($input)
:mixed{
  return vec[8];
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_filter() : usage variations - callback function with different return values***\n";

$input = vec[0, 1, -1, 10, 100, 1000, 'Hello', null, true];

echo "callback function with int return value\n";
var_dump( array_filter($input, callback1<>) );

echo "callback function with float return value\n";
var_dump( array_filter($input, callback2<>) );

echo "callback function with string return value\n";
var_dump( array_filter($input, callback3<>) );

echo "callback function with null return value\n";
var_dump( array_filter($input, callback4<>) );

echo "callback function with array as return value\n";
var_dump( array_filter($input, callback5<>) );

echo "Done";
}
