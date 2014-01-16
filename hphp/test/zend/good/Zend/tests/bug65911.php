<?php
class A {}

class B
{
	public function go()
	{
		$this->foo = 'bar';
		echo A::$this->foo; // should not output 'bar'
	}
}

$obj = new B();
$obj->go();
?>