<?php
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy(): Try copying source file to desntination file, where destination file name is identical to source name */

$file_path = dirname(__FILE__);

echo "*** Test copy(): Trying to create a copy of file with the same source name ***\n";
$file = $file_path."/copy_variation10.tmp";
$file_handle =  fopen($file, "w");
fwrite($file_handle, str_repeat(b"Hello2world...\n", 100));
fclose($file_handle);

var_dump( copy($file, $file) ); 
var_dump( file_exists($file) );  
var_dump( filesize($file) );

echo "*** Done ***\n";
?>
<?php
unlink(dirname(__FILE__)."/copy_variation10.tmp");
?>
