<?php

echo "===Array===\n";

$a = array('zero' => 0, 'one' => 1, 'two' => 2);
$it = new ArrayIterator($a);

var_dump($it->count());
foreach($it as $key => $val)
{
	echo "$key=>$val\n";
	var_dump($it->count());
}
var_dump($it->count());

echo "===Object===\n";

class test
{
	public $zero = 0;
	protected $pro;
	public $one = 1;
	private $pri;
	public $two = 2;
}

$o = new test;
$it = new ArrayIterator($o);

var_dump($it->count());
foreach($it as $key => $val)
{
	echo "$key=>$val\n";
	var_dump($it->count());
}
var_dump($it->count());

?>
===DONE===
<?php exit(0); ?>
