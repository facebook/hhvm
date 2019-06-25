<?hh

/* Prototype: string sha1_file( string filename[, bool raw_output] )
 * Description: Calculate the sha1 hash of a file
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing sha1_file() : basic functionality ***\n";

/* Creating an empty file */
if (($handle = fopen( "sha1_EmptyFile.txt", "w+")) == FALSE)
return false;

/* Creating a data file */
if (($handle2 = fopen( "sha1_DataFile.txt", "w+")) == FALSE)
return false;

/* Writing into file */
$filename = "sha1_DataFile.txt";
$content = b"Add this to the file\n";
if (is_writable($filename)) {
  if (fwrite($handle2, $content) === FALSE) {
    echo "Cannot write to file ($filename)";
    exit;
  }
}

// close the files
fclose($handle);
fclose($handle2);

/* Testing error conditions */
echo "\n*** Testing for error conditions ***\n";

echo "\n-- No filename --\n";
var_dump( sha1_file("") );

echo "\n-- invalid filename --\n";
var_dump( sha1_file("rewncwYcn89q") );

echo "\n-- Zero arguments --\n";
 try { var_dump ( sha1_file() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- More than valid number of arguments ( valid is 2) --\n";
try { var_dump ( sha1_file("sha1_EmptyFile.txt", true, NULL) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Hexadecimal Output for Empty file as Argument --\n";
var_dump( sha1_file("sha1_EmptyFile.txt") );

echo "\n-- Raw Binary Output for Empty file as Argument --\n";
var_dump( bin2hex(sha1_file("sha1_EmptyFile.txt", true)));

echo "\n-- Hexadecimal Output for a valid file with some contents --\n";
var_dump( sha1_file("sha1_DataFile.txt") );

echo "\n-- Raw Binary Output for a valid file with some contents --\n";
var_dump ( bin2hex(sha1_file("sha1_DataFile.txt", true)));

// remove temp files
unlink("sha1_DataFile.txt");
unlink("sha1_EmptyFile.txt");

echo "===DONE===\n";
}
