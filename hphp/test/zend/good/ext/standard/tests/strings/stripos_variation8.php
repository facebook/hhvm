<?hh
/* Prototype  : int stripos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of first occurrence of a case-insensitive string
 * Source code: ext/standard/string.c
*/

/* Test stripos() function with strings containing repetitive chars for haystak
 *  and with various needles & offsets 
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing stripos() function: strings repetitive chars ***\n";
$haystack = "aBAbaBAbaBabAbAbaBa";
$needles = vec[
  "aba",
  "aBA",
  "ABA",
  "Aba",
  "BAb",
  "bab",
  "bAb",
  "BAB"
];

/* loop through to consider various offsets in getting the position of the needle in haystack string */
$count = 1;
for($index = 0; $index < count($needles); $index++) {
  echo "\n-- Iteration $count --\n";
  for($offset = 0; $offset <= strlen($haystack); $offset++ ) {
    var_dump( stripos($haystack, $needles[$index], $offset) );
  }
  $count++;
}
echo "*** Done ***";
}
