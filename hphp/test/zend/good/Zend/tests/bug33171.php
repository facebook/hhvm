<?php
class A
{
	private $c = "A's c";
}

class B extends A
{
	private $c = "B's c";
		
	public function go()
	{
		foreach ($this as $key => $val)
		{
			echo "$key => $val\n";
		}
	}
};

$x = new B;
$x->go();
?>