<?php

class foo {
	public function a() {
	}
}

$test = new foo;

$test->a()->a;
print "ok\n";

$test->a()->a = 1;
print "ok\n";

?>