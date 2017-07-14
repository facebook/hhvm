<?php

class foo {
	private $var;
	function __get($e) {
		return $this;
	}
}

function &noref() { $foo = 1; return $foo; }

$foo = new foo;
$foo->i = &noref();
var_dump($foo);

?>
