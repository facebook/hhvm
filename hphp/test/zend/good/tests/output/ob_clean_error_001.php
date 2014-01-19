<?php
/* Prototype  : proto bool ob_clean(void)
 * Description: Clean (delete) the current output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */

echo "*** Testing ob_clean() : error conditions ***\n";

// One argument
echo "\n-- Testing ob_clean() function with one argument --\n";
$extra_arg = 10;;
var_dump( ob_clean($extra_arg) );

echo "Done";
?>