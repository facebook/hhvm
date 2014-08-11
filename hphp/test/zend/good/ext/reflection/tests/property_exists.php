<?php

class A
{
	public    $a = 1;
	protected $b = 2;
	private   $c = 3;
	
	public    $empty;
	public    $init = 1;
	
	function __toString()
	{
		return 'obj(' . get_class($this) . ')';
	}
	
	static function test($oc, $props)
	{
		echo '===' . __CLASS__ . "===\n";
		foreach($props as $p2) {
			echo $oc, '::$' , $p2, "\n";
			var_dump(property_exists($oc, $p2));
		}
	}
}

class B extends A
{
	private   $c = 4;
	
	static function test($oc, $props)
	{
		echo '===' . __CLASS__ . "===\n";
		foreach($props as $p2) {
			echo $oc, '::$' , $p2, "\n";
			var_dump(property_exists($oc, $p2));
		}
	}
}

class C extends B
{
	private   $d = 5;
	
	static function test($oc, $props)
	{
		echo '===' . __CLASS__ . "===\n";
		foreach($props as $p2) {
			echo $oc, '::$' , $p2, "\n";
			var_dump(property_exists($oc, $p2));
		}
	}
}

$oA = new A;
$oA->e = 6;

$oC = new C;

$pc = array($oA, 'A', 'B', 'C', $oC);
$pr = array('a', 'b', 'c', 'd', 'e');

foreach($pc as $p1) {
	if (is_object($p1)) {
		$p1->test($p1, $pr);
	} else {
		$r = new ReflectionMethod($p1, 'test');
		$r->invoke(NULL, $p1, $pr);
	}
	echo "===GLOBAL===\n";
	foreach($pr as $p2) {
		echo $p1, '::$' , $p2, "\n";
		var_dump(property_exists($p1, $p2));
	}
}

echo "===PROBLEMS===\n";
var_dump(property_exists(NULL, 'empty'));
var_dump(property_exists(25,'empty'));
var_dump(property_exists('',''));
var_dump(property_exists('A',''));
var_dump(property_exists('A','123'));
var_dump(property_exists('A','init'));
var_dump(property_exists('A','empty'));
var_dump(property_exists(new A, ''));
var_dump(property_exists(new A, '123'));
var_dump(property_exists(new A, 'init'));
var_dump(property_exists(new A, 'empty'));
?>
===DONE===
<?php exit(0); ?>
