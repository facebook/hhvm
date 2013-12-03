<?php

/*  Prototype: string file_get_contents( string $filename[, bool $use_include_path[,
 *                                       resource $context[, int $offset[, int $maxlen]]]] )
 *  Description: Reads entire file into a string
 */

/*  Prototype: int file_put_contents( string $filename, mixed $data[, int $flags[, resource $context]] )
 *  Description: Write a string to a file
 */

$file_path = dirname(__FILE__);
include($file_path."/file.inc");

echo "*** Testing the basic functionality of file_put_contents() and file_get_contents() functions ***\n";

echo "-- Testing with simple valid data file --\n";

$file_name = "/file_put_contents.tmp";
fill_buffer($text_buffer, "text", 100);
file_put_contents( $file_path.$file_name, $text_buffer );

var_dump( file_get_contents($file_path.$file_name) );

echo "\n-- Testing with empty file --\n";

$file_name = "/file_put_contents1.tmp";
file_put_contents( $file_path.$file_name, "");
var_dump( file_get_contents( $file_path.$file_name ) );

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/file_put_contents.tmp");
unlink($file_path."/file_put_contents1.tmp");
?>