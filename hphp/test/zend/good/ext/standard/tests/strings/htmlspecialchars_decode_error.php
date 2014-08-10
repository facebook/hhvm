<?php
/* Prototype  : string htmlspecialchars_decode(string $string [, int $quote_style])
 * Description: Convert special HTML entities back to characters 
 * Source code: ext/standard/html.c
*/

echo "*** Testing htmlspecialchars_decode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing htmlspecialchars_decode() function with Zero arguments --\n";
var_dump( htmlspecialchars_decode() );

//Test htmlspecialchars_decode with one more than the expected number of arguments
echo "\n-- Testing htmlspecialchars_decode() function with more than expected no. of arguments --\n";
$string = "<html>hello &amp; &gt; &lt; &quot; &#039; world</html>";
$quote_style = ENT_COMPAT;
$extra_arg = 10;
var_dump( htmlspecialchars_decode($string, $quote_style, $extra_arg) );

echo "Done";
?>
