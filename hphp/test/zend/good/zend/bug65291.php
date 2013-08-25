<?php

trait TestTrait
{
	public static function testStaticFunction()
	{
		return __CLASS__;
	}
}
class Tester
{
	use TestTrait;
}

$foo = Tester::testStaticFunction();
get_defined_constants();
get_defined_constants(true);

echo $foo;
?>