<?php

class ArrayIteratorEx extends ArrayIterator
{
	public    $pub2 = 1;
	protected $pro2 = 2;
	private   $pri2 = 3;

	function __construct($ar, $flags = 0)
	{
		echo __METHOD__ . "()\n";
		parent::__construct($ar, $flags);
		$this->imp2 = 4;
	}

	function dump()
	{
		echo __METHOD__ . "()\n";
		var_dump(array('Flags'=>$this->getFlags()
		              ,'OVars'=>get_object_vars($this)
		              ,'$this'=>$this));
	}

	function setFlags($flags)
	{
		echo __METHOD__ . "($flags)\n";
		ArrayIterator::setFlags($flags);
	}
}

class ArrayObjectEx extends ArrayObject
{
	public    $pub1 = 1;
	protected $pro1 = 2;
	private   $pri1 = 3;
	
	function __construct($ar = array(), $flags = 0)
	{
		echo __METHOD__ . "()\n";
		parent::__construct($ar, $flags);
		$this->imp1 = 4;
	}

	function exchange()
	{
		echo __METHOD__ . "()\n";
		$this->exchangeArray($this);
	}

	function dump()
	{
		echo __METHOD__ . "()\n";
		var_dump(array('Flags'=>$this->getFlags()
		              ,'OVars'=>get_object_vars($this)
		              ,'$this'=>$this));
	}

	function show()
	{
		echo __METHOD__ . "()\n";
		foreach($this as $n => $v)
		{
			var_dump(array($n => $v));
		}
	}
	
	function setFlags($flags)
	{
		echo __METHOD__ . "($flags)\n";
		ArrayObject::setFlags($flags);
	}
	
	function getIterator()
	{
		echo __METHOD__ . "()\n";
		$it = new ArrayIteratorEx($this, $this->getFlags());
		$it->dyn2 = 5;
		$it->dump();
		return $it;
	}
}

function check($obj, $flags)
{
	echo "===CHECK===\n";

	$obj->setFlags($flags);
	$obj->dump();
	$obj->show();

	echo "===FOREACH===\n";
	
	$it = $obj->getIterator();
	foreach($it as $n => $v)
	{
		var_dump(array($n => $v));
	}
	
	echo "===PROPERTY===\n";
	
	var_dump($obj->pub1);
	var_dump(isset($obj->a));
	$obj->setFlags($flags | 2);
	var_dump($obj->pub1);
	var_dump(isset($obj->a));
	
	var_dump($it->pub2);
	var_dump(isset($it->pub1));
	$it->setFlags($flags | 2);
	var_dump($it->pub2);
	var_dump(isset($it->pub1));
}

$obj = new ArrayObjectEx(array(0=>1,'a'=>25, 'pub1'=>42), 0);
$obj->dyn1 = 5;

check($obj, 0);
check($obj, 1);

echo "#####EXCHANGE#####\n";

$obj->exchange();

check($obj, 0);
check($obj, 1);

?>
===DONE===
<?php exit(0); ?>