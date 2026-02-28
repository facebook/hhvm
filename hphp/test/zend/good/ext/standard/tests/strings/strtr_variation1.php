<?hh
/* Prototype  : string strtr(string $str, string $from[, string $to]);
                string strtr(string $str, array $replace_pairs);
 * Description: Translates characters in str using given translation tables
 * Source code: ext/standard/string.c
*/

/* Testing strtr() function by passing the
 *   combination of numeric & regular strings for 'str' argument and
 *   corresponding translation pair of chars for 'from', 'to' & 'replace_pairs' arguments
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strtr() : numeric & regular double quoted strings ***\n";
/* definitions of required input variables */
$count = 1;
$heredoc_str = <<<EOD
123
abc
1a2b3c
EOD;
//array of string inputs for $str
$str_arr = vec[
  //double quoted strings
  "123",
  "abc",
  "1a2b3c",

  //single quoted strings
  '123',
  'abc',
  '1a2b3c',

  //heredoc string
  $heredoc_str
];
$from = "123abc";
$to = "abc123";
$replace_pairs = dict["1" => "a", "a" => 1, "2b3c" => "b2c3", "b2c3" => "3c2b"];

/* loop through to test strtr() with each element of $str_arr */
for($index = 0; $index < count($str_arr); $index++) {
  echo "-- Iteration $count --\n";

  $str = $str_arr[$index];  //getting the $str_arr element in $str variable

  //strtr() call in three args syntax form
  var_dump( strtr($str, $from, $to) );

  //strtr() call in two args syntax form
  var_dump( strtr($str, $replace_pairs) );

  $count++;
}
echo "*** Done ***";
}
