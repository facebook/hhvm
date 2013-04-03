<?php
class foo {
	const bar = "fubar\n";
	
	function foo($arg = self::bar) {
		echo $arg;
	}
}

new foo();
?>