<?php
class Foo {
	const test = "ok";
}
$class = new ReflectionClass("Foo");
echo $class;
?>
