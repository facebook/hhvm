<?php
/* 
 * proto mixed min(mixed arg1 [, mixed arg2 [, mixed ...]])
 * Function is implemented in ext/standard/array.c
*/ 


echo "\n*** Testing Error Conditions ***\n";

var_dump(min());
var_dump(min(1));
var_dump(min(array()));
var_dump(min(new stdclass));

?>