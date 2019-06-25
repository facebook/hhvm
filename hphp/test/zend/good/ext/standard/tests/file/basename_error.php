<?hh
/* Prototype  : string basename(string path [, string suffix])
 * Description: Returns the filename component of the path 
 * Source code: ext/standard/string.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing basename() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing basename() function with Zero arguments --\n";
try { var_dump( basename() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test basename with one more than the expected number of arguments
echo "\n-- Testing basename() function with more than expected no. of arguments --\n";
$path = 'string_val';
$suffix = 'string_val';
$extra_arg = 10;
try { var_dump( basename($path, $suffix, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
