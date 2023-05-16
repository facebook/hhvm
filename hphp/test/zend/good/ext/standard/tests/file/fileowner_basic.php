<?hh
/* Prototype: int fileowner ( string $filename )
 * Description: Returns the user ID of the owner of the file, or
 *              FALSE in case of an error.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing fileowner(): basic functionality ***\n"; 

echo "-- Testing with the file or directory created by owner --\n";
var_dump( fileowner(__FILE__) );
var_dump( fileowner(".") );
var_dump( fileowner("./..") );

/* Newly created files and dirs */
$file_name = sys_get_temp_dir().'/'.'fileowner_basic.tmp';
$file_handle = fopen($file_name, "w");
$string = "Hello, world\n1234\n123Hello";
fwrite($file_handle, $string);
var_dump( fileowner($file_name) );
fclose($file_handle);

$dir_name = sys_get_temp_dir().'/'.'fileowner_basic';
mkdir($dir_name);
var_dump( fileowner($dir_name) );

echo "*** Done ***\n";

unlink($file_name);
rmdir($dir_name);
}
