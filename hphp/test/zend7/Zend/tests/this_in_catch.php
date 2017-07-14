<?php
class C {
	function foo() {
		try {
			throw new Exception();
		} catch (Exception $this) {
		}
		var_dump($this);
	}
}
$obj = new C;
$obj->foo();
?>
