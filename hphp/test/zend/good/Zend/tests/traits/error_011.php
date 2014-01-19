<?php

trait foo {
	public function test() { return 3; }
}
trait c {
	public function test() { return 2; }
}

trait b {
	public function test() { return 1; }
}

class bar {
	use foo, c, b;
}

$x = new bar;
var_dump($x->test());

?>