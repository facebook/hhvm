<?php
class A {
	function __construct() {
		debug_print_backtrace();
		$bt = debug_backtrace();
		foreach ($bt as $t) {
			print $t['class'].$t['type'].$t['function']."\n";
		}
	}

	function foo() {
		debug_print_backtrace();
		$bt = debug_backtrace();
		foreach ($bt as $t) {
                        print $t['class'].$t['type'].$t['function']."\n";
		}
	}

	static function bar() {
		debug_print_backtrace();
		$bt = debug_backtrace();
		foreach ($bt as $t) {
			print $t['class'].$t['type'].$t['function']."\n";
		}
	}
}

class B extends A {
	function __construct() {
		parent::__construct();
	}

	function foo() {
		parent::foo();
	}

	static function bar() {
		parent::bar();
	}
}

$b = new B();
$b->foo();
B::bar();
?>