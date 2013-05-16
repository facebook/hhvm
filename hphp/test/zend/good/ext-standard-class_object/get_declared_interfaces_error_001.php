<?php
/* Prototype  : proto array get_declared_interfaces()
 * Description: Returns an array of all declared interfaces. 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

echo "*** Testing get_declared_interfaces() : error conditions ***\n";

// One argument
echo "\n-- Testing get_declared_interfaces() function with one argument --\n";
$extra_arg = 10;;
var_dump( get_declared_interfaces($extra_arg) );

echo "Done";
?>