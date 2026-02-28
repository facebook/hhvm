<?hh

/* Prototype: string sha1_file( string filename[, bool raw_output] )
 * Description: Calculate the sha1 hash of a file
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing sha1_file() : basic functionality ***\n";

/* Creating an empty file */
$empty_file = sys_get_temp_dir().'/'.'sha1_EmptyFile.txt';
fclose(fopen($empty_file, 'w+'));

/* Creating a data file */
$data_file = sys_get_temp_dir().'/'.'sha1_DataFile.txt';
file_put_contents($data_file, "Add this to the file\n");

/* Testing error conditions */
echo "\n*** Testing for error conditions ***\n";

echo "\n-- No filename --\n";
var_dump( sha1_file("") );

echo "\n-- invalid filename --\n";
var_dump( sha1_file("rewncwYcn89q") );

echo "\n-- Zero arguments --\n";
 try { var_dump ( sha1_file() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- More than valid number of arguments ( valid is 2) --\n";
try { var_dump ( sha1_file($empty_file, true, NULL) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Hexadecimal Output for Empty file as Argument --\n";
var_dump( sha1_file($empty_file) );

echo "\n-- Raw Binary Output for Empty file as Argument --\n";
var_dump( bin2hex(sha1_file($empty_file, true)));

echo "\n-- Hexadecimal Output for a valid file with some contents --\n";
var_dump( sha1_file($data_file) );

echo "\n-- Raw Binary Output for a valid file with some contents --\n";
var_dump ( bin2hex(sha1_file($data_file, true)));

// remove temp files
unlink($data_file);
unlink($empty_file);

echo "===DONE===\n";
}
