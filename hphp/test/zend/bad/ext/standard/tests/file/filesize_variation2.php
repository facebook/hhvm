<?php
/* 
 Prototype   : int filesize ( string $filename );
 Description : Returns the size of the file in bytes, or FALSE 
   (and generates an error of level E_WARNING) in case of an error.
*/

$file_path = dirname(__FILE__);
require($file_path."/file.inc");

echo "*** Testing filesize(): usage variations ***\n"; 

echo "\n*** Testing size of a dir, sub-dir and file with filesize() ***\n";
echo "-- Creating a base dir, and checking its size --\n";
mkdir( $file_path."/filesize_variation2");
var_dump( filesize( $file_path."/filesize_variation2"));
clearstatcache();

echo "-- Creating a file inside base dir, and checking dir & file size --\n"; 
create_files($file_path."/filesize_variation2", 1, "numeric", 0755, 1, "w", "filesize_variation");
var_dump( filesize( $file_path."/filesize_variation2"));
clearstatcache();
var_dump( filesize( $file_path."/filesize_variation2/filesize_variation1.tmp"));
clearstatcache();
delete_files($file_path."/filesize_variation2", 1, "filesize_variation");

echo "-- Creating an empty sub-dir in base-dir, and checking size of base and sub dir --\n";
mkdir( $file_path."/filesize_variation2/filesize_variation2_sub");
var_dump( filesize( $file_path."/filesize_variation2")); // size of base dir
clearstatcache();
var_dump( filesize( $file_path."/filesize_variation2/filesize_variation2_sub")); // size of subdir
clearstatcache();

echo "-- Creating a file inside sub-dir, and checking size of base, subdir and file created --\n";
// create only the file, as base and subdir is already created
$filename =  $file_path."/filesize_variation2/filesize_variation2_sub/filesize_variation2.tmp";
$file_handle = fopen($filename, "w");
fwrite($file_handle, str_repeat("Hello,World ", 1000) ); // create file of size 12000 bytes
fclose($file_handle);
// size of base dir
var_dump( filesize( $file_path."/filesize_variation2"));
clearstatcache();
// size of subdir
var_dump( filesize( $file_path."/filesize_variation2/filesize_variation2_sub"));
clearstatcache();
// size of file inside subdir
var_dump( filesize( $file_path."/filesize_variation2/filesize_variation2_sub/filesize_variation2.tmp"));
clearstatcache();

echo "*** Done ***\n";
?>
<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/filesize_variation2/filesize_variation2_sub/filesize_variation2.tmp");
rmdir($file_path."/filesize_variation2/filesize_variation2_sub");
rmdir($file_path."/filesize_variation2");
?>