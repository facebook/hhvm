<?hh
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/

/*
 * testing functionality of strip_tags() by giving single quoted strings as values for $str argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strip_tags() : usage variations ***\n";


$single_quote_string = vec[
  '<html> \$ -> This represents the dollar sign</html><?hh echo hello ?>',
  '<html>\t\r\v The quick brown fo\fx jumped over the lazy dog</p>',
  '<a>This is a hyper text tag</a>',
  '<? <html>hello world\\t</html>?>',
  '<p>This is a paragraph</p>',
  '<b>This is \ta text in bold letters\r\s\malong with slashes\n</b>'
];

$quotes = "<html><a><p><b><?hh";

//loop through the various elements of strings array to test strip_tags() functionality
$iterator = 1;
foreach($single_quote_string as $string_value)
{
      echo "-- Iteration $iterator --\n";
      var_dump( strip_tags($string_value, $quotes) );
      $iterator++;
}

echo "Done";
}
