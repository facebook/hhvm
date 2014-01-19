<?php
/* Prototype: array glob ( string $pattern [, int $flags] );
   Description: Find pathnames matching a pattern
*/

$file_path = dirname(__FILE__);

// temp dir created
mkdir("$file_path/glob_error");
// temp file created
$fp = fopen("$file_path/glob_error/wonder12345", "w");
fclose($fp);

echo "*** Testing glob() : error conditions ***\n";

echo "-- Testing glob() with unexpected no. of arguments --\n";
var_dump( glob() );  // args < expected
var_dump( glob(dirname(__FILE__)."/glob_error/wonder12345", GLOB_ERR, 3) );  // args > expected

echo "\n-- Testing glob() with invalid arguments --\n";
var_dump( glob(dirname(__FILE__)."/glob_error/wonder12345", '') );
var_dump( glob(dirname(__FILE__)."/glob_error/wonder12345", "string") );

echo "Done\n";
?>
<?php
// temp file deleted
unlink(dirname(__FILE__)."/glob_error/wonder12345");
// temp dir deleted
rmdir(dirname(__FILE__)."/glob_error");
?>