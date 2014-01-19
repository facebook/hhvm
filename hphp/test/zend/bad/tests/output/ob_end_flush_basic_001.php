<?php
/* Prototype  : proto bool ob_end_flush(void)
 * Description: Flush (send) the output buffer, and delete current output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */

echo "*** Testing ob_end_flush() : basic functionality ***\n";

// Zero arguments
echo "\n-- Testing ob_end_flush() function with Zero arguments --\n";
var_dump(ob_end_flush());

ob_start();
var_dump(ob_end_flush());

ob_start();
echo "Hello\n";
var_dump(ob_end_flush());

var_dump(ob_end_flush());

echo "Done";
?>