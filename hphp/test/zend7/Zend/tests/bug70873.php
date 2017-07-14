<?php

class A {
	private static $x = 1;
}

class B extends A {
	function bar() {
		var_dump(self::$x);
	}
};

class C extends A {
	function bar() {
		var_dump(A::$x);
	}
};


$a = new B;
$a->bar();

$b = new C;
$b->bar();
?>
