<?php
/* Prototype  : proto int ob_get_level(void)
 * Description: Return the nesting level of the output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */

echo "*** Testing ob_get_level() : basic functionality ***\n";

// Zero arguments
echo "\n-- Testing ob_get_level() function with Zero arguments --\n";
var_dump(ob_get_level());

ob_start();
var_dump(ob_get_level());

ob_start();
var_dump(ob_get_level());

ob_end_flush();
var_dump(ob_get_level());

ob_end_flush();
var_dump(ob_get_level());

ob_end_flush();
var_dump(ob_get_level());


echo "Done";
?>