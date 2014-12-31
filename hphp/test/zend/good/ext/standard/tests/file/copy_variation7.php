<?php
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy() function: Trying to create copy of links */

$file_path = dirname(__FILE__);

echo "*** Testing copy() with symlink and hardlink ***\n";
$file = $file_path."/copy_variation7.tmp";
$file_handle = fopen($file, "w");
fwrite( $file_handle, str_repeat(b"Hello World, this is 2007 year ....\n", 100) );
fclose($file_handle);

$symlink = $file_path."/copy_variation7_symlink.tmp";
$hardlink = $file_path."/copy_variation7_hardlink.tmp";

symlink($file, $symlink);  //creating symlink
link($file, $hardlink);  //creating hardlink

echo "Size of source files => \n";
var_dump( filesize($file_path."/copy_variation7_symlink.tmp") );  //size of the symlink itself
clearstatcache();
var_dump( filesize($file_path."/copy_variation7_hardlink.tmp") );  //size of the file
clearstatcache();

echo "-- Now applying copy() on source link to create copies --\n";
echo "-- With symlink --\n";
var_dump( copy($symlink, $file_path."/copy_copy_variation7_symlink.tmp") );
var_dump( file_exists($file_path."/copy_copy_variation7_symlink.tmp") );
var_dump( is_link($file_path."/copy_copy_variation7_symlink.tmp") );
var_dump( is_file($file_path."/copy_copy_variation7_symlink.tmp") );
var_dump( filesize($file_path."/copy_copy_variation7_symlink.tmp") );
clearstatcache();

echo "-- With hardlink --\n";
var_dump( copy($hardlink, $file_path."/copy_copy_variation7_hardlink.tmp") );
var_dump( file_exists($file_path."/copy_copy_variation7_hardlink.tmp") );
var_dump( is_link($file_path."/copy_copy_variation7_hardlink.tmp") );
var_dump( is_file($file_path."/copy_copy_variation7_hardlink.tmp") );
var_dump( filesize($file_path."/copy_copy_variation7_hardlink.tmp") );
clearstatcache();

echo "*** Done ***\n";
?>

<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/copy_copy_variation7_symlink.tmp");
unlink($file_path."/copy_copy_variation7_hardlink.tmp");
unlink($file_path."/copy_variation7_symlink.tmp");
unlink($file_path."/copy_variation7_hardlink.tmp");
unlink($file_path."/copy_variation7.tmp");
?>
