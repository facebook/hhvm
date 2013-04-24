<?php
/* Prototype : bool unlink ( string $filename [, resource $context] );
   Description : Deletes filename
*/

/* Delete file having default permission but its dir having readonly permission
   Delete file having readonly permission but dir having default permission
*/


$file_path = dirname(__FILE__);

// temp dir name used here
$dirname = "$file_path/unlink_variation1";
// temp filename used here
$filename = "$dirname/unlink_variation1.tmp";

echo "\n*** Testing unlink() on file inside a directory ***\n";
// create temp dir
mkdir($dirname);
// create temp file inside $dirname
$fp = fopen($filename, "w");
fclose($fp);

echo "-- Unlink file having default permission and its dir having read only permission --\n";
// remove write permission of $dirname
var_dump( chmod($dirname, 0444) );
// now try deleting $filename
var_dump( unlink($filename) );  // expected false
var_dump( file_exists($filename) );  // confirm file is deleted

// remove the dir
var_dump( chmod($dirname, 0777) );
var_dump( unlink($filename) );  // expected true
var_dump( rmdir($dirname) );

echo "\n-- Unlinking file without write permission, its dir having default permission --\n";
// create the temp dir
var_dump( mkdir($dirname) );
// create the temp file
$fp = fopen($filename, "w");
fclose($fp);

// remove write permission from file
var_dump( chmod($filename, 0444) );
// now try deleting temp file inside $dirname
var_dump( unlink($filename) );  // expected: true
var_dump( file_exists($filename) );  // expected: false

// remove temp dir
var_dump( rmdir($dirname) );

echo "Done\n";
?>