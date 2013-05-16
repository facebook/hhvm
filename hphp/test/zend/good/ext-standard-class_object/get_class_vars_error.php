<?php
/* Prototype  : array get_class_vars(string class_name)
 * Description: Returns an array of default properties of the class.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

echo "*** Testing get_class_vars() : error conditions ***\n";


//Test get_class_vars with one more than the expected number of arguments
echo "\n-- Testing get_class_vars() function with more than expected no. of arguments --\n";
$obj = new stdclass();
$extra_arg = 10;
var_dump(get_class_vars($obj,$extra_arg) );

// Testing get_class_vars with one less than the expected number of arguments
echo "\n-- Testing get_class_vars() function with less than expected no. of arguments --\n";
var_dump(get_class_vars());

?>
===DONE===