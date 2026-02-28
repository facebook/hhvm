<?hh
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strip_tags() : basic functionality ***\n";

// Calling strip_tags() with all possible arguments
$string = "<html><p>hello</p><b>world</b><a href=\"#fragment\">Other text</a></html><?hh echo hello ?>";

$allowed_tags_array=vec[
  "<html>",
  '<html>',
  "<p>",
  '<p>',
  "<a>",
  '<a>',
  "<?hh",
  '<?hh',
  "<html><p><a><?hh"
];

// loop through the $string with various $allowed_tags_array to test strip_tags
// on various allowed tags
$iteration = 1;
foreach($allowed_tags_array as $tags)
{
  echo "-- Iteration $iteration --\n";
  var_dump( strip_tags($string, $tags) );
  $iteration++;
}

echo "Done";
}
