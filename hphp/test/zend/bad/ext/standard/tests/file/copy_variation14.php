<?php
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy(): Trying to create a copy of non-existing source in an existing destination 
     and an existing source in non-existing destiantion */

$file_path = dirname(__FILE__);

echo "*** Test copy() function: Trying to create a copy of non-existing source in existing destination ***";
$file = $file_path."/copy_variation14.tmp";
$file_handle =  fopen($file, "w");
fwrite($file_handle, str_repeat(b"Hello2world...\n", 100));
fclose($file_handle);

var_dump( copy($file_path."/nosuchfile.tmp", $file_path."/copy_nosuchfile.tmp") );  //With non-existing source
var_dump( file_exists($file_path."/copy_nosuchfile.tmp") );

echo "\n*** Test copy() function: Trying to create copy of an existing source in non-existing destination ***";
var_dump( copy($file, $file_path."/nodir/copy_nosuchfile.tmp") );  //With non-existing dir path
var_dump( file_exists($file_path."/nodir/copy_nosuchfile.tmp") );
var_dump( filesize($file) );  //size of the source

echo "*** Done ***\n";
?>

<?php
unlink(dirname(__FILE__)."/copy_variation14.tmp");
?>
