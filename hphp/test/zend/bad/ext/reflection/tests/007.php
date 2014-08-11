<?php

function test($class)
{
	echo "====>$class\n";
	try
	{
		$ref = new ReflectionClass($class);
	}
	catch (ReflectionException $e)
	{
		var_dump($e->getMessage());
		return; // only here
	}

	echo "====>newInstance()\n";
	try
	{
		var_dump($ref->newInstance());
	}
	catch (ReflectionException $e)
	{
		var_dump($e->getMessage());
	}
	
	echo "====>newInstance(25)\n";
	try
	{
		var_dump($ref->newInstance(25));
	}
	catch (ReflectionException $e)
	{
		var_dump($e->getMessage());
	}

	echo "====>newInstance(25, 42)\n";
	try
	{
		var_dump($ref->newInstance(25, 42));
	}
	catch (ReflectionException $e)
	{
		var_dump($e->getMessage());
	}
	
	echo "\n";
}

function __autoload($class)
{
	echo __FUNCTION__ . "($class)\n";
}

test('Class_does_not_exist');

Class NoCtor
{
}

test('NoCtor');

Class WithCtor
{
	function __construct()
	{
		echo __METHOD__ . "()\n";
		var_dump(func_get_args());
	}
}

test('WithCtor');

Class WithCtorWithArgs
{
	function __construct($arg)
	{
		echo __METHOD__ . "($arg)\n";
		var_dump(func_get_args());
	}
}

test('WithCtorWithArgs');

?>
===DONE===
<?php exit(0); ?>
