<?php

abstract class foobar {
	private function test2() {
	}	
}
class foo extends foobar {
	private $foo = 1;
	private function test() {
	}
	protected function test3() {
	}
}
class bar extends foo {
	private function foobar() {
	}
}

Reflection::export(new ReflectionClass(new bar));

?>
