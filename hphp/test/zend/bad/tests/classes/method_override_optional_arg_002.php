<?php

abstract class A {
	function foo($arg = 1) {}
}

class B extends A {
	function foo() {
		echo "foo\n";
	}
}

$b = new B();
$b->foo();

?>