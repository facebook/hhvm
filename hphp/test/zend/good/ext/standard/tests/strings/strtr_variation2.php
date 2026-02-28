<?hh
/* Prototype  : string strtr(string $str, string $from[, string $to]);
                string strtr(string $str, array $replace_pairs);
 * Description: Translates characters in str using given translation tables
 * Source code: ext/standard/string.c
*/

/* Testing strtr() function by passing the
 *   string containing various special characters for 'str' argument and
 *   corresponding translation pair of chars for 'from', 'to' & 'replace_pairs' arguments
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strtr() : string containing special chars for 'str' arg ***\n";

/* definitions of required input variables */
$count = 1;

$heredoc_str = <<<EOD
%
#$*&
text & @()
EOD;

//array of string inputs for $str
$str_arr = vec[
  //double quoted strings
  "%",
  "#$*",
  "text & @()",

  //single quoted strings
  '%',
  '#$*',
  'text & @()',

  //heredoc string
  $heredoc_str
];

$from = "%#$*&@()";
$to = "specials";
$replace_pairs = dict["$" => "%", "%" => "$", "#*&@()" => "()@&*#"];

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
}
