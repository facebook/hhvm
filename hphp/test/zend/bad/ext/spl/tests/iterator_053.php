<?php

class MyRegexIterator extends RegexIterator
{
	public $uk, $re;
	
	function __construct($it, $re, $mode, $flags = 0)
	{
		$this->uk = $flags & self::USE_KEY;
		$this->re = $re;
		parent::__construct($it, $re, $mode, $flags);
	}

	function show()
	{
		foreach($this as $k => $v)
		{
			var_dump($k);
			var_dump($v);
		}
	}
	
	function accept()
	{
		@preg_match_all($this->re, (string)($this->uk ? $this->key() : $this->current()), $sub);
		$ret = parent::accept();
		var_dump($sub == $this->current());
		return $ret;
	}
}

$ar = new ArrayIterator(array('1','1,2','1,2,3','',NULL,array(),'FooBar',',',',,'));
$it = new MyRegexIterator($ar, '/(\d),(\d)/', RegexIterator::ALL_MATCHES, RegexIterator::USE_KEY);
$it->show();

$it = new MyRegexIterator($ar, '/(\d)/', RegexIterator::ALL_MATCHES, RegexIterator::USE_KEY);
$it->show();

var_dump($ar);

?>
===DONE===
<?php exit(0); ?>