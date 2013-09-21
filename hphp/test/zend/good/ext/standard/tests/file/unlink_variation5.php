<?php
/* Prototype : bool unlink ( string $filename [, resource $context] );
   Description : Deletes filename
*/

/* delete files with different file permission(0000 to 0777) */

$file_path = dirname(__FILE__);

// temp file used
$filename = "$file_path/unlink_variation5.tmp";

echo "*** Testing unlink() on a file ***\n";

for($mode = 0000; $mode <= 0777; $mode++ ) {
  // create temp file
  $fp = fopen($filename, "w");
  fclose($fp);
  // changing mode of file
  echo "File permission : $mode\n";
  var_dump( chmod($filename, $mode) );
  var_dump( unlink($filename) );  // deleting file
  var_dump( file_exists($filename) );  // confirm file deleted
}

echo "Done\n";
?>