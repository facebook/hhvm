<?php
class Foo {
	public    $p1;
	protected $p2;
	private   $p3;

	function __isset($name) {
		var_dump($name);
		return false;
	}
}
$obj = new ReflectionObject(new Foo());
var_dump($obj->hasProperty("p1"));
var_dump($obj->hasProperty("p2"));
var_dump($obj->hasProperty("p3"));
var_dump($obj->hasProperty("p4"));
?>
