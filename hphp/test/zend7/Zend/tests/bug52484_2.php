<?php

class A {
	function __set($prop, $val) {
		$this->$prop = $val;
	}
}

$a = new A();
$prop = "\0";

$a->$prop = 2;

?>
