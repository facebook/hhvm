<?php

namespace Baz;

class Foo {
	public static function bar() {
		function foo() {
			var_dump(__FUNCTION__);
			var_dump(__METHOD__);
			var_dump(__CLASS__);
		}

		foo();
		
		var_dump(__FUNCTION__);
		var_dump(__METHOD__);
		var_dump(__CLASS__);
		
		return function() {var_dump(__FUNCTION__); var_dump(__METHOD__); var_dump(__CLASS__); };
	}
}

$c = Foo::bar();

$c();
?>
