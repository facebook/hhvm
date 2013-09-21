<?php
/* Prototype  : array array_chunk(array $array, int $size [, bool $preserve_keys])
 * Description: Split array into chunks 
 * Source code: ext/standard/array.c
*/

/*
* Testing array_chunk() function with unexpected values for 'preserve_keys' 
*/

echo "*** Testing array_chunk() : usage variations ***\n";

// input array
$input = array(1, 2);
$size = 10;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//array of values to iterate over
$values = array(

        // int data
/*1*/   0,
        1,
        12345,
        -2345,

        // float data
/*5*/   10.5,
        -10.5,
        10.5e10,
        10.6E-10,
        .5,

        // null data
/*10*/  NULL,
        null,

        // empty data
/*12*/  "",
        '',

        // string data
/*14*/  "string",
        'string',

        // object data
/*16*/  new stdclass(),

        // undefined data
/*17*/  @undefined_var,

        // unset data
/*18*/  @unset_var

);

$count = 1;

// loop through each element of the array for preserve_keys
foreach($values as $value) {
  echo "\n-- Iteration $count --\n";
  var_dump( array_chunk($input, $size, $value) );
  $count++;
}

echo "Done";
?>