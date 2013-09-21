<?php
/* Prototype: string chr ( int $ascii );
   Description: Returns a one-character string containing the character specified by ascii. 

   Prototype: int ord ( string $string );
   Description: Returns the ASCII value of the first character of string
*/
echo "*** Testing ord() & chr() basic operations ***\n";
for($i=0; $i<256; $i++) echo !ord(chr($i)) == $i;

/* miscelleous input */
echo "\n*** Testing chr() usage variations ***\n";
$arr_test = array( 
  "true", 
  "false",
  true,
  false,
  "",             
  " ",           
  "a",
  299,
  321,
  NULL,
  '\0',
  "0",
  -312, 
  12.999,
  -1.05009,
  1100011,
  "aaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbccccccccccccccccccccccccccccccdddddddddddddddddddddddddddddddddddddddddd",
  "abcd\nabcd\tabcd\0abcd\rabcdNULLabcdefgh",
  "abcd\x00abcd\x00abcd\x00abcdefghij",
);
$counter=1;
foreach($arr_test as $var){
  echo "-- Iteration $counter --\n";
  var_dump( chr($var) );
  $counter++;
}

echo "\n*** Testing ord() usage variations ***\n";
$counter=1;
foreach($arr_test as $var){
  echo "-- Iteration $counter --\n";
  var_dump( ord($var) );
  $counter++;
}

/* Error conditions */
echo "\n*** Testing chr() error conditions ***\n";
//zero arguments
var_dump( chr() ); 
// more than expected no. of args
var_dump( chr($arr_test[0], $arr_test[1]) ); 


echo "\n*** Testing ord() error conditions ***\n";
// zero arguments
var_dump( ord() ); 
// more than expected no. of args
var_dump( ord($arr_test[0], $arr_test[1]) ); 

echo "Done\n";
?>