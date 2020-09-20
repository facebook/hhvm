<?hh

/* Prototype  : string stripcslashes  ( string $str  )
 * Description: Returns a string with backslashes stripped off. Recognizes C-like \n, \r ..., 
 *              octal and hexadecimal representation.
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing stripcslashes() : unexpected number of arguments ***";


echo "\n-- Testing stripcslashes() function with no arguments --\n";
try { var_dump( stripcslashes() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing stripcslashes() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( stripcslashes("abc def", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
