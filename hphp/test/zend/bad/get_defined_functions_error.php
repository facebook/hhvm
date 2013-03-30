<?php

/* Prototype  : array get_defined_functions  ( void  )
 * Description: Gets an array of all defined functions.
 * Source code: Zend/zend_builtin_functions.c
*/


echo "*** Testing get_defined_functions() : error conditions ***\n";


echo "\n-- Testing get_defined_functions() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( get_defined_functions($extra_arg) );

?> 
===Done===