<?php
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/

echo "*** Testing strip_tags() : basic functionality ***\n";

// Calling strip_tags() with all possible arguments
$string = "<html><p>hello</p><b>world</b><a href=\"#fragment\">Other text</a></html><?php echo hello ?>";

$allowed_tags_array=array(
  "<html>",
  '<html>',
  "<p>",
  '<p>',
  "<a>",
  '<a>',
  "<?php",
  '<?php',
  "<html><p><a><?php"
);

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
?>
