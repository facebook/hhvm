<?php

class test {
	public function test() {
		return ArrayIterator::current();
	}
}
$test = new test();
$test->test();

echo "Done\n";
?>