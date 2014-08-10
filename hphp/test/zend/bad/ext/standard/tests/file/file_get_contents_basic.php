<?php

/*  Prototype: string file_get_contents( string $filename[, bool $use_include_path[,
 *                                       resource $context[, int $offset[, int $maxlen]]]] )
 *  Description: Reads entire file into a string
 */

$file_path = dirname(__FILE__);
include($file_path."/file.inc");

echo "*** Testing the basic functionality of the file_get_contents() function ***\n";

echo "-- Testing with simple valid data file --\n";


create_files($file_path, 1, "text", 0755, 100, "w", "file", 1, "byte");
var_dump( file_get_contents($file_path."/file1.tmp") );
delete_files($file_path, 1);

echo "\n-- Testing with empty file --\n";

create_files($file_path, 1, "empty", 0755, 100, "w", "file", 1, "byte");
var_dump( file_get_contents($file_path."/file1.tmp") );
delete_files($file_path, 1);

echo "\n*** Done ***";
?>
