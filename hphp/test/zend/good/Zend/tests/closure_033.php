<?php

class Test {
	public $func;
	function __construct() {
		$this->func = function() {
			echo __METHOD__ . "()\n";
		};
	}
	private function func() {
		echo __METHOD__ . "()\n";
	}
}

$o = new Test;
$f = $o->func;
$f();
$o->func();

?>
===DONE===