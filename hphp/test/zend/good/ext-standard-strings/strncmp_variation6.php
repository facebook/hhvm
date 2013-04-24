<?php
/* Prototype  : int strncmp ( string $str1, string $str2, int $len );
 * Description: Binary safe case-sensitive string comparison of the first n characters
 * Source code: Zend/zend_builtin_functions.c
*/

/* Test strncmp() function with binary values passed to 'str1' & 'str2' and with the null terminated strings */

echo "*** Test strncmp() function: with binary inputs ***\n";

echo "\n-- Checking with all 256 characters given, in binary format --\n";
/* A binary function should work with all 256 characters that a character(8-bit) can take */
/* loop through to get all 256 character's equivelent binary value, and check working of strncmp() */
$count = 1;
for($ASCII = 0; $ASCII <= 255; $ASCII++) {
  $str1 = decbin($ASCII);  //ASCII value in binary form
  $str2 = decbin( ord( chr($ASCII) ) );  //Getting equivelent ASCII value for the character in binary form
  echo "-- Iteration $count --\n";
  var_dump( strncmp($str1, $str2, 8) );  //comparing all the 8-bits; expected: int(0)
  var_dump( strncmp($str1, $str2, 4) );  //comparing only 4-bits; expected: int(0)
  $count++;
}

echo "\n-- Checking with out of character's range, given in binary format --\n";
/* Checking with the out of range ASCII value(given in binary format) */ 
$str1 = decbin(256);
$str2 = decbin( ord( chr(256) ));
var_dump( strncmp($str1, $str2, 8) );  //comparing all the 8-bits; expected: int(1)

echo "\n*** Done ***\n";
?>