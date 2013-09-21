<?php
error_reporting(E_ALL);

trait A {
	public function M1() {}
	public function M2() {}
}

trait B {
	public function M1() {}
	public function M2() {}
}

class MyClass {
	use A;
	use B;
}
?>