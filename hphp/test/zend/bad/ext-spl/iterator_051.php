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

$ar = new ArrayIterator(array('1'=>0,'1,2'=>1,'1,2,3'=>2,0=>3,'FooBar'=>4,','=>5,',,'=>6));
$it = new MyRegexIterator($ar, '/(\d),(\d)/', RegexIterator::GET_MATCH, RegexIterator::USE_KEY);
$it->show();

$it = new MyRegexIterator($ar, '/(\d)/', RegexIterator::GET_MATCH, RegexIterator::USE_KEY);
$it->show();

var_dump($ar);

?>
===DONE===
<?php exit(0); ?>