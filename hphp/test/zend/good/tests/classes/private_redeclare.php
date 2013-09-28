<?php
class base {
	private function show() {
		echo "base\n";
	}
	function test() {
		$this->show();
	}
}

$t = new base();
$t->test();

class derived extends base {
	function show() {
		echo "derived\n";
	}
	function test() {
		echo "test\n";
		$this->show();
		parent::test();
		parent::show();
	}
}

$t = new derived();
$t->test();
?>