<?hh
/* Prototype  : string join( string $glue, array $pieces )
 * Description: Join array elements with a string
 * Source code: ext/standard/string.c
 * Alias of function: implode()
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing join() : error conditions ***\n";

// Zero argument
echo "\n-- Testing join() function with Zero arguments --\n";
try { var_dump( join() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// More than expected number of arguments
echo "\n-- Testing join() function with more than expected no. of arguments --\n";
$glue = 'string_val';
$pieces = vec[1, 2];
$extra_arg = 10;

try { var_dump( join($glue, $pieces, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Less than expected number of arguments 
echo "\n-- Testing join() with less than expected no. of arguments --\n";
$glue = 'string_val';
 
var_dump( join($glue));

echo "Done\n";
}
