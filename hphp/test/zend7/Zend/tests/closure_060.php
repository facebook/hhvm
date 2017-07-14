<?php

class A {
	private static $priv = 7;

	static function get() {
		return function() {
			var_dump(isset(A::$priv));
		};
	}
}

$closure = A::get();
$closure(); // init rt_cache
$closure->call(new class(){}, null);
$closure();

?>
