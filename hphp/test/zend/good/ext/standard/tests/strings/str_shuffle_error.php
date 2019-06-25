<?hh

/* Prototype  : string str_shuffle  ( string $str  )
 * Description: Randomly shuffles a string
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing str_shuffle() : error conditions ***\n";
echo "\n-- Testing str_shuffle() function with no arguments --\n";
try { var_dump( str_shuffle() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing str_shuffle() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( str_shuffle("Hello World", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
