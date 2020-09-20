<?hh
/* Prototype  : proto bool file_exists(string filename)
 * Description: Returns true if filename exists 
 * Source code: ext/standard/filestat.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing file_exists() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing file_exists() function with Zero arguments --\n";
try { var_dump( file_exists() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test file_exists with one more than the expected number of arguments
echo "\n-- Testing file_exists() function with more than expected no. of arguments --\n";
$filename = 'string_val';
$extra_arg = 10;
try { var_dump( file_exists($filename, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
