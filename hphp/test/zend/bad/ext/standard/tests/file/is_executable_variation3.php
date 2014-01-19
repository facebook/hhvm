<?php
/* Prototype: bool is_executable ( string $filename );
   Description: Tells whether the filename is executable
*/

/* test is_executable() with invalid arguments */

echo "*** Testing is_executable(): usage variations ***\n";

$file_handle = fopen(__FILE__, "r");
unset($file_handle);

echo "\n*** Testing is_executable() on invalid files ***\n";
$invalid_files = array(
  0,
  1234,
  -2.34555,
  TRUE,
  FALSE,
  NULL,
  " ",
  @array(),
  @$file_handle
);
/* loop through to test each element in the above array 
   is an executable file */
foreach( $invalid_files as $invalid_file ) {
  var_dump( is_executable($invalid_file) );
  clearstatcache();
}

echo "Done\n";
?>