<?php
class A {
	function __call($strMethod, $arrArgs) {
		echo "$strMethod\n";
	}
}

class C {
	function __call($strMethod, $arrArgs) {
		echo "$strMethod\n";
	}
}

class B extends A {
	function test() {
		self::test1();
		parent::test2();
		static::test3();
		A::test4();
		B::test5();
		C::test6();
	}
}

$a = new A();
$a->test();

$b = new B();
$b->test();
?>