<?php

class foo {
	static $x = array();
	
	public function &a() {
		self::$x = array(1, 2, 3);
		return self::$x;
	}
	
	public function b() {
		$x = array(1);
		$x[] = 2;
		return $x;
	}
}

$foo = new foo;

// Changing the static variable
$foo->a()[0] = 2;
var_dump($foo::$x);

$foo->b()[] = new stdClass;

$h = $foo->b();
var_dump($h);

$h[0] = 3;
var_dump($h);

?>