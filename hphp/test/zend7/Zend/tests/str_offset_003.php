<?php
// Test negative string offsets

function foo($x) {
	var_dump($x);
}

$str = "abcdef";
var_dump($str[-10]);
var_dump($str[-3]);
var_dump($str[2][-2]);
var_dump($str[2][-1]);

foo($str[-10]);
foo($str[-3]);
foo($str[2][-2]);
foo($str[2][-1]);
?>
