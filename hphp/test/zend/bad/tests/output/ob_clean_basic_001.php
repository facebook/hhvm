<?php
/* Prototype  : proto bool ob_clean(void)
 * Description: Clean (delete) the current output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */

echo "*** Testing ob_clean() : basic functionality ***\n";

// Zero arguments
echo "\n-- Testing ob_clean() function with Zero arguments --\n";
var_dump( ob_clean() );

ob_start();
echo "You should never see this.";
var_dump(ob_clean());

echo "Ensure the buffer is still active after the clean.";
$out = ob_get_clean();
var_dump($out);

echo "Done";
?>