<?hh
/* Prototype  : bool chmod(string filename, int mode)
 * Description: Change file mode 
 * Source code: ext/standard/filestat.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing chmod() : error conditions ***\n";


//Test chmod with one more than the expected number of arguments
echo "\n-- Testing chmod() function with more than expected no. of arguments --\n";
$filename = 'string_val';
$mode = 10;
$extra_arg = 10;
try { var_dump( chmod($filename, $mode, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing chmod with one less than the expected number of arguments
echo "\n-- Testing chmod() function with less than expected no. of arguments --\n";
$filename = 'string_val';
try { var_dump( chmod($filename) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// testing chmod with a non-existing file
$filename = "___nonExisitingFile___";
var_dump(chmod($filename, 0777));

echo "===DONE===\n";
}
