<?php

class d {
	private function test2() {
		print "Bar\n";
	}
}

abstract class a extends d {
	public function test() {
		$this->test2();
	}
}

abstract class b extends a {
}

class c extends b {
	public function __construct() {
		$this->test();
	}	
}

new c;

?>
