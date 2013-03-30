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