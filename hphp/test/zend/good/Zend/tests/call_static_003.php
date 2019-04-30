<?php

class Foo {
	public function __call($a, $b) {
		print "nonstatic\n";
		var_dump($a);
	}
	public function test() {
		$this->fOoBaR();
		self::foOBAr();
		$this::fOOBAr();
	}
}

$a = new Foo;
$a->test();

