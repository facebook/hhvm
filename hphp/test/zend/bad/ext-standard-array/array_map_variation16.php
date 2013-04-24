<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing non-permmited built-in functions and language constructs i.e.
 *   echo(), array(), empty(), eval(), exit(), isset(), list(), print()
 */

echo "*** Testing array_map() : non-permmited built-in functions ***\n";

// array to be passed as arguments
$arr1 = array(1, 2);

// built-in functions & language constructs
$callback_names = array(
/*1*/  'echo',
       'array',
       'empty',
/*4*/  'eval',
       'exit',
       'isset',
       'list',
/*8*/  'print'
);
for($count = 0; $count < count($callback_names); $count++)
{
  echo "-- Iteration ".($count + 1)." --\n";
  var_dump( array_map($callback_names[$count], $arr1) );
}

echo "Done";
?>