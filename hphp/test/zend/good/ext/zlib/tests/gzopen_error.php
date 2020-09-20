<?hh
/* Prototype  : resource gzopen(string filename, string mode [, int use_include_path])
 * Description: Open a .gz-file and return a .gz-file pointer 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing gzopen() : error conditions ***\n";


//Test gzopen with one more than the expected number of arguments
echo "\n-- Testing gzopen() function with more than expected no. of arguments --\n";
$filename = 'string_val';
$mode = 'string_val';
$use_include_path = 10;
$extra_arg = 10;
try { var_dump( gzopen($filename, $mode, $use_include_path, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing gzopen with one less than the expected number of arguments
echo "\n-- Testing gzopen() function with less than expected no. of arguments --\n";
$filename = 'string_val';
try { var_dump( gzopen($filename) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
