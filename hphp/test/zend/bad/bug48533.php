<?php

class foo {
	private function a() {
		var_dump(1);	
	}
	public function b() {
		var_dump(2);
	}
	protected function c() {
		var_dump(3);
	}
	static function __callstatic($a, $b) {
		var_dump('__callStatic::'. $a);
	}
	public function __call($a, $b) {
		var_dump('__call::'. $a);
	}
}

$x = new foo;
$x->a();
$x->b();
$x->c();
$x::a();
$x::b();
$x::c();

?>