<?php
/* 
 * proto bool function_exists(string function_name)
 * Function is implemented in Zend/zend_builtin_functions.c
*/ 

echo "*** Testing function_exists() : basic functionality ***\n";

echo "Internal function: ";
var_dump(function_exists('function_exists'));

echo "User defined function: ";
function f() {}
var_dump(function_exists('f'));

echo "Case sensitivity: ";
var_dump(function_exists('F'));

echo "Non existent function: ";
var_dump(function_exists('g'));

echo "Method: ";
Class C {
	static function f() {}
}
var_dump(function_exists('C::f'));
?>
===Done===