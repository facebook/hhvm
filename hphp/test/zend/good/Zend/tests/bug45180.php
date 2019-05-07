<?php

class foo {
	public function test() {
		foo::ABC();
		$this->ABC();
		foo::XYZ();
		self::WWW();
		FOO::ABC();
	}
	function __call($a, $b) {
		print "__call:\n";
		var_dump($a);
	}
}

$x = new foo;

$x->test();
