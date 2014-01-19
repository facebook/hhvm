<?php

trait foo {
	public function test() { return 3; }
}

class bar {
	use foo { test as static; }
}

$x = new bar;
var_dump($x->test());

?>