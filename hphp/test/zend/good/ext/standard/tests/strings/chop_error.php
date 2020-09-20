<?hh
/* Prototype  : string chop ( string $str [, string $charlist] )
 * Description: Strip whitespace (or other characters) from the end of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing chop() : error conditions
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chop() : error conditions ***\n";

// Zero argument
echo "\n-- Testing chop() function with Zero arguments --\n";
try { var_dump( chop() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// More than expected number of arguments
echo "\n-- Testing chop() function with more than expected no. of arguments --\n";
$str = 'string_val ';
$charlist = 'string_val';
$extra_arg = 10;

try { var_dump( chop($str, $charlist, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump( $str );

echo "Done\n";
}
