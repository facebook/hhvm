<?php

class Menu extends ArrayObject
{
	function getIterator()
	{
		echo __METHOD__ . "\n";
		return new RecursiveArrayIterator($this);
	}
}

class MenuOutput extends RecursiveIteratorIterator
{
	function __construct(Menu $it)
	{
		parent::__construct($it);
	}
	function rewind()
	{
		echo "<ul>\n";
		parent::rewind();
	}
	function beginChildren()
	{
		echo str_repeat('  ',$this->getDepth())."<ul>\n";
	}

	function endChildren()
	{
		echo str_repeat('  ',$this->getDepth())."</ul>\n";
	}
	function valid()
	{
		if (!parent::valid()) {
			echo "<ul>\n";
			return false;
		}
		return true;
	}
}

$arr = array("a", array("ba", array("bba", "bbb"), array(array("bcaa"))), array("ca"), "d");
$obj = new Menu($arr);
$rit = new MenuOutput($obj);
foreach($rit as $k=>$v)
{
	echo str_repeat('  ',$rit->getDepth()+1)."$k=>$v\n";
}
?>
===DONE===
<?php exit(0); ?>