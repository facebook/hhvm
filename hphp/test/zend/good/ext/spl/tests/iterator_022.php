<?php

class MyRecursiveArrayIterator extends RecursiveArrayIterator
{
	function getChildren()
	{
		echo __METHOD__ . "\n";
		return $this->current();
	}

	function valid()
	{
		if (!parent::valid())
		{
			echo __METHOD__ . " = false\n";
			return false;
		}
		else
		{
			return true;
		}
	}
}

class RecursiveArrayIteratorIterator extends RecursiveIteratorIterator
{
	private $max_depth;
	private $over = 0;
	private $skip = false;

	function __construct($it, $max_depth)
	{
		$this->max_depth = $max_depth;
		parent::__construct($it);
	}

	function rewind()
	{
		echo __METHOD__ . "\n";
		$this->skip = false;
		parent::rewind();
	}

	function valid()
	{
		echo __METHOD__ . "\n";
		if ($this->skip)
		{
			$this->skip = false;
			$this->next();
		}
		return parent::valid();
	}

	function current()
	{
		echo __METHOD__ . "\n";
		return parent::current();
	}

	function key()
	{
		echo __METHOD__ . "\n";
		return parent::key();
	}

	function next()
	{
		echo __METHOD__ . "\n";
		parent::next();
	}

	function callHasChildren()
	{
		$this->skip = false;
		$has = parent::callHasChildren();
		$res = $this->getDepth() < $this->max_depth && $has;
		echo __METHOD__ . "(".$this->getDepth().") = ".($res?"yes":"no")."/".($has?"yes":"no")."\n";
		if ($has && !$res)
		{
			$this->over++;
			if ($this->over == 2) {
				$this->skip = true;
			}
		}
		return $res;
	}
	
	function callGetChildren()
	{
		if ($this->over == 2)
		{
			echo __METHOD__ . "(skip)\n";
			return NULL;
		}
		echo __METHOD__ . "(ok:{$this->over})\n";
		return new MyRecursiveArrayIterator($this->current());
	}

	function beginChildren()
	{
		echo __METHOD__ . "(".$this->getDepth().")\n";
	}

	function endChildren()
	{
		echo __METHOD__ . "(".$this->getDepth().")\n";
	}
}

try
{
	foreach(new RecursiveArrayIteratorIterator(new MyRecursiveArrayIterator(array("a", array("ba", array("bba", "bbb"), array(array("bcaa"), array("bcba"))), array("ca"), "d")), 2) as $k=>$v)
	{
		if (is_array($v)) $v = join('',$v);
		echo "$k=>$v\n";
	}
}
catch(UnexpectedValueException $e)
{
	echo $e->getMessage() . "\n";
}

?>
===DONE===
<?php exit(0); ?>