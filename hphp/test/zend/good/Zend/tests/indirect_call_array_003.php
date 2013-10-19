<?php

class foo {
	public function __call($a, $b) {
		printf("From %s:\n", __METHOD__);
		var_dump($a);
		var_dump($this);
	}
	static public function __callStatic($a, $b) {
		printf("From %s:\n", __METHOD__);
		var_dump($a);
		var_dump($this);
	}
}

$arr = array('foo', 'abc');
$arr();

$foo = new foo;
$arr = array($foo, 'abc');
$arr();


?>