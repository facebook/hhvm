<?hh
/* Prototype  : proto int similar_text(string str1, string str2 [, float percent])
* Description: Calculates the similarity between two strings
* Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
  $percent = -1.0;
$extra_arg = 10;
echo "\n-- Testing similar_text() function with more than expected no. of arguments --\n";
try { similar_text("abc", "def", inout $percent, $extra_arg); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing similar_text() function with less than expected no. of arguments --\n";
try { similar_text("abc"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
