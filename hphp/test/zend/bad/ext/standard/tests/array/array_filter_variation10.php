<?php
/* Prototype  : array array_filter(array $input [, callback $callback [, bool $use_type = ARRAY_FILTER_USE_VALUE]])
 * Description: Filters elements from the array via the callback. 
 * Source code: ext/standard/array.c
*/

/*
* Using array keys as an argument to the 'callback'
*/

echo "*** Testing array_filter() : usage variations - using array keys in 'callback' ***\n";

$input = array(0, 1, -1, 10, 100, 1000, 'Hello', null);
$small = array(123);

function dump($value, $key)
{
  echo "$key = $value\n";
}

var_dump( array_filter($input, 'dump', true) );

echo "*** Testing array_filter() : usage variations - 'callback' filters based on key value ***\n";

function dump2($value, $key)
{
  return $key > 4;
}

var_dump( array_filter($input, 'dump2', true) );

echo "*** Testing array_filter() : usage variations - 'callback' expecting second argument ***\n";

var_dump( array_filter($small, 'dump', false) );

echo "*** Testing array_filter() with various use types ***\n";

$mixed = array(1 => 'a', 2 => 'b', 'a' => 1, 'b' => 2);

var_dump(array_filter($mixed, 'is_numeric', ARRAY_FILTER_USE_KEY));

var_dump(array_filter($mixed, 'is_numeric', 0));

var_dump(array_filter($mixed, 'is_numeric', ARRAY_FILTER_USE_BOTH));

echo "Done"
?>
