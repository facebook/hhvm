<?php
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function by passing heredoc string containing quote chars for haystack
 *  and with various needles
*/

echo "*** Testing strrchr() function: with heredoc strings ***\n";
$quote_char_str = <<<EOD
"things" "in" "double" "quote"
'things' 'in' 'single' 'quote'
EOD;

$needles = array(
  "things",
  "\"things\"",
  "\'things\'",
  "in",
  "quote",
  $quote_char_str //needle as haystack
);

//loop through to test strrchr() with each needle
foreach($needles as $needle) {
  var_dump( strrchr($quote_char_str, $needle) );
}
echo "*** Done ***";
?>