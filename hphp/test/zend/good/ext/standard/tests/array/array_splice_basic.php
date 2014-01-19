<?php
/* 
 * proto array array_splice(array input, int offset [, int length [, array replacement]])
 * Function is implemented in ext/standard/array.c
*/ 

echo "*** Testing array_splice() basic operations ***\n";
echo "test truncation \n";
$input = array("red", "green", "blue", "yellow");
var_dump (array_splice($input, 2));
var_dump ($input);
// $input is now array("red", "green")

echo "test removing entries from the middle \n";
$input = array("red", "green", "blue", "yellow");
var_dump (array_splice($input, 1, -1));
var_dump ($input);
// $input is now array("red", "yellow")

echo "test substitution at end \n";
$input = array("red", "green", "blue", "yellow");
var_dump (array_splice($input, 1, count($input), "orange"));
var_dump ($input);
// $input is now array("red", "orange")

$input = array("red", "green", "blue", "yellow");
var_dump (array_splice($input, -1, 1, array("black", "maroon")));
var_dump ($input);
// $input is now array("red", "green",
//          "blue", "black", "maroon")

echo "test insertion \n";
$input = array("red", "green", "blue", "yellow");
var_dump (array_splice($input, 3, 0, "purple"));
var_dump ($input);
// $input is now array("red", "green",
//          "blue", "purple", "yellow");


?>