<?hh
/* Prototype  : string addslashes ( string $str )
 * Description: Returns a string with backslashes before characters that need to be quoted in database queries etc.
 * Source code: ext/standard/string.c
*/

/*
 * Testing addslashes() for error conditions
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing addslashes() : error conditions ***\n";

// Zero argument
echo "\n-- Testing addslashes() function with Zero arguments --\n";
try { var_dump( addslashes() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// More than expected number of arguments
echo "\n-- Testing addslashes() function with more than expected no. of arguments --\n";
$str = '"hello"\"world"';
$extra_arg = 10;

try { var_dump( addslashes($str, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump( $str );

echo "Done\n";
}
