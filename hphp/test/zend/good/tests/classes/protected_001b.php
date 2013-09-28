<?php

class pass {
	protected function fail() {
		echo "Call fail()\n";
	}

	public function good() {
		$this->fail();
	}
}

$t = new pass();
$t->good();
$t->fail();// must fail because we are calling from outside of class pass

echo "Done\n"; // shouldn't be displayed
?>