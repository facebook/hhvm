<?php

trait T {
	protected function test() {
		echo "okey";
	}
}


class A {
	protected function test() {
	}
}

class B extends A {
	use T;
	public function foo() {
		$this->test();
	}
}


$b = new B();
$b->foo();
?>
