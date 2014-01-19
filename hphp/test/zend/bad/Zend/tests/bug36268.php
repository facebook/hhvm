<?php
class Foo {
	function __destruct() {
		echo "Ha!\n";
	}
}
$x = new Foo();
bar();
?>