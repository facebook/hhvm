<?php
class Foo {
	const A = self::B;
	const B = "ok";
}
var_dump(defined("Foo::A"));
?>