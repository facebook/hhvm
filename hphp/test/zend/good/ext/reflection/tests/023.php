<?php
class C1 {
	private   $p1 = 1;
	protected $p2 = 2;
	public    $p3 = 3;
}
class C2 extends C1 {
	private   $p4 = 4;
	protected $p5 = 5;
	public    $p6 = 6;
}
$class = new ReflectionClass("C2");
var_dump($class->getDefaultProperties());
?>
