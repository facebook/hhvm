<?hh
/* Prototype  : string stripslashes ( string $str )
 * Description: Returns an un-quoted string
 * Source code: ext/standard/string.c
*/

/*
 * Testing stripslashes() for error conditions
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing stripslashes() : error conditions ***\n";

// Zero argument
echo "\n-- Testing stripslashes() function with Zero arguments --\n";
try { var_dump( stripslashes() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// More than expected number of arguments
echo "\n-- Testing stripslashes() function with more than expected no. of arguments --\n";
$str = '\"hello\"\"world\"';
$extra_arg = 10;

try { var_dump( stripslashes($str, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump( $str );

echo "Done\n";
}
