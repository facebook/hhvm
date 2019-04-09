<?php
class A {
	static $x = "A";
	function testit() {
		$this->v1 = new self;
		$this->v2 = new self;
	}
}

class B extends A {
	static $x = "B";
	function testit() {
		parent::testit();
		$this->v3 = new self;
		$this->v4 = new parent;
		$this->v4 = static::$x;
	}
}
$t = new B();
$t->testit();
var_dump($t);
