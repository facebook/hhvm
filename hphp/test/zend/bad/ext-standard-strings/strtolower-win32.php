<?php
/* Prototype: 
     string strtolower ( string $str );
   Description:
     Returns string with all alphabetic characters converted to lowercase.
*/

echo "*** Testing strtolower() with all 256 chars ***\n";
for ($i=0; $i<=255; $i++){
  $char = chr($i);
  print(bin2hex($char))." => ".(bin2hex(strtolower("$char")))."\n";
}

echo "*** Testing strlower() with basic strings ***\n";
$str = "Mary Had A liTTle LAmb and ShE loveD IT So\n";
var_dump(strtolower($str));

echo "\n*** Testing strtolower() with various strings ***";
/* strings to pass strtolower() */ 
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
  var_dump( strtolower($string) );
  $count++;
}

echo "\n*** Testing strtolower() with two different case strings ***\n";
if (strtolower("HeLLo woRLd") === strtolower("hEllo WORLD"))
  echo "strings are same, with Case Insensitive\n";
else
  echo "strings are not same\n";

echo "\n*** Testing error conditions ***";
var_dump( strtolower() ); /* Zero arguments */
var_dump( strtolower("a", "b") ); /* Arguments > Expected */

echo "*** Done ***";
?>