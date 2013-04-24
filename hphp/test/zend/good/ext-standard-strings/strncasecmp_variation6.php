<?php
/* Prototype  : int strncasecmp ( string $str1, string $str2, int $len );
 * Description: Binary safe case-insensitive string comparison of the first n characters
 * Source code: Zend/zend_builtin_functions.c
*/

/* Test strncasecmp() function with binary values passed to 'str1' & 'str2' */

echo "*** Test strncasecmp() function: with binary inputs ***\n";

/* A binary function should work with all 256 characters that a character(8-bit) can take */
echo "\n-- Checking with all 256 characters given, in binary format --\n";
/* loop through to get all 256 character's equivelent binary value, and check working of strncasecmp() */
$count = 1;
for($ASCII = 0; $ASCII <= 255; $ASCII++) {
  $str1 = decbin($ASCII);  //ASCII value in binary form
  $str2 = decbin( ord( chr($ASCII) ) );  //Getting equivelent ASCII value for the character in binary form
  echo "-- Iteration $count --\n";
  var_dump( strncasecmp($str1, $str2, 8) );  //comparing all the 8-bits; expected: int(0)
  var_dump( strncasecmp($str1, $str2, 4) );  //comparing only 4-bits; expected: int(0)
  $count++;
}

echo "\n-- Checking with out of character's range, given in binary format --\n";
$str1 = decbin(256);
$str2 = decbin( ord( chr(256) ));
var_dump( strncasecmp($str1, $str2, 8) );  //comparing all the 8-bits; expected: int(1)

echo "\n*** Done ***\n";
?>