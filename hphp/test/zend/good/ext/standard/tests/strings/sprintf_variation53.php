<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : with  white spaces in format strings ***\n";

// initializing the format array
$formats = array(
  "% d", "%  d", "%   d",
  "% f", "%  f", "%   f",
  "% F", "%  F", "%   F",
  "% b", "%  b", "%   b",
  "% c", "%  c", "%   c",
  "% e", "%  e", "%   e",
  "% u", "%  u", "%   u",
  "% o", "%  o", "%   o",
  "% x", "%  x", "%   x",
  "% X", "%  X", "%   X",
  "% E", "%  E", "%   E"
);

// initializing the args array

foreach($formats as $format) {
  var_dump( sprintf($format, 1234) );
}

echo "Done";
?>