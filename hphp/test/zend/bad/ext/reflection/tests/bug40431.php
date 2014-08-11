<?php

echo "=== 1st test ===\n";
$Obj = new stdClass;
$Obj->value = 'value';
$RefObj = new ReflectionObject($Obj);

$props = $RefObj->getProperties();

var_dump($props);
var_dump($props[0]->isStatic());
var_dump($props[0]->isPrivate());
var_dump($props[0]->isPublic());
var_dump($props[0]->isProtected());

echo "=== 2nd test ===\n";

class test1 {
}

class test2 extends test1{
}

$Obj = new test2;
$Obj->value = 'value';
$RefObj = new ReflectionObject($Obj);

$props = $RefObj->getProperties();

var_dump($props);
var_dump($props[0]->isStatic());
var_dump($props[0]->isPrivate());
var_dump($props[0]->isPublic());
var_dump($props[0]->isProtected());

echo "=== 3rd test ===\n";

class test3 {
}

$Obj = new test3;
$Obj->value = 'value';
$RefObj = new ReflectionObject($Obj);

$props = $RefObj->getProperties();

var_dump($props);
var_dump($props[0]->isStatic());
var_dump($props[0]->isPrivate());
var_dump($props[0]->isPublic());
var_dump($props[0]->isProtected());

echo "=== 4th test ===\n";

class test5 {
	private $value = 1;
}

class test4 extends test5{
}

$Obj = new test4;
$Obj->value = 'value';
$RefObj = new ReflectionObject($Obj);

$props = $RefObj->getProperties();

var_dump($props);
var_dump($props[0]->isStatic());
var_dump($props[0]->isPrivate());
var_dump($props[0]->isPublic());
var_dump($props[0]->isProtected());

echo "Done\n";
?>
