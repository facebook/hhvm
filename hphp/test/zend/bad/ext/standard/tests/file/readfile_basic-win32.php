<?php
/* Prototype: int readfile ( string $filename [, bool $use_include_path [, resource $context]] );
   Description: Outputs a file
*/
// common file used
require(dirname(__FILE__) . '/file.inc');

echo "*** Testing readfile() : basic functionality ***\n";
$file_path = dirname(__FILE__);
$file_prefix = "readfile_basic";  // temp files created with this prefix

// the content that is filled into the temp files as created
$filetypes = array("numeric", "text", "empty", "alphanumeric", "text_with_new_line");
// different file modes
$filemodes = array("w", "wt", "wb", "w+", "w+b", "w+t",
                   "a", "at", "ab", "a+", "a+b", "a+t",
                   "x", "xb", "xt", "x+", "x+b", "x+t");

// create file, read the file content, delete file
foreach($filetypes as $type) {
  echo "\n-- File filled with content type: $type --\n";
  foreach($filemodes as $mode) {
    echo "-- File opened with mode: $mode --\n";
      if ( strstr($mode, "x") ) {
         $fp = fopen($file_path."/".$file_prefix."1.tmp", $mode);
         fill_file($fp, $type, 100);
         fclose($fp);
      } else {
        // creating file in write mode
        create_files($file_path, 1, $type, 0755, 100, $mode, $file_prefix, 1, "byte");
      }
      $count = readfile($file_path."/".$file_prefix."1.tmp");
      echo "\n";
      var_dump($count);
      // delete files created
      delete_files($file_path, 1, $file_prefix, 1);
  }
}
echo "Done\n";
?>