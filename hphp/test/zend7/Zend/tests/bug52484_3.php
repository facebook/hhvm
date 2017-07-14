<?php

class A {	
	function __get($prop) {
		var_dump($this->$prop);
	}
}

$a = new A();
$prop = "\0";

var_dump($a->$prop);

?>
