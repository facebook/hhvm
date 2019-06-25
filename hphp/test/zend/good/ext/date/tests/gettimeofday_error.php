<?hh
/* Prototype  : array gettimeofday([bool get_as_float])
 * Description: Returns the current time as array 
 * Source code: ext/standard/microtime.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing gettimeofday() : error conditions ***\n";


//Test gettimeofday with one more than the expected number of arguments
echo "\n-- Testing gettimeofday() function with more than expected no. of arguments --\n";
$get_as_float = true;
$extra_arg = 10;
try { var_dump( gettimeofday($get_as_float, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
