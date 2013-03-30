<?php
class A
{
	public $q;

	function __construct()
	{
		$this->q = 3;//array();
	}

	function __get($name)
	{
		return $this->q;
	}
}

$a = new A;

$b = "short";
$c =& $a->whatever;
$c = "long";
print_r($a);
$a->whatever =& $b;
$b = "much longer";
print_r($a);
?>