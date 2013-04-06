<?php

class Base {
	private function __destruct() {
		echo __METHOD__ . "\n";
	}
}

class Derived extends Base {
}

$obj = new Derived;

?>
===DONE===