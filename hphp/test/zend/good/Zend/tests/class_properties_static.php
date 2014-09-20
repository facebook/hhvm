<?php
class Foo {
	public $b1 = 1 + 1;
	public $b2 = 1 << 2;
	public $b3 = "foo " . " bar " . " baz";
}
$f = new Foo;
var_dump(
	$f->b1,
	$f->b2,
	$f->b3
);
?>
