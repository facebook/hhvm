<?php

class test {
	public function method() {
		return ArrayIterator::current();
	}
}
$test = new test();
$test->method();

echo "Done\n";
?>
