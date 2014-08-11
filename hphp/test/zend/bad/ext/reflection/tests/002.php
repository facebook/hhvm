<?php

class ReflectionMethodEx extends ReflectionMethod
{
	public $foo = "xyz";
	
	function __construct($c,$m)
	{
		echo __METHOD__ . "\n";
		parent::__construct($c,$m);
	}
}

$r = new ReflectionMethodEx('ReflectionMethodEx','getName');

var_dump($r->class);
var_dump($r->name);
var_dump($r->foo);
@var_dump($r->bar);

try
{
	$r->class = 'bullshit';
}
catch(ReflectionException $e)
{
	echo $e->getMessage() . "\n";
}
try
{
$r->name = 'bullshit';
}
catch(ReflectionException $e)
{
	echo $e->getMessage() . "\n";
}

$r->foo = 'bar';
$r->bar = 'baz';

var_dump($r->class);
var_dump($r->name);
var_dump($r->foo);
var_dump($r->bar);

?>
===DONE===
