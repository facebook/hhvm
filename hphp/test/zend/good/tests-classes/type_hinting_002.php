<?php

class Foo {
	function a(NonExisting $foo) {}
}

$o = new Foo;
$o->a($o);
?>