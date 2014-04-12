<?php
class Foo {
}

$mirror = new ReflectionClass("ReflectionClass");
$foo_reflectorA = new ReflectionClass("Foo");
$foo_reflectorB = new ReflectionClass("Foo");
$foo_reflectorB->getMethods();
$info_property = $mirror->getProperty("info");
$info_property->setAccessible(true);
$infoA = $info_property->getValue($foo_reflectorA);
$infoB = $info_property->getValue($foo_reflectorB);
var_dump($infoA == $infoB);

