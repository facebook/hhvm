<?php
class C {
	function foo() {
		eval('var_dump($this);');
		eval('var_dump($this);');
	}
}
$x = new C;
$x->foo();
