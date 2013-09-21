<?php

class MyRecursiveArrayIterator extends RecursiveArrayIterator
{
	function valid()
	{
		if (!parent::valid())
		{
			echo __METHOD__ . "() = false\n";
			return false;
		}
		else
		{
			return true;
		}
	}

	function getChildren()
	{
		echo __METHOD__ . "()\n";
		return parent::getChildren();
	}
	
	function rewind()
	{
		echo __METHOD__ . "()\n";
		parent::rewind();
	}
}

class RecursiveArrayIteratorIterator extends RecursiveIteratorIterator
{
	private $max_depth;
	private $over = 0;

	function __construct($it, $max_depth)
	{
		$this->max_depth = $max_depth;
		parent::__construct($it);
	}

	function rewind()
	{
		echo __METHOD__ . "() - BEGIN\n";
		parent::rewind();
		echo __METHOD__ . "() - DONE\n";
	}

	function valid()
	{
		echo __METHOD__ . "()\n";
		return parent::valid();
	}

	function current()
	{
		echo __METHOD__ . "()\n";
		return parent::current();
	}

	function key()
	{
		echo __METHOD__ . "()\n";
		return parent::key();
	}

	function next()
	{
		echo __METHOD__ . "()\n";
		parent::next();
	}

	function callHasChildren()
	{
		$has = parent::callHasChildren();
		$res = $this->getDepth() < $this->max_depth && $has;
		echo __METHOD__ . "(".$this->getDepth().") = ".($res?"yes":"no")."/".($has?"yes":"no")."\n";
		return $res;
	}

	function beginChildren()
	{
		echo __METHOD__ . "(".$this->getDepth().")\n";
		parent::beginChildren();
	}

	function endChildren()
	{
		echo __METHOD__ . "(".$this->getDepth().")\n";
		parent::endChildren();
	}
}

$p = 0;
$it = new RecursiveArrayIteratorIterator(new MyRecursiveArrayIterator(array("a", array("ba", array("bba", "bbb"), array(array("bcaa"), array("bcba"))), array("ca"), "d")), 2);
foreach($it as $k=>$v)
{
	if (is_array($v)) $v = join('',$v);
	echo "$k=>$v\n";
	if ($p++ == 5)
	{
		echo "===BREAK===\n";
		break;
	}
}

echo "===FOREND===\n";

$it->rewind();

echo "===CHECK===\n";

var_dump($it->valid());
var_dump($it->current() == "a");

?>
===DONE===
<?php exit(0); ?>