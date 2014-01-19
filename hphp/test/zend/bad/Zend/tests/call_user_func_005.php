<?php

class foo {
	public static function __callstatic($x, $y) {
		var_dump($x,$y);
		return 1;
	}
	
	public function teste() {
		return foo::x(function &($a=1,$b) { });
	}
}

var_dump(call_user_func(array('foo', 'teste')));

?>