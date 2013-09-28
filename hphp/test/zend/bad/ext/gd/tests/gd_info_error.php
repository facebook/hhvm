<?php
/* Prototype  : array gd_info()
 * Description: Retrieve information about the currently installed GD library
 * Source code: ext/gd/gd.c
 */
$extra_arg_number = 10;
$extra_arg_string = "Hello";

echo "*** Testing gd_info() : error conditions ***\n";

echo "\n-- Testing gd_info() function with more than expected number of arguments --\n";
var_dump(gd_info($extra_arg_number));
var_dump(gd_info($extra_arg_string, $extra_arg_number));
?>
===DONE===