<?php
ini_set('output_buffering', 0);

/* Prototype  : bool ob_get_flush(void)
 * Description: Get current buffer contents, flush (send) the output buffer, and delete current output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */

echo "*** Testing ob_get_flush() : error conditions ***\n";

// One extra argument
$extra_arg = 10;
var_dump( ob_get_flush( $extra_arg ) );

// No ob_start() executed
var_dump( ob_get_flush() );

?>
===DONE===