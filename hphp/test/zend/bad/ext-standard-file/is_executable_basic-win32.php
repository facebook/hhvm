<?php
/* Prototype: bool is_executable ( string $filename );
   Description: Tells whether the filename is executable
*/
require dirname(__FILE__).'/file.inc';

echo "*** Testing is_executable(): basic functionality ***\n";

// create a file
$filename = dirname(__FILE__)."/is_executable.tmp";
create_file($filename);

$counter = 1;
/* loop to check if the file with new mode is executable
   using is_executable() */
for($mode = 0000; $mode <= 0777; $mode++) {
  echo "-- Changing mode of file to $mode --\n";
  chmod($filename, $mode);  // change mode of file
  var_dump( is_executable($filename) );
  $counter++;
  clearstatcache();
}

// delete the temp file
delete_file($filename);

echo "Done\n";
?>