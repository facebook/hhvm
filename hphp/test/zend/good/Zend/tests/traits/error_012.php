<?php

trait foo {
	public function test() { return 3; }
}

class bar {
	use foo { test as protected; }
}

$x = new bar;
var_dump($x->test());

?>