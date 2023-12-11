<?hh
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function by passing multi-line heredoc string for haystack and
 *    with various needles
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strrchr() function: with heredoc strings ***\n";
$multi_line_str = <<<EOD
Example of string
spanning multiple lines
using heredoc syntax.
EOD;

$needles = vec[
  "ing", 
  "", 
  " ",
  $multi_line_str //needle as haystack 
];

//loop through to test strrchr() with each needle
foreach($needles as $needle) {  
  var_dump( strrchr($multi_line_str, $needle) );
}

echo "*** Done ***";
}
