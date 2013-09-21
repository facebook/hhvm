<?php
/* Prototype: bool is_writable ( string $filename );
   Description: Tells whether the filename is writable.

   is_writeable() is an alias of is_writable()
*/

// include common file test functions
require dirname(__FILE__).'/file.inc';

echo "*** Testing is_writable(): basic functionality ***\n";

// create a file
$filename = dirname(__FILE__)."/is_writable.tmp";
create_file($filename);

$counter = 1;
/* loop to check if the file with new mode is writable
   using is_writable() */
for($mode = 0000; $mode <= 0777; $mode++) {
  echo "-- Changing mode of file to $mode --\n";
  chmod($filename, $mode);  // change mode of file
  var_dump( is_writeable($filename) );
  var_dump( is_writable($filename) );
  clearstatcache();
  $counter++;
}

// delete the temp file
delete_file($filename);

echo "Done\n";
?>