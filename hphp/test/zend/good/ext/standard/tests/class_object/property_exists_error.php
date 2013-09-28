<?php
/* Prototype  : bool property_exists(mixed object_or_class, string property_name)
 * Description: Checks if the object or class has a property 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

echo "*** Testing property_exists() : error conditions ***\n";

$object_or_class = "obj";
$property_name = 'string_val';
$extra_arg = 10;


echo "\n-- Testing property_exists() function with more than expected no. of arguments --\n";
var_dump( property_exists($object_or_class, $property_name, $extra_arg) );


echo "\n-- Testing property_exists() function with less than expected no. of arguments --\n";
var_dump( property_exists($object_or_class) );

echo "\n-- Testing property_exists() function with incorrect arguments --\n";
var_dump( property_exists(10, $property_name) );

?>
===DONE===