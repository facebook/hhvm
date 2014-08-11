<?php
$r0 = new ReflectionObject();
var_dump($r0->getName());

$r1 = new ReflectionObject(new stdClass);
var_dump($r1->getName());

class C { }
$myInstance = new C;
$r2 = new ReflectionObject($myInstance);
var_dump($r2->getName());

$r3 = new ReflectionObject($r2);
var_dump($r3->getName());

?>
