<?hh
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function by passing heredoc string containing 
 *  blank-line for haystack and with various needles
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strrchr() function: with heredoc strings ***\n";
$blank_line = <<<EOD

EOD;

$needles = vec[
  "\n",
  '\n',
  "\r",
  "\r\n",
  "\t",
  "",
  $blank_line //needle as haystack
];

//loop through to test strrchr() with each needle
foreach($needles as $needle) {
  var_dump( strrchr($blank_line, $needle) );
}
echo "*** Done ***";
}
