<?php
class A
{
	public function __construct() {
		echo __METHOD__ . "\n";
	}
	protected function A()
	{
		echo __METHOD__ . "\n";
	}
}
class B extends A
{
	public function __construct() {
		echo __METHOD__ . "\n";
		parent::__construct();
	}
	public function A()
	{
		echo __METHOD__ . "\n";
		parent::A();
	}
}
$b = new B();
$b->A();
?>
===DONE===