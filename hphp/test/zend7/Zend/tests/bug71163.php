<?php
function __autoload($name) { 
	eval ("class $name extends Exception { public static function foo() {}}");
	throw new Exception("boom");
}

function test2() {
	try {
		Test::foo();
	} catch (Exception $e) {
		echo "okey";
	}
}

function test() {
	test2();
}

test();
?>
