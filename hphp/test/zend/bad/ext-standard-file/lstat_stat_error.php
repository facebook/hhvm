<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

echo "*** Testing lstat() for error conditions ***\n";
$file_path = dirname(__FILE__); 
var_dump( lstat() );  // args < expected
var_dump( lstat(__FILE__, 2) );  // args > expected
var_dump( lstat("$file_path/temp.tmp") ); // non existing file
var_dump( lstat(22) ); // scalar argument
$arr = array(__FILE__);
var_dump( lstat($arr) ); // array argument

echo "\n*** Testing stat() for error conditions ***\n";
var_dump( stat() );  // args < expected
var_dump( stat(__FILE__, 2) );  // file, args > expected
var_dump( stat(dirname(__FILE__), 2) );  //dir, args > expected

var_dump( stat("$file_path/temp.tmp") ); // non existing file
var_dump( stat("$file_path/temp/") ); // non existing dir
var_dump( stat(22) ); // scalar argument
var_dump( stat($arr) ); // array argument

echo "Done\n";
?>