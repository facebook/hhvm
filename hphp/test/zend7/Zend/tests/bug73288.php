<?php

class NoClone {
	public function __clone() {
		throw new Exception("No Cloneable");
	}
}

class C {
	public function __get($name) {
		return new NoClone;
	}
}

function test_clone() {
	$c = new C;
	$b = clone $c->x;
}

test_clone();
?>
