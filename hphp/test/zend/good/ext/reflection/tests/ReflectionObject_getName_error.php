<?php
class C { }
$myInstance = new C;
$r2 = new ReflectionObject($myInstance);

$r3 = new ReflectionObject($r2);

var_dump($r3->getName(null));
var_dump($r3->getName('x','y'));
var_dump($r3->getName(0));
?>
