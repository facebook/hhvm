<?php

class a {}
abstract class b {}
final class c {}

class x
{
	function __construct() {}
	function __destruct() {}
	private function a() {}
	private static function b() {}
	protected function c() {}
	protected static function d() {}
	public function e() {}
	public static function f() {}
	final function g() {}
	function h() {}
}

abstract class y
{
	abstract function a();
	abstract protected function b();
}

function dump_modifierNames($class) {
	$obj = new ReflectionClass($class);
	var_dump($obj->getName(), Reflection::getModifierNames($obj->getModifiers()));
}

function dump_methodModifierNames($class) {
	$obj = new ReflectionClass($class);
	foreach($obj->getMethods() as $method) {
		var_dump($obj->getName() . "::" . $method->getName(), Reflection::getModifierNames($method->getModifiers()));
	}
}

dump_modifierNames('a');
dump_modifierNames('b');
dump_modifierNames('c');

dump_methodModifierNames('x');
dump_methodModifierNames('y');

?>
==DONE==
