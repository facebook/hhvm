<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing array having binary values for $arr1 argument
 */

echo "*** Testing array_map() : array with binary data for 'arr1' argument ***\n";

function callback1($a)
{
  return ($a);
}
function callback2($a, $b)
{
  return array($a => $b);
}

// array with binary data
$arr1 = array(b"hello", b"world", "1", b"22.22");

echo "-- checking binary safe array with one parameter callback function --\n";
var_dump( array_map('callback1', $arr1) );

echo "-- checking binary safe array with two parameter callback function --\n";
var_dump( array_map(b"callback2", $arr1) );

echo "Done";
?>