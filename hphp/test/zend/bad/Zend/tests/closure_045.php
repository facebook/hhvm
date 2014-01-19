<?php

class A {
static function foo() {
	return function () {};
}
}

$a = A::foo();
$a->bindTo(new A);

echo "Done.\n";
