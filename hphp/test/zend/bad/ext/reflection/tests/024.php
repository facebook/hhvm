<?php
class C1 {
	private   $p1 = 1;
	protected $p2 = 2;
	public    $p3 = 3;
}

$x = new C1();
$x->z = 4;
$x->p3 = 5;

$obj = new ReflectionObject($x);
echo $obj;
?>
