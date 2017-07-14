<?php
class Foo {
	public function bar() {
		print "Success\n";
	}
}

function test(&$instance, &$method) {
	$instance->{$method}();
}

$instance = new Foo;
$method = "bar";

test($instance, $method);
?>
