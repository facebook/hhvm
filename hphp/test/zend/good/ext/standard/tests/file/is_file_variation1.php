<?hh
/* Prototype: bool is_file ( string $filename );
   Description: Tells whether the filename is a regular file
     Returns TRUE if the filename exists and is a regular file
*/

/* Testing is_file() with file containing data, truncating its size 
     and the file created by touch() */
<<__EntryPoint>> function main(): void {

echo "-- Testing is_file() with file containing data --\n";
$filename = sys_get_temp_dir().'/'.'is_file_variation1.tmp';
$file_handle = fopen($filename, "w" );
fwrite( $file_handle, "Hello, world....." ); // exptected true
fclose($file_handle);
var_dump( is_file($filename) );
clearstatcache();

echo "\n-- Testing is_file() after truncating filesize to zero bytes --\n";
$file_handle = fopen($filename, "r");
ftruncate($file_handle, 0);
fclose($file_handle);
var_dump( is_file($filename) ); // expected true
clearstatcache();
unlink($filename);

echo "\n-- Testing is_file() with an empty file --\n";
/* created by fopen() */
fclose( fopen($filename, "w") );
var_dump( is_file($filename) );  //expected true
clearstatcache();
unlink($filename);

/* created by touch() */
touch($filename);
var_dump( is_file($filename) ); // expected true
clearstatcache();
unlink($filename);

echo "\n*** Done ***";
}
