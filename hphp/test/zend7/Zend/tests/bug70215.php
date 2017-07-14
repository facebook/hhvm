<?php

class A {
	public static function __invoke() {
		echo __CLASS__;
	}
}

class B extends A { }

$b = new B;

$b();

?>
