<?php
/* Prototype  : array get_extension_funcs  ( string $module_name  )
 * Description: Returns an array with the names of the functions of a module.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

echo "*** Testing get_extension_funcs() : error conditions ***\n";

echo "\n-- Too few arguments --\n";
var_dump(get_extension_funcs());

$extra_arg = 1;
echo "\n-- Too many arguments --\n";
var_dump(get_extension_funcs("standard", $extra_arg));

echo "\n-- Invalid extension name --\n";
var_dump(get_extension_funcs("foo"));

?>
===DONE===