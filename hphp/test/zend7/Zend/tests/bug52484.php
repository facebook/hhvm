<?php

class A {	
	function __unset($prop) {
		unset($this->$prop);
	}
}

$a = new A();
$prop = "\0";

unset($a->$prop);

?>
