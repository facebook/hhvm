<?php
/* Prototype  : proto array get_object_vars(object obj)
 * Description: Returns an array of object properties 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

echo "*** Testing get_object_vars() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing get_object_vars() function with Zero arguments --\n";
var_dump( get_object_vars() );

//Test get_object_vars with one more than the expected number of arguments
echo "\n-- Testing get_object_vars() function with more than expected no. of arguments --\n";
$obj = new stdclass();
$extra_arg = 10;
var_dump( get_object_vars($obj, $extra_arg) );

echo "Done";
?>
