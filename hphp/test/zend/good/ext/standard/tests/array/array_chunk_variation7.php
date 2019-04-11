<?php
/* Prototype  : array array_chunk(array $array, int $size [, bool $preserve_keys])
 * Description: Split array into chunks
 *            : Chunks an array into size large chunks
 * Source code: ext/standard/array.c
*/

/*
 * Testing array_chunk() function with following conditions 
 *   1. input array containing references
*/

echo "*** Testing array_chunk() : usage variations ***\n";

$size = 2;

echo "\n-- Testing array_chunk(), input array containing references \n";

$n0 = 1;
$n1 = 2;
$n2 = 3;
$n3 = 4;
// reference array
$input_array = array (
  "one" => &$n0,
  "two" => &$n1,
  "three" => &$n2,
  "four" => &$n3,
);

var_dump( array_chunk($input_array, $size) );
var_dump( array_chunk($input_array, $size, true) );
var_dump( array_chunk($input_array, $size, false) );

echo "Done";
