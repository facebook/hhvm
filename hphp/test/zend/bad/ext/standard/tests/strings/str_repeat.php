<?php
/* Prototype: string str_repeat ( string $input, int $multiplier );
   Description: Returns input repeated multiplier times. multiplier has to be 
     greater than or equal to 0. If the multiplier is set to 0, the function 
     will return an empty string. 
*/

echo "*** Testing str_repeat() with possible strings ***";
$variations = array(
  'a',
  'foo',
  'barbazbax',
  "\x00",
  '\0', 
  NULL, 
  TRUE,
  4,
  1.23,
  "",
  " "
);

/* variations in string and multiplier as an integer */
foreach($variations as $input) {
  echo "\n--- str_repeat() of '$input' ---\n" ;
  for($n=0; $n<4; $n++) {
    echo "-- after repeating $n times is => ";
    echo str_repeat($input, $n)."\n";
  }
}

/* variations in multiplier as well as string to be repeated. Same varient 
   values are used as string to be repeated as well as multiplier */
echo "\n\n*** Testing str_repeat() with various strings & multiplier value ***";
foreach ( $variations as $input ) {
  echo "\n--- str_repeat() of '$input' ---\n" ;
  foreach ( $variations as $multiplier ) {
    echo "-- after repeating '$multiplier' times is => ";
    var_dump( str_repeat($input, $multiplier) );
  }
}


echo "\n*** Testing str_repeat() with complex strings containing 
       other than 7-bit chars ***\n";
$str = chr(0).chr(128).chr(129).chr(234).chr(235).chr(254).chr(255);
var_dump(str_repeat($str, chr(51)));  // ASCII value of '3' given
var_dump(str_repeat($str, 3));


echo "\n\n*** Testing error conditions ***";
var_dump( str_repeat() );  // Zero args
var_dump( str_repeat($input[0]) );  // args < expected
var_dump( str_repeat($input[0], 3, 4) );  // args > expected
var_dump( str_repeat($input[0], -1) );  // Invalid arg for multiplier

echo "Done\n";
?>