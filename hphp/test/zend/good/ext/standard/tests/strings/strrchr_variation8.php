<?hh
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function by passing empty heredoc string for haystack
 *  and with various needles
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strrchr() function: with heredoc strings ***\n";
$empty_str = <<<EOD
EOD;

$needles = vec[
  "",
  '',
  FALSE,
  NULL,
  "\0",
  $empty_str //needle as haystack
];

//loop through to test strrchr() with each needle
foreach($needles as $needle) {
  var_dump( strrchr($empty_str, $needle) );
}
echo "*** Done ***";
}
