<?hh
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string 
 * Source code: ext/standard/string.c
*/

<<__EntryPoint>> function main(): void {
echo "*** Testing strip_tags() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing strip_tags() function with Zero arguments --\n";
try { var_dump( strip_tags() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test strip_tags with one more than the expected number of arguments
echo "\n-- Testing strip_tags() function with more than expected no. of arguments --\n";
$str = "<html>hello</html>";
$allowable_tags = "<html>";
$extra_arg = 10;
try { var_dump( strip_tags($str, $allowable_tags, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
