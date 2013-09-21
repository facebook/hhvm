<?php
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

echo "*** Testing array_filter() : usage variation - different 'callback' functions***\n";

// Initialize variables
$input = array(0, -1, 2, 3.4E-3, 'hello', "value", "key" => 4, 'null' => NULL);

// callback function without parameters and with return value
function callback1()
{
  return 1;
}
echo "-- Callback function without parameter and with return --\n";
var_dump( array_filter($input, "callback1") );

// callback function with parameter and without return value
function callback2($input)
{
}
echo "-- Callback funciton with parameter and without return --\n";
var_dump( array_filter($input, "callback2") );


// callback function without parameter and without return value
function callback3()
{
}
echo "-- Callback function without parameter and return --\n";
var_dump( array_filter($input, "callback3") );

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
echo "-- Callback function with parameter and return --\n";
var_dump( array_filter($input, "callback4") );

echo "Done"
?>