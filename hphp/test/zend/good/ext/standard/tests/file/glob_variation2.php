<?hh
/* Prototype: array glob ( string $pattern [, int $flags] );
   Description: Find pathnames matching a pattern
*/
<<__EntryPoint>> function main(): void {
$file_path = dirname(__FILE__);

// temp dirname used here
$dir_name = 'glob_test';

// create temp directory
mkdir("$file_path/$dir_name");

// create temp file
$fp = fopen("$file_path/$dir_name/file.text", "w");
fclose($fp);

echo "Testing glob() with relative paths:\n";

chdir("$file_path/$dir_name");
var_dump( glob("./*") );
var_dump( glob("../$dir_name/*"));

chdir("$file_path");
var_dump( glob("$dir_name/*"));
var_dump( glob("$dir_name"));

echo "Done\n";
error_reporting(0);
$file_path = dirname(__FILE__);
unlink("$file_path/glob_test/file.text");
rmdir("$file_path/glob_test/");
}
