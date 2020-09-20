<?hh

/* Prototype  : mixed count_chars  ( string $string  [, int $mode  ] )
 * Description: Return information about characters used in a string
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing count_chars() : error conditions ***\n";

echo "\n-- Testing count_chars() function with no arguments --\n";
try { var_dump( count_chars() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing count_chars() function with more than expected no. of arguments --\n";
$string = "Hello World\n"; 
$mode = 1;
$extra_arg = 10;
try { var_dump( count_chars($string, $mode, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
