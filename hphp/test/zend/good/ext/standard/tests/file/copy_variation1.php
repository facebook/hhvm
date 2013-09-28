<?php
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy() function: In creation of destination file names containing numerics/strings 
     and checking the existence and size of destination files
*/

echo "*** Test copy() function: destination file names containing numerics/strings ***\n";
$file_path = dirname(__FILE__);
$src_file_name = $file_path."/copy_variation1.tmp";
$file_handle = fopen($src_file_name, "w");
fwrite( $file_handle, str_repeat(b"Hello2World...\n", 100) );
fclose($file_handle);

/* array of destination file names */
$dest_files = array(

  /* File names containing numerics, strings */
  "copy.tmp",  //regular file name
  "copy_copy_variation1.tmp",
  ".tmp",  //file name only with extension
  "123.tmp",  //file name starts with numeric and with regular extension
  "copy_variation1.123",  //file name with numeric extension
  "123",  //numeric
  "123copy_variation1.tmp",  //file name containing numeric & string
  "copy_variation.tmp123",  //file name containing string & numeric
  chr(99).chr(111).chr(112).chr(121).chr(49).".tmp"  //file name containing ASCII values
);

echo "Size of the source file before copy operation => ";
var_dump( filesize("$src_file_name") );
clearstatcache();

echo "\n-- Now applying copy() on source file to create copies --";
$count = 1;
foreach($dest_files as $dest_file) {
  echo "\n-- Iteration $count --\n";

  $dest_file_name = "$file_path/$dest_file";

  echo "Copy operation => ";
  var_dump( copy($src_file_name, $dest_file_name) );

  echo "Existence of destination file => ";
  var_dump( file_exists($dest_file_name) );

  echo "Destination file name => ";
  print($dest_file_name);
  echo "\n";

  echo "Size of source file => ";
  var_dump( filesize($src_file_name) );
  clearstatcache();

  echo "Size of destination file => ";
  var_dump( filesize($dest_file_name) );
  clearstatcache();

  unlink($dest_file_name);

  $count++;
}

echo "*** Done ***\n";
?>
<?php
unlink(dirname(__FILE__)."/copy_variation1.tmp");
?>
