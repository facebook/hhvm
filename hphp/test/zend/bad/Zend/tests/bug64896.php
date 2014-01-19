<?php
$bar = NULL;
class bad
{
	private $_private = array();

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

gc_disable();

unserialize(serialize($foo));
gc_collect_cycles();
var_dump($bar); 
/*  will output:
object(bad)#4 (1) {
  ["_private":"bad":private]=>
  &UNKNOWN:0
}
*/
?>