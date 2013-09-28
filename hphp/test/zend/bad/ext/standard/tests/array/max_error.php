<?php
/* 
 * proto mixed min(mixed arg1 [, mixed arg2 [, mixed ...]])
 * Function is implemented in ext/standard/array.c
*/ 


echo "\n*** Testing Error Conditions ***\n";

var_dump(max());
var_dump(max(1));
var_dump(max(array()));
var_dump(max(new stdclass));

?>