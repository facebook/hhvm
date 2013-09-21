<?php

abstract class Base
{
	abstract function someMethod($param);
}

class Ext extends Base
{
	function someMethod($param = "default")
	{
		echo $param, "\n";
	}
}

$a = new Ext();
$a->someMethod("foo");
$a->someMethod();