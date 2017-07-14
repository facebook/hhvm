<?php
function test($func) {
	try {
		$a = $func("");
	} catch (Error $e) {
		throw new Exception();
	}
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
