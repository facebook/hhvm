<?php

class foo {
	public function test() {
		call_user_func(array('FOO', 'ABC'));
		call_user_func(array($this, 'ABC'));
		foo::XYZ();
		self::WWW();
		call_user_func('FOO::ABC');
	}
	function __call($a, $b) {
		print "__call:\n";
		var_dump($a);
	}
	static public function __callStatic($a, $b) {
		print "__callstatic:\n";
		var_dump($a);
	}
}

$x = new foo;

$x->test();

$x::A();

foo::B();

$f = 'FOO';

$f::C();

$f::$f();

foo::$f();

?>