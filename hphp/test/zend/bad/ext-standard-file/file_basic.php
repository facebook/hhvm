<?php
/* 
 * Prototype: array file ( string filename [,int use-include_path [,resource context]] );
 * Description: Reads entire file into an array
 *              Returns the  file in an array
 */
require(dirname(__FILE__) . '/file.inc');
$file_path = dirname(__FILE__);
echo "*** Testing file() with basic types of files ***\n";
$filetypes = array("numeric", "text", "empty", "text_with_new_line");

foreach( $filetypes as $type ) {
  create_files($file_path, 1, $type, 0755, 100, "w", "file_basic", 1, "byte");
  print_r( file($file_path."/file_basic1.tmp") );
  delete_files($file_path, 1, "file_basic");
}

echo "*** Testing for return type of file() function ***\n";
foreach( $filetypes as $type ) {
  create_files($file_path, 1, $type, 0755, 1, "w", "file_basic");
  $ret_arr =  file($file_path."/file_basic1.tmp");
  var_dump( is_array($ret_arr) );
  delete_files($file_path, 1, "file_basic");
}

echo "\n--- Done ---";
?>