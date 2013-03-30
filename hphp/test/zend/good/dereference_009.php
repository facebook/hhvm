<?php

error_reporting(E_ALL);

$a = array();

function &a() {
	return $GLOBALS['a'];
}

var_dump($h =& a());
$h[] = 1;
var_dump(a()[0]);

$h[] = array($h);
var_dump(a()[1][0][0]);

?>