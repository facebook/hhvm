<?php
/* Prototype: int readfile ( string $filename [, bool $use_include_path [, resource $context]] );
   Description: Outputs a file
*/

$context = stream_context_create();

echo "*** Test readfile(): error conditions ***\n";
echo "-- Testing readfile() with unexpected no. of arguments --\n";
var_dump( readfile() );  // args < expected
var_dump( readfile(__FILE__, true, $context, 4) );  // args > expected

echo "\n-- Testing readfile() with invalid arguments --\n";
// invalid arguments
var_dump( readfile(NULL) );  // NULL as $filename
var_dump( readfile('') );  // empty string as $filename
var_dump( readfile(false) );  // boolean false as $filename
var_dump( readfile(__FILE__, false, '') );  // empty string as $context
var_dump( readfile(__FILE__, true, false) );  // boolean false as $context

echo "\n-- Testing readfile() with non-existent file --\n";
$non_existent_file = dirname(__FILE__)."/non_existent_file.tmp";
var_dump( readfile($non_existent_file) );

echo "Done\n";
?>