<?php
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy(): Trying to copy the file to a destination, where destination is an existing dir */

$file_path = dirname(__FILE__);

echo "*** Test copy() function: Trying to create a copy of source file as a dir ***\n";
$file = $file_path."/copy_variation11.tmp";
$file_handle =  fopen($file, "w");
fwrite($file_handle, str_repeat(b"Hello, world...", 20));
fclose($file_handle);

$dir = $file_path."/copy_variation11";
mkdir($dir);

echo "Size of source before copy operation => ";
var_dump( filesize($file) );  //size of source before copy
clearstatcache();
echo "Size of destination before copy operation => ";
var_dump( filesize($dir) );  //size of destination before copy
clearstatcache();

echo "\n-- Now applying copy() operation --\n";
var_dump( copy($file, $dir) );  //expected: bool(false) 

var_dump( file_exists($file) );  //expected: bool(true)
var_dump( file_exists($dir) );  //expected: bool(true)

var_dump( is_file($file) );  //expected: bool(true)
var_dump( is_dir($file) );  //expected: bool(false)

var_dump( is_file($dir) ); //expected: bool(false)
var_dump( is_dir($dir) );  //expected: bool(true)

var_dump( filesize($file) );   //size of source after copy
var_dump( filesize($dir) );   //size of destination after copy

echo "*** Done ***\n";
?>

<?php error_reporting(0); ?>
<?php
unlink(dirname(__FILE__)."/copy_variation11.tmp");
rmdir(dirname(__FILE__)."/copy_variation11");
?>
