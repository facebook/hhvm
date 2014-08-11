<?php

class foo2 {
	const BAR = 'foobar';
}

class foo extends foo2 {
	const BAR = "foo's bar";
	
	function test($a = self::BAR) {
	}
	
	function test2($a = parent::BAR) {
	}

	function test3($a = foo::BAR) {
	}
	
	function test4($a = foo2::BAR) {
	}
}

ReflectionObject::export(new foo);

?>
