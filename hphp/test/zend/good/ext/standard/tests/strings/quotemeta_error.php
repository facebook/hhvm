<?hh

/* Prototype  : string quotemeta  ( string $str  )
 * Description: Quote meta characters
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing quotemeta() : error conditions ***\n";

echo "\n-- Testing quotemeta() function with no arguments --\n";
try { var_dump( quotemeta()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing quotemeta() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump(quotemeta("How are you ?", $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
