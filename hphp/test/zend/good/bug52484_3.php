<?php

class A {	
	function __get($prop) {
		var_dump($this->$prop);
	}
}

$a = new A();
$prop = null;

var_dump($a->$prop);

?>