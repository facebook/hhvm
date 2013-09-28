<?php

class RecursiveArrayIteratorIterator extends RecursiveIteratorIterator
{
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
$obj = new RecursiveArrayIterator($arr);
$rit = new RecursiveArrayIteratorIterator($obj);
foreach($rit as $k=>$v)
{
	echo str_repeat('  ',$rit->getDepth()+1)."$k=>$v\n";
}
?>
===DONE===
<?php exit(0); ?>