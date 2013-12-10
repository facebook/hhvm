<?php
/* Prototype  : int vfprintf(resource stream, string format, array args)
 * Description: Output a formatted string into a stream 
 * Source code: ext/standard/formatted_print.c
 * Alias to functions: 
 */

// Open handle
$file = 'vfprintf_error3.phpt.txt';
$fp = fopen( $file, "a+" );

echo "\n-- Testing vfprintf() function with wrong variable types as argument --\n";
var_dump( vfprintf( $fp, array( 'foo %d', 'bar %s' ), 3.55552 ) );

rewind( $fp );
var_dump( stream_get_contents( $fp ) );
ftruncate( $fp, 0 );
rewind( $fp );

var_dump( vfprintf( $fp, "Foo %y fake", "not available" ) );

rewind( $fp );
var_dump( stream_get_contents( $fp ) );
ftruncate( $fp, 0 );
rewind( $fp );

// Close handle
fclose( $fp );

?>
===DONE===
<?php

$file = 'vfprintf_error3.phpt.txt';
unlink( $file );

?>