<?php
class Foo {
	public $test = "ok";
}
$class = new ReflectionClass("Foo");
$props = $class->getDefaultProperties();
echo $props["test"];
?>
