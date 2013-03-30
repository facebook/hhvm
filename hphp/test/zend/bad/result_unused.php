<?php
$x = array(1);
$a = "x";
$$a;

$x = array(array(2));
$x[0];

$x = "str";
$x[0];
$x[3];

class Foo {
	public $prop = array(3);
	function __get($name) {
		return array(4);
	}
}
$x = new Foo();
$x->prop;
$x->y;
echo "ok\n";