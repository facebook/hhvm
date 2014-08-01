<?php
function foo($v) {
	global $a;
	return $a[$v];
}
$a = array(PHP_VERSION => 1);
var_dump(foo(PHP_VERSION));