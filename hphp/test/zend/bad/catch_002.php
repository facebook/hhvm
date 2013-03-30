<?php

class MyObject
{
	function __construct()
	{
		throw new Exception();
		echo __METHOD__ . "() Must not be reached\n";
	}

	function __destruct()
	{
		echo __METHOD__ . "() Must not be called\n";
	}
}

try
{
	new MyObject();
}
catch(Exception $e)
{
	echo "Caught\n";
}

?>
===DONE===