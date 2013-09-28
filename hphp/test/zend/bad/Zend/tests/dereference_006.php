<?php

error_reporting(E_ALL);

function &foo(&$foo) {
	return $foo;
}

$a = array(1);
foo($a)[0] = 2;
var_dump($a);

foo($a)[] = 3;
var_dump($a);

?>