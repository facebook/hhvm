<?php

error_reporting(E_ALL);

class  foo {
	public function bar() {
		$x = array();
		$x[] = 3;
		$x[] = array(1, 5);
		$x[] = new foo;
		return $x;
	}
}

$foo = new foo;

var_dump($x = $foo->bar()[1]);
var_dump($foo->bar()[1][1]);
var_dump($x[0]);
var_dump($x = $foo->bar()[2]);
var_dump($x->bar());
var_dump($x->bar()[0]);

$x = array();
$x[] = new foo;
var_dump($x[0]->bar()[2]);
var_dump($foo->bar()[2]->bar()[1]);
var_dump($foo->bar()[2]->bar()[2]->bar()[1][0]);
var_dump($foo->bar()[2]->bar()[2]->bar()[1][0][1]);
var_dump($foo->bar()[2]->bar()[2]->bar()[4]);
var_dump($foo->bar()[3]->bar());

?>