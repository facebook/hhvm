<?php
/*
   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

$file_path = dirname(__FILE__); 
$arr = array(__FILE__);

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