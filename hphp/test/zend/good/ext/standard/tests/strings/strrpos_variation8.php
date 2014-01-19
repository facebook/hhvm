<?php
/* Prototype  : int strrpos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of 'needle' in 'haystack'.
 * Source code: ext/standard/string.c
*/

/* Test strrpos() function with strings containing multiple occurrences of 'needle' in the 'haystack'
 *  and with various needles & offsets 
*/

echo "*** Testing strrpos() function: strings repetitive chars ***\n";
$haystack = "ababababAbaBa";
$needle = "aba";

/* loop through to consider various offsets in getting the position of the needle in haystack string */
$count = 1;
for($offset = -1; $offset <= strlen($haystack); $offset++ ) {
  echo "-- Iteration $count --\n";
  var_dump( strrpos($haystack, $needle, $offset) );
  $count++;
}
echo "*** Done ***";
?>