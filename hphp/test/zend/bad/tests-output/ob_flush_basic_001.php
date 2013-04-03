<?php
/* Prototype  : proto bool ob_flush(void)
 * Description: Flush (send) contents of the output buffer. The last buffer content is sent to next buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */

echo "*** Testing ob_flush() : basic functionality ***\n";

// Zero arguments
echo "\n-- Testing ob_flush() function with Zero arguments --\n";
var_dump(ob_flush());

ob_start();
echo "This should get flushed.\n";
var_dump(ob_flush());

echo "Ensure the buffer is still active after the flush.\n";
$out = ob_flush();
var_dump($out);

echo "Done";

?>