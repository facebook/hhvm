<?hh
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/

/*
 * testing functionality of strip_tags() by giving valid value for $str and invalid values for $allowable_tags argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strip_tags() : usage variations ***\n";

$strings = "<html>hello</html> \tworld... <p>strip_tags_test\v\f</p><?hh hello\t wo\rrld?>";

$quotes = vec[
  "<nnn>",
  '<nnn>',
  "<abc>",
  '<abc>',
  "<%?php",
  '<%?php',
  "<<html>>",
  '<<html>>'
];

//loop through the various elements of strings array to test strip_tags() functionality
$iterator = 1;
foreach($quotes as $string_value)
{
      echo "-- Iteration $iterator --\n";
      var_dump( strip_tags($strings, $string_value) );
      $iterator++;
}

echo "Done";
}
