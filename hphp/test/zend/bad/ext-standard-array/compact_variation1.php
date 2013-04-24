<?php
/* Prototype  : proto array compact(mixed var_names [, mixed ...])
* Description: Creates a hash containing variables and their values
* Source code: ext/standard/array.c
* Alias to functions:
*/
/*
* compact variations - arrays with references
*/
echo "*** Testing compact() : usage variations  - arrays containg references ***\n";
$a = 1;
$b = 2;
$c = 3;
$string = "c";
$arr1 = array("a", &$arr1);
$arr2 = array("a", array(array(array("b"))));
$arr2[1][0][0][] = &$arr2;
$arr2[1][0][0][] = &$arr2[1];
$arr3 = array(&$string);
var_dump(compact($arr1));
var_dump(compact($arr2));
var_dump(compact($arr3));
echo "Done";
?>