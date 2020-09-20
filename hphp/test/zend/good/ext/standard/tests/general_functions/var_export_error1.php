<?hh
/* Prototype  : mixed var_export(mixed var [, bool return])
 * Description: Outputs or returns a string representation of a variable 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing var_export() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing var_export() function with Zero arguments --\n";
try { var_dump( var_export() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test var_export with one more than the expected number of arguments
echo "\n-- Testing var_export() function with more than expected no. of arguments --\n";
$var = 1;
$return = true;
$extra_arg = 10;
try { var_dump( var_export($var, $return, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
