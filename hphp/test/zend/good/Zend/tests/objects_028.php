<?php

class bar {
	public function __call($a, $b) {
		print "hello\n";
	}
}

class foo extends bar {
	public function __construct() {
		static::bar();
		parent::bar();
	}
}


new foo;

?>