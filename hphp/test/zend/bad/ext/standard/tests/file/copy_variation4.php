<?php
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy() function: In creation of destination file names with empty string, nulls & bools 
     and checking the existence and size of destination files
*/

echo "*** Test copy() function: destination file names with empty string, nulls & bools ***\n";
$file_path = dirname(__FILE__);
$src_file_name = $file_path."/copy_variation4.tmp";
$file_handle = fopen($src_file_name, "w");
fwrite( $file_handle, str_repeat(b"Hello2World...\n", 100) );
fclose($file_handle);

/* array of destination file names */
$dest_files = array(

  /* File names containing(or with) nulls */
  "", 
  NULL,
  "\0",
  FALSE,
  false,
  TRUE,
  true
);

echo "Size of the source file before copy operation => ";
var_dump( filesize($src_file_name) );
clearstatcache();

echo "\n-- Now applying copy() on source file to create copies --";
$count = 1;
foreach($dest_files as $dest_file) {
  echo "\n-- Iteration $count --\n";
  $dest_file_name = $file_path."/$dest_file";

  echo "Existence of destination file before copy => ";
  var_dump( file_exists($dest_file_name) );

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

<?php error_reporting(0); ?>
<?php
unlink(dirname(__FILE__)."/copy_variation4.tmp");
?>
