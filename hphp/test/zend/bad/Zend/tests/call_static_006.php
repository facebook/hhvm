<?php

class foo {
	public function aa() {
		print "ok\n";
	}
	static function __callstatic($a, $b) {
		var_dump($a);
	}
}

foo::aa();

$b = 'AA';
foo::$b();

foo::__construct();

?>