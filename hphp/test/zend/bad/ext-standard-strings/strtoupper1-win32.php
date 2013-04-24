<?php
/* Prototype: 
     string strtoupper ( string $string );
   Description: 
     Returns string with all alphabetic characters converted to uppercase
*/ 

echo "*** Testing strtoupper() with all 256 chars ***\n";
for ($i=0; $i<=255; $i++){
  $char = chr($i);
  print(bin2hex($char))." => ".(bin2hex(strtoupper("$char")))."\n";
}

echo "\n*** Testing strtoupper() with basic strings ***\n";
$str = "Mary Had A liTTle LAmb and ShE loveD IT So\n";
var_dump(strtoupper($str));

echo "\n*** Testing strtoupper() with various strings ***";
/* strings to pass strtoupper() */ 
$strings = array (
  "",
  "string",
  "stRINg0234",
  "1.233.344StrinG12333",
  "$$$$$$!!!!@@@@@@@ ABCDEF !!!***",
  "ABCD\0abcdABCD", 
  NULL,
  TRUE,
  FALSE,
  array()
);

$count = 0;
/* loop through to check possible variations */
foreach ($strings as $string) {
  echo "\n-- Iteration $count --\n";
  var_dump( strtoupper($string) );
  $count++;
}

echo "\n*** Testing strtoupper() with two different case strings ***\n";
if (strtoupper("HeLLo woRLd") === strtoupper("hEllo WORLD"))
  echo "strings are same, with Case Insensitive\n";
else
  echo "strings are not same\n";

echo "\n*** Testing error conditions ***";
var_dump( strtoupper() ); /* Zero arguments */
var_dump( strtoupper("a", "b") ); /* Arguments > Expected */

echo "*** Done ***";
?>