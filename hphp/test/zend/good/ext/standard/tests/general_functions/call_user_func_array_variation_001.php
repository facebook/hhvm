<?php

function by_val($arg) {
	$arg = 'changed';
}

function by_ref(&$arg) {
	$arg = 'changed';
}

echo "------ Calling by_val() with unreferenced argument ------\n";
$arg = array('original');
call_user_func_array('by_val', $arg);
var_dump($arg);

echo "------ Calling by_ref() with unreferenced argument ------\n";
$arg = array('original');
call_user_func_array('by_ref', $arg);
var_dump($arg);

echo "------ Calling by_val() with referenced argument ------\n";
$arg = array('original');
$ref = &$arg[0];
call_user_func_array('by_val', $arg);
var_dump($arg);

echo "------ Calling by_ref() with referenced argument ------\n";
$arg = array('original');
$ref = &$arg[0];
call_user_func_array('by_ref', $arg);
var_dump($arg);

?>