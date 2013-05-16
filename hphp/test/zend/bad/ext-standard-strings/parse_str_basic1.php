<?php
		
/* Prototype  : void parse_str  ( string $str  [, array &$arr  ] )
 * Description: Parses the string into variables
 * Source code: ext/standard/string.c
*/

echo "*** Testing parse_str() : basic functionality ***\n";

echo "Basic test WITHOUT result arg\n";
$s1 = "first=val1&second=val2&third=val3";
var_dump(parse_str($s1));
var_dump($first, $second, $third);

echo "\nBasic test WITH undefined var for result arg\n";
$s1 = "first=val1&second=val2&third=val3";
var_dump(parse_str($s1, $res1));
var_dump($res1);

echo "\nBasic test WITH existing non-array var for result arg\n";
$res2 =99;
$s1 = "first=val1&second=val2&third=val3";
var_dump(parse_str($s1, $res2));
var_dump($res2);

echo "\nBasic test with an existing array as results array\n";
$res3_array = array(1,2,3,4);
var_dump(parse_str($s1, $res3_array));
var_dump($res3_array); 

?>
===DONE===