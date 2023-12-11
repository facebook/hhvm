<?hh
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function by passing heredoc string containing special chars for haystack
 * and with various needles 
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strrchr() function: with heredoc strings ***\n";
$special_chars_str = <<<EOD
Example of heredoc string contains
$#%^*&*_("_")!#@@!$#$^^&*(special)
chars.
EOD;

$heredoc_needle = <<<EOD
^^&*(
EOD;

$needles = vec[
  "!@@!",
  '_',
  '("_")',
  "$*",
  "(special)",
  $heredoc_needle,  //needle as heredoc string
  $special_chars_str  //needle as haystack
];

//loop through to test strrchr() with each needle
foreach($needles as $needle) {
  var_dump( strrchr($special_chars_str, $needle) );
}
echo "*** Done ***";
}
