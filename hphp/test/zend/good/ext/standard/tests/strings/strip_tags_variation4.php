<?hh
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/

/*
 * testing functionality of strip_tags() by giving invalid values for $str and valid values for $allowable_tags argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strip_tags() : usage variations ***\n";

// unexpected values for $string
$strings = vec[
  "<abc>hello</abc> \t\tworld... <ppp>strip_tags_test</ppp>",
  '<abc>hello</abc> \t\tworld... <ppp>strip_tags_test</ppp>',
  "<%?php hello\t world?%>",
  '<%?php hello\t world?%>',
  "<<htmL>>hello<</htmL>>",
  '<<htmL>>hello<</htmL>>',
  "<a.>HtMl text</.a>",
  '<a.>HtMl text</.a>',
  "<nnn>I am not a valid html text</nnn>",
  '<nnn>I am not a valid html text</nnn>',
  "<nnn>I am a quoted (\") string with special chars like \$,\!,\@,\%,\&</nnn>",
  '<nnn>I am a quoted (\") string with special chars like \$,\!,\@,\%,\&</nnn>',
];

//valid html and php tags
$quotes = "<p><a><?hh<html>";

//loop through the various elements of strings array to test strip_tags() functionality
$iterator = 1;
foreach($strings as $string_value)
{
      echo "-- Iteration $iterator --\n";
      var_dump( strip_tags($string_value, $quotes) );
      $iterator++;
}

echo "Done";
}
