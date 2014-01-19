<?php
class a {
	public function __CONSTRUCT() { echo __METHOD__ . "\n"; }
	public function a() { echo __METHOD__ . "\n"; }
}
class b extends a {}
class c extends b { 
	function C() {
		b::b();
	}
}
$c = new c();
?>
===DONE===