<?php
/* Prototype  : array array_diff_key(array arr1, array arr2 [, array ...])
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments. 
 * Source code: ext/standard/array.c
 */

echo "*** Testing array_diff_key() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$array1 = array(

	  'first' => array('blue' => 1, 'red' => 2),
              
      'second' => array('yellow' => 7),
              
      'third' => array(0 => 'zero'),
);

$array2 = array (

	  'first' => array('blue' => 1, 'red' => 2,),
              
      'second' => array('cyan' => 8),
              
      'fourth' => array(2 => 'two'), 
);

echo "\n-- Testing array_diff_key() function with multi dimensional array --\n";
var_dump( array_diff_key($array1, $array2) );
var_dump( array_diff_key($array2, $array1) );
?>
===DONE===