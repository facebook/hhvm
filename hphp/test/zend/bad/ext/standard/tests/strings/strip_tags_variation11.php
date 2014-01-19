<?php

echo "*** Testing strip_tags() : obscure functionality ***\n";

// array of arguments 
$string_array = array (
  'hello <img title="<"> world',
  'hello <img title=">"> world',
  'hello <img title=">_<"> world',
  "hello <img title='>_<'> world"
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