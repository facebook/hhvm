<?hh

/* Prototype: string md5_file( string filename[, bool raw_output] )
 * Description: Calculate the MD5 hash of a given file
 */

<<__EntryPoint>> function main(): void {
/* Creating an empty file */
$empty_file = sys_get_temp_dir().'/'.'md5_EmptyFile.txt';
fclose(fopen($empty_file, 'w+'));

/* Creating a data file */
$data_file = sys_get_temp_dir().'/'.'md5_DataFile.txt';
file_put_contents($data_file, "Add this to the file\n");

/* Testing error conditions */
echo "\n*** Testing for error conditions ***\n";

/* No filename */
var_dump( md5_file("") );

/* invalid filename */
var_dump( md5_file("aZrq16u") );

/* Zero arguments */
 try { var_dump ( md5_file() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* More than valid number of arguments ( valid is 2)  */
try { var_dump ( md5_file($empty_file, true, NULL) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Hexadecimal Output for Empty file as input */
echo "\n*** Hexadecimal Output for Empty file as Argument ***\n";
var_dump( md5_file($empty_file) );

/* Raw Binary Output for Empty file as input */
echo "\n*** Raw Binary Output for Empty file as Argument ***\n";
var_dump( md5_file($empty_file, true) );

/* Normal operation with hexadecimal output */
echo "\n*** Hexadecimal Output for a valid file with some contents ***\n";
var_dump( md5_file($data_file) );

/* Normal operation with raw binary output */
echo "\n*** Raw Binary Output for a valid file with some contents ***\n";
var_dump ( md5_file($data_file, true) );

// remove temp files
unlink($data_file);
unlink($empty_file);

echo "\nDone";
}
