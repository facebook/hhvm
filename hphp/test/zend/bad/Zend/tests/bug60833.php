<?php
class A {
	static $x = "A";
	function testit() {
		$this->v1 = new sELF;
		$this->v2 = new SELF;
	}
}

class B extends A {
	static $x = "B";
	function testit() {
		PARENT::testit();
		$this->v3 = new sELF;
		$this->v4 = new PARENT;
		$this->v4 = STATIC::$x;
	}
}
$t = new B();
$t->testit();
var_dump($t);
?>