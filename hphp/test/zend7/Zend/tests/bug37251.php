<?php
class Foo {	
	function bar(array $foo) {
	}
}

try {
	$foo = new Foo();
	$foo->bar();
} catch (Error $e) {
	echo 'OK';
}
