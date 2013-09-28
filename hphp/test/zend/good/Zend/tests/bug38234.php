<?php
class Foo {
	function __clone() {
		throw new Exception();
	}
}
try {
	$x = new Foo();
	$y = clone $x;
} catch (Exception $e) {
}
echo "ok\n";
?>