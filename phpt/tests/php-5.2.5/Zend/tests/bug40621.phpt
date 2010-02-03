--TEST--
Bug #40621 (Crash when constructor called inappropriately (statically))
--FILE--
<?php

class Foo {
	private function __construct() { }
	function get() {
		self::__construct();
	}
}

Foo::get();

echo "Done\n";
?>
