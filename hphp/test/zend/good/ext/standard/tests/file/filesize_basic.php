<?hh
/* 
 * Prototype: int filesize ( string $filename );
 * Description: Returns the size of the file in bytes, or FALSE 
 *              (and generates an error of level E_WARNING) in case of an error.
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing size of files and directories with filesize() ***\n"; 



var_dump( filesize(__FILE__) );
var_dump( filesize(".") );

/* Empty file */
$file_name = sys_get_temp_dir().'/'.'filesize_basic.tmp';
$file_handle = fopen($file_name, "w");
fclose($file_handle);
var_dump( filesize($file_name) );

echo "*** Done ***\n";

unlink($file_name);
}
