<?php

class bar {
	public function __set($a, $b) {
		print "hello\n";
	}
}

class foo extends bar {
	public function __construct() {
		static::$f = 1;
	}
	public function __set($a, $b) {
		print "foo\n";
	}
}


new foo;

?>