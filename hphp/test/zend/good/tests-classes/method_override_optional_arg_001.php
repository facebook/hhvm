<?php

class A {
	function foo($arg1 = 1) {
	}
}

class B extends A {
	function foo($arg1 = 2, $arg2 = 3) {
		var_dump($arg1);
		var_dump($arg2);
	}
}

class C extends A {
	function foo() {
	}
}

$b = new B();

$b->foo(1);

?>