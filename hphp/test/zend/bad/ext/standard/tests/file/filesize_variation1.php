<?php
/* 
 Prototype   : int filesize ( string $filename );
 Description : Returns the size of the file in bytes, or FALSE 
   (and generates an error of level E_WARNING) in case of an error.
*/

$file_path = dirname(__FILE__);
require($file_path."/file.inc");

echo "*** Testing filesize(): usage variations ***\n"; 

echo "*** Checking filesize() with different size of files ***\n";
for($size = 1; $size <10000; $size = $size+1000)
{
  create_files($file_path, 1, "numeric", 0755, $size, "w", "filesize_variation");
  var_dump( filesize( $file_path."/filesize_variation1.tmp") );
  clearstatcache();
  delete_files($file_path, 1, "filesize_variation");
}

echo "Done\n";

