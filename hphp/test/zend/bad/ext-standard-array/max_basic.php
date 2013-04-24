<?php
/* 
 * proto mixed max(mixed arg1 [, mixed arg2 [, mixed ...]])
 * Function is implemented in ext/standard/array.c
*/ 

echo "\n*** Testing sequences of numbers ***\n";

var_dump(max(2,1,2));
var_dump(max(-2,1,2));
var_dump(max(2.1,2.11,2.09));
var_dump(max("", "t", "b"));
var_dump(max(false, true, false));
var_dump(max(true, false, true));
var_dump(max(1, true, false, true));
var_dump(max(0, true, false, true));
var_dump(max(0, 1, array(2,3)));

echo "\nDone\n";
?>