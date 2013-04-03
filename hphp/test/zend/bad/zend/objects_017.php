<?php

class foo {
	private $test = 1;
}

function test() {
	return new foo;
}

test()->test = 2;

?>