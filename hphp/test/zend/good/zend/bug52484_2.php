<?php

class A {
	function __set($prop, $val) {
		$this->$prop = $val;
	}
}

$a = new A();
$prop = null;

$a->$prop = 2;

?>