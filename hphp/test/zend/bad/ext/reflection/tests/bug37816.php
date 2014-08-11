<?php

class TestClass
{
	protected $p = 2;
}

$o = new TestClass;

$r = new ReflectionProperty($o, 'p');

try
{
	$x = $r->getValue($o);
}
catch (Exception $e)
{
	echo 'Caught: ' . $e->getMessage() . "\n";
}

?>
===DONE===
