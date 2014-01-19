<?php
class test {
	public function __toString() {
		debug_backtrace();
		return 'lowercase';
	}
}

	$test = new test();
	echo strtoupper($test);
?>