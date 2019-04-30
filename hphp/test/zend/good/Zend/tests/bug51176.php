<?php
class Foo
{
	public function start()
	{
		self::bar();
		static::bar();
		Foo::bar();
	}

	public function __call($n, $a)
	{
		echo "instance\n";
	}
}

$foo = new Foo();
$foo->start();

