<?hh
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/

/*
 * testing whether strip_tags() is binary safe or not
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strip_tags() : usage variations ***\n";

//various string inputs
$strings = vec[
  "<html> I am html string </html>".chr(0)."<?hh I am php string ?>",
  "<html> I am html string\0 </html><?hh I am php string ?>",
  b"<a>I am html string</a>",
  "<html>I am html string</html>".decbin(65)."<?hh I am php string?>"
];

//loop through the strings array to check if strip_tags() is binary safe
$iterator = 1;
foreach($strings as $value)
{
      echo "-- Iteration $iterator --\n";
      var_dump( strip_tags($value) );
      $iterator++;
}

echo "Done";
}
