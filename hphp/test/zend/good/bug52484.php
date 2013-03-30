<?php

class A {	
	function __unset($prop) {
		unset($this->$prop);
	}
}

$a = new A();
$prop = null;

unset($a->$prop);

?>