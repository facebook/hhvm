<?php

class SubClass extends BaseClass
{
}

abstract class BaseClass
{
	public function __call($name, $arguments)
	{
		return $this->$name();
	}

	private function foobar()
	{
		return 'okey';
	}
}

$test = new SubClass();
echo $test->foobar();
?>
