<?php
/* Prototype  : string htmlspecialchars_decode(string $string [, int $quote_style])
 * Description: Convert special HTML entities back to characters
 * Source code: ext/standard/html.c
*/

/*
 * testing whether htmlspecialchars_decode() is binary safe or not
*/

echo "*** Testing htmlspecialchars_decode() : usage variations ***\n";

//various string inputs
$strings = array (
  "\tHello \$world ".chr(0)."\&!)The big brown fox jumped over the\t\f lazy dog\v\n",
  "\tHello \"world\"\t\v \0 This is a valid\t string",
  "This converts\t decimal to \$string".decbin(65)."Hello world",
  b"This is a binary\t \v\fstring"
);

//loop through the strings array to check if htmlspecialchars_decode() is binary safe
$iterator = 1;
foreach($strings as $value) {
      echo "-- Iteration $iterator --\n";
      if ($iterator < 4) {
      	var_dump( htmlspecialchars_decode($value) );
      } else {
      	var_dump( bin2hex(htmlspecialchars_decode($value)));      
      }
      
      $iterator++;
}

echo "Done";
?>