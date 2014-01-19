<?php
/* 
   Prototype: array file ( string filename [,int use-include_path [,resource context]] );
   Description: Reads entire file into an array
                Returns the  file in an array
*/
$file_path = dirname(__FILE__);
echo "\n*** Testing error conditions ***";
$file_handle = fopen($file_path."/file.tmp", "w");
var_dump( file() );  // Zero No. of args

$filename = $file_path."/file.tmp";
var_dump( file($filename, $filename, $filename, $filename) );  // more than expected number of arguments

var_dump( file($filename, "INCORRECT_FLAG", NULL) );  //  Incorrect flag
var_dump( file($filename, 10, NULL) );  //  Incorrect flag

var_dump( file("temp.tmp") );  // non existing filename 
fclose($file_handle);

echo "\n--- Done ---";
?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/file.tmp");
?>