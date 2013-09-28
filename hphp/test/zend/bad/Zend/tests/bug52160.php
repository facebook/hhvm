<?php

class bar {
	function __construct() { }
	static function bar() {
		var_dump(1);
	}
}

bar::bar();

class foo {
	static function foo() {
		var_dump(2);
	}
	function __construct() { }
}

foo::foo();

class baz {
	static function baz() {
		var_dump(3);
	}
}

?>