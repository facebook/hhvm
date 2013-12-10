<?php
/* Prototype  : int vfprintf(resource stream, string format, array args)
 * Description: Output a formatted string into a stream 
 * Source code: ext/standard/formatted_print.c
 * Alias to functions: 
 */

// Open handle
$file = 'vfprintf_error2.phpt.txt';
$fp = fopen( $file, "a+" );

echo "\n-- Testing vfprintf() function with less than expected no. of arguments --\n";
$format = 'string_val';
var_dump( vfprintf($fp, $format) );
var_dump( vfprintf( $fp ) );
var_dump( vfprintf() );

// Close handle
fclose($fp);

?>
===DONE===
<?php

$file = 'vfprintf_error2.phpt.txt';
unlink( $file );

?>