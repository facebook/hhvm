<?php

class_alias(get_class(new class { protected $foo = 1; }), "AnonBase");
var_dump((new class extends AnonBase {
	function getFoo() {
		return $this->foo;
	}
})->getFoo());
?>
