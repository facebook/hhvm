<?php
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy(): Trying to copy source file to destination file with and without write permissions */

$file_path = dirname(__FILE__);

echo "*** Test copy() function: destination with/without write permissions ***\n";
$src_file_name = $file_path."/copy_variation9.tmp";
$file_handle =  fopen($src_file_name, "w");
fwrite($file_handle, str_repeat(b"Hello2world...\n", 100));
fclose($file_handle);

$dest_file_name =  $file_path."/copy_copy_variation9.tmp";


echo "\n-- With write permissions --\n";
var_dump( file_exists($src_file_name) );
var_dump( copy($src_file_name, $dest_file_name) );
var_dump( file_exists($dest_file_name) );
var_dump( filesize($dest_file_name) );

echo "\n-- Without write permissions --\n";
chmod($file_path."/copy_copy_variation9.tmp", 0555);  //No write permissions
var_dump( file_exists($src_file_name) );
var_dump( copy($src_file_name, $dest_file_name) );
var_dump( file_exists($dest_file_name) );
var_dump( filesize($dest_file_name) );

echo "*** Done ***\n";
?>
<?php
unlink(dirname(__FILE__)."/copy_copy_variation9.tmp");
unlink(dirname(__FILE__)."/copy_variation9.tmp");
?>