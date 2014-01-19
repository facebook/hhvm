<?php

trait foo {
	public function test() { return 3; }
}

class baz {
	final public function test() { return 4; }
}

class bar extends baz {
	use foo { test as public; }
}

$x = new bar;
var_dump($x->test());

?>