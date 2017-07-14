<?php
$bar = NULL;
class bad
{
	public $_private = array();

	public function __construct()
	{
		$this->_private[] = 'php';
	}

	public function __destruct()
	{
		global $bar;
		$bar = $this;
	}
}

$foo = new stdclass;
$foo->foo = $foo;
$foo->bad = new bad;

unserialize(serialize($foo));
//unset($foo);

gc_collect_cycles();
var_dump($bar);
