<?hh
/* Prototype  : string strtr(string $str, string $from[, string $to]);
                string strtr(string $str, array $replace_pairs);
 * Description: Translates characters in str using given translation tables
 * Source code: ext/standard/string.c
*/

/* Testing strtr() function by passing the
 *   empty string & null for 'str' argument and
 *   corresponding translation pair of chars for 'from', 'to' & 'replace_pairs' arguments
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strtr() : empty string & null for 'str' arg ***\n";
/* definitions of required input variables */
$count = 1;

$heredoc_str = <<<EOD

EOD;

//array of string inputs for $str
$str_arr = vec[
  "",
  '',
  NULL,
  null,
  FALSE,
  false,
  $heredoc_str
];

$from = "";
$to = "TEST";
$replace_pairs = dict["" => "t", '' => "TEST"];


/* loop through to test strtr() with each element of $str_arr */
for($index = 0; $index < count($str_arr); $index++) {
  echo "-- Iteration $count --\n";

  $str = $str_arr[$index];  //getting the array element in 'str' variable

  //strtr() call in three args syntax form
  try { var_dump( strtr($str, $from, $to) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  //strtr() call in two args syntax form
  try { var_dump( strtr($str, $replace_pairs) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  $count++;
}
echo "*** Done ***";
}
