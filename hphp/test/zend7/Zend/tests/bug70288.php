<?php
class A {
	public function __get($name) {
		return new Stdclass();
	}
}

function test(&$obj) {
	var_dump($obj);
}
$a = new A;
test($a->dummy);
?>
