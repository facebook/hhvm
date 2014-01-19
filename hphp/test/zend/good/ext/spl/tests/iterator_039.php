<?php

class NumericArrayIterator implements Iterator
{
	protected $a;
	protected $i = 0;

	public function __construct($a)
	{
		echo __METHOD__ . "\n";
		$this->a = $a;
	}

	public function valid()
	{
		echo __METHOD__ . "\n";
		return $this->i < count($this->a);
	}

	public function rewind()
	{
		echo __METHOD__ . "\n";
		$this->i = 0;
	}

	public function key()
	{
		echo __METHOD__ . "\n";
		return $this->i;
	}

	public function current()
	{
		echo __METHOD__ . "\n";
		return $this->a[$this->i];
	}

	public function next()
	{
		echo __METHOD__ . "\n";
		$this->i++;
	}
}

$it = new LimitIterator(new NumericArrayIterator(array(12, 25, 42, 56)));

foreach($it as $k => $v)
{
	var_dump($k);
	var_dump($v);
}

echo "===SEEK===\n";

$it->seek(2);

echo "===LOOP===\n";

foreach(new NoRewindIterator($it) as $k => $v)
{
	var_dump($k);
	var_dump($v);
}

?>
===DONE===
<?php exit(0); ?>