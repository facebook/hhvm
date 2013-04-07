<?php

trait foo {	
	public function foo() {
		return 1;
	}
}

trait foo2 {
	public function foo() {
		return 2;
	}
}


class A extends foo {
	use foo {
		foo2::foo insteadof foo;
		foo2::foo insteadof foo;
	}
}

?>