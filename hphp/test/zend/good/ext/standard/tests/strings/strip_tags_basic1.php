<?php
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string 
 * Source code: ext/standard/string.c
*/

echo "*** Testing strip_tags() : basic functionality ***\n";

// array of arguments 
$string_array = array (
  "<html>hello</html>",
  '<html>hello</html>',
  "<?php echo hello ?>",
  '<?php echo hello ?>',
  "<? echo hello ?>",
  '<? echo hello ?>',
  "<% echo hello %>",
  '<% echo hello %>',
  "<script language=\"PHP\"> echo hello </script>",
  '<script language=\"PHP\"> echo hello </script>',
  "<html><b>hello</b><p>world</p></html>",
  '<html><b>hello</b><p>world</p></html>',
  "<html><!-- COMMENT --></html>",
  '<html><!-- COMMENT --></html>'
);
  
  		
// Calling strip_tags() with default arguments
// loop through the $string_array to test strip_tags on various inputs
$iteration = 1;
foreach($string_array as $string)
{
  echo "-- Iteration $iteration --\n";
  var_dump( strip_tags($string) );
  $iteration++;
}

echo "Done";
?>