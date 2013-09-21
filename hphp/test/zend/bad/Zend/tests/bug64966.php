<?php
error_reporting(E_ALL);
set_error_handler(function($error) { throw new Exception(); }, E_RECOVERABLE_ERROR);

function test($func) {
	$a = $func("");
	return true;
}
class A {
	public function b() {
		test("strlen");
		test("iterator_apply");
	}
}

$a = new A();
$a->b();
?>