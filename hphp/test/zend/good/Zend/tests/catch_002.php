<?php

class MyObject
{
	function __construct()
	{
		throw new Exception();
		echo __METHOD__ . "() Must not be reached\n";
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

echo "===DONE===\n";
