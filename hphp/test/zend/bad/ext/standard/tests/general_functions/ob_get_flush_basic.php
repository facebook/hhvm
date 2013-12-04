<?php
ini_set('output_buffering', 0);

/* Prototype  : bool ob_get_flush(void)
 * Description: Get current buffer contents, flush (send) the output buffer, and delete current output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */

echo "*** Testing ob_get_flush() : basic functionality ***\n";

ob_start();

echo "testing ob_get_flush() with some\nNewlines too\n";
$string = ob_get_flush();

var_dump( "this is printed before returning the string" );
var_dump( $string );
var_dump( ob_list_handlers() );

// Empty string expected
ob_start();
$string = ob_get_flush();
var_dump($string)

?>
===DONE===