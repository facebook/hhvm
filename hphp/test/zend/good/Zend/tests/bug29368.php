<?php

class Foo
{
	function __construct()
	{
		echo __METHOD__ . "\n";
		throw new Exception;
	}
	function __destruct()
	{
		echo __METHOD__ . "\n";
	}
}

try
{
	$bar = new Foo;
} catch(Exception $exc)
{
	echo "Caught exception!\n";
}

unset($bar);

?>
===DONE===