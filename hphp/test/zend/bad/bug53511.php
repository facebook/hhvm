<?php
class Foo {
	function __destruct() {
		throw new Exception("ops 1");
	}
}

function test() {
	$e = new Foo();
	try {
		throw new Exception("ops 2");
	} catch (Exception $e) {
		echo $e->getMessage()."\n";
	}
}

test();
echo "bug\n";