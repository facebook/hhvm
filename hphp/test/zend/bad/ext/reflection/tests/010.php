<?php
class Foo {
	function func() {
	}
}
class Bar extends Foo {
	function func() {
	}
}
$m = new ReflectionMethod("Bar::func");
echo $m;
?>
