<?php
/* Prototype  : mixed substr_replace  ( mixed $string  , string $replacement  , int $start  [, int $length  ] )
 * Description: Replace text within a portion of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing substr_replace() for error conditions
*/

echo "*** Testing substr_replace() : error conditions ***\n";

$s1 = "Good morning";

echo "\n-- Testing substr_replace() function with less than expected no. of arguments --\n";
var_dump(substr_replace());
var_dump(substr_replace($s1, "evening"));

echo "\n-- Testing substr_replace() function with more than expected no. of arguments --\n";
var_dump(substr_replace($s1, "evening", 5, 7, true));

echo "\n-- Testing substr_replace() function with start and length different types --\n";
var_dump(substr_replace($s1, "evening", array(5))); 
var_dump(substr_replace($s1, "evening", 5, array(8))); 
  
echo "\n-- Testing substr_replace() function with start and length with a different number of elements --\n";
var_dump(substr_replace($s1, "evening", array(5, 1), array(8))); 

echo "\n-- Testing substr_replace() function with start and length as arrays but string not--\n";
var_dump(substr_replace($s1, "evening", array(5), array(8))); 
  
?>
===DONE===