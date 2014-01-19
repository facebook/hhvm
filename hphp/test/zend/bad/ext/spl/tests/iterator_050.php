<?php

class MyRegexIterator extends RegexIterator
{
	function show()
	{
		foreach($this as $k => $v)
		{
			var_dump($k);
			var_dump($v);
		}
	}
}

$ar = new ArrayIterator(array('1','1,2','1,2,3','',NULL,array(),'FooBar',',',',,'));
$it = new MyRegexIterator($ar, '/(\d),(\d)/', RegexIterator::GET_MATCH);
$it->show();

$it = new MyRegexIterator($ar, '/(\d)/', RegexIterator::GET_MATCH);
$it->show();

var_dump($ar);

?>
===DONE===
<?php exit(0); ?>