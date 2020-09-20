<?hh
/* Prototype  : string strtok ( string $str, string $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() for error conditions
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strtok() : error conditions ***\n";

// Zero argument
echo "\n-- Testing strtok() function with Zero arguments --\n";
try { var_dump( strtok() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// More than expected number of arguments
echo "\n-- Testing strtok() function with more than expected no. of arguments --\n";
$str = 'sample string';
$token = ' ';
$extra_arg = 10;

try { var_dump( strtok($str, $token, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump( $str );

// Less than expected number of arguments 
echo "\n-- Testing strtok() with less than expected no. of arguments --\n";
$str = 'string val';
 
var_dump( strtok($str));
var_dump( $str );

echo "Done\n";
}
