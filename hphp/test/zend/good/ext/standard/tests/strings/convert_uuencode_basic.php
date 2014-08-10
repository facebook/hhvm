<?php

/* Prototype  : string convert_uuencode  ( string $data  )
 * Description: Uuencode a string
 * Source code: ext/standard/uuencode.c
*/

echo "*** Testing convert_uuencode() : basic functionality ***\n";

// array with different values for $string
$strings =  array (

  //double quoted strings
  b"123",
  b"abc",
  b"1a2b3c",
  b"Here is a simple string to test convert_uuencode/decode",
  b"\t This String contains \t\t some control characters\r\n",
  b"\x90\x91\x00\x93\x94\x90\x91\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f",
  
   //single quoted strings
  b'123',
  b'abc',
  b'1a2b3c',
  b'\t This String contains \t\t some control characters\r\n',
  
);  

// loop through with each element of the $strings array to test convert_uuencode() function
$count = 1;
foreach($strings as $string) {
  echo "-- Iteration $count --\n";
  var_dump( convert_uuencode($string) );
  $count ++;
}


?>
===DONE=== 
