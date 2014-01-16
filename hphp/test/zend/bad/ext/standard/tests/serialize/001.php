<?php
ini_set('serialize_precision', 100);
 
class t
{
	function t()
	{
		$this->a = "hallo";
	}
}

class s
{
	public $a;
	public $b;
	public $c;

	function s()
	{
		$this->a = "hallo";
		$this->b = "php";
		$this->c = "world";
		$this->d = "!";
	}

	function __sleep()
	{
		echo "__sleep called\n";
		return array("a","c");
	}

	function __wakeup()
	{
		echo "__wakeup called\n";
	}
}


echo serialize(NULL)."\n";
echo serialize((bool) true)."\n";
echo serialize((bool) false)."\n";
echo serialize(1)."\n";
echo serialize(0)."\n";
echo serialize(-1)."\n";
echo serialize(2147483647)."\n";
echo serialize(-2147483647)."\n";
echo serialize(1.123456789)."\n";
echo serialize(1.0)."\n";
echo serialize(0.0)."\n";
echo serialize(-1.0)."\n";
echo serialize(-1.123456789)."\n";
echo serialize("hallo")."\n";
echo serialize(array(1,1.1,"hallo",NULL,true,array()))."\n";

$t = new t();
$data = serialize($t);
echo "$data\n";
$t = unserialize($data);
var_dump($t);

$t = new s();
$data = serialize($t);
echo "$data\n";
$t = unserialize($data);
var_dump($t);

$a = array("a" => "test");
$a[ "b" ] = &$a[ "a" ];
var_dump($a);
$data = serialize($a);
echo "$data\n";
$a = unserialize($data);
var_dump($a);
?>