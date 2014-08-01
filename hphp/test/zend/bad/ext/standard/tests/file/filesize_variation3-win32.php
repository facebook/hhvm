<?php
/* 
 Prototype   : int filesize ( string $filename );
 Description : Returns the size of the file in bytes, or FALSE 
   (and generates an error of level E_WARNING) in case of an error.
*/

$file_path = dirname(__FILE__);

echo "*** Testing filesize(): usage variations ***\n"; 
$filename =  $file_path."/filesize_variation3-win32.tmp";
$file_handle = fopen($filename, "w");
fwrite($file_handle, (binary)str_repeat("Hello,World ", 1000) ); // create file of size 12000 bytes
fclose($file_handle);

echo "-- Testing filesize() after truncating the file to a new length --\n";
// truncate the file created earlier in subdir, the size of the file is 12000bytes
// truncate the same file, in the loop , each time with the decrement in size by 1200 bytes,
//  until -1200bytes size
for($size = filesize($filename); $size>=-1200; $size-=1200) {
  $file_handle = fopen($filename, "r+");
  var_dump( ftruncate($file_handle, $size) );
  fclose($file_handle);
  var_dump( filesize($filename) );
  clearstatcache();
}

echo "*** Done ***\n";
?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/filesize_variation3-win32.tmp");
?>