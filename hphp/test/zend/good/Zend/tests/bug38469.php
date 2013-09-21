<?php
$a = array();
$a[0] = $a;
var_dump($a);
$b = array(array());
$b[0][0] = $b;
var_dump($b);

function f() {
	$a = array();
	$a[0] = $a;
	var_dump($a);
	$b = array(array());
	$b[0][0] = $b;
	var_dump($b);
}
f();
?>