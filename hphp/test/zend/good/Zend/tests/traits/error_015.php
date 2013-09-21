<?php

trait foo {
	public function test() { return 3; }
}

trait baz {
	public function test() { return 4; }
}

class bar {
	use foo, baz {
		baz::test as zzz;
	}
}

$x = new bar;
var_dump($x->test());

?>