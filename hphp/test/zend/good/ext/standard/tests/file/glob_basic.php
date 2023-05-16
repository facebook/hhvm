<?hh
/* Prototype: array glob ( string $pattern [, int $flags] );
   Description: Find pathnames matching a pattern
*/

function sort_var_dump($results) {
   sort(inout $results);
   var_dump($results);
}
<<__EntryPoint>> function main(): void {
echo "*** Testing glob() : basic functions ***\n";

// temp dirname used here
$dirname = sys_get_temp_dir().'/'.'glob_basic';

// temp dir created
mkdir($dirname);

// temp files created
$fp = fopen("$dirname/wonder12345", "w");
fclose($fp);
$fp = fopen("$dirname/wonder.txt", "w");
fclose($fp);
$fp = fopen("$dirname/file.text", "w");
fclose($fp);

// glob() with default arguments
sort_var_dump( glob($dirname."/*") );
sort_var_dump( glob($dirname."/*.txt") );
sort_var_dump( glob($dirname."/*.t?t") );
sort_var_dump( glob($dirname."/*.t*t") );
sort_var_dump( glob($dirname."/*.?") );
sort_var_dump( glob($dirname."/*.*") );

echo "Done\n";

unlink("$dirname/wonder12345");
unlink("$dirname/wonder.txt");
unlink("$dirname/file.text");
rmdir($dirname);
}
