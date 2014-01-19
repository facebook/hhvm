<?php
class A
{
	public function __destruct()
	{
		gc_collect_cycles();
	}
	
	public function getB()
	{
		$this->data['foo'] = new B($this);
		$this->data['bar'] = new B($this);
		// Return either of the above
		return $this->data['foo'];
	}
}

class B
{
	public function B($A)
	{
		$this->A = $A;
	}

	public function __destruct()
	{
	}
}

for ($i = 0; $i < 2; $i++)
{
	$Aobj = new A;
	$Bobj = $Aobj->getB();
	unset($Bobj);
	unset($Aobj);
}

echo "DONE\n";
?>