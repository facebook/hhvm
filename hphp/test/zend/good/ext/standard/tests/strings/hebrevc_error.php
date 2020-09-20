<?hh

/* Prototype  : string hebrevc  ( string $hebrew_text  [, int $max_chars_per_line  ] )
 * Description: Convert logical Hebrew text to visual text
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing hebrevc() : error conditions ***\n";

echo "\n-- Testing hebrevc() function with no arguments --\n";
try { var_dump( hebrevc() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing hebrevc() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( hebrevc("Hello World", 5, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
