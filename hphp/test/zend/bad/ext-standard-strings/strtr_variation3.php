<?php
/* Prototype  : string strtr(string $str, string $from[, string $to]);
                string strtr(string $str, array $replace_pairs);
 * Description: Translates characters in str using given translation tables
 * Source code: ext/standard/string.c
*/

/* Testing strtr() function by passing the
 *   string containing various escape sequences for 'str' argument and
 *   corresponding translation pair of chars for 'from', 'to' & 'replace_pairs' arguments
*/

echo "*** Testing strtr() : string containing escape sequences for 'str' arg ***\n";
/* definitions of required input variables */
$count = 1;

$heredoc_str = <<<EOD
\tes\t\\stt\r
\\test\\\strtr
\ntest\r\nstrtr
\$variable
\"quotes
EOD;

//array of string inputs for $str
$str_arr = array(
  //double quoted strings
  "\tes\t\\stt\r",
  "\\test\\\strtr",
  "\ntest\r\nstrtr",
  "\$variable",
  "\"quotes",

  //single quoted strings
  '\tes\t\\stt\r',
  '\\test\\\strtr',
  '\ntest\r\nstrtr',
  '\$variable',
  '\"quotes',

  //heredoc string
  $heredoc_str
);

$from = "\n\r\t\\";
$to = "TEST";
$replace_pairs = array("\n" => "t", "\r\n" => "T", "\n\r\t\\" => "TEST");

/* loop through to test strtr() with each element of $str_arr */
for($index = 0; $index < count($str_arr); $index++) {
  echo "-- Iteration $count --\n";

  $str = $str_arr[$index];  //getting the array element in 'str' variable

  //strtr() call in three args syntax form
  var_dump( strtr($str, $from, $to) );

  //strtr() call in two args syntax form
  var_dump( strtr($str, $replace_pairs) );

  $count++;
}
echo "*** Done ***";
?>