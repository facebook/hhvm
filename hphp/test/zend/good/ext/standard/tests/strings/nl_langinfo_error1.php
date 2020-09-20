<?hh

/* Prototype  : string nl_langinfo  ( int $item  )
 * Description: Query language and locale information
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing nl_langinfo() : error conditions ***\n";

echo "\n-- Testing nl_langinfo() function with no arguments --\n";
try { var_dump( nl_langinfo() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing nl_langinfo() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( nl_langinfo(ABDAY_2, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
