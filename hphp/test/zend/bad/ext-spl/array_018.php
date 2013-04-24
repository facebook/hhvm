<?php

try
{
	$foo = new ArrayObject();
	$foo->offsetSet("\0", "Foo");
}
catch (Exception $e)
{
	var_dump($e->getMessage());
}

var_dump($foo);

try
{
	$foo = new ArrayObject();
	$data = explode("=", "=Foo");
	$foo->offsetSet($data[0], $data[1]);
}
catch (Exception $e)
{
	var_dump($e->getMessage());
}

var_dump($foo);

?>
===DONE===