<?php
class foo {
	static protected $bar = 'baz';
	static public $a = 'a';
}

$class = new ReflectionClass('foo');
$properties = $class->getStaticProperties();
var_dump($properties, array_keys($properties));
var_dump(isset($properties['*bar']));
var_dump(isset($properties["\0*\0bar"]));
var_dump(isset($properties["bar"]));
?>
