<?php

error_reporting(E_ALL);

function a() {
	return 1;
}

$a = 1;
var_dump($a[1]);
var_dump(a()[1]);

function b() {
	return new stdClass;
}

var_dump(b()[1]);

?>