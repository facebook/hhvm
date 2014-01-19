<?php

class test {
	function foo() {
		$a = new test;
		$this = &$a;
	}
}

$t = new test;
$t->foo();

echo "Done\n";
?>