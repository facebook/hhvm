<?hh
/* Prototype  : string ucwords ( string $str )
 * Description: Uppercase the first character of each word in a string
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing ucwords() : error conditions ***\n";

// Zero argument
echo "\n-- Testing ucwords() function with Zero arguments --\n";
try { var_dump( ucwords() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// More than expected number of arguments
echo "\n-- Testing ucwords() function with more than expected no. of arguments --\n";
$str = 'string_val';
$extra_arg = 10;

try { var_dump( ucwords($str, $extra_arg, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// check if there were any changes made to $str
var_dump($str);

echo "Done\n";
}
