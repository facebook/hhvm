<?php
trait Foo
{
	function bar($a = self::class) {
		var_dump($a);
	}
}

class B {
	use Foo;
}

$b = new B;

$b->bar();
?>
