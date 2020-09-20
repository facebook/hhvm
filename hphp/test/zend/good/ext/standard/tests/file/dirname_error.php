<?hh
/* Prototype  : string dirname(string path)
 * Description: Returns the directory name component of the path 
 * Source code: ext/standard/string.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing dirname() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing dirname() function with Zero arguments --\n";
try { var_dump( dirname() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test dirname with one more than the expected number of arguments
echo "\n-- Testing dirname() function with more than expected no. of arguments --\n";
$path = 'string_val';
$extra_arg = 10;
try { var_dump( dirname($path, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
