<?php
/* Prototype  : int vfprintf(resource stream, string format, array args)
 * Description: Output a formatted string into a stream 
 * Source code: ext/standard/formatted_print.c
 * Alias to functions: 
 */

// Open handle
$file = 'vfprintf_test.txt';
$fp = fopen( $file, "a+" );

echo "\n-- Testing vfprintf() function with other strangeties  --\n";
var_dump( vfprintf( 'foo', 'bar', array( 'baz' ) ) );
var_dump( vfprintf( $fp, 'Foo %$c-0202Sd', array( 2 ) ) );

// Close handle
fclose( $fp );

?>
===DONE===
<?php

$file = 'vfprintf_test.txt';
unlink( $file );

?>