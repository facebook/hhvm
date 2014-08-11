<?php

class foo {
}
$foo = new foo;

print "User class\n";
$obj = new ReflectionClass($foo);
var_dump($obj->isCloneable());
$obj = new ReflectionObject($foo);
var_dump($obj->isCloneable());
$h = clone $foo;

class bar {
	private function __clone() {
	}
}
$bar = new bar;
print "User class - private __clone\n";
$obj = new ReflectionClass($bar);
var_dump($obj->isCloneable());
$obj = new ReflectionObject($bar);
var_dump($obj->isCloneable());
$h = clone $foo;

print "Closure\n";
$closure = function () { };
$obj = new ReflectionClass($closure);
var_dump($obj->isCloneable());
$obj = new ReflectionObject($closure);
var_dump($obj->isCloneable());
$h = clone $closure;

print "Internal class - SimpleXMLElement\n";
$obj = new ReflectionClass('simplexmlelement');
var_dump($obj->isCloneable());
$obj = new ReflectionObject(new simplexmlelement('<test></test>'));
var_dump($obj->isCloneable());
$h = clone new simplexmlelement('<test></test>');

print "Internal class - XMLWriter\n";
$obj = new ReflectionClass('xmlwriter');
var_dump($obj->isCloneable());
$obj = new ReflectionObject(new XMLWriter);
var_dump($obj->isCloneable());
$h = clone new xmlwriter;

?>
