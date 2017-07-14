<?php

class C {
	public function test() {
		return new stdClass;
	}
}

$b = new stdClass;
$b->c = new C;
$b->c->test()->d = 'str';

?>
===DONE===
