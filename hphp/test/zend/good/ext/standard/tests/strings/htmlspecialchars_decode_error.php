<?hh
/* Prototype  : string htmlspecialchars_decode(string $string [, int $quote_style])
 * Description: Convert special HTML entities back to characters 
 * Source code: ext/standard/html.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing htmlspecialchars_decode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing htmlspecialchars_decode() function with Zero arguments --\n";
try { var_dump( htmlspecialchars_decode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test htmlspecialchars_decode with one more than the expected number of arguments
echo "\n-- Testing htmlspecialchars_decode() function with more than expected no. of arguments --\n";
$string = "<html>hello &amp; &gt; &lt; &quot; &#039; world</html>";
$quote_style = ENT_COMPAT;
$extra_arg = 10;
try { var_dump( htmlspecialchars_decode($string, $quote_style, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
