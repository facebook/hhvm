<?php

class A extends MessageFormatter {
	function __construct() {}
}

$a = new A;
try {
	$b = clone $a;
} catch (Exception $e) {
	var_dump($e->getMessage());
}
