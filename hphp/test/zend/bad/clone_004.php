<?php 

error_reporting(E_ALL|E_STRICT);

class foo {
	public function __get($a) {
		return new $this;
	}
}

$c = new foo;

$a = clone $c->b[1];

?>